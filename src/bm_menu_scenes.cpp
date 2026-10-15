#include "bm_menu_scenes.h"

#include "bm_audio.h"
#include "bm_flow.h"
#include "bm_run.h"
#include "bm_run_scenes.h"

#include "bn_keypad.h"
#include "bn_string.h"

#include "bn_sprite_items_spr_particles.h"

namespace bm
{
    // ------------------------------------------------------------------ menu input

    void menu_input::update()
    {
        _up = _down = _left = _right = _ok = _back = _start = false;
        if(_lock > 0)
        {
            --_lock;
            return;
        }
        using k = bn::keypad::key_type;
        _ok = bn::keypad::pressed(k::A);
        _back = bn::keypad::pressed(k::B);
        _start = bn::keypad::pressed(k::START);
        const bool hu = bn::keypad::held(k::UP);
        const bool hd = bn::keypad::held(k::DOWN);
        const bool hl = bn::keypad::held(k::LEFT);
        const bool hr = bn::keypad::held(k::RIGHT);
        if(bn::keypad::pressed(k::UP) || bn::keypad::pressed(k::DOWN) || bn::keypad::pressed(k::LEFT) ||
           bn::keypad::pressed(k::RIGHT))
        {
            _repeat = 0;
            _up = bn::keypad::pressed(k::UP);
            _down = bn::keypad::pressed(k::DOWN);
            _left = bn::keypad::pressed(k::LEFT);
            _right = bn::keypad::pressed(k::RIGHT);
            return;
        }
        if(hu || hd || hl || hr)
        {
            ++_repeat;
            if(_repeat > 18 && _repeat % 5 == 0)
            {
                _up = hu;
                _down = hd;
                _left = hl;
                _right = hr;
            }
        }
        else
        {
            _repeat = 0;
        }
    }

    // ------------------------------------------------------------------ title

    title_scene::title_scene() :
        _backdrop(backdrop_id::title, 3)
    {
        _version.show(236, 150, "V1.0", text_color::gray, align::right);
        audio::play_music(audio::music_id::title);
        for(int i = 0; i < 6; ++i)
        {
            _block_y[i] = -20 - i * 30;
        }
    }

    bn::optional<scene_id> title_scene::update()
    {
        ++_frame;
        _input.update();
        _backdrop.update();
        const bool blink = (_frame / 24) % 2 == 0;
        if(blink)
        {
            _press.show(120, 134, "PRESS START", text_color::yellow, align::center);
        }
        else
        {
            _press.hide();
        }
        // Falling glowing shards around the logo.
        for(int i = 0; i < 6; ++i)
        {
            _block_y[i] += 1 + (i % 3);
            if(_block_y[i] > 170)
            {
                _block_y[i] = -10;
            }
            const int x = 12 + ((i * 53 + _frame / 120 * 17) % 216);
            if(! _blocks[i])
            {
                _blocks[i] = bn::sprite_items::spr_particles.create_sprite(0, 0, 24 + i % 4);
            }
            _blocks[i]->set_position(x - 120, _block_y[i] - 80);
        }
        if(_input.start() || _input.ok())
        {
            audio::sfx(audio::sfx_id::menu_ok);
            return scene_id::main_menu;
        }
        return bn::nullopt;
    }

    // ------------------------------------------------------------------ main menu

    main_menu_scene::main_menu_scene() :
        _backdrop(backdrop_id::menu, 3)
    {
        audio::play_music(audio::music_id::menu);
        _refresh();
    }

    void main_menu_scene::_refresh()
    {
        const app& a = the_app();
        _count = 0;
        _entries[_count++] = { "ADVENTURE", "6 WORLDS, 30 STAGES, 6 BOSSES", game_mode::adventure, scene_id::world_map,
                               false, "" };
        _entries[_count++] = { "CHAOS RUN", "3 ACTS, 12 FIGHTS, 31 UPGRADES", game_mode::chaos_run, scene_id::run_map,
                               false, "" };
        _entries[_count++] = { "CLASSIC", "PURE FALLING-BLOCK PUZZLE", game_mode::classic, scene_id::gameplay, false, "" };
        _entries[_count++] = { "CHAOS ARCADE", "ENDLESS SCORE ATTACK WITH POWERS", game_mode::arcade, scene_id::gameplay,
                               false, "" };
        _entries[_count++] = { "DUAL BOARD", "TWO BOARDS, ONE HEAT METER", game_mode::dual_relay, scene_id::gameplay,
                               false, "" };
        (void) a;
        if(_index >= _count)
        {
            _index = 0;
        }
        _title.show_big(120, 14, "BLOCK MAYHEM", big_color::fire, align::center);
        for(int i = 0; i < 9; ++i)
        {
            if(i >= _count)
            {
                _items[i].hide();
                continue;
            }
            const entry& e = _entries[i];
            const text_color c = e.locked ? text_color::gray : (i == _index ? text_color::yellow : text_color::white);
            bn::string<24> t;
            if(i == _index)
            {
                t = "> ";
            }
            t.append(e.name);
            _items[i].show(120, 50 + i * 13, t, c, align::center);
        }
        const entry& sel = _entries[_index];
        _info.show(120, 146, sel.locked ? sel.lock_reason : sel.info, sel.locked ? text_color::red : text_color::cyan,
                   align::center);
    }

    bn::optional<scene_id> main_menu_scene::update()
    {
        ++_frame;
        _input.update();
        _backdrop.update();
        if(_input.up())
        {
            _index = (_index + _count - 1) % _count;
            audio::sfx(audio::sfx_id::menu_move);
            _refresh();
        }
        else if(_input.down())
        {
            _index = (_index + 1) % _count;
            audio::sfx(audio::sfx_id::menu_move);
            _refresh();
        }
        else if(_input.ok() || _input.start())
        {
            const entry& e = _entries[_index];
            if(e.locked)
            {
                audio::sfx(audio::sfx_id::denied);
                return bn::nullopt;
            }
            audio::sfx(audio::sfx_id::menu_ok);
            app& a = the_app();
            a.flow.mode = e.mode;
            a.flow.seed = a.frame * 2654435761u + 12345u;
            if(e.mode == game_mode::chaos_run)
            {
                begin_chaos_run(a.flow.seed);
            }
            return e.target;
        }
        else if(_input.back())
        {
            audio::sfx(audio::sfx_id::menu_back);
            return scene_id::title;
        }
        return bn::nullopt;
    }

    // ------------------------------------------------------------------ results

    results_scene::results_scene() :
        _backdrop(backdrop_id::menu, 3)
    {
        app& a = the_app();
        const session& s = *a.game;
        const bool won = a.flow.outcome == session_outcome::won;
        const bool adventure = s.config().mode == game_mode::adventure;
        const bool run = s.config().mode == game_mode::chaos_run;
        if(run)
        {
            const chaos_run& r = *a.run;
            const bool boss_fight = s.config().boss != nullptr;
            _title.show_big(120, 10, ! won ? "RUN OVER" : (boss_fight ? "BOSS DOWN!" : "CLEARED!"),
                            won ? big_color::gold : big_color::blood, align::center);
            int line = 0;
            auto row = [this, &line](const bn::string_view& text, text_color c)
            {
                if(line < 9)
                {
                    _lines[line].show(120, 36 + line * 12, text, c, align::center);
                    ++line;
                }
            };
            row(s.config().stage ? s.config().stage->name : "", text_color::cyan);
            if(won)
            {
                bn::string<40> b = boss_fight ? "BOSS BONUS +" : "CLEAR BONUS +";
                const uint32_t act_number = uint32_t(r.act() + 1);
                const uint32_t base = uint32_t(boss_fight ? cfg::run_boss_bonus : cfg::run_encounter_bonus) * act_number;
                b.append(bn::to_string<12>(base));
                row(b, text_color::yellow);
                if(s.secondary().def.kind != objective_kind::none)
                {
                    bn::string<40> g = s.secondary().done ? "BONUS GOAL +" : "BONUS GOAL MISSED";
                    if(s.secondary().done)
                    {
                        g.append(bn::to_string<12>(uint32_t(cfg::run_secondary_bonus) * act_number));
                    }
                    row(g, s.secondary().done ? text_color::green : text_color::gray);
                }
            }
            bn::string<40> sc = "RUN SCORE ";
            sc.append(bn::to_string<12>(r.score()));
            row(sc, text_color::white);
            bn::string<40> en = "ENCOUNTERS ";
            en.append(bn::to_string<4>(r.cleared()));
            en.append("/");
            en.append(bn::to_string<4>(chaos_run::total_encounters));
            row(en, text_color::white);
            const char* hint = r.phase() == run_phase::choose ? "A: CHOOSE AN UPGRADE" : "A: RUN SUMMARY";
            _hint.show(120, 150, hint, text_color::gray, align::center);
            audio::play_music(won ? audio::music_id::victory : audio::music_id::game_over, won);
            return;
        }
        if(adventure)
        {
            _title.show_big(120, 10, won ? "STAGE CLEAR" : "STAGE FAILED", won ? big_color::gold : big_color::blood,
                            align::center);
        }
        else
        {
            _title.show_big(120, 12, won ? "VICTORY" : "RESULTS", won ? big_color::gold : big_color::steel, align::center);
        }
        int line = 0;
        auto row = [this, &line](const char* name, const bn::string_view& value, text_color c)
        {
            if(line >= 9)
            {
                return;
            }
            bn::string<40> t = name;
            t.append(value);
            _lines[line].show(120, 36 + line * 12, t, c, align::center);
            ++line;
        };
        if(adventure && s.config().stage)
        {
            bn::string<40> name = bn::to_string<4>(s.config().stage->world);
            name.append("-");
            name.append(bn::to_string<4>(s.config().stage->index));
            name.append(" ");
            name.append(s.config().stage->name);
            row(name.c_str(), "", text_color::cyan);
            if(won)
            {
                row("RANK  ", rank_name(a.flow.rank), a.flow.rank >= 4 ? text_color::yellow : text_color::white);
                if(s.secondary().def.kind != objective_kind::none)
                {
                    row(s.secondary().done ? "BONUS GOAL: DONE" : "BONUS GOAL: MISSED", "",
                        s.secondary().done ? text_color::green : text_color::gray);
                }
            }
        }
        else
        {
            row(mode_name(s.config().mode), "", text_color::cyan);
        }
        row("SCORE  ", bn::to_string<16>(s.score()), text_color::yellow);
        row("ROWS  ", bn::to_string<16>(s.lines()), text_color::white);
        const uint32_t secs = s.stats().frames / 60;
        bn::string<16> time = bn::to_string<16>(secs / 60);
        time.append(":");
        if(secs % 60 < 10)
        {
            time.append("0");
        }
        time.append(bn::to_string<4>(secs % 60));
        row("TIME  ", time, text_color::white);
        row("MAX CHAIN  ", bn::to_string<8>(s.stats().max_chain), text_color::orange);
        row("QUADS  ", bn::to_string<8>(s.stats().quads), text_color::white);
        if(s.config().heat)
        {
            row("OVERHEATS  ", bn::to_string<8>(s.stats().overheat_success), text_color::red);
        }
        if(a.flow.new_record && line < 9)
        {
            row("NEW RECORD!", "", text_color::green);
        }
        if(adventure)
        {
            _hint.show(120, 150, won ? "A: CONTINUE   B: STAGES" : "A: RETRY   B: STAGES", text_color::gray, align::center);
        }
        else
        {
            _hint.show(120, 150, "A: PLAY AGAIN   B: MENU", text_color::gray, align::center);
        }
        audio::play_music(won ? audio::music_id::victory : audio::music_id::game_over, won);
    }

    bn::optional<scene_id> results_scene::update()
    {
        ++_frame;
        _input.update();
        _backdrop.update();
        app& a = the_app();
        const bool adventure = a.flow.mode == game_mode::adventure;
        const bool won = a.flow.outcome == session_outcome::won;
        if(a.flow.mode == game_mode::chaos_run)
        {
            if((a.flow.autoplay && _frame > 90) || _input.ok() || _input.start())
            {
                audio::sfx(audio::sfx_id::menu_ok);
                return a.run->phase() == run_phase::choose ? scene_id::upgrade_select : scene_id::run_summary;
            }
            return bn::nullopt;
        }
        if(a.flow.autoplay && _frame > 90)
        {
            return adventure ? scene_id::world_map : scene_id::main_menu;
        }
        if(_input.ok() || _input.start())
        {
            audio::sfx(audio::sfx_id::menu_ok);
            a.flow.seed = a.flow.seed * 1103515245u + a.frame;
            if(adventure && won)
            {
                if(a.flow.stage < 5)
                {
                    ++a.flow.stage;
                    return scene_id::stage_card;
                }
                return scene_id::world_map;     // world cleared: celebration on the map
            }
            return scene_id::gameplay;
        }
        if(_input.back())
        {
            audio::sfx(audio::sfx_id::menu_back);
            return adventure ? scene_id::stage_card : scene_id::main_menu;
        }
        return bn::nullopt;
    }
}
