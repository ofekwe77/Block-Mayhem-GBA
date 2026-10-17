#include "bm_extra_scenes.h"

#include "bm_achievements.h"
#include "bm_audio.h"
#include "bm_challenges.h"
#include "bm_flow.h"
#include "bm_run.h"

#include "bn_keypad.h"
#include "bn_string.h"

#include "bn_sprite_items_spr_cursor.h"
#include "bn_sprite_items_spr_goals.h"
#include "bn_sprite_items_spr_icons.h"

namespace bm
{
    namespace
    {
        bn::string<16> num(uint32_t v)
        {
            return bn::to_string<16>(v);
        }

        bn::string<16> clock(uint32_t frames)
        {
            const uint32_t secs = frames / 60;
            bn::string<16> t;
            if(secs >= 3600)
            {
                t = bn::to_string<8>(secs / 3600);
                t.append("H ");
                t.append(bn::to_string<4>((secs / 60) % 60));
                t.append("M");
                return t;
            }
            t = bn::to_string<8>(secs / 60);
            t.append(":");
            if(secs % 60 < 10)
            {
                t.append("0");
            }
            t.append(bn::to_string<4>(secs % 60));
            return t;
        }
    }

    // ================================================================== options

    namespace
    {
        int options_last_index = 0;     // the cursor returns to the entry the player left from

        constexpr const char* option_names[9] = {
            "DIFFICULTY", "GHOST PIECE", "SCREEN SHAKE", "FLASHES", "MUSIC VOLUME", "SFX VOLUME", "CUSTOMIZE",
            "SOUND TEST", "CREDITS"
        };
    }

    options_scene::options_scene() :
        _backdrop(backdrop_id::menu, 3)
    {
        audio::play_music(audio::music_id::menu);
        _index = options_last_index;
        _title.show_big(120, 6, "OPTIONS", big_color::steel, align::center);
        _hint.show(120, 151, "LEFT/RIGHT: CHANGE   B: SAVE", text_color::gray, align::center);
        _refresh();
    }

    options_scene::~options_scene()
    {
        options_last_index = _index;
    }

    void options_scene::_refresh()
    {
        const game_settings& g = the_app().save.settings;
        for(int i = 0; i < rows; ++i)
        {
            const int y = 28 + i * 12;
            const bool sel = i == _index;
            bn::string<24> name = sel ? "> " : "  ";
            name.append(option_names[i]);
            _names[i].show(14, y, name, sel ? text_color::yellow : text_color::white, align::left);
            bn::string<16> v;
            _bars[i].hide();
            switch(i)
            {
            case 0:
            {
                static constexpr const char* d[3] = { "BEGINNER", "STANDARD", "EXPERT" };
                v = d[g.difficulty > 2 ? 1 : g.difficulty];
                break;
            }
            case 1: v = g.ghost ? "ON" : "OFF"; break;
            case 2:
            {
                static constexpr const char* d[3] = { "OFF", "REDUCED", "FULL" };
                v = d[g.shake > 2 ? 2 : g.shake];
                break;
            }
            case 3: v = g.flashes ? "FULL" : "REDUCED"; break;
            case 4:
            case 5:
            {
                const int vol = i == 4 ? g.music_volume : g.sfx_volume;
                v = num(uint32_t(vol));
                bn::string<12> bar;
                for(int k = 0; k < 10; ++k)
                {
                    bar.append(k < vol ? "|" : ".");
                }
                _bars[i].show(140, y, bar, vol ? text_color::cyan : text_color::gray, align::left);
                break;
            }
            default:
                v = ">";
                break;
            }
            const bool arrows = i < 6 && sel;
            bn::string<20> shown;
            if(arrows)
            {
                shown = "< ";
                shown.append(v);
                shown.append(" >");
            }
            else
            {
                shown = v;
            }
            _values[i].show(226, y, shown, sel ? text_color::yellow : text_color::cyan, align::right);
        }
        const char* info = "";
        switch(_index)
        {
        case 0:
        {
            static constexpr const char* d[3] = { "SLOWER FALL, LONGER WARNINGS", "THE INTENDED CHALLENGE",
                                                  "FASTER FALL, SHORTER WARNINGS" };
            info = d[g.difficulty > 2 ? 1 : g.difficulty];
            break;
        }
        case 1: info = "SHOWS WHERE THE PIECE LANDS"; break;
        case 2: info = "CAMERA SHAKE ON BIG EFFECTS"; break;
        case 3: info = "SOFTER, SHORTER SCREEN FLASHES"; break;
        case 4: info = "MUSIC LOUDNESS"; break;
        case 5: info = "SOUND EFFECT LOUDNESS"; break;
        case 6: info = "BLOCK, FRAME, TITLE AND PARTICLE STYLES"; break;
        case 7: info = "UNLOCKED MUSIC AND EFFECTS"; break;
        default: info = "THE MAKERS AND THE TOOLS"; break;
        }
        _info.show(120, 139, info, text_color::white, align::center);
    }

    void options_scene::_change(int delta)
    {
        game_settings& g = the_app().save.settings;
        auto wrap = [delta](uint8_t& v, int count)
        {
            v = uint8_t((int(v) + delta + count) % count);
        };
        switch(_index)
        {
        case 0: wrap(g.difficulty, 3); break;
        case 1: wrap(g.ghost, 2); break;
        case 2: wrap(g.shake, 3); break;
        case 3: wrap(g.flashes, 2); break;
        case 4:
        case 5:
        {
            uint8_t& v = _index == 4 ? g.music_volume : g.sfx_volume;
            const int nv = int(v) + delta;
            v = uint8_t(nv < 0 ? 0 : (nv > 10 ? 10 : nv));
            audio::apply_volumes();
            break;
        }
        default:
            return;
        }
        audio::sfx(audio::sfx_id::menu_move);
        _refresh();
    }

    bn::optional<scene_id> options_scene::update()
    {
        ++_frame;
        _input.update();
        _backdrop.update();
        app& a = the_app();
        if(_input.up())
        {
            _index = (_index + rows - 1) % rows;
            audio::sfx(audio::sfx_id::menu_move);
            _refresh();
        }
        else if(_input.down())
        {
            _index = (_index + 1) % rows;
            audio::sfx(audio::sfx_id::menu_move);
            _refresh();
        }
        else if(_input.left())
        {
            _change(-1);
        }
        else if(_input.right())
        {
            _change(1);
        }
        else if(_input.ok() || _input.start())
        {
            if(_index >= 6)
            {
                audio::sfx(audio::sfx_id::menu_ok);
                a.write_save();
                return _index == 6 ? scene_id::customize : (_index == 7 ? scene_id::sound_test : scene_id::credits);
            }
            _change(1);
        }
        else if(_input.back())
        {
            audio::sfx(audio::sfx_id::menu_back);
            a.write_save();
            return scene_id::main_menu;
        }
        return bn::nullopt;
    }

    // ================================================================== records

    records_scene::records_scene() :
        _backdrop(backdrop_id::menu, 3)
    {
        audio::play_music(audio::music_id::menu);
        _title.show_big(120, 4, "RECORDS", big_color::gold, align::center);
        _hint.show(120, 151, "LEFT/RIGHT: PAGE    B: BACK", text_color::gray, align::center);
        _refresh();
    }

    void records_scene::_refresh()
    {
        const save_data& sv = the_app().save;
        for(int i = 0; i < lines; ++i)
        {
            _left[i].hide();
            _right[i].hide();
        }
        static constexpr const char* titles[pages] = {
            "CLASSIC / CHAOS ARCADE", "DUAL BOARD / BOSS RUSH", "ADVENTURE", "CHAOS RUN", "LIFETIME"
        };
        bn::string<40> head = "< ";
        head.append(titles[_index]);
        head.append(" >");
        _page.show(120, 26, head, text_color::yellow, align::center);
        int row = 0;
        auto pair = [this, &row](const bn::string_view& name, const bn::string_view& value, text_color c)
        {
            if(row < lines)
            {
                _left[row].show(20, 40 + row * 9, name, text_color::white, align::left);
                _right[row].show(220, 40 + row * 9, value, c, align::right);
                ++row;
            }
        };
        auto column = [this](int col, const char* name, const uint32_t* table)
        {
            label* labels = col ? _right : _left;
            const int x = col ? 180 : 60;
            labels[0].show(x, 42, name, text_color::cyan, align::center);
            for(int i = 0; i < 5; ++i)
            {
                bn::string<20> t = bn::to_string<4>(i + 1);
                t.append(". ");
                t.append(table[i] ? num(table[i]) : bn::string<16>("-"));
                labels[1 + i].show(x, 56 + i * 12, t, i == 0 && table[i] ? text_color::yellow : text_color::white,
                                   align::center);
            }
        };
        switch(_index)
        {
        case 0:
            column(0, "CLASSIC", sv.classic_high);
            column(1, "CHAOS ARCADE", sv.arcade_high);
            break;

        case 1:
        {
            column(0, "DUAL BOARD", sv.dual_high);
            _right[0].show(180, 42, "BOSS RUSH", text_color::cyan, align::center);
            _right[1].show(180, 56, "BEST SCORE", text_color::white, align::center);
            _right[2].show(180, 68, sv.boss_rush_best_score ? num(sv.boss_rush_best_score) : bn::string<16>("-"),
                           text_color::yellow, align::center);
            _right[3].show(180, 84, "BEST FULL RUSH", text_color::white, align::center);
            _right[4].show(180, 96, sv.boss_rush_best_frames ? clock(sv.boss_rush_best_frames) : bn::string<16>("-"),
                           text_color::yellow, align::center);
            _right[5].show(180, 112, sv.boss_rush_unlocked ? "UNLOCKED" : "LOCKED",
                           sv.boss_rush_unlocked ? text_color::green : text_color::gray, align::center);
            break;
        }

        case 2:
        {
            int cleared = 0;
            int ranks[6] = {};
            uint32_t total = 0;
            for(const stage_record& r : sv.stages)
            {
                cleared += r.cleared ? 1 : 0;
                ++ranks[r.best_rank > 5 ? 5 : r.best_rank];
                total += r.best_score;
            }
            int bosses = 0;
            for(int w = 1; w <= 6; ++w)
            {
                bosses += sv.stages[stage_number(w, 5)].cleared ? 1 : 0;
            }
            bn::string<16> c = num(uint32_t(cleared));
            c.append("/30");
            pair("STAGES CLEARED", c, text_color::yellow);
            bn::string<16> b = num(uint32_t(bosses));
            b.append("/6");
            pair("BOSSES DEFEATED", b, text_color::yellow);
            bn::string<16> wo = num(sv.worlds_unlocked);
            wo.append("/6");
            pair("WORLDS OPEN", wo, text_color::white);
            pair("S+ RANKS", num(uint32_t(ranks[5])), text_color::white);
            pair("S RANKS", num(uint32_t(ranks[4])), text_color::white);
            pair("A RANKS", num(uint32_t(ranks[3])), text_color::white);
            pair("TOTAL BEST SCORE", num(total), text_color::yellow);
            pair("CAMPAIGN", sv.campaign_complete ? "COMPLETE" : "IN PROGRESS",
                 sv.campaign_complete ? text_color::green : text_color::gray);
            break;
        }

        case 3:
        {
            pair("RUNS STARTED", num(sv.runs_started), text_color::white);
            pair("RUNS COMPLETED", num(sv.runs_completed), text_color::yellow);
            pair("BEST SCORE", sv.run_best_score ? num(sv.run_best_score) : bn::string<16>("-"), text_color::yellow);
            bn::string<16> act = "-";
            if(sv.run_best_act >= 4)
            {
                act = "COMPLETE";
            }
            else if(sv.run_best_act)
            {
                act = "ACT ";
                act.append(num(sv.run_best_act));
            }
            pair("FURTHEST", act, text_color::white);
            int seen = 0;
            for(int i = 0; i < upgrade_count; ++i)
            {
                seen += (sv.stats.upgrades_seen >> i) & 1;
            }
            bn::string<16> s = num(uint32_t(seen));
            s.append("/");
            s.append(num(uint32_t(upgrade_count)));
            pair("UPGRADES SEEN", s, text_color::cyan);
            break;
        }

        default:
        {
            const lifetime_stats& st = sv.stats;
            pair("ROWS CLEARED", num(st.lines), text_color::white);
            pair("PIECES LOCKED", num(st.pieces), text_color::white);
            pair("QUAD BLASTS", num(st.quads), text_color::white);
            pair("BOMBS / FIRES", bn::string<24>(num(st.bombs)).append(" / ").append(num(st.fires)), text_color::white);
            pair("SPARKS / ICE", bn::string<24>(num(st.sparks)).append(" / ").append(num(st.ice_broken)),
                 text_color::white);
            pair("CHAINS / BEST", bn::string<24>(num(st.chains)).append(" / ").append(num(st.max_chain)),
                 text_color::orange);
            pair("OVERHEATS WON", bn::string<24>(num(st.overheat_success)).append(" / ").append(num(st.overheats)),
                 text_color::red);
            pair("ABILITIES USED", num(st.abilities_used), text_color::white);
            pair("BOSSES BEATEN", num(st.bosses_defeated), text_color::white);
            pair("CROSS OVERS", num(st.cross_combos), text_color::white);
            pair("PLAY TIME", clock(st.play_seconds * 60), text_color::cyan);
            break;
        }
        }
    }

    bn::optional<scene_id> records_scene::update()
    {
        _input.update();
        _backdrop.update();
        if(_input.left() || _input.right())
        {
            _index = (_index + (_input.left() ? pages - 1 : 1)) % pages;
            audio::sfx(audio::sfx_id::menu_move);
            _refresh();
        }
        else if(_input.back() || _input.ok())
        {
            audio::sfx(audio::sfx_id::menu_back);
            return scene_id::main_menu;
        }
        return bn::nullopt;
    }

    // ================================================================== achievements

    achievements_scene::achievements_scene() :
        _backdrop(backdrop_id::menu, 3)
    {
        audio::play_music(audio::music_id::menu);
        _hint.show_tiny(120, 152, "UP/DOWN: BROWSE    B: BACK", text_color::gray, align::center);
        _refresh();
    }

    void achievements_scene::_refresh()
    {
        const save_data& sv = the_app().save;
        bn::string<32> t = "ACHIEVEMENTS ";
        t.append(num(uint32_t(achievements_unlocked(sv))));
        t.append("/");
        t.append(num(uint32_t(achievement_total)));
        _title.show(120, 6, t, text_color::yellow, align::center);
        for(int r = 0; r < visible; ++r)
        {
            const int id = _top + r;
            const int y = 22 + r * 14;
            if(id >= achievement_total)
            {
                _names[r].hide();
                _progress[r].hide();
                _icons[r].reset();
                continue;
            }
            const achievement_def& d = achievement(id);
            const bool done = (sv.achievements >> id) & 1;
            const bool secret = d.hidden && ! done;
            const int icon = secret ? 0 : d.icon;
            if(! _icons[r])
            {
                _icons[r] = bn::sprite_items::spr_goals.create_sprite(0, 0, icon);
            }
            _icons[r]->set_tiles(bn::sprite_items::spr_goals.tiles_item(), icon);
            _icons[r]->set_position(22 - 120, y + 4 - 80);
            _names[r].show(34, y, secret ? "???" : d.name,
                           done ? text_color::yellow : (id == _index ? text_color::white : text_color::gray), align::left);
            bn::string<16> p;
            text_color pc = text_color::gray;
            if(done)
            {
                p = "★";
                pc = text_color::yellow;
            }
            else if(d.target > 1 && ! secret)
            {
                int value = sv.achievement_progress[id];
                if(id == ach_arsenal)
                {
                    int bits = 0;
                    for(int k = 0; k < 8; ++k)
                    {
                        bits += (value >> k) & 1;
                    }
                    value = bits;
                }
                p = num(uint32_t(value));
                p.append("/");
                p.append(num(d.target));
            }
            else
            {
                p = "-";
            }
            _progress[r].show(226, y, p, pc, align::right);
        }
        if(! _cursor)
        {
            _cursor = bn::sprite_items::spr_cursor.create_sprite(0, 0, 0);
        }
        _cursor->set_position(22 - 120, 22 + (_index - _top) * 14 + 4 - 80);
        const achievement_def& sel = achievement(_index);
        const bool sel_done = (sv.achievements >> _index) & 1;
        _desc.show(120, 138, sel.hidden && ! sel_done ? "HIDDEN - KEEP PLAYING" : sel.desc,
                   sel_done ? text_color::green : text_color::cyan, align::center);
    }

    bn::optional<scene_id> achievements_scene::update()
    {
        _input.update();
        _backdrop.update();
        if(_input.up() || _input.down())
        {
            _index = (_index + (_input.up() ? achievement_total - 1 : 1)) % achievement_total;
            if(_index < _top)
            {
                _top = _index;
            }
            else if(_index >= _top + visible)
            {
                _top = _index - visible + 1;
            }
            audio::sfx(audio::sfx_id::menu_move);
            _refresh();
        }
        else if(_input.back() || _input.ok())
        {
            audio::sfx(audio::sfx_id::menu_back);
            return scene_id::main_menu;
        }
        return bn::nullopt;
    }

    // ================================================================== credits

    namespace
    {
        struct credit_line
        {
            const char* text;
            text_color color;
        };

        constexpr credit_line credit_lines[] = {
            { "BLOCK MAYHEM", text_color::orange },
            { "THE SPICE PROTOCOL", text_color::yellow },
            { "", text_color::white },
            { "ORIGINAL DESIGN, CODE, PIXEL", text_color::white },
            { "ART, MUSIC AND SOUND EFFECTS", text_color::white },
            { "MADE FOR THIS PROJECT", text_color::white },
            { "", text_color::white },
            { "DEVELOPED WITH CLAUDE", text_color::cyan },
            { "(ANTHROPIC)", text_color::gray },
            { "", text_color::white },
            { "- THE WORLDS -", text_color::yellow },
            { "CLASSIC CHAOS", text_color::white },
            { "FROZEN FACTORY", text_color::white },
            { "VOLCANO MADNESS", text_color::white },
            { "ELECTRIC CITY", text_color::white },
            { "GRAVITY DIMENSION", text_color::white },
            { "THE FINAL MELTDOWN", text_color::white },
            { "", text_color::white },
            { "- THE BOSSES -", text_color::red },
            { "THE BLOCK MONSTER", text_color::white },
            { "THE CRYO TITAN", text_color::white },
            { "THE MAGMA HYDRA", text_color::white },
            { "THE ELECTRIC EMPEROR", text_color::white },
            { "THE GRAVITY KING", text_color::white },
            { "THE CORE OF MAYHEM", text_color::white },
            { "", text_color::white },
            { "- BUILT WITH -", text_color::yellow },
            { "BUTANO ENGINE", text_color::white },
            { "BY GUSTAVO VALIENTE (ZLIB)", text_color::gray },
            { "DEVKITARM / DEVKITPRO", text_color::white },
            { "MAXMOD AUDIO", text_color::white },
            { "TESTED IN THE MGBA CORE", text_color::white },
            { "", text_color::white },
            { "NO BLOCKS WERE HARMED", text_color::gray },
            { "...ALL RIGHT, MANY WERE.", text_color::gray },
            { "", text_color::white },
            { "THANK YOU FOR PLAYING!", text_color::orange },
        };

        constexpr int credit_count = int(sizeof(credit_lines) / sizeof(credit_lines[0]));
    }

    credits_scene::credits_scene() :
        _backdrop(backdrop_id::menu, 3)
    {
        audio::play_music(audio::music_id::ending);
        for(int& l : _line_of_slot)
        {
            l = -1;
        }
        _place();
    }

    void credits_scene::_place()
    {
        const int offset = _frame / 2;
        for(int i = 0; i < credit_count; ++i)
        {
            const int y = 168 + i * 12 - offset;
            const int slot = i % slots;
            if(y < -10 || y > 168)
            {
                if(_line_of_slot[slot] == i)
                {
                    _lines[slot].hide();
                    _line_of_slot[slot] = -1;
                }
                continue;
            }
            if(_line_of_slot[slot] != i)
            {
                _line_of_slot[slot] = i;
                if(credit_lines[i].text[0])
                {
                    _lines[slot].show(120, y, credit_lines[i].text, credit_lines[i].color, align::center);
                }
                else
                {
                    _lines[slot].hide();
                }
            }
            else
            {
                _lines[slot].move(120, y);
            }
        }
    }

    bn::optional<scene_id> credits_scene::update()
    {
        ++_frame;
        _input.update();
        _backdrop.update();
        app& a = the_app();
        const scene_id exit_to = a.flow.credits_from_ending ? scene_id::title : scene_id::options;
        if(_input.back() || _input.start())
        {
            audio::sfx(audio::sfx_id::menu_back);
            a.flow.credits_from_ending = false;
            return exit_to;
        }
        const bool fast = _input.ok() || a.flow.autoplay;
        _frame += fast ? 3 : 0;
        _place();
        const int end = 168 + credit_count * 12 - _frame / 2;
        if(a.flow.credits_from_ending && end < 40)
        {
            _the_end.show_big(120, 60, "THE END", big_color::gold, align::center);
        }
        // Leave after the last line scrolled away (the ending holds THE END a little longer).
        if(end < (a.flow.credits_from_ending ? -160 : -40))
        {
            a.flow.credits_from_ending = false;
            return exit_to;
        }
        return bn::nullopt;
    }

    // ================================================================== sound test

    sound_test_scene::sound_test_scene() :
        _backdrop(backdrop_id::menu, 3)
    {
        audio::stop_music();
        _title.show_big(120, 6, "SOUND TEST", big_color::neon, align::center);
        _hint.show(120, 151, "UP/DOWN  LEFT/RIGHT  A: PLAY  B: BACK", text_color::gray, align::center);
        _refresh();
    }

    sound_test_scene::~sound_test_scene() = default;

    void sound_test_scene::_refresh()
    {
        const save_data& sv = the_app().save;
        bn::string<24> m = _row == 0 ? "> MUSIC  < " : "  MUSIC  < ";
        if(_track < 10)
        {
            m.append("0");
        }
        m.append(num(uint32_t(_track)));
        m.append(" >");
        _music.show(120, 44, m, _row == 0 ? text_color::yellow : text_color::white, align::center);
        const bool open = track_unlocked(sv, _track);
        _music_name.show(120, 58, open ? audio::track_name(audio::music_id(_track)) : "???  KEEP PLAYING",
                         open ? text_color::cyan : text_color::gray, align::center);
        bn::string<24> e = _row == 1 ? "> SOUND  < " : "  SOUND  < ";
        if(_effect + 1 < 10)
        {
            e.append("0");
        }
        e.append(num(uint32_t(_effect + 1)));
        e.append(" >");
        _sfx.show(120, 86, e, _row == 1 ? text_color::yellow : text_color::white, align::center);
        _sfx_name.show(120, 100, audio::sfx_name(audio::sfx_id(_effect)), text_color::cyan, align::center);
        int unlocked = 0;
        for(int t = 1; t < int(audio::music_id::count); ++t)
        {
            unlocked += track_unlocked(sv, t) ? 1 : 0;
        }
        bn::string<32> info = "TRACKS UNLOCKED ";
        info.append(num(uint32_t(unlocked)));
        info.append("/");
        info.append(num(uint32_t(int(audio::music_id::count) - 1)));
        _info.show(120, 126, info, text_color::white, align::center);
    }

    bn::optional<scene_id> sound_test_scene::update()
    {
        _input.update();
        _backdrop.update();
        const int tracks = int(audio::music_id::count) - 1;
        const int effects = int(audio::sfx_id::count);
        if(_input.up() || _input.down())
        {
            _row = 1 - _row;
            audio::sfx(audio::sfx_id::menu_move);
            _refresh();
        }
        else if(_input.left() || _input.right())
        {
            const int d = _input.left() ? -1 : 1;
            if(_row == 0)
            {
                _track = 1 + (_track - 1 + d + tracks) % tracks;
            }
            else
            {
                _effect = (_effect + d + effects) % effects;
            }
            _refresh();
        }
        else if(_input.ok())
        {
            if(_row == 0)
            {
                if(track_unlocked(the_app().save, _track))
                {
                    audio::play_music(audio::music_id(_track));
                }
                else
                {
                    audio::sfx(audio::sfx_id::denied);
                }
            }
            else
            {
                audio::sfx(audio::sfx_id(_effect));
            }
        }
        else if(_input.back())
        {
            audio::sfx(audio::sfx_id::menu_back);
            return scene_id::options;
        }
        return bn::nullopt;
    }

    // ================================================================== loadout setup

    mode_setup_scene::mode_setup_scene() :
        _backdrop(backdrop_id::menu, 3)
    {
        app& a = the_app();
        if(a.flow.mode == game_mode::dual_relay)
        {
            _chosen[0] = a.flow.dual_ability;
            _chosen_count = (a.save.abilities_unlocked >> _chosen[0]) & 1 ? 1 : 0;
        }
        else
        {
            for(int i = 0; i < 3; ++i)
            {
                const uint8_t ab = a.flow.arcade_loadout[i];
                if((a.save.abilities_unlocked >> ab) & 1)
                {
                    bool dup = false;
                    for(int k = 0; k < _chosen_count; ++k)
                    {
                        dup = dup || _chosen[k] == ab;
                    }
                    if(! dup)
                    {
                        _chosen[_chosen_count++] = ab;
                    }
                }
            }
        }
        _unlocked = challenges_unlocked(a.save);
        _challenges = uint8_t(a.save.settings.challenges & _unlocked);
        _show_page(_has_abilities() ? 0 : 1);
    }

    bn::optional<scene_id> mode_setup_scene::_launch()
    {
        app& a = the_app();
        audio::sfx(audio::sfx_id::menu_ok);
        if(a.flow.mode == game_mode::dual_relay)
        {
            a.flow.dual_ability = _chosen[0];
        }
        else if(a.flow.mode == game_mode::arcade)
        {
            // Fewer than three picks: the remaining slots repeat nothing (empty slots are skipped).
            for(int k = 0; k < 3; ++k)
            {
                a.flow.arcade_loadout[k] = _chosen[k < _chosen_count ? k : 0];
            }
            a.flow.arcade_count = uint8_t(_chosen_count);
        }
        if(a.save.settings.challenges != _challenges)
        {
            a.save.settings.challenges = _challenges;      // remembered for the next game
            a.write_save();
        }
        return scene_id::gameplay;
    }

    bool mode_setup_scene::_has_abilities() const
    {
        return the_app().flow.mode != game_mode::classic;
    }

    int mode_setup_scene::_max_slots() const
    {
        return the_app().flow.mode == game_mode::dual_relay ? 1 : 3;
    }

    void mode_setup_scene::_show_page(int page)
    {
        _page = page;
        const game_mode mode = the_app().flow.mode;
        if(_page == 0)
        {
            for(label& l : _ch_names)
            {
                l.hide();
            }
            for(label& l : _ch_values)
            {
                l.hide();
            }
            _title.show(120, 6, mode == game_mode::dual_relay ? "DUAL BOARD: PICK ONE ABILITY" : "CHAOS ARCADE: PICK 3 ABILITIES",
                        text_color::yellow, align::center);
            _hint.show(120, 151, "A: PICK   R: CHALLENGES   START: PLAY", text_color::gray, align::center);
            _refresh();
            return;
        }
        for(int i = 0; i < 8; ++i)
        {
            if(_icons[i])
            {
                _icons[i]->set_visible(false);
            }
            _names[i].hide();
            _slots[i].hide();
        }
        if(_cursor)
        {
            _cursor->set_visible(false);
        }
        const char* title = mode == game_mode::classic ? "CLASSIC: CHALLENGES" :
                            (mode == game_mode::dual_relay ? "DUAL BOARD: CHALLENGES" : "CHAOS ARCADE: CHALLENGES");
        _title.show(120, 6, title, text_color::yellow, align::center);
        _hint.show(120, 151, _has_abilities() ? "A: TOGGLE   R: ABILITIES   START: PLAY" : "A: TOGGLE   START: PLAY   B: BACK",
                   text_color::gray, align::center);
        _refresh_challenges();
    }

    void mode_setup_scene::_refresh_challenges()
    {
        auto multiplier = [](int percent)
        {
            bn::string<8> t = "x";
            t.append(num(uint32_t(percent / 100)));
            t.append(".");
            if(percent % 100 < 10)
            {
                t.append("0");
            }
            t.append(num(uint32_t(percent % 100)));
            return t;
        };
        for(int i = 0; i < challenge_total; ++i)
        {
            const challenge_def& d = challenge(i);
            const bool open = (_unlocked >> i) & 1;
            const bool on = (_challenges >> i) & 1;
            const bool sel = i == _ch_index;
            const int y = 30 + i * 13;
            bn::string<24> name = sel ? "> " : "  ";
            name.append(open ? d.name : "? ? ?");
            _ch_names[i].show(14, y, name, sel ? text_color::yellow : (open ? text_color::white : text_color::gray),
                              align::left);
            bn::string<16> v;
            if(! open)
            {
                v = "LOCKED";
            }
            else
            {
                v = on ? "ON  " : "OFF  ";
                v.append(multiplier(d.score_percent));
            }
            _ch_values[i].show(226, y, v, ! open ? text_color::gray : (on ? text_color::green : text_color::cyan),
                               align::right);
        }
        const challenge_def& d = challenge(_ch_index);
        if((_unlocked >> _ch_index) & 1)
        {
            _info[0].show(120, 116, d.desc, text_color::white, align::center);
        }
        else
        {
            bn::string<40> l = "CLEAR WORLD ";
            l.append(num(d.unlock_world));
            l.append(" TO UNLOCK");
            _info[0].show(120, 116, l, text_color::red, align::center);
        }
        bn::string<40> total = "SCORE ";
        total.append(multiplier(challenge_score_percent(_challenges)));
        if(! _challenges)
        {
            total = "NO CHALLENGES: NORMAL RULES";
        }
        _info[1].show(120, 130, total, _challenges ? text_color::violet : text_color::gray, align::center);
    }

    void mode_setup_scene::_refresh()
    {
        const save_data& sv = the_app().save;
        for(int i = 0; i < cfg::ability_count; ++i)
        {
            const int x = 36 + (i % 4) * 56;
            const int y = 40 + (i / 4) * 46;
            const bool open = (sv.abilities_unlocked >> i) & 1;
            if(! _icons[i])
            {
                _icons[i] = bn::sprite_items::spr_icons.create_sprite(x - 120, y - 80, i);
            }
            _icons[i]->set_visible(open);
            int order = -1;
            for(int k = 0; k < _chosen_count; ++k)
            {
                if(_chosen[k] == i)
                {
                    order = k;
                }
            }
            _names[i].show_tiny(x, y + 11, open ? ability_short(i) : "LOCKED",
                                ! open ? text_color::gray : (order >= 0 ? text_color::yellow : text_color::white),
                                align::center);
            if(order >= 0)
            {
                _slots[i].show(x + 12, y - 12, num(uint32_t(order + 1)), text_color::green, align::center);
            }
            else
            {
                _slots[i].hide();
            }
        }
        if(! _cursor)
        {
            _cursor = bn::sprite_items::spr_cursor.create_sprite(0, 0, 0);
        }
        _cursor->set_position(36 + (_index % 4) * 56 - 120, 40 + (_index / 4) * 46 - 80);
        const bool open = (sv.abilities_unlocked >> _index) & 1;
        if(open)
        {
            bn::string<40> l1 = ability_name(_index);
            l1.append("  COST ");
            l1.append(num(uint32_t(cfg::ability_cost[_index])));
            l1.append("  CD ");
            l1.append(num(uint32_t(cfg::ability_cooldown[_index])));
            _info[0].show(120, 122, l1, text_color::cyan, align::center);
            _info[1].show(120, 133, ability_desc(_index), text_color::white, align::center);
        }
        else
        {
            const int st = ability_unlock_stage(_index);
            bn::string<40> l1 = "LOCKED";
            if(st >= 0)
            {
                l1 = "CLEAR ADVENTURE STAGE ";
                l1.append(num(uint32_t(st / 5 + 1)));
                l1.append("-");
                l1.append(num(uint32_t(st % 5 + 1)));
            }
            _info[0].show(120, 122, l1, text_color::red, align::center);
            _info[1].show(120, 133, "TO UNLOCK THIS ABILITY", text_color::gray, align::center);
        }
    }

    bn::optional<scene_id> mode_setup_scene::update()
    {
        ++_frame;
        _input.update();
        _backdrop.update();
        app& a = the_app();
        if(_cursor && _page == 0)
        {
            _cursor->set_tiles(bn::sprite_items::spr_cursor.tiles_item(), (_frame >> 4) & 1);
        }
        if(_has_abilities() && (bn::keypad::r_pressed() || bn::keypad::l_pressed() || bn::keypad::select_pressed()))
        {
            audio::sfx(audio::sfx_id::menu_move);
            _show_page(1 - _page);
            return bn::nullopt;
        }
        if(_page == 1)
        {
            if(_input.up() || _input.down())
            {
                _ch_index = (_ch_index + (_input.up() ? challenge_total - 1 : 1)) % challenge_total;
                audio::sfx(audio::sfx_id::menu_move);
                _refresh_challenges();
            }
            else if(_input.ok() || _input.left() || _input.right())
            {
                if(! ((_unlocked >> _ch_index) & 1))
                {
                    audio::sfx(audio::sfx_id::denied);
                    return bn::nullopt;
                }
                _challenges = uint8_t(_challenges ^ (1u << _ch_index));
                audio::sfx((_challenges >> _ch_index) & 1 ? audio::sfx_id::menu_ok : audio::sfx_id::menu_back);
                _refresh_challenges();
            }
            else if(_input.start())
            {
                if(_has_abilities() && ! _chosen_count)
                {
                    audio::sfx(audio::sfx_id::denied);
                    _show_page(0);
                    return bn::nullopt;
                }
                return _launch();
            }
            else if(_input.back())
            {
                audio::sfx(audio::sfx_id::menu_back);
                return scene_id::main_menu;
            }
            return bn::nullopt;
        }
        if(_input.left() || _input.right())
        {
            _index = (_index % 4 + (_input.left() ? 3 : 1)) % 4 + (_index / 4) * 4;
            audio::sfx(audio::sfx_id::menu_move);
            _refresh();
        }
        else if(_input.up() || _input.down())
        {
            _index = (_index + 4) % 8;
            audio::sfx(audio::sfx_id::menu_move);
            _refresh();
        }
        else if(_input.ok())
        {
            if(! ((a.save.abilities_unlocked >> _index) & 1))
            {
                audio::sfx(audio::sfx_id::denied);
                return bn::nullopt;
            }
            int found = -1;
            for(int k = 0; k < _chosen_count; ++k)
            {
                if(_chosen[k] == _index)
                {
                    found = k;
                }
            }
            if(found >= 0)
            {
                for(int k = found; k + 1 < _chosen_count; ++k)
                {
                    _chosen[k] = _chosen[k + 1];
                }
                --_chosen_count;
                audio::sfx(audio::sfx_id::menu_back);
            }
            else if(_chosen_count < _max_slots())
            {
                _chosen[_chosen_count++] = uint8_t(_index);
                audio::sfx(audio::sfx_id::menu_ok);
            }
            else if(_max_slots() == 1)
            {
                _chosen[0] = uint8_t(_index);
                audio::sfx(audio::sfx_id::menu_ok);
            }
            else
            {
                audio::sfx(audio::sfx_id::denied);
            }
            _refresh();
        }
        else if(_input.start())
        {
            if(! _chosen_count)
            {
                audio::sfx(audio::sfx_id::denied);
                return bn::nullopt;
            }
            return _launch();
        }
        else if(_input.back())
        {
            audio::sfx(audio::sfx_id::menu_back);
            return scene_id::main_menu;
        }
        return bn::nullopt;
    }
}
