#include "bm_adventure_scenes.h"

#include "bm_audio.h"
#include "bm_flow.h"

#include "bn_string.h"

#include "bn_sprite_items_spr_cursor.h"
#include "bn_sprite_items_spr_icons.h"
#include "bn_sprite_items_spr_particles.h"

namespace bm
{
    namespace
    {
        // Screen positions of the six world islands (must match tools/gfx/backgrounds.py WORLD_POS).
        constexpr int world_pos[6][2] = { { 36, 118 }, { 78, 74 }, { 120, 116 }, { 160, 68 }, { 198, 110 }, { 214, 34 } };
    }

    // ------------------------------------------------------------------ world map

    world_map_scene::world_map_scene() :
        _backdrop(backdrop_id::worldmap, 3)
    {
        app& a = the_app();
        audio::play_music(audio::music_id::menu);
        _selected = a.flow.world > 0 && a.flow.world <= a.save.worlds_unlocked ? a.flow.world - 1 : 0;
        _title.show_big(120, 2, "WORLD MAP", big_color::gold, align::center);
        for(int w = 0; w < 6; ++w)
        {
            if(w + 1 > a.save.worlds_unlocked)
            {
                _locks[w] = bn::sprite_items::spr_icons.create_sprite(world_pos[w][0] - 120, world_pos[w][1] - 14 - 80, 8);
            }
            else if(a.save.stages[stage_number(w + 1, 5)].cleared)
            {
                _stars[w] = bn::sprite_items::spr_particles.create_sprite(world_pos[w][0] - 120 + 14,
                                                                          world_pos[w][1] - 22 - 80, 20);
            }
        }
        // A world completed in the last stage gets a short celebration.
        if(a.flow.mode == game_mode::adventure && a.flow.outcome == session_outcome::won && a.flow.stage == 5)
        {
            _celebrate = 150;
            audio::sfx(audio::sfx_id::unlock);
        }
        _refresh();
    }

    void world_map_scene::_refresh()
    {
        app& a = the_app();
        const int w = _selected + 1;
        bn::string<40> t = "WORLD ";
        t.append(bn::to_string<4>(w));
        t.append("  ");
        t.append(world_name(w));
        _world.show(120, 132, t, text_color::yellow, align::center);
        bn::string<40> info;
        if(w > a.save.worlds_unlocked)
        {
            info = "LOCKED - DEFEAT THE PREVIOUS BOSS";
            _info.show(120, 142, info, text_color::red, align::center);
        }
        else
        {
            info = "STAGES ";
            info.append(bn::to_string<4>(world_cleared_stages(a.save, w)));
            info.append("/5  VS ");
            info.append(boss(w).short_name);
            _info.show(120, 142, info, text_color::white, align::center);
        }
        int total = 0;
        for(const stage_record& r : a.save.stages)
        {
            total += r.cleared ? 1 : 0;
        }
        bn::string<24> p = "CAMPAIGN ";
        p.append(bn::to_string<4>(total));
        p.append("/30");
        _progress.show(236, 152, p, text_color::cyan, align::right);
        _hint.show(4, 152, "A:ENTER B:BACK", text_color::gray, align::left);
    }

    bn::optional<scene_id> world_map_scene::update()
    {
        ++_frame;
        _input.update();
        _backdrop.update();
        app& a = the_app();
        if(! _cursor)
        {
            _cursor = bn::sprite_items::spr_cursor.create_sprite(0, 0, 2);
        }
        const int bounce = (_frame / 6) % 4 < 2 ? 0 : 2;
        _cursor->set_position(world_pos[_selected][0] - 120, world_pos[_selected][1] - 36 - 80 + bounce);
        _cursor->set_tiles(bn::sprite_items::spr_cursor.tiles_item(), 2 + ((_frame >> 4) & 1));
        if(_celebrate > 0)
        {
            --_celebrate;
            if(_celebrate % 20 == 0)
            {
                audio::sfx(audio::sfx_id::chain);
            }
            _title.show_big(120, 2, (_celebrate & 16) ? "WORLD CLEAR!" : "WORLD MAP", big_color::gold, align::center);
            if(! _celebrate)
            {
                _title.show_big(120, 2, "WORLD MAP", big_color::gold, align::center);
            }
        }

        const int unlocked = a.save.worlds_unlocked;
        if(_input.right() || _input.up())
        {
            if(_selected + 1 < 6)
            {
                ++_selected;
                audio::sfx(audio::sfx_id::menu_move);
                _refresh();
            }
        }
        else if(_input.left() || _input.down())
        {
            if(_selected > 0)
            {
                --_selected;
                audio::sfx(audio::sfx_id::menu_move);
                _refresh();
            }
        }
        else if(_input.ok() || _input.start())
        {
            if(_selected + 1 > unlocked)
            {
                audio::sfx(audio::sfx_id::denied);
                return bn::nullopt;
            }
            audio::sfx(audio::sfx_id::menu_ok);
            a.flow.mode = game_mode::adventure;
            a.flow.world = uint8_t(_selected + 1);
            // Start on the first stage that is not cleared yet.
            a.flow.stage = 1;
            for(int i = 1; i <= 5; ++i)
            {
                if(! a.save.stages[stage_number(a.flow.world, i)].cleared)
                {
                    a.flow.stage = uint8_t(i);
                    break;
                }
            }
            return scene_id::stage_card;
        }
        else if(_input.back())
        {
            audio::sfx(audio::sfx_id::menu_back);
            return scene_id::main_menu;
        }
        return bn::nullopt;
    }

    // ------------------------------------------------------------------ stage select / stage card

    stage_select_scene::stage_select_scene() :
        _backdrop(backdrop_id::menu, 3)
    {
        app& a = the_app();
        _selected = a.flow.stage > 0 ? a.flow.stage - 1 : 0;
        audio::play_music(audio::music_id::menu);
        _refresh();
    }

    void stage_select_scene::_refresh()
    {
        app& a = the_app();
        const int w = a.flow.world;
        bn::string<40> t = "WORLD ";
        t.append(bn::to_string<4>(w));
        t.append(" ");
        t.append(world_name(w));
        _title.show(120, 6, t, text_color::yellow, align::center);
        for(int i = 0; i < 5; ++i)
        {
            const stage_def& st = stage(w, i + 1);
            const stage_record& rec = a.save.stages[stage_number(w, i + 1)];
            const bool open = stage_unlocked(a.save, w, i + 1);
            bn::string<40> row;
            row.append(i == _selected ? "> " : "  ");
            row.append(bn::to_string<4>(w));
            row.append("-");
            row.append(bn::to_string<4>(i + 1));
            row.append(" ");
            row.append(open ? st.name : "LOCKED");
            if(rec.cleared)
            {
                row.append("  ");
                row.append(rank_name(rec.best_rank));
            }
            text_color c = ! open ? text_color::gray : (i == _selected ? text_color::yellow :
                                                       (st.boss ? text_color::red : text_color::white));
            _rows[i].show(16, 22 + i * 11, row, c, align::left);
        }

        const stage_def& st = stage(w, _selected + 1);
        _idea.show(120, 82, st.idea, text_color::cyan, align::center);
        int g = 0;
        for(const objective_def& o : st.objectives)
        {
            if(o.kind == objective_kind::none || g >= 3)
            {
                continue;
            }
            bn::string<40> line = "GOAL: ";
            objective_text(o, line);
            _goals[g].show(120, 92 + g * 10, line, text_color::white, align::center);
            ++g;
        }
        if(st.boss)
        {
            bn::string<40> line = "GOAL: DEFEAT ";
            line.append(boss(st.boss).name);
            _goals[g++].show(120, 92, line, text_color::red, align::center);
        }
        const int bonus_y = 94 + g * 10;
        for(; g < 3; ++g)
        {
            _goals[g].hide();
        }
        if(st.secondary.kind != objective_kind::none)
        {
            bn::string<40> line = "BONUS: ";
            objective_text(st.secondary, line);
            _extra.show(120, bonus_y, line, text_color::green, align::center);
        }
        else
        {
            _extra.hide();
        }
        const stage_record& rec = a.save.stages[stage_number(w, _selected + 1)];
        if(rec.cleared)
        {
            bn::string<40> best = "BEST ";
            best.append(bn::to_string<12>(rec.best_score));
            best.append("  RANK ");
            best.append(rank_name(rec.best_rank));
            _best.show(120, 138, best, text_color::orange, align::center);
        }
        else
        {
            _best.hide();
        }
        for(int i = 0; i < 3; ++i)
        {
            if(i < st.loadout_count)
            {
                if(! _icons[i])
                {
                    _icons[i] = bn::sprite_items::spr_icons.create_sprite(0, 0, st.loadout[i]);
                }
                _icons[i]->set_tiles(bn::sprite_items::spr_icons.tiles_item(), st.loadout[i]);
                _icons[i]->set_position(196 + i * 18 - 120, 30 - 80);
            }
            else
            {
                _icons[i].reset();
            }
        }
        _hint.show(120, 150, stage_unlocked(a.save, w, _selected + 1) ? "A: START   B: MAP" : "CLEAR THE PREVIOUS STAGE",
                   stage_unlocked(a.save, w, _selected + 1) ? text_color::gray : text_color::red, align::center);
    }

    bn::optional<scene_id> stage_select_scene::update()
    {
        ++_frame;
        _input.update();
        _backdrop.update();
        app& a = the_app();
        if(_input.up() && _selected > 0)
        {
            --_selected;
            audio::sfx(audio::sfx_id::menu_move);
            _refresh();
        }
        else if(_input.down() && _selected < 4)
        {
            ++_selected;
            audio::sfx(audio::sfx_id::menu_move);
            _refresh();
        }
        else if(_input.ok() || _input.start())
        {
            if(! stage_unlocked(a.save, a.flow.world, _selected + 1))
            {
                audio::sfx(audio::sfx_id::denied);
                return bn::nullopt;
            }
            audio::sfx(audio::sfx_id::menu_ok);
            a.flow.mode = game_mode::adventure;
            a.flow.stage = uint8_t(_selected + 1);
            a.flow.seed = a.frame * 2654435761u + uint32_t(a.flow.world * 97 + a.flow.stage);
            return scene_id::gameplay;
        }
        else if(_input.back())
        {
            audio::sfx(audio::sfx_id::menu_back);
            return scene_id::world_map;
        }
        return bn::nullopt;
    }
}
