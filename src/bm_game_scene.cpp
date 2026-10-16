#include "bm_game_scene.h"

#include "bm_achievements.h"
#include "bm_audio.h"
#include "bm_flow.h"
#include "bm_profile.h"
#include "bm_run.h"
#include "bm_sprite_util.h"

#include "bn_bg_palettes.h"
#include "bn_timer.h"
#include "bn_timers.h"
#include "bn_color.h"
#include "bn_sprite_palettes.h"
#include "bn_string.h"

#include "bn_sprite_items_spr_arrows.h"
#include "bn_sprite_items_spr_cursor.h"
#include "bn_sprite_items_spr_goals.h"
#include "bn_sprite_items_spr_icons.h"

namespace bm
{
    namespace
    {
        // World theme of the current session (Adventure stage world, Chaos Run act world, else World 1).
        int session_world(const app& a)
        {
            if(a.flow.mode == game_mode::boss_rush)
            {
                return a.flow.boss_rush_worlds[a.flow.boss_rush_index < a.flow.boss_rush_count ?
                                               a.flow.boss_rush_index : 0];
            }
            if(a.flow.mode == game_mode::adventure)
            {
                return a.flow.world;
            }
            if(a.flow.mode == game_mode::chaos_run)
            {
                return a.run->encounter().def.world;
            }
            return 1;
        }

        layout_kind layout_for(const app& a)
        {
            if(a.flow.mode == game_mode::dual_relay)
            {
                return layout_kind::dual;
            }
            if(a.flow.mode == game_mode::chaos_run)
            {
                return a.run->encounter().def.boss ? layout_kind::boss : layout_kind::single;
            }
            if(a.flow.mode == game_mode::adventure || a.flow.mode == game_mode::boss_rush)
            {
                const stage_def& st = a.flow.mode == game_mode::boss_rush ? stage(session_world(a), 5)
                                                                           : stage(a.flow.world, a.flow.stage);
                if(st.boss)
                {
                    return boss(st.boss).dual ? layout_kind::dual_boss : layout_kind::boss;
                }
                return st.dual ? layout_kind::dual : layout_kind::single;
            }
            return layout_kind::single;
        }

        audio::music_id music_for(const session& s, const app& a)
        {
            if(s.config().mode == game_mode::classic)
            {
                return audio::music_id::classic;
            }
            if(s.config().boss)
            {
                return s.config().boss_id == 6 ? audio::music_id::final_boss : audio::music_id::boss;
            }
            if(s.config().mode == game_mode::adventure || s.config().mode == game_mode::chaos_run)
            {
                return audio::music_id(int(audio::music_id::world1) + session_world(a) - 1);
            }
            return audio::music_id::world1;
        }

        // HUD anchor positions per layout (screen pixels). Every element has its own slot so nothing overlaps;
        // shared slots (dual boss centre line) are resolved by priority: warning > tip > heat label.
        struct hud_pos
        {
            int16_t score_x = 200, score_y = 98;
            bool score_titled = true;               // "SCORE" title above the value, else one value line
            align score_align = align::center;
            int16_t level_y = 120, lines_y = 132;   // < 0 hidden
            int16_t heat_x = 36, heat_y = 48;       // heat intensity label (MILD .. INFERNO, OVERHEAT)
            bool middle_tiny = false;               // warning / tip / heat label in the compact font
            int16_t icon_x[3] = { 20, 20, 20 };     // ability icon centres
            int16_t icon_y[3] = { 66, 92, 118 };
            int16_t cost_dx = 12, cost_dy = -4;     // cost label offset from the icon centre
            bool cost_under = false;                // compact cost centred under the icon (red = cooldown)
            int16_t status_x = 200, status_y = 153;
            align status_align = align::center;
            int16_t objective_x = 2, objective_y = 150;
            bool objective_center = false;          // rows centred on objective_x, else starting at it
            int16_t objective_step = -9;            // offset between objective rows (negative: bottom-up)
            int16_t warning_x = 120, warning_y = 0;
            int16_t settle_x = 200, settle_y = 142;
            int16_t tip_x = 120, tip_y = 0;
        };

        constexpr hud_pos single_hud = {};

        constexpr hud_pos boss_hud = {
            .score_x = 196, .score_y = 141, .level_y = -1, .lines_y = -1, .heat_x = 128, .heat_y = 121,
            .icon_x = { 112, 128, 144 }, .icon_y = { 138, 138, 138 }, .cost_dx = 0, .cost_dy = 10, .cost_under = true,
            .status_x = 196, .status_y = 121, .objective_x = 196, .objective_y = 30, .objective_center = true,
            .objective_step = 9, .settle_x = 196, .settle_y = 131
        };

        // Dual: the 64-pixel centre column holds NEXT, HOLD, score, the equipped ability, heat and goals.
        constexpr hud_pos dual_hud = {
            .score_x = 120, .score_y = 89, .score_titled = false, .level_y = 130, .lines_y = 139,
            .heat_x = 120, .heat_y = 121, .icon_x = { 104, 104, 104 }, .icon_y = { 104, 104, 104 }, .cost_dx = 10,
            .cost_dy = -4, .status_x = 120, .status_y = 148, .objective_x = 120, .objective_y = 130,
            .objective_center = true, .objective_step = 9, .settle_x = 120, .settle_y = 148
        };

        // Dual boss: the boss owns the centre top; score / ability / status move above the boards.
        constexpr hud_pos dual_boss_hud = {
            .score_x = 86, .score_y = 0, .score_titled = false, .score_align = align::right, .level_y = -1,
            .lines_y = -1, .heat_x = 120, .heat_y = 63, .middle_tiny = true, .icon_x = { 8, 8, 8 },
            .icon_y = { 8, 8, 8 }, .cost_dx = 10, .cost_dy = -8, .status_x = 238, .status_y = 0,
            .status_align = align::right, .objective_x = 120, .objective_y = 130, .objective_center = true,
            .objective_step = 9, .warning_x = 120, .warning_y = 63, .settle_x = 120, .settle_y = 63, .tip_x = 120,
            .tip_y = 63
        };

        const hud_pos& hud_for(layout_kind kind)
        {
            switch(kind)
            {
            case layout_kind::boss: return boss_hud;
            case layout_kind::dual: return dual_hud;
            case layout_kind::dual_boss: return dual_boss_hud;
            case layout_kind::single:
            default: return single_hud;
            }
        }

        // Spice-level colour of the heat intensity label.
        text_color heat_color(const session& s, int frame)
        {
            if(s.overheat_active())
            {
                return (frame & 8) ? text_color::red : text_color::yellow;
            }
            const int h = s.heat();
            if(h >= 80) return (frame & 16) ? text_color::red : text_color::orange;
            if(h >= 60) return text_color::red;
            if(h >= 40) return text_color::orange;
            if(h >= 20) return text_color::yellow;
            return text_color::cyan;
        }

        // Objective icon frame (spr_goals is indexed by objective_kind).
        int goal_icon(objective_kind kind)
        {
            const int k = int(kind);
            return k >= 0 && k <= int(objective_kind::time_limit) ? k : 0;
        }

        backdrop_id backdrop_for(const app& a)
        {
            return backdrop_id(int(backdrop_id::world1) + (session_world(a) - 1));
        }

        constexpr const char* chain_names[9] = {
            "", "", "NICE!", "HOT COMBO!", "SPICY!", "SUPER CHAIN!", "MAYHEM!", "INSANE!", "TOTAL MELTDOWN!"
        };

        constexpr big_color chain_colors[9] = {
            big_color::gold, big_color::gold, big_color::gold, big_color::fire, big_color::fire,
            big_color::neon, big_color::blood, big_color::blood, big_color::blood
        };

        bn::string<16> number(uint32_t v)
        {
            return bn::to_string<16>(v);
        }
    }

    game_scene::game_scene() :
        _app(the_app()),
        _s(*_app.game),
        _camera(bn::camera_ptr::create(0, 0)),
        _backdrop(backdrop_for(_app), 3),
        _view(layout_for(_app), 1),
        _fx(_camera)
    {
        _view.bg().set_camera(_camera);
        _backdrop.set_camera(_camera);
        _score_value.prepare(8, false);
        _fx.set_options(_app.save.settings.shake, _app.save.settings.flashes);
        _start_session();
    }

    game_scene::~game_scene()
    {
        bn::bg_palettes::set_fade_intensity(0);
        bn::sprite_palettes::set_fade_intensity(0);
    }

    void game_scene::_start_session()
    {
        _s.start(make_session_config(_app));
        _view.set_world_tint(session_world(_app));
        if(_app.flow.autoplay)
        {
            _app.autoplayer.reset(bot_strategy::resource_aware, _app.flow.seed);
        }
        _fx.clear();
        _end_timer = 0;
        _end_recorded = false;
        _shown_score = 0xFFFFFFFFu;
        _shown_level = -1;
        _shown_lines = -1;
        _freeze_tint = 0;
        _tip_index = 0;
        _tip_timer = 0;
        for(int i = 0; i < 3; ++i)
        {
            _icons[i].reset();
            _ability_cost[i].hide();
        }
        _boss.reset();
        _board_b_shown = ! _s.board_sealed(1);
        _view.set_board_b_visible(_board_b_shown);
        _boss_name.hide();
        if(_s.has_boss())
        {
            const view_layout& lay = _view.layout();
            const bool sealed = _s.board_sealed(1);
            _boss.emplace(_s.config().boss_id, _camera, sealed ? 192 : lay.boss_x, sealed ? 86 : lay.boss_y);
        }
        audio::play_music(music_for(_s, _app));
    }

    void game_scene::_cell_center(int board, int x, int y, int& sx, int& sy) const
    {
        sx = _view.cell_screen_x(board, x) + 4;
        sy = _view.cell_screen_y(board, y) + 4;
    }

    namespace
    {
        bool cosmetic_event(event_type t)
        {
            switch(t)
            {
            case event_type::cell_destroyed:
            case event_type::special_triggered:
            case event_type::bomb_blast:
            case event_type::fire_burst:
            case event_type::spark_arc:
            case event_type::magnet_pull:
            case event_type::rainbow_link:
            case event_type::charge_pulse:
            case event_type::coat_hit:
                return true;
            default:
                return false;
            }
        }
    }

    void game_scene::_handle_events()
    {
        // Structural events (row flashes, chain waves, boss and hazard events, scoring) are handled at once;
        // cosmetic ones queue up and drain at a fixed rate (the oldest are dropped if the queue overflows).
        event e;
        while(_s.events().pop(e))
        {
            if(cosmetic_event(e.type))
            {
                if(_cosmetic_count == cosmetic_capacity)
                {
                    _cosmetic_head = (_cosmetic_head + 1) % cosmetic_capacity;
                    --_cosmetic_count;
                }
                _cosmetic[(_cosmetic_head + _cosmetic_count) % cosmetic_capacity] = e;
                ++_cosmetic_count;
            }
            else
            {
                _handle_event(e);
            }
        }
        for(int n = 0; n < cosmetic_per_frame && _cosmetic_count; ++n)
        {
            const event c = _cosmetic[_cosmetic_head];
            _cosmetic_head = (_cosmetic_head + 1) % cosmetic_capacity;
            --_cosmetic_count;
            _handle_event(c);
        }
    }

    void game_scene::_handle_event(const event& e)
    {
        bn::timer event_timer;
        _handle_event_body(e);
        const int type = int(e.type);
        if(type >= 0 && type < 96)
        {
            profile::event_ticks[type] += uint32_t(event_timer.elapsed_ticks());
        }
    }

    void game_scene::_handle_event_body(const event& e)
    {
        const view_layout& lay = _view.layout();
        const int board_cols = _s.board_player(0).grid().cols();
        {
            ++_events_handled;
            const int b = e.board < 0 ? 0 : (e.board > 1 ? 1 : e.board);
            int sx = 0;
            int sy = 0;
            if(_boss && e.type >= event_type::boss_damage && e.type <= event_type::boss_defeated)
            {
                _boss->on_event(e, _fx);
            }
            switch(e.type)
            {
            case event_type::hazard_fired:
                _fx.callout(lay.kind == layout_kind::dual ? 120 : _view.cell_screen_x(b, board_cols / 2), 48,
                            hazard_name(e.a), big_color::ice, 50);
                _fx.shake(shake_profile::medium);
                _backdrop.excite(60, 2);
                break;

            case event_type::hazard_warning:
                audio::sfx(audio::sfx_id::danger);
                break;

            case event_type::boss_phase:
                _fx.callout(lay.kind == layout_kind::boss ? 196 : 120, 60,
                            _s.config().boss && e.a + 1 >= _s.config().boss->phase_count ? "FINAL PHASE!" : "PHASE UP!",
                            big_color::blood, 80);
                break;

            case event_type::boss_defeated:
                _backdrop.excite(240, 3);
                break;

            case event_type::piece_moved:
                audio::sfx(audio::sfx_id::move);
                break;

            case event_type::piece_rotated:
                audio::sfx(audio::sfx_id::rotate);
                break;

            case event_type::piece_held:
                audio::sfx(audio::sfx_id::hold);
                break;

            case event_type::hard_drop:
                audio::sfx(audio::sfx_id::hard_drop);
                if(e.a >= 3)
                {
                    _fx.shake(shake_profile::small);
                }
                _cell_center(b, 0, e.d + 1, sx, sy);
                sx = _view.cell_screen_x(b, 0) + e.b * 4;
                for(int i = 0; i < 4; ++i)
                {
                    _fx.particle(particle_kind::dust, sx - 8 + i * 5, sy + 6, bn::fixed(i - 2) / 4, -bn::fixed(1) / 2, 14);
                }
                break;

            case event_type::piece_locked:
                audio::sfx(audio::sfx_id::lock);
                break;

            case event_type::rows_full:
            {
                const uint32_t rows = uint32_t(uint16_t(e.a)) | (uint32_t(uint16_t(e.b)) << 16);
                _view.flash_rows(b, rows, e.c >= 4 ? 24 : 16);
                for(int y = 0; y < 20; ++y)
                {
                    if(! ((rows >> y) & 1))
                    {
                        continue;
                    }
                    for(int x = 0; x < board_cols && _fx.can_spawn(); x += 2)
                    {
                        // Shard drift per column from small tables (no divisions in the hot path).
                        static constexpr int8_t drift_x[5] = { -3, 2, -1, 3, -2 };
                        static constexpr int8_t lift[5] = { 0, 1, 2, 0, 1 };
                        const int k = x >> 1;
                        _cell_center(b, x, y, sx, sy);
                        _fx.particle(particle_kind(int(particle_kind::shard_white) + (k & 3)), sx, sy,
                                     bn::fixed(drift_x[k % 5]) / 4, -bn::fixed(3) / 2 - bn::fixed(lift[k % 5]) / 4, 24,
                                     bn::fixed(1) / 8);
                    }
                }
                static constexpr audio::sfx_id clear_sfx[5] = {
                    audio::sfx_id::clear1, audio::sfx_id::clear1, audio::sfx_id::clear2, audio::sfx_id::clear3,
                    audio::sfx_id::clear4
                };
                audio::sfx(clear_sfx[e.c > 4 ? 4 : e.c]);
                if(e.c >= 4)
                {
                    _fx.shake(shake_profile::medium);
                    _fx.flash(bn::color(31, 31, 31), 35, 8);
                    _fx.callout(_view.cell_screen_x(b, board_cols / 2), 56, "QUAD BLAST!", big_color::gold, 70);
                    _backdrop.excite(90, 2);
                }
                else if(e.c == 3)
                {
                    _fx.shake(shake_profile::small);
                }
                break;
            }

            case event_type::lines_cleared:
                break;

            case event_type::cell_destroyed:
                if(_fx.can_spawn())
                {
                    _cell_center(b, e.a, e.b, sx, sy);
                    _fx.particle(particle_kind::debris, sx, sy, bn::fixed((sx & 7) - 3) / 4, -bn::fixed(5) / 4,
                                 22, bn::fixed(1) / 8);
                }
                break;

            case event_type::special_triggered:
                _cell_center(b, e.a, e.b, sx, sy);
                switch(trigger_kind(e.c))
                {
                case trigger_kind::bomb: audio::sfx(audio::sfx_id::bomb); break;
                case trigger_kind::fire: audio::sfx(audio::sfx_id::fire); break;
                case trigger_kind::spark: audio::sfx(audio::sfx_id::spark); break;
                case trigger_kind::magnet: audio::sfx(audio::sfx_id::magnet); break;
                case trigger_kind::rainbow: audio::sfx(audio::sfx_id::rainbow); break;
                case trigger_kind::pulse: audio::sfx(audio::sfx_id::spark); break;
                default: break;
                }
                break;

            case event_type::bomb_blast:
                _cell_center(b, e.a, e.b, sx, sy);
                _fx.explosion(sx, sy);
                _fx.burst(particle_kind::ember, sx, sy, 5, 3);
                _fx.shake(shake_profile::medium);
                _fx.flash(bn::color(31, 24, 8), 20, 6);
                _fx.hitstop(2);
                _backdrop.excite(40, 1);
                break;

            case event_type::fire_burst:
                if(_fx.can_spawn())
                {
                    _cell_center(b, e.a, e.b, sx, sy);
                    _fx.burst(particle_kind::ember, sx, sy, 6, 2);
                    _fx.particle(particle_kind::smoke, sx, sy - 4, 0, -bn::fixed(1) / 2, 24);
                }
                break;

            case event_type::spark_arc:
            {
                if(! _fx.can_spawn())
                {
                    break;
                }
                _cell_center(b, e.a, e.b, sx, sy);
                static constexpr int dx[4] = { 0, 1, 0, -1 };
                static constexpr int dy[4] = { -1, 0, 1, 0 };
                for(int k = 1; k <= e.d; ++k)
                {
                    _fx.particle(particle_kind::electric, sx + dx[e.c] * k * 8, sy + dy[e.c] * k * 8, 0, 0, 8 + k);
                }
                break;
            }

            case event_type::magnet_pull:
            {
                int tx = 0;
                int ty = 0;
                _cell_center(b, e.a, e.b, sx, sy);
                _cell_center(b, e.c, e.d, tx, ty);
                _fx.particle(particle_kind::star, sx, sy, bn::fixed(tx - sx) / 10, bn::fixed(ty - sy) / 10, 10);
                break;
            }

            case event_type::magnet_fizzle:
                _cell_center(b, e.a, e.b, sx, sy);
                _fx.popup(sx, sy - 6, "+HEAT", text_color::orange, 30);
                break;

            case event_type::rainbow_link:
                _cell_center(b, e.c, e.d, sx, sy);
                _fx.burst(particle_kind::star, sx, sy, 3, 1);
                break;

            case event_type::charge_pulse:
                _cell_center(b, e.a, e.b, sx, sy);
                _fx.burst(particle_kind::electric, sx, sy, 4, 2);
                break;

            case event_type::coat_hit:
                if(_fx.can_spawn())
                {
                    _cell_center(b, e.a, e.b, sx, sy);
                    _fx.particle(coat_kind(e.c) == coat_kind::ice ? particle_kind::crystal : particle_kind::spark, sx,
                                 sy, 0, -bn::fixed(1) / 2, 14);
                }
                audio::sfx(coat_kind(e.c) == coat_kind::ice ? audio::sfx_id::ice : audio::sfx_id::armor);
                break;

            case event_type::chain_wave:
                if(e.a >= 2)
                {
                    const int tier = e.a > 8 ? 8 : e.a;
                    _fx.callout(lay.kind == layout_kind::dual ? 120 : _view.cell_screen_x(b, board_cols / 2), 40,
                                chain_names[tier], chain_colors[tier], 60);
                    audio::sfx(tier >= 5 ? audio::sfx_id::chain_big : audio::sfx_id::chain);
                    if(tier >= 4)
                    {
                        _fx.hitstop(3);
                        _fx.shake(tier >= 6 ? shake_profile::large : shake_profile::medium);
                        _backdrop.excite(120, tier >= 6 ? 3 : 2);
                    }
                }
                break;

            case event_type::score:
            {
                const uint32_t pts = uint32_t(uint16_t(e.a)) | (uint32_t(uint16_t(e.b)) << 16);
                if(pts >= 300)
                {
                    bn::string<16> t = "+";
                    t.append(number(pts));
                    _fx.popup(_view.cell_screen_x(_s.active_board(), board_cols / 2), 104, t, text_color::yellow, 36);
                }
                break;
            }

            case event_type::combo:
            {
                bn::string<16> t = "COMBO x";
                t.append(number(uint32_t(e.a + 1)));
                _fx.popup(_view.cell_screen_x(b, board_cols / 2), 88, t, text_color::cyan, 40);
                break;
            }

            case event_type::b2b:
                _fx.popup(_view.cell_screen_x(b, board_cols / 2), 72, "STREAK!", text_color::orange, 40);
                break;

            case event_type::perfect_clear:
                _fx.callout(_view.cell_screen_x(b, board_cols / 2), 64, "ALL CLEAR!", big_color::toxic, 80);
                audio::sfx(audio::sfx_id::chain_big);
                break;

            case event_type::level_up:
            {
                bn::string<16> t = "LEVEL ";
                t.append(number(uint32_t(e.a)));
                _fx.popup(_view.cell_screen_x(b, board_cols / 2), 120, t, text_color::green, 50);
                audio::sfx(audio::sfx_id::level_up);
                break;
            }

            case event_type::overheat_start:
                _fx.callout(lay.kind == layout_kind::dual ? 120 : _view.cell_screen_x(b, board_cols / 2), 64,
                            "OVERHEAT!", big_color::fire, 80);
                _fx.flash(bn::color(31, 8, 0), 40, 12);
                _fx.shake(shake_profile::large);
                _backdrop.excite(720, 2);
                audio::sfx(audio::sfx_id::overheat_start);
                audio::set_intensity(audio::intensity::overheat);
                break;

            case event_type::overheat_warning:
                _fx.popup(_view.cell_screen_x(b, board_cols / 2), 96, "CLEAR 2 ROWS!", text_color::red, 60);
                break;

            case event_type::overheat_success:
                _fx.callout(lay.kind == layout_kind::dual ? 120 : _view.cell_screen_x(b, board_cols / 2), 64,
                            "HEAT MASTERED!", big_color::gold, 80);
                audio::sfx(audio::sfx_id::overheat_success);
                break;

            case event_type::overheat_fail:
                _fx.callout(lay.kind == layout_kind::dual ? 120 : _view.cell_screen_x(b, board_cols / 2), 64,
                            "OVERLOAD!", big_color::blood, 70);
                audio::sfx(audio::sfx_id::overheat_fail);
                break;

            case event_type::overheat_end:
                audio::set_intensity(audio::intensity::normal);
                break;

            case event_type::ability_selected:
                audio::sfx(audio::sfx_id::menu_move);
                break;

            case event_type::ability_denied:
                audio::sfx(audio::sfx_id::denied);
                _fx.popup(lay.kind == layout_kind::single ? 32 : 120, 150,
                          e.b == 1 ? "NEED HEAT" : (e.b == 2 ? "COOLDOWN" : "NOT NOW"), text_color::red, 30);
                break;

            case event_type::ability_targeting:
                audio::sfx(audio::sfx_id::menu_move);
                break;

            case event_type::ability_cancelled:
                audio::sfx(audio::sfx_id::menu_back);
                break;

            case event_type::ability_fired:
                switch(e.a)
                {
                case cfg::ability_laser:
                    _cell_center(b, e.b, 2, sx, sy);
                    _fx.beam(sx, sy - 12, _view.cell_screen_y(b, _s.board_player(b).grid().rows()), 18);
                    _fx.shake(shake_profile::medium);
                    _fx.flash(bn::color(16, 28, 31), 25, 6);
                    audio::sfx(audio::sfx_id::laser);
                    break;
                case cfg::ability_nuclear_burst:
                    _cell_center(b, e.b, e.c, sx, sy);
                    _fx.explosion(sx - 10, sy - 10);
                    _fx.explosion(sx + 10, sy + 10);
                    _fx.explosion(sx, sy);
                    _fx.burst(particle_kind::ember, sx, sy, 10, 4);
                    _fx.shake(shake_profile::large);
                    _fx.flash(bn::color(31, 31, 20), 45, 10);
                    _fx.hitstop(4);
                    audio::sfx(audio::sfx_id::nuke);
                    break;
                case cfg::ability_time_freeze:
                    audio::sfx(audio::sfx_id::freeze);
                    _fx.callout(_view.cell_screen_x(b, board_cols / 2), 64, "FREEZE!", big_color::ice, 50);
                    break;
                case cfg::ability_gravity_pulse:
                    audio::sfx(audio::sfx_id::gravity);
                    _fx.shake(shake_profile::medium);
                    break;
                case cfg::ability_shield_matrix:
                    audio::sfx(audio::sfx_id::shield);
                    break;
                case cfg::ability_emergency_vent:
                    audio::sfx(audio::sfx_id::vent);
                    for(int i = 0; i < 6; ++i)
                    {
                        _fx.particle(particle_kind::smoke, _view.cell_screen_x(b, 1 + i * 3 % board_cols),
                                     _view.cell_screen_y(b, 4), 0, -bn::fixed(1), 30);
                    }
                    if(e.d == 99)
                    {
                        _fx.callout(_view.cell_screen_x(b, board_cols / 2), 64, "SECOND CHANCE!", big_color::toxic, 80);
                    }
                    break;
                case cfg::ability_chain_spark:
                    _cell_center(b, e.b, e.c, sx, sy);
                    _fx.burst(particle_kind::electric, sx, sy, 6, 2);
                    audio::sfx(audio::sfx_id::spark);
                    break;
                case cfg::ability_shape_shift:
                    audio::sfx(audio::sfx_id::shift);
                    break;
                default:
                    break;
                }
                break;

            case event_type::shield_block:
                _fx.callout(_view.cell_screen_x(b, board_cols / 2), 64, "BLOCKED!", big_color::ice, 60);
                audio::sfx(audio::sfx_id::shield);
                break;

            case event_type::garbage_inserted:
                _fx.shake(shake_profile::medium);
                audio::sfx(audio::sfx_id::garbage);
                break;

            case event_type::gravity_shift:
            {
                static constexpr const char* arrows[4] = { "↓", "←", "→", "↑" };
                bn::string<20> t = e.b ? "SETTLE " : "GRAVITY ";
                t.append(arrows[e.a & 3]);
                _fx.popup(_view.cell_screen_x(b, board_cols / 2), 64, t, text_color::violet, 50);
                _fx.shake(shake_profile::medium);
                _backdrop.excite(60, 1);
                break;
            }

            case event_type::cross_combo:
                _fx.callout(120, 40, "CROSS OVER!", big_color::neon, 70);
                _fx.flash(bn::color(24, 12, 31), 25, 8);
                audio::sfx(audio::sfx_id::chain_big);
                break;

            case event_type::board_switched:
                audio::sfx(audio::sfx_id::switch_board);
                break;

            case event_type::danger_on:
                audio::sfx(audio::sfx_id::danger);
                break;

            case event_type::game_over:
                audio::sfx(audio::sfx_id::game_over);
                break;

            case event_type::stage_clear:
                audio::sfx(audio::sfx_id::stage_clear);
                break;

            default:
                break;
            }
        }
    }

    void game_scene::_update_targeting_sprites()
    {
        if(! _s.targeting())
        {
            _cursor.reset();
            _marker.reset();
            _arrow.reset();
            _target_hint.hide();
            return;
        }
        const int b = _s.active_board();
        const int ability = _s.targeting_ability();
        const int x = _view.cell_screen_x(b, _s.target_x()) + 4;
        const int y = _view.cell_screen_y(b, _s.target_y()) + 4;
        switch(ability)
        {
        case cfg::ability_laser:
            if(! _marker)
            {
                _marker = bn::sprite_items::spr_cursor.create_sprite(0, 0, 2);
                _marker_shown = 2;
            }
            _marker->set_position(x - 120, _view.cell_screen_y(b, 2) - 80 - 10);
            set_frame(*_marker, bn::sprite_items::spr_cursor, 2 + ((_frame >> 3) & 1), _marker_shown);
            _target_hint.show(120, 152, "AIM LEFT/RIGHT  A:FIRE  B:CANCEL", text_color::cyan, align::center);
            break;

        case cfg::ability_nuclear_burst:
        case cfg::ability_chain_spark:
            if(! _cursor)
            {
                _cursor = bn::sprite_items::spr_cursor.create_sprite(0, 0, 0);
                _cursor_shown = 0;
            }
            _cursor->set_position(x - 120, y - 80);
            set_frame(*_cursor, bn::sprite_items::spr_cursor, (_frame >> 3) & 1, _cursor_shown);
            if(ability == cfg::ability_nuclear_burst)
            {
                _cursor->set_scale(2 + bn::fixed((_frame >> 2) & 3) / 8);
            }
            _target_hint.show(120, 152, "AIM WITH D-PAD  A:FIRE  B:CANCEL", text_color::cyan, align::center);
            break;

        case cfg::ability_gravity_pulse:
            if(! _arrow)
            {
                _arrow = bn::sprite_items::spr_arrows.create_sprite(0, 0, 0);
                _arrow_shown = 0;
            }
            _arrow->set_position(_view.cell_screen_x(b, _s.board_player(b).grid().cols() / 2) - 120,
                                 _view.cell_screen_y(b, 10) - 80);
            set_frame(*_arrow, bn::sprite_items::spr_arrows, _s.target_option() & 3, _arrow_shown);
            _target_hint.show(120, 152, "PICK DIRECTION  A:PULSE  B:CANCEL", text_color::violet, align::center);
            break;

        case cfg::ability_shape_shift:
        {
            static constexpr const char* letters[8] = { "", "I", "O", "T", "S", "Z", "J", "L" };
            bn::string<40> t = "SHIFT TO ";
            t.append(letters[int(_s.shape_option(_s.target_option()))]);
            t.append(_s.shape_option_legal(_s.target_option()) ? "  A:OK  B:CANCEL" : "  BLOCKED");
            _target_hint.show(120, 152, t, _s.shape_option_legal(_s.target_option()) ? text_color::green : text_color::red,
                              align::center);
            break;
        }

        default:
            break;
        }
    }

    void game_scene::_show_middle(label& l, int x, int y, const bn::string_view& t, text_color c, bool tiny)
    {
        if(tiny)
        {
            l.show_tiny(x, y + 1, t, c, align::center);
        }
        else
        {
            l.show(x, y, t, c, align::center);
        }
    }

    void game_scene::_update_hud()
    {
        const view_layout& lay = _view.layout();
        const hud_pos& hp = hud_for(lay.kind);
        const bool dual = lay.kind == layout_kind::dual || lay.kind == layout_kind::dual_boss;
        const int hold_cx = lay.hold_x * 8 + 24;
        const int next_cx = lay.next_x * 8 + 24;
        _hold_label.show(hold_cx, lay.hold_y * 8 + 8, "HOLD", text_color::gray, align::center);
        _next_label.show(next_cx, lay.next_y * 8 + 8, "NEXT", text_color::gray, align::center);

        // Score / level / rows.
        const int sx = hp.score_x;
        const int sy = hp.score_y;
        if(hp.score_titled)
        {
            _score_title.show(sx, sy, "SCORE", text_color::gray, align::center);
        }
        if(_s.score() != _shown_score)
        {
            _shown_score = _s.score();
            _score_value.show(sx, hp.score_titled ? sy + 9 : sy, number(_s.score()), text_color::white, hp.score_align);
        }
        // Dual layouts reuse the objective rows for level / rows in endless play.
        const bool stat_rows = ! dual || _s.objective_count() == 0;
        if(hp.level_y >= 0 && stat_rows)
        {
            if(_s.level() != _shown_level || ! _level_value.shown())
            {
                _shown_level = _s.level();
                bn::string<16> t = "LV ";
                t.append(number(uint32_t(_s.level())));
                _level_value.show(sx, hp.level_y, t, text_color::green, align::center);
            }
        }
        else
        {
            _level_value.hide();
        }
        if(hp.lines_y >= 0 && stat_rows)
        {
            if(_s.lines() != _shown_lines || ! _lines_value.shown())
            {
                _shown_lines = _s.lines();
                bn::string<16> t = "ROWS ";
                t.append(number(uint32_t(_s.lines())));
                _lines_value.show(sx, hp.lines_y, t, text_color::cyan, align::center);
            }
        }
        else
        {
            _lines_value.hide();
        }

        // Incoming attack / hazard warning with its countdown in locked pieces (top priority on its line).
        const pending_action& ba = _s.boss_action();
        const pending_action& ha = _s.hazard_action();
        const pending_action* warn = ba.source ? &ba : (ha.source ? &ha : nullptr);
        if(warn)
        {
            bn::string<32> t = warn->source == 2 ? attack_name(warn->kind) : hazard_name(warn->kind);
            t.append(" IN ");
            t.append(number(uint32_t(warn->countdown > 0 ? warn->countdown : 0)));
            _show_middle(_warning, hp.warning_x, hp.warning_y, t,
                         warn->countdown <= 1 ? text_color::red : text_color::orange, hp.middle_tiny);
            _warning.set_visible(warn->countdown > 1 || (_frame & 8));
        }
        else
        {
            _warning.hide();
        }

        // Stage tips: short contextual overlays during the first seconds of a stage (they yield to warnings).
        const stage_def* st = _s.config().stage;
        if(st && _s.state() == session_state::playing && _tip_index < 3)
        {
            if(st->tips[_tip_index])
            {
                if((warn && hp.tip_y == hp.warning_y) || (_ach_timer > 0 && hp.tip_y < 20))
                {
                    _tip.hide();        // warnings and achievement banners own the top line
                }
                else
                {
                    _show_middle(_tip, hp.tip_x, hp.tip_y, st->tips[_tip_index], text_color::yellow, hp.middle_tiny);
                }
                if(++_tip_timer > 200)
                {
                    _tip_timer = 0;
                    ++_tip_index;
                    _tip.hide();
                }
            }
            else
            {
                _tip_index = 3;
                _tip.hide();
            }
        }
        else if(_s.state() != session_state::playing)
        {
            _tip.hide();
        }

        // Status line: freeze / shield.
        bn::string<24> status;
        if(_s.freeze_frames() > 0)
        {
            status = "FREEZE";
        }
        else if(_s.shield_pieces() > 0)
        {
            status = "SHIELD ";
            status.append(number(uint32_t(_s.shield_pieces())));
        }
        const bool heat = _s.config().heat;
        if(! status.empty())
        {
            _status.show(hp.status_x, hp.status_y, status, text_color::cyan, hp.status_align);
        }
        else if(heat || dual)
        {
            _status.hide();
        }

        // Gravity stages: the current settle direction (yields when its slot is shared and busy).
        bool settle = false;
        if(st)
        {
            for(const hazard_def& h : st->hazards)
            {
                settle = settle || h.kind == hazard_kind::settle_cycle;
            }
        }
        const bool settle_busy = (hp.settle_y == hp.status_y && hp.settle_x == hp.status_x && ! status.empty()) ||
                                 (hp.settle_y == hp.warning_y && hp.settle_x == hp.warning_x && warn);
        if(settle && ! settle_busy)
        {
            static constexpr const char* arrows[4] = { "↓", "←", "→", "↑" };
            bn::string<16> t = "SETTLE ";
            t.append(arrows[int(_s.settle_direction()) & 3]);
            _settle.show(hp.settle_x, hp.settle_y, t, text_color::violet, align::center);
        }
        else
        {
            _settle.hide();
        }
        if(dual && hp.lines_y == hp.status_y && (! status.empty() || _settle.shown()))
        {
            _lines_value.hide();
        }

        // Freeze tint.
        const int want_tint = _s.freeze_frames() > 0 ? 1 : 0;
        if(want_tint != _freeze_tint)
        {
            _freeze_tint = want_tint;
            _fx.set_base_tint(bn::color(12, 20, 31), want_tint ? bn::fixed(0.18) : bn::fixed(0));
        }

        // Dual board: forced-switch countdown above the active board.
        if(dual && ! _s.board_sealed(1) && _s.state() == session_state::playing)
        {
            bn::string<16> t = "SWITCH ";
            t.append(number(uint32_t(cfg::dual_forced_switch_locks - _s.forced_switch_locks())));
            const int bx = _view.cell_screen_x(_s.active_board(), _s.board_player(0).grid().cols() / 2);
            _switch_label.show(bx, 8, t, text_color::cyan, align::center);
        }
        else
        {
            _switch_label.hide();
        }

        // Heat intensity label (Classic shows time, pieces and quads instead).
        if(! heat && ! dual)
        {
            const uint32_t secs = _s.stats().frames / 60;
            bn::string<16> t = "TIME ";
            t.append(number(secs / 60));
            t.append(":");
            if(secs % 60 < 10)
            {
                t.append("0");
            }
            t.append(number(secs % 60));
            _heat_title.show(36, 64, t, text_color::white, align::center);
            bn::string<16> p = "PIECES ";
            p.append(number(_s.stats().pieces));
            _heat_name.show(36, 78, p, text_color::gray, align::center);
            bn::string<16> q = "QUADS ";
            q.append(number(_s.stats().quads));
            _status.show(36, 92, q, text_color::yellow, align::center);
        }
        if(heat)
        {
            const bool shared = hp.heat_x == hp.warning_x && hp.heat_y == hp.warning_y;
            if(shared && (_warning.shown() || _tip.shown()))
            {
                _heat_title.hide();
            }
            else
            {
                _show_middle(_heat_title, hp.heat_x, hp.heat_y, heat_label(_s.heat(), _s.overheat_active()),
                             heat_color(_s, _frame), hp.middle_tiny);
            }
            _heat_name.hide();
        }

        // Ability slots: icon plus cost (green = affordable) or cooldown (red).
        const int slots = _s.loadout_count();
        for(int i = 0; i < 3; ++i)
        {
            if(i >= slots)
            {
                _icons[i].reset();
                _ability_cost[i].hide();
                continue;
            }
            const int ability = _s.loadout(i);
            const int ix = hp.icon_x[i];
            const int iy = hp.icon_y[i];
            if(! _icons[i])
            {
                _icons[i] = bn::sprite_items::spr_icons.create_sprite(ix - 120, iy - 80, ability);
            }
            const bool ready = _s.ability_ready(ability);
            const bool selected = i == _s.selected_slot();
            _icons[i]->set_visible(ready || selected || (_frame & 16));
            bn::string<8> t;
            text_color tc = text_color::gray;
            if(_s.cooldown(ability) > 0)
            {
                t = hp.cost_under ? "" : "CD";
                t.append(number(uint32_t(_s.cooldown(ability))));
                tc = text_color::red;
            }
            else
            {
                t = number(uint32_t(_s.ability_cost(ability)));
                tc = _s.heat() >= _s.ability_cost(ability) ? text_color::green : text_color::gray;
            }
            const text_color shown = selected && ready ? text_color::yellow : tc;
            if(hp.cost_under)
            {
                _ability_cost[i].show_tiny(ix + hp.cost_dx, iy + hp.cost_dy, t, shown, align::center);
            }
            else
            {
                _ability_cost[i].show(ix + hp.cost_dx, iy + hp.cost_dy, t, shown, align::left);
            }
            if(selected)
            {
                if(! _icon_frame)
                {
                    _icon_frame = bn::sprite_items::spr_cursor.create_sprite(0, 0, 0);
                    _icon_frame_shown = 0;
                }
                _icon_frame->set_position(ix - 120, iy - 80);
                set_frame(*_icon_frame, bn::sprite_items::spr_cursor, (_frame >> 4) & 1, _icon_frame_shown);
            }
        }
        if(! slots)
        {
            _icon_frame.reset();
        }

        // Objectives: icon + progress (the stage card teaches the icons).
        int visible = 0;
        for(int i = 0; i < _s.objective_count(); ++i)
        {
            visible += _s.objective(i).def.kind != objective_kind::boss_defeat ? 1 : 0;
        }
        int row = 0;
        for(int i = 0; i < 3; ++i)
        {
            if(i >= _s.objective_count() || _s.objective(i).def.kind == objective_kind::boss_defeat)
            {
                _objective[i].hide();
                _goal_icons[i].reset();
                continue;
            }
            const objective_state& o = _s.objective(i);
            bn::string<24> t = number(o.value);
            t.append("/");
            t.append(number(o.def.target));
            // Rows run top-down from objective_y (positive step) or bottom-up ending at it (negative step).
            const int y = hp.objective_step > 0 ? hp.objective_y + row * hp.objective_step
                                                : hp.objective_y + (visible - 1 - row) * hp.objective_step;
            int x = hp.objective_x;
            if(hp.objective_center)
            {
                x -= (10 + text::small_width(t)) / 2;
            }
            if(! _goal_icons[i])
            {
                _goal_icons[i] = bn::sprite_items::spr_goals.create_sprite(0, 0, goal_icon(o.def.kind));
                _goal_icons[i]->set_bg_priority(0);
            }
            _goal_icons[i]->set_position(x + 4 - 120, y + 4 - 80);
            _objective[i].show(x + 10, y, t, o.done ? text_color::green : text_color::white, align::left);
            ++row;
        }

        _update_targeting_sprites();
    }

    void game_scene::_update_achievements()
    {
        // Live checks twice a second; the final check happens in record_session when the session ends.
        if(_s.state() == session_state::playing && _frame % 30 == 0)
        {
            achievement_input in;
            in.live = &_s;
            _app.pending_achievements |= update_achievements(_app.save, in);
        }
        const bool warning = _s.boss_action().source || _s.hazard_action().source;
        if(_ach_timer > 0 && warning && _s.state() == session_state::playing)
        {
            // An attack warning needs the top line: put the banner back in the queue and show it later.
            _app.pending_achievements |= 1u << _ach_current;
            _ach_timer = 0;
            _ach_title.hide();
            _ach_name.hide();
            _ach_icon.reset();
            return;
        }
        if(_ach_timer > 0)
        {
            --_ach_timer;
            // Slide in from above, hold, slide out.
            const int t = 150 - _ach_timer;
            const int dy = t < 10 ? (t - 10) * 2 : (_ach_timer < 10 ? (_ach_timer - 10) * 2 : 0);
            _ach_title.move(120, dy);
            _ach_name.move(124, 8 + dy);
            if(_ach_icon)
            {
                _ach_icon->set_position(_ach_icon_x - 120, 8 + dy + 4 - 80);
            }
            if(! _ach_timer)
            {
                _ach_title.hide();
                _ach_name.hide();
                _ach_icon.reset();
            }
            return;
        }
        if(! _app.pending_achievements || (warning && _s.state() == session_state::playing))
        {
            return;     // never cover an incoming-attack warning; the banner waits for it to resolve
        }
        int id = 0;
        while(id < 32 && ! ((_app.pending_achievements >> id) & 1))
        {
            ++id;
        }
        _app.pending_achievements &= ~(1u << id);
        _ach_current = id;
        const achievement_def& d = achievement(id);
        _ach_title.show_tiny(120, -20, "ACHIEVEMENT UNLOCKED", text_color::yellow, align::center);
        _ach_name.show(124, -12, d.name, text_color::white, align::center);
        const int w = text::small_width(d.name);
        _ach_icon_x = 124 - w / 2 - 8;
        _ach_icon = bn::sprite_items::spr_goals.create_sprite(_ach_icon_x - 120, -12 + 4 - 80, d.icon);
        _ach_icon->set_bg_priority(0);
        _ach_timer = 150;
        _tip.hide();
        audio::sfx(audio::sfx_id::unlock);
    }

    void game_scene::_show_pause(bool show)
    {
        static constexpr const char* items[5] = { "RESUME", "RESTART", "CONTROLS", "GHOST", "QUIT" };
        if(! show)
        {
            for(label& l : _pause_items)
            {
                l.hide();
            }
            _center_text.hide();
            _center_sub.hide();
            bn::bg_palettes::set_fade_intensity(0);
            bn::sprite_palettes::set_fade_intensity(0);
            return;
        }
        bn::bg_palettes::set_fade(bn::color(0, 0, 4), bn::fixed(0.55));
        _center_text.show_big(120, 22, "PAUSE", big_color::steel, align::center);
        if(_show_controls)
        {
            static constexpr const char* lines[6] = {
                "D-PAD: MOVE / SOFT DROP", "UP: HARD DROP", "A / B: ROTATE", "L: HOLD   START: PAUSE",
                "R: ABILITY   SELECT: CYCLE", "B: BACK"
            };
            for(int i = 0; i < 6; ++i)
            {
                _pause_items[i].show(120, 56 + i * 12, lines[i], text_color::white, align::center);
            }
            return;
        }
        for(int i = 0; i < 5; ++i)
        {
            bn::string<24> t = items[i];
            if(i == 3)
            {
                t.append(_app.save.settings.ghost ? ": ON" : ": OFF");
            }
            if(i == 4 && _confirm_quit)
            {
                t = "QUIT? A:YES  B:NO";
            }
            _pause_items[i].show(120, 60 + i * 14, t, i == _pause_index ? text_color::yellow : text_color::white,
                                 align::center);
        }
        _pause_items[5].hide();
    }

    bn::optional<scene_id> game_scene::_update_pause()
    {
        const input_frame in = _app.gameplay_input();
        auto pressed = [&in](uint16_t k) { return in.hit(k); };
        if(_show_controls)
        {
            if(pressed(key_b) || pressed(key_a) || pressed(key_start))
            {
                _show_controls = false;
                _show_pause(true);
                audio::sfx(audio::sfx_id::menu_back);
            }
            return bn::nullopt;
        }
        if(_confirm_quit)
        {
            if(pressed(key_a))
            {
                _show_pause(false);
                _paused = false;
                _app.flow.outcome = session_outcome::quit;
                audio::sfx(audio::sfx_id::menu_back);
                if(_app.flow.mode == game_mode::chaos_run)
                {
                    // Quitting an encounter abandons the run (no free retries).
                    _app.run->abandon();
                    record_run_end(_app);
                    _app.write_save();
                    return scene_id::run_summary;
                }
                if(_app.flow.mode == game_mode::boss_rush)
                {
                    end_boss_rush(_app, false);
                    _app.write_save();
                    return scene_id::boss_rush;
                }
                return scene_id::main_menu;
            }
            if(pressed(key_b))
            {
                _confirm_quit = false;
                _show_pause(true);
                audio::sfx(audio::sfx_id::menu_back);
            }
            return bn::nullopt;
        }
        if(pressed(key_start) || pressed(key_b))
        {
            _paused = false;
            _show_pause(false);
            audio::sfx(audio::sfx_id::menu_back);
            audio::resume_music();
            return bn::nullopt;
        }
        if(pressed(key_up))
        {
            _pause_index = (_pause_index + 4) % 5;
            _show_pause(true);
            audio::sfx(audio::sfx_id::menu_move);
        }
        else if(pressed(key_down))
        {
            _pause_index = (_pause_index + 1) % 5;
            _show_pause(true);
            audio::sfx(audio::sfx_id::menu_move);
        }
        else if(pressed(key_a))
        {
            audio::sfx(audio::sfx_id::menu_ok);
            switch(_pause_index)
            {
            case 0:
                _paused = false;
                _show_pause(false);
                audio::resume_music();
                break;
            case 1:
                _paused = false;
                _show_pause(false);
                _app.flow.seed = _app.flow.seed * 1103515245u + 12345u;
                _start_session();
                break;
            case 2:
                _show_controls = true;
                for(label& l : _pause_items)
                {
                    l.hide();
                }
                _show_pause(true);
                break;
            case 3:
                _app.save.settings.ghost = _app.save.settings.ghost ? 0 : 1;
                _show_pause(true);
                break;
            case 4:
            default:
                _confirm_quit = true;
                _show_pause(true);
                break;
            }
        }
        return bn::nullopt;
    }

    bn::optional<scene_id> game_scene::update()
    {
        ++_frame;
        if(_paused)
        {
            return _update_pause();
        }

        const input_frame in = _app.gameplay_input();
        const session_state st = _s.state();
        if(in.hit(key_start) && (st == session_state::playing || st == session_state::intro ||
                                 st == session_state::transition))
        {
            _paused = true;
            _pause_index = 0;
            _confirm_quit = false;
            _show_pause(true);
            audio::pause_music();
            audio::sfx(audio::sfx_id::pause);
            return bn::nullopt;
        }

        // Section profiling for the emulator stress scenarios: the breakdown of the most expensive frame
        // since the last stats reset (per mille of a frame per section).
        bn::timer section;
        bm_test_interface& ti = *_app.test;
        uint32_t cost[7] = {};
        int slot = 0;
        (void) text::take_generated_count();
        text::generated_ticks_ref() = 0;
        text::generated_max_ref()[0] = 0;
        for(uint32_t& t : profile::event_ticks)
        {
            t = 0;
        }
        _events_handled = 0;
        auto mark = [&section, &cost, &slot]()
        {
            cost[slot++] = uint32_t(section.elapsed_ticks_with_restart()) * 1000u / uint32_t(bn::timers::ticks_per_frame());
        };
        if(! _fx.hitstop_frames())
        {
            _s.update(in);
        }
        mark();
        _handle_events();
        mark();
        _view.update(_s, _app.save.settings.ghost, _frame);
        mark();
        _backdrop.update();
        mark();
        _fx.update();
        mark();
        if(_boss)
        {
            _boss->update(_s, _frame, _fx);
        }
        mark();
        _update_hud();
        _update_achievements();
        mark();
        const uint32_t total = cost[0] + cost[1] + cost[2] + cost[3] + cost[4] + cost[5] + cost[6];
        if(total > ti.prof_sim + ti.prof_events + ti.prof_view + ti.prof_backdrop + ti.prof_fx + ti.prof_boss +
                   ti.prof_hud)
        {
            ti.prof_sim = cost[0];
            ti.prof_events = cost[1];
            ti.prof_view = cost[2];
            ti.prof_backdrop = cost[3];
            ti.prof_fx = cost[4];
            ti.prof_boss = cost[5];
            ti.prof_hud = cost[6];
            ti.prof_events_n = uint32_t(_events_handled);
            ti.prof_texts = uint32_t(text::take_generated_count());
            // Board view sub-sections: boards, telegraphs + HP, previews + meter, palette, map upload request.
            ti.prof_view_boards = profile::slots[0];
            ti.prof_view_marks = profile::slots[1];
            ti.prof_view_panels = profile::slots[2];
            ti.prof_view_palette = profile::slots[3];
            ti.prof_view_upload = profile::slots[4];
            uint32_t best = 0, second = 0;
            int best_type = -1, second_type = -1;
            for(int i = 0; i < 96; ++i)
            {
                const uint32_t t = profile::event_ticks[i];
                if(t > best)
                {
                    second = best;
                    second_type = best_type;
                    best = t;
                    best_type = i;
                }
                else if(t > second)
                {
                    second = t;
                    second_type = i;
                }
            }
            ti.prof_ev_type1 = uint32_t(best_type);
            ti.prof_ev_cost1 = profile::per_mille(int(best));
            ti.prof_ev_type2 = uint32_t(second_type);
            ti.prof_ev_cost2 = profile::per_mille(int(second));
            ti.prof_text_max = profile::per_mille(text::generated_max_ref()[0]);
            ti.prof_text_font = uint32_t(text::generated_max_ref()[1]);
            ti.prof_text_len = uint32_t(text::generated_max_ref()[2]);
            ti.prof_text_cost = uint32_t(text::generated_ticks_ref()) * 1000u / uint32_t(bn::timers::ticks_per_frame());
        }

        // Final boss: the sealed second board opens at its dual phase and the boss moves to the centre.
        const bool want_b = ! _s.board_sealed(1);
        if(want_b != _board_b_shown)
        {
            _board_b_shown = want_b;
            _view.set_board_b_visible(want_b);
            if(want_b && _boss)
            {
                _boss->move_to(_view.layout().boss_x, _view.layout().boss_y, 60);
                _fx.callout(120, 80, "SECOND BOARD!", big_color::neon, 90);
            }
        }

        // Boss phase indicator.
        if(_boss && _s.state() != session_state::intro && ! _boss->defeat_done())
        {
            bn::string<16> t = "PHASE ";
            t.append(number(uint32_t(_s.boss_phase() + 1)));
            t.append("/");
            t.append(number(uint32_t(_s.config().boss->phase_count)));
            const bool dual_boss = _view.layout().kind == layout_kind::dual_boss;
            const bool right = ! dual_boss || _s.board_sealed(1);
            const text_color c = _s.boss_phase() >= 2 ? text_color::red : text_color::orange;
            if(right)
            {
                _boss_name.show(196, 17, t, c, align::center);
            }
            else
            {
                _boss_name.show_tiny(120, 9, t, c, align::center);     // compact tag between the HP bar and the boss
            }
        }
        else
        {
            _boss_name.hide();
        }

        if(_s.state() == session_state::intro)
        {
            const int t = _s.intro_timer();
            const layout_kind kind = _view.layout().kind;
            const int cx = kind == layout_kind::single ? _view.cell_screen_x(0, _s.board_player(0).grid().cols() / 2) : 120;
            if(_s.has_boss() && t > 40)
            {
                // Boss entrance card.
                _center_text.show_big(120, 46, _s.config().boss->short_name, big_color::blood, align::center);
                _center_sub.show(120, 66, _s.config().boss->title, text_color::white, align::center);
            }
            else
            {
                _center_sub.hide();
                _center_text.show_big(cx, 64, t > 40 ? "READY" : "GO!", t > 40 ? big_color::gold : big_color::toxic,
                                      align::center);
            }
        }
        else if(_s.state() == session_state::playing)
        {
            _center_text.hide();
            if(_end_timer == 0)
            {
                _center_sub.hide();
            }
        }

        // A defeated boss finishes its destruction sequence before the stage-clear screen. Safety net: if the
        // defeat event never reached the rig, start the sequence after half a second anyway.
        if(_boss && _s.state() == session_state::won && _boss->state() != boss_anim::defeat && ! _boss->defeat_done())
        {
            if(++_boss_fallback > 30)
            {
                event fallback;
                fallback.type = event_type::boss_defeated;
                _boss->on_event(fallback, _fx);
            }
        }
        const bool boss_dying = _boss && _s.state() == session_state::won && ! _boss->defeat_done();
        if((_s.state() == session_state::won || _s.state() == session_state::lost) && ! boss_dying)
        {
            if(! _end_recorded)
            {
                _end_recorded = true;
                _app.flow.outcome = _s.state() == session_state::won ? session_outcome::won : session_outcome::lost;
                record_session(_app);
                _app.write_save();
                _fx.shake(_s.state() == session_state::won ? shake_profile::medium : shake_profile::large);
                audio::play_music(_s.state() == session_state::won ? audio::music_id::victory : audio::music_id::game_over,
                                  false);
            }
            ++_end_timer;
            const bool won = _s.state() == session_state::won;
            const int cx = _view.layout().kind == layout_kind::dual ? 120 :
                           _view.cell_screen_x(0, _s.board_player(0).grid().cols() / 2);
            _center_text.show_big(cx, 56, won ? "CLEAR!" : "GAME OVER", won ? big_color::gold : big_color::blood,
                                  align::center);
            if(_end_timer > 40)
            {
                _center_sub.show(cx, 80, "PRESS A", text_color::white, align::center);
            }
            if(_end_timer > 60 && (in.hit(key_a) || in.hit(key_start) || _app.flow.autoplay || _end_timer > 600))
            {
                return scene_id::results;
            }
        }
        return bn::nullopt;
    }
}
