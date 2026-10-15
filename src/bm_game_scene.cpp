#include "bm_game_scene.h"

#include "bm_audio.h"
#include "bm_flow.h"

#include "bn_bg_palettes.h"
#include "bn_color.h"
#include "bn_sprite_palettes.h"
#include "bn_string.h"

#include "bn_sprite_items_spr_arrows.h"
#include "bn_sprite_items_spr_cursor.h"
#include "bn_sprite_items_spr_icons.h"

namespace bm
{
    namespace
    {
        layout_kind layout_for(const app& a)
        {
            if(a.flow.mode == game_mode::dual_relay)
            {
                return layout_kind::dual;
            }
            return layout_kind::single;
        }

        backdrop_id backdrop_for(const app& a)
        {
            if(a.flow.mode == game_mode::adventure || a.flow.mode == game_mode::boss_rush)
            {
                return backdrop_id(int(backdrop_id::world1) + (a.flow.world - 1));
            }
            return backdrop_id::world1;
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
        _view.set_world_tint(_app.flow.mode == game_mode::adventure ? _app.flow.world : 1);
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
        for(int i = 0; i < 3; ++i)
        {
            _icons[i].reset();
            _ability_cost[i].hide();
        }
        audio::play_music(_s.config().mode == game_mode::classic ? audio::music_id::classic : audio::music_id::world1);
    }

    void game_scene::_cell_center(int board, int x, int y, int& sx, int& sy) const
    {
        sx = _view.cell_screen_x(board, x) + 4;
        sy = _view.cell_screen_y(board, y) + 4;
    }

    void game_scene::_handle_events()
    {
        event e;
        const view_layout& lay = _view.layout();
        const int board_cols = _s.board_player(0).grid().cols();
        while(_s.events().pop(e))
        {
            const int b = e.board < 0 ? 0 : (e.board > 1 ? 1 : e.board);
            int sx = 0;
            int sy = 0;
            switch(e.type)
            {
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
                    for(int x = 0; x < board_cols; x += 2)
                    {
                        _cell_center(b, x, y, sx, sy);
                        _fx.particle(particle_kind(int(particle_kind::shard_white) + (x / 2) % 4), sx, sy,
                                     bn::fixed((x * 7) % 9 - 4) / 6, -bn::fixed(3) / 2 - bn::fixed(x % 3) / 4, 24,
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
                _cell_center(b, e.a, e.b, sx, sy);
                _fx.particle(particle_kind::debris, sx, sy, bn::fixed(int(sx * 13) % 7 - 3) / 4, -bn::fixed(5) / 4,
                             22, bn::fixed(1) / 8);
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
                _cell_center(b, e.a, e.b, sx, sy);
                _fx.burst(particle_kind::ember, sx, sy, 6, 2);
                _fx.particle(particle_kind::smoke, sx, sy - 4, 0, -bn::fixed(1) / 2, 24);
                break;

            case event_type::spark_arc:
            {
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
                _cell_center(b, e.a, e.b, sx, sy);
                _fx.particle(coat_kind(e.c) == coat_kind::ice ? particle_kind::crystal : particle_kind::spark, sx, sy,
                             0, -bn::fixed(1) / 2, 14);
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
                _fx.shake(shake_profile::medium);
                break;

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
            }
            _marker->set_position(x - 120, _view.cell_screen_y(b, 2) - 80 - 10);
            _marker->set_tiles(bn::sprite_items::spr_cursor.tiles_item(), 2 + ((_frame >> 3) & 1));
            _target_hint.show(120, 152, "AIM LEFT/RIGHT  A:FIRE  B:CANCEL", text_color::cyan, align::center);
            break;

        case cfg::ability_nuclear_burst:
        case cfg::ability_chain_spark:
            if(! _cursor)
            {
                _cursor = bn::sprite_items::spr_cursor.create_sprite(0, 0, 0);
            }
            _cursor->set_position(x - 120, y - 80);
            _cursor->set_tiles(bn::sprite_items::spr_cursor.tiles_item(), (_frame >> 3) & 1);
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
            }
            _arrow->set_position(_view.cell_screen_x(b, _s.board_player(b).grid().cols() / 2) - 120,
                                 _view.cell_screen_y(b, 10) - 80);
            _arrow->set_tiles(bn::sprite_items::spr_arrows.tiles_item(), _s.target_option() & 3);
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

    void game_scene::_update_hud()
    {
        const view_layout& lay = _view.layout();
        const bool dual = lay.kind == layout_kind::dual;
        const int hold_cx = lay.hold_x * 8 + 24;
        const int next_cx = lay.next_x * 8 + 24;
        _hold_label.show(hold_cx, lay.hold_y * 8 + 8, "HOLD", text_color::gray, align::center);
        _next_label.show(next_cx, lay.next_y * 8 + 8, "NEXT", text_color::gray, align::center);

        const int sx = dual ? 120 : 200;
        const int sy = dual ? 116 : 98;
        _score_title.show(sx, sy, "SCORE", text_color::gray, align::center);
        if(_s.score() != _shown_score)
        {
            _shown_score = _s.score();
            _score_value.show(sx, sy + 9, number(_s.score()), text_color::white, align::center);
        }
        if(! dual)
        {
            if(_s.level() != _shown_level)
            {
                _shown_level = _s.level();
                bn::string<16> t = "LV ";
                t.append(number(uint32_t(_s.level())));
                _level_value.show(sx, sy + 22, t, text_color::green, align::center);
            }
            if(_s.lines() != _shown_lines)
            {
                _shown_lines = _s.lines();
                bn::string<16> t = "ROWS ";
                t.append(number(uint32_t(_s.lines())));
                _lines_value.show(sx, sy + 34, t, text_color::cyan, align::center);
            }
        }
        else if(_s.lines() != _shown_lines)
        {
            _shown_lines = _s.lines();
            bn::string<16> t = "ROWS ";
            t.append(number(uint32_t(_s.lines())));
            _lines_value.show(120, sy + 20, t, text_color::cyan, align::center);
        }

        // Heat + abilities (Classic shows time and pieces instead).
        const bool heat = _s.config().heat;
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
            if(dual)
            {
                bn::string<20> t = "HEAT ";
                t.append(number(uint32_t(_s.overheat_active() ? 100 : _s.heat())));
                _heat_title.show(120, 140, t, _s.overheat_active() ? text_color::red : text_color::orange, align::center);
                _heat_name.show(120, 150, heat_label(_s.heat(), _s.overheat_active()), text_color::yellow, align::center);
            }
            else
            {
                const int mx = lay.meter_x * 8 + 4;
                _heat_title.show(mx, lay.meter_y * 8 - 9, "HEAT", text_color::orange, align::center);
                const text_color c = _s.overheat_active() ? ((_frame & 8) ? text_color::red : text_color::yellow)
                                                          : (_s.heat() >= 60 ? text_color::red : text_color::yellow);
                _heat_name.show(mx - 2, (lay.meter_y + lay.meter_tiles + 2) * 8 + 1,
                                heat_label(_s.heat(), _s.overheat_active()), c, align::center);
            }
        }

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
            const int ix = dual ? 104 : 20;
            const int iy = dual ? 96 : 66 + i * 26;
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
                t = "CD";
                t.append(number(uint32_t(_s.cooldown(ability))));
                tc = text_color::red;
            }
            else
            {
                t = number(uint32_t(_s.ability_cost(ability)));
                tc = _s.heat() >= _s.ability_cost(ability) ? text_color::green : text_color::gray;
            }
            _ability_cost[i].show(ix + 12, iy - 4, t, selected ? (ready ? text_color::yellow : tc) : tc);
            if(selected)
            {
                if(! _icon_frame)
                {
                    _icon_frame = bn::sprite_items::spr_cursor.create_sprite(0, 0, 0);
                }
                _icon_frame->set_position(ix - 120, iy - 80);
                _icon_frame->set_tiles(bn::sprite_items::spr_cursor.tiles_item(), (_frame >> 4) & 1);
            }
        }
        if(! slots)
        {
            _icon_frame.reset();
        }

        // Status line: freeze / shield / chill / surge.
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
        if(! status.empty())
        {
            _status.show(dual ? 120 : 40, dual ? 160 - 8 : 150, status, text_color::cyan, align::center);
        }
        else if(heat || dual)
        {
            _status.hide();
        }

        // Freeze tint.
        const int want_tint = _s.freeze_frames() > 0 ? 1 : 0;
        if(want_tint != _freeze_tint)
        {
            _freeze_tint = want_tint;
            bn::bg_palettes::set_fade(bn::color(12, 20, 31), want_tint ? bn::fixed(0.18) : bn::fixed(0));
        }

        // Objectives (non-endless modes).
        for(int i = 0; i < 3; ++i)
        {
            if(i >= _s.objective_count())
            {
                _objective[i].hide();
                continue;
            }
            const objective_state& o = _s.objective(i);
            bn::string<24> t;
            switch(o.def.kind)
            {
            case objective_kind::lines: t = "ROWS "; break;
            case objective_kind::bombs: t = "BOMBS "; break;
            case objective_kind::chains: t = "CHAINS "; break;
            case objective_kind::magnets: t = "MAGNETS "; break;
            case objective_kind::overheat_success: t = "OVERHEAT "; break;
            case objective_kind::cross_combos: t = "CROSS "; break;
            case objective_kind::survive_pieces: t = "PIECES "; break;
            case objective_kind::sparks: t = "SPARKS "; break;
            case objective_kind::fires: t = "FIRES "; break;
            case objective_kind::ice_broken: t = "ICE "; break;
            case objective_kind::quads: t = "QUADS "; break;
            case objective_kind::twists: t = "TWISTS "; break;
            default: t = "GOAL "; break;
            }
            t.append(number(o.value));
            t.append("/");
            t.append(number(o.def.target));
            _objective[i].show(dual ? 120 : 32, dual ? 60 + i * 9 : 120 - 9 * (_s.objective_count() - 1 - i) - 0 + 30,
                               t, o.done ? text_color::green : text_color::white, align::center);
        }

        _update_targeting_sprites();
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

        if(! _fx.hitstop_frames())
        {
            _s.update(in);
        }
        _handle_events();
        _view.update(_s, _app.save.settings.ghost, _frame);
        _backdrop.update();
        _fx.update();
        _update_hud();

        if(_s.state() == session_state::intro)
        {
            const int t = _s.intro_timer();
            _center_text.show_big(_view.cell_screen_x(0, _s.board_player(0).grid().cols() / 2) + (_view.layout().kind == layout_kind::dual ? 40 : 0),
                                  64, t > 40 ? "READY" : "GO!", t > 40 ? big_color::gold : big_color::toxic,
                                  align::center);
        }
        else if(_s.state() == session_state::playing)
        {
            _center_text.hide();
        }

        if(_s.state() == session_state::won || _s.state() == session_state::lost)
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
