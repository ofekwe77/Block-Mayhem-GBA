#include "bm_run_scenes.h"

#include "bm_audio.h"
#include "bm_boss_view.h"
#include "bm_flow.h"
#include "bm_run.h"

#include "bn_string.h"

#include "bn_sprite_items_spr_cursor.h"
#include "bn_sprite_items_spr_goals.h"
#include "bn_sprite_items_spr_icons.h"
#include "bn_sprite_items_spr_particles.h"
#include "bn_sprite_items_spr_upgrades.h"

namespace bm
{
    namespace
    {
        backdrop_id act_backdrop(int act)
        {
            return act <= 0 ? backdrop_id::world1 : (act == 1 ? backdrop_id::world3 : backdrop_id::world6);
        }

        // ARMORY upgrades show the ability they equip; every other upgrade has its own badge.
        bn::sprite_ptr make_upgrade_icon(int id, int x, int y)
        {
            const upgrade_def& u = upgrade(id);
            if(u.grants_ability >= 0)
            {
                return bn::sprite_items::spr_icons.create_sprite(x - 120, y - 80, u.grants_ability);
            }
            return bn::sprite_items::spr_upgrades.create_sprite(x - 120, y - 80, id);
        }

        text_color rarity_color(rarity r, int frame)
        {
            switch(r)
            {
            case rarity::legendary: return (frame & 16) ? text_color::yellow : text_color::orange;
            case rarity::rare: return text_color::violet;
            case rarity::common:
            default: return text_color::white;
            }
        }

        const char* rarity_name(rarity r)
        {
            switch(r)
            {
            case rarity::legendary: return "LEGENDARY";
            case rarity::rare: return "RARE";
            case rarity::common:
            default: return "COMMON";
            }
        }

        bn::string<16> clock_text(uint32_t frames)
        {
            const uint32_t secs = frames / 60;
            bn::string<16> t = bn::to_string<8>(secs / 60);
            t.append(":");
            if(secs % 60 < 10)
            {
                t.append("0");
            }
            t.append(bn::to_string<4>(secs % 60));
            return t;
        }
    }

    void begin_chaos_run(uint32_t seed)
    {
        app& a = the_app();
        a.run->start(seed);
        a.flow.mode = game_mode::chaos_run;
        if(a.save.runs_started < 0xFFFF)
        {
            ++a.save.runs_started;
        }
        a.write_save();
    }

    // ================================================================== run map

    run_map_scene::run_map_scene() :
        _backdrop(act_backdrop(the_app().run->act()), 3)
    {
        _backdrop.set_dim(bn::fixed(0.45));
        audio::play_music(audio::music_id::menu);
        _refresh();
    }

    void run_map_scene::_refresh()
    {
        app& a = the_app();
        const chaos_run& r = *a.run;

        bn::string<32> t = "CHAOS RUN  ACT ";
        t.append(bn::to_string<4>(r.act() + 1));
        t.append("/3");
        _title.show(6, 3, t, text_color::yellow, align::left);
        bn::string<24> sc = "SCORE ";
        sc.append(bn::to_string<12>(r.score()));
        _score.show(234, 3, sc, text_color::white, align::right);

        // The whole run: three acts of three encounters and a boss.
        for(int act = 0; act < chaos_run::acts; ++act)
        {
            const int y = 17 + act * 14;
            const bool current = act == r.act();
            const bool done = act < r.act();
            bn::string<8> at = "ACT ";
            at.append(bn::to_string<4>(act + 1));
            _acts[act].show(6, y, at, done ? text_color::green : (current ? text_color::white : text_color::gray),
                            align::left);
            for(int k = 0; k < chaos_run::slots_per_act; ++k)
            {
                const int node = act * chaos_run::slots_per_act + k;
                const int x = 60 + k * 26;
                int frame = 0;      // cleared: star
                if(node >= r.index())
                {
                    frame = k == chaos_run::slots_per_act - 1 ? int(objective_kind::boss_defeat)
                                                               : int(run_template_objective(r.template_at(act, k)));
                }
                if(! _nodes[node])
                {
                    _nodes[node] = bn::sprite_items::spr_goals.create_sprite(x + 4 - 120, y + 4 - 80, frame);
                }
                else
                {
                    _nodes[node]->set_tiles(bn::sprite_items::spr_goals.tiles_item(), frame);
                }
                if(node == r.index())
                {
                    if(! _cursor)
                    {
                        _cursor = bn::sprite_items::spr_cursor.create_sprite(0, 0, 0);
                    }
                    _cursor->set_position(x + 4 - 120, y + 4 - 80);
                }
            }
            _boss_names[act].show_tiny(150, y + 1, boss(r.boss_of_act(act)).short_name,
                                       current ? text_color::red : (done ? text_color::green : text_color::gray),
                                       align::left);
        }

        // Next encounter in detail.
        const run_encounter& e = r.encounter();
        const stage_def& d = e.def;
        bn::string<40> name = bn::to_string<4>(r.act() + 1);
        name.append("-");
        name.append(bn::to_string<4>(r.slot() + 1));
        name.append("  ");
        name.append(d.name);
        _name.show(120, 62, name, d.boss ? text_color::red : text_color::yellow, align::center);
        _idea.show(120, 73, d.idea, text_color::cyan, align::center);

        int g = 0;
        auto goal = [this, &g](const bn::string_view& text, text_color c, int icon)
        {
            if(g >= 3)
            {
                return;
            }
            const int y = 86 + g * 10;
            const int width = text::small_width(text) + 10;
            const int x = 120 - width / 2;
            _goals[g].show(x + 10, y, text, c, align::left);
            if(! _goal_icons[g])
            {
                _goal_icons[g] = bn::sprite_items::spr_goals.create_sprite(0, 0, icon);
            }
            _goal_icons[g]->set_tiles(bn::sprite_items::spr_goals.tiles_item(), icon);
            _goal_icons[g]->set_position(x + 4 - 120, y + 4 - 80);
            ++g;
        };
        if(d.boss)
        {
            bn::string<40> line = "DEFEAT IT  (HP ";
            line.append(bn::to_string<4>(e.boss_hp_percent));
            line.append("%)");
            goal(line, text_color::red, int(objective_kind::boss_defeat));
        }
        for(const objective_def& o : d.objectives)
        {
            if(o.kind != objective_kind::none && o.kind != objective_kind::boss_defeat)
            {
                bn::string<40> line;
                objective_text(o, line);
                goal(line, text_color::white, int(o.kind));
            }
        }
        if(d.secondary.kind != objective_kind::none)
        {
            bn::string<40> line = "BONUS: ";
            objective_text(d.secondary, line);
            goal(line, text_color::green, int(d.secondary.kind));
        }
        for(int i = g; i < 3; ++i)
        {
            _goals[i].hide();
            _goal_icons[i].reset();
        }
        if(d.hazards[0].kind != hazard_kind::none)
        {
            bn::string<40> line = "HAZARD: ";
            line.append(hazard_name(int(d.hazards[0].kind)));
            _hazard.show(120, 118, line, text_color::orange, align::center);
        }
        else
        {
            _hazard.show(120, 118, "NO HAZARDS", text_color::gray, align::center);
        }

        // Equipped abilities and build size.
        const run_build& b = r.build();
        for(int i = 0; i < 3; ++i)
        {
            if(i < b.loadout_count)
            {
                if(! _loadout[i])
                {
                    _loadout[i] = bn::sprite_items::spr_icons.create_sprite(0, 0, b.loadout[i]);
                }
                _loadout[i]->set_tiles(bn::sprite_items::spr_icons.tiles_item(), b.loadout[i]);
                _loadout[i]->set_position(14 + i * 18 - 120, 138 - 80);
            }
            else
            {
                _loadout[i].reset();
            }
        }
        bn::string<40> info = "UPGRADES ";
        info.append(bn::to_string<4>(b.upgrades_owned()));
        info.append("  SYNERGIES ");
        info.append(bn::to_string<4>(b.synergies_active()));
        info.append("  LV ");
        info.append(bn::to_string<4>(d.base_level));
        _info.show_tiny(234, 136, info, text_color::white, align::right);
        _show_hint();
    }

    void run_map_scene::_show_hint()
    {
        if(_confirm)
        {
            _hint.show(120, 151, "ABANDON THE RUN?  A: YES  B: NO", text_color::red, align::center);
        }
        else
        {
            _hint.show(120, 151, "A: FIGHT    B: ABANDON RUN", text_color::gray, align::center);
        }
    }

    bn::optional<scene_id> run_map_scene::update()
    {
        ++_frame;
        _input.update();
        _backdrop.update();
        app& a = the_app();
        if(_cursor)
        {
            _cursor->set_tiles(bn::sprite_items::spr_cursor.tiles_item(), (_frame >> 4) & 1);
        }
        if(a.flow.autoplay && _frame > 60)
        {
            a.flow.mode = game_mode::chaos_run;
            return scene_id::gameplay;
        }
        if(_confirm)
        {
            if(_input.ok())
            {
                audio::sfx(audio::sfx_id::menu_back);
                a.run->abandon();
                record_run_end(a);
                a.write_save();
                return scene_id::run_summary;
            }
            if(_input.back())
            {
                _confirm = false;
                audio::sfx(audio::sfx_id::menu_move);
                _show_hint();
            }
            return bn::nullopt;
        }
        if(_input.ok() || _input.start())
        {
            audio::sfx(audio::sfx_id::menu_ok);
            a.flow.mode = game_mode::chaos_run;
            return scene_id::gameplay;
        }
        if(_input.back())
        {
            _confirm = true;
            audio::sfx(audio::sfx_id::denied);
            _show_hint();
        }
        return bn::nullopt;
    }

    // ================================================================== upgrade selection

    upgrade_scene::upgrade_scene() :
        _backdrop(backdrop_id::menu, 3)
    {
        audio::play_music(audio::music_id::menu);
        app& a = the_app();
        // Offered upgrades enter the codex.
        for(int i = 0; i < a.run->offer_count(); ++i)
        {
            a.save.stats.upgrades_seen |= 1u << a.run->offer(i);
        }
        _refresh();
    }

    upgrade_scene::~upgrade_scene() = default;

    void upgrade_scene::_refresh()
    {
        app& a = the_app();
        const chaos_run& r = *a.run;
        const run_build& b = r.build();
        bn::string<40> t = r.boss_slot() ? "BOSS DOWN!  CHOOSE AN UPGRADE" : "CHOOSE AN UPGRADE";
        _title.show(120, 3, t, text_color::yellow, align::center);
        for(int i = 0; i < 3; ++i)
        {
            const int y = 17 + i * 45;
            if(i >= r.offer_count())
            {
                _names[i].hide();
                _levels[i].hide();
                _desc[i][0].hide();
                _desc[i][1].hide();
                _extra[i].hide();
                _icons[i].reset();
                continue;
            }
            const int id = r.offer(i);
            const upgrade_def& u = upgrade(id);
            if(! _icons[i])
            {
                _icons[i] = make_upgrade_icon(id, 20, y + 12);
            }
            _names[i].show(36, y, u.name, rarity_color(u.rare, _frame), align::left);
            bn::string<16> lv;
            if(u.max_stacks > 1)
            {
                lv = "LV ";
                lv.append(bn::to_string<4>(b.stacks[id] + 1));
                lv.append("/");
                lv.append(bn::to_string<4>(u.max_stacks));
            }
            else
            {
                lv = rarity_name(u.rare);
            }
            _levels[i].show_tiny(234, y + 1, lv, u.rare == rarity::common ? text_color::gray : rarity_color(u.rare, 0),
                                 align::right);
            _desc[i][0].show(36, y + 10, u.desc[0], i == _index ? text_color::white : text_color::gray, align::left);
            _desc[i][1].show(36, y + 19, u.desc[1], i == _index ? text_color::white : text_color::gray, align::left);
            const int syn = synergy_completed_by(b, id);
            if(syn >= 0)
            {
                bn::string<48> line = "SYNERGY ";
                line.append(synergy(syn).name);
                line.append(": ");
                line.append(synergy(syn).desc);
                _extra[i].show_tiny(36, y + 30, line, text_color::green, align::left);
            }
            else if(u.grants_ability >= 0)
            {
                _extra[i].show_tiny(36, y + 30, "TAKES A FREE ABILITY SLOT (R / SELECT)", text_color::cyan, align::left);
            }
            else
            {
                _extra[i].hide();
            }
        }
        if(! _cursor)
        {
            _cursor = bn::sprite_items::spr_cursor.create_sprite(0, 0, 0);
        }
        _cursor->set_position(20 - 120, 17 + _index * 45 + 12 - 80);
        _hint.show_tiny(120, 152, "UP / DOWN: COMPARE      A: TAKE IT", text_color::gray, align::center);
    }

    bn::optional<scene_id> upgrade_scene::update()
    {
        ++_frame;
        _input.update();
        _backdrop.update();
        app& a = the_app();
        chaos_run& r = *a.run;
        if(_cursor)
        {
            _cursor->set_tiles(bn::sprite_items::spr_cursor.tiles_item(), (_frame >> 4) & 1);
        }
        // Legendary cards sparkle and pulse.
        int sparkle = 0;
        for(int i = 0; i < r.offer_count() && sparkle < 4; ++i)
        {
            const upgrade_def& u = upgrade(r.offer(i));
            if(u.rare == rarity::legendary)
            {
                _names[i].show(36, 17 + i * 45, u.name, rarity_color(u.rare, _frame), align::left);
                for(int k = 0; k < 2 && sparkle < 4; ++k, ++sparkle)
                {
                    const int phase = (_frame / 6 + k * 3 + i) % 6;
                    const int sx = 20 + (k ? 9 : -9) + (phase % 3) - 1;
                    const int sy = 17 + i * 45 + 12 + (k ? -8 : 8) - (phase / 3);
                    if(! _sparkles[sparkle])
                    {
                        _sparkles[sparkle] = bn::sprite_items::spr_particles.create_sprite(0, 0, 20);
                    }
                    _sparkles[sparkle]->set_position(sx - 120, sy - 80);
                    _sparkles[sparkle]->set_tiles(bn::sprite_items::spr_particles.tiles_item(), 20 + ((_frame >> 3) & 1));
                }
            }
        }
        if(a.flow.autoplay && _frame > 60)
        {
            r.choose(0);
            a.write_save();
            return scene_id::run_map;
        }
        if(_input.up() && r.offer_count())
        {
            _index = (_index + r.offer_count() - 1) % r.offer_count();
            audio::sfx(audio::sfx_id::menu_move);
            _refresh();
        }
        else if(_input.down() && r.offer_count())
        {
            _index = (_index + 1) % r.offer_count();
            audio::sfx(audio::sfx_id::menu_move);
            _refresh();
        }
        else if(_input.ok() || _input.start())
        {
            const upgrade_def& u = upgrade(r.offer_count() ? r.offer(_index) : 0);
            audio::sfx(u.rare == rarity::legendary ? audio::sfx_id::overheat_success : audio::sfx_id::menu_ok);
            r.choose(_index);
            a.write_save();     // persists the upgrade codex
            return scene_id::run_map;
        }
        return bn::nullopt;
    }

    // ================================================================== run summary

    run_summary_scene::run_summary_scene() :
        _backdrop(backdrop_id::menu, 3)
    {
        app& a = the_app();
        const chaos_run& r = *a.run;
        const bool won = r.phase() == run_phase::victory;
        _title.show_big(120, 6, won ? "RUN COMPLETE" : "RUN OVER", won ? big_color::gold : big_color::blood,
                        align::center);
        int line = 0;
        auto row = [this, &line](const bn::string_view& text, text_color c)
        {
            if(line < 6)
            {
                _lines[line].show(120, 28 + line * 11, text, c, align::center);
                ++line;
            }
        };
        if(won)
        {
            row("ALL THREE ACTS CLEARED!", text_color::green);
        }
        else
        {
            bn::string<40> t = "FELL IN ACT ";
            t.append(bn::to_string<4>(r.act() + 1));
            t.append(", ENCOUNTER ");
            t.append(bn::to_string<4>(r.slot() + 1));
            row(t, text_color::orange);
        }
        bn::string<40> enc = "ENCOUNTERS CLEARED ";
        enc.append(bn::to_string<4>(r.cleared()));
        enc.append("/");
        enc.append(bn::to_string<4>(chaos_run::total_encounters));
        row(enc, text_color::white);
        bn::string<40> sc = "SCORE ";
        sc.append(bn::to_string<12>(r.score()));
        row(sc, text_color::yellow);
        bn::string<40> tm = "TIME ";
        tm.append(clock_text(r.frames()));
        tm.append("   ROWS ");
        tm.append(bn::to_string<8>(r.lines()));
        tm.append("   CHAIN ");
        tm.append(bn::to_string<4>(r.max_chain()));
        row(tm, text_color::white);
        if(a.flow.new_record)
        {
            row("NEW BEST RUN!", text_color::green);
        }

        // Final build: every owned upgrade (stack count in the corner).
        const run_build& b = r.build();
        int n = 0;
        int labels = 0;
        for(int id = 0; id < upgrade_count && n < 24; ++id)
        {
            if(! b.stacks[id])
            {
                continue;
            }
            const int x = 18 + (n % 12) * 18;
            const int y = 96 + (n / 12) * 20;
            _build_icons[n] = make_upgrade_icon(id, x, y);
            if(b.stacks[id] > 1 && labels < 12)
            {
                bn::string<4> s = "X";
                s.append(bn::to_string<2>(b.stacks[id]));
                _stack_labels[labels++].show_tiny(x + 9, y + 3, s, text_color::yellow, align::right);
            }
            ++n;
        }
        if(! n)
        {
            _lines[5].show(120, 92, "NO UPGRADES TAKEN", text_color::gray, align::center);
        }
        bn::string<64> syn;
        for(int s = 0; s < synergy_count; ++s)
        {
            if(b.synergy_active(s))
            {
                syn.append(syn.empty() ? "SYNERGIES: " : ", ");
                syn.append(synergy(s).name);
            }
        }
        if(! syn.empty())
        {
            _synergies.show_tiny(120, 138, syn, text_color::green, align::center);
        }
        _hint.show(120, 151, "A: NEW RUN    B: MENU", text_color::gray, align::center);
        audio::play_music(won ? audio::music_id::victory : audio::music_id::game_over, won);
    }

    bn::optional<scene_id> run_summary_scene::update()
    {
        ++_frame;
        _input.update();
        _backdrop.update();
        app& a = the_app();
        if(a.flow.autoplay && _frame > 120)
        {
            return scene_id::main_menu;
        }
        if(_input.ok() || _input.start())
        {
            audio::sfx(audio::sfx_id::menu_ok);
            begin_chaos_run(a.frame * 2654435761u + a.run->seed());
            return scene_id::run_map;
        }
        if(_input.back())
        {
            audio::sfx(audio::sfx_id::menu_back);
            return scene_id::main_menu;
        }
        return bn::nullopt;
    }
}
