#include "bm_board_view.h"

#include "bm_profile.h"
#include "bm_tile_ids.h"

#include "bn_bg_tiles.h"
#include "bn_color.h"
#include "bn_memory.h"
#include "bn_regular_bg_item.h"

#include "bn_bg_palette_items_board_palette.h"
#include "bn_regular_bg_tiles_items_board_tiles.h"

namespace bm
{
    namespace
    {

        view_layout make_layout(layout_kind kind)
        {
            view_layout l;
            l.kind = kind;
            switch(kind)
            {
            case layout_kind::boss:
                l.boards[0] = { 2, 1 };
                l.next_x = 13; l.next_y = 1; l.next_count = 2;
                l.hold_x = 13; l.hold_y = 9;
                l.meter_x = -1; l.meter_y = 0; l.meter_tiles = 0;
                l.heat_bar_x = 13; l.heat_bar_y = 14; l.heat_bar_tiles = 4;
                l.hp_x = 19; l.hp_y = 1; l.hp_tiles = 9;
                l.boss_x = 196; l.boss_y = 88;
                break;

            case layout_kind::dual:
                // Centre column (map x 11..18): NEXT, HOLD, score, ability, heat bar, goals.
                l.boards[0] = { 2, 3 };
                l.boards[1] = { 20, 3 };
                l.next_x = 12; l.next_y = 1; l.next_count = 1;
                l.hold_x = 12; l.hold_y = 6;
                l.meter_x = -1; l.meter_y = 0; l.meter_tiles = 0;
                l.heat_bar_x = 12; l.heat_bar_y = 14; l.heat_bar_tiles = 4;
                break;

            case layout_kind::dual_boss:
                // Boss on top of the centre column, heat bar under it, then NEXT and HOLD.
                l.boards[0] = { 2, 3 };
                l.boards[1] = { 20, 3 };
                l.next_x = 12; l.next_y = 10; l.next_count = 1;
                l.hold_x = 12; l.hold_y = 15;
                l.meter_x = -1; l.meter_y = 0; l.meter_tiles = 0;
                l.heat_bar_x = 12; l.heat_bar_y = 9; l.heat_bar_tiles = 4;
                l.hp_x = 11; l.hp_y = 0; l.hp_tiles = 6;
                l.boss_x = 120; l.boss_y = 40;
                break;

            case layout_kind::single:
            default:
                l.boards[0] = { 10, 1 };
                l.hold_x = 1; l.hold_y = 1;
                l.next_x = 22; l.next_y = 1; l.next_count = 3;
                l.meter_x = 7; l.meter_y = 7; l.meter_tiles = 9;
                break;
            }
            return l;
        }

        // Frame / well colours per world (metal, accent, well, panel).
        struct world_tint
        {
            bn::color metal;
            bn::color accent;
            bn::color well;
            bn::color panel;
        };

        constexpr world_tint tints[7] = {
            { bn::color(14, 15, 18), bn::color(31, 25, 5), bn::color(2, 2, 4), bn::color(3, 3, 7) },     // default
            { bn::color(14, 15, 18), bn::color(31, 25, 5), bn::color(2, 2, 4), bn::color(3, 3, 7) },     // W1 steel + yellow
            { bn::color(17, 22, 27), bn::color(20, 29, 31), bn::color(2, 3, 6), bn::color(4, 6, 10) },   // W2 ice
            { bn::color(16, 10, 8), bn::color(31, 17, 4), bn::color(4, 1, 1), bn::color(7, 3, 3) },      // W3 volcano
            { bn::color(12, 8, 18), bn::color(8, 29, 31), bn::color(2, 1, 5), bn::color(5, 2, 9) },      // W4 neon
            { bn::color(10, 16, 14), bn::color(26, 12, 31), bn::color(1, 3, 3), bn::color(3, 5, 6) },    // W5 cosmic
            { bn::color(16, 12, 16), bn::color(31, 8, 12), bn::color(3, 1, 3), bn::color(6, 2, 6) },     // W6 meltdown
        };

        // Block themes (PART 35.4): base colour of the seven shapes in 8-bit RGB, shaded like the generated
        // palette (tools/gfx/tiles.py shade_bank). Theme 0 keeps the generated colours. Every theme keeps the
        // seven shapes apart in hue and brightness; special symbols use their own fixed colours.
        struct rgb8
        {
            uint8_t r, g, b;
        };

        constexpr rgb8 block_themes[5][7] = {
            { { 40, 200, 232 }, { 248, 208, 40 }, { 176, 72, 232 }, { 72, 216, 80 }, { 240, 64, 72 }, { 56, 104, 240 },
              { 248, 144, 32 } },                                                                   // CLASSIC
            { { 86, 180, 233 }, { 240, 228, 66 }, { 204, 121, 167 }, { 0, 158, 115 }, { 213, 94, 0 }, { 0, 114, 178 },
              { 230, 159, 0 } },                                                                    // CONTRAST
            { { 0, 255, 240 }, { 255, 255, 64 }, { 255, 64, 255 }, { 64, 255, 64 }, { 255, 40, 96 }, { 64, 96, 255 },
              { 255, 150, 0 } },                                                                    // NEON
            { { 150, 225, 245 }, { 250, 235, 150 }, { 210, 170, 245 }, { 165, 235, 175 }, { 250, 165, 175 },
              { 165, 185, 250 }, { 250, 200, 145 } },                                               // PASTEL
            { { 255, 200, 90 }, { 230, 215, 60 }, { 130, 70, 170 }, { 90, 180, 60 }, { 225, 40, 30 }, { 150, 85, 50 },
              { 255, 120, 40 } },                                                                   // CHILI
        };

        bn::color to_color(int r, int g, int b)
        {
            return bn::color(r >> 3, g >> 3, b >> 3);
        }

        rgb8 mix8(rgb8 a, int tr, int tg, int tb, int k100)
        {
            return { uint8_t(a.r + (tr - a.r) * k100 / 100), uint8_t(a.g + (tg - a.g) * k100 / 100),
                     uint8_t(a.b + (tb - a.b) * k100 / 100) };
        }

        // Board frame themes (metal, accent); theme 0 uses the world's own colours.
        constexpr int frame_theme_count = 5;
        const bn::color frame_metal[frame_theme_count] = {
            bn::color(0, 0, 0), bn::color(24, 25, 27), bn::color(28, 14, 22), bn::color(6, 6, 7), bn::color(27, 21, 6)
        };
        const bn::color frame_accent[frame_theme_count] = {
            bn::color(0, 0, 0), bn::color(10, 22, 31), bn::color(12, 28, 20), bn::color(31, 10, 4), bn::color(31, 31, 20)
        };

        bn::color mix(bn::color a, bn::color b, int k, int total)
        {
            return bn::color(a.red() + (b.red() - a.red()) * k / total, a.green() + (b.green() - a.green()) * k / total,
                             a.blue() + (b.blue() - a.blue()) * k / total);
        }

        constexpr bn::color hot_core[8] = {
            bn::color(29, 6, 4), bn::color(31, 9, 4), bn::color(31, 14, 4), bn::color(31, 20, 6),
            bn::color(31, 26, 10), bn::color(31, 20, 6), bn::color(31, 14, 4), bn::color(31, 9, 4)
        };

        constexpr bn::color electric[4] = {
            bn::color(12, 27, 31), bn::color(20, 30, 31), bn::color(31, 31, 31), bn::color(8, 20, 31)
        };

        bn::regular_bg_ptr create_board_bg(const bn::regular_bg_map_item& map_item)
        {
            // Dynamic maps address tiles absolutely, so tile offsets must be disabled while creating.
            bn::bg_tiles::set_allow_offset(false);
            bn::regular_bg_ptr bg = bn::regular_bg_item(bn::regular_bg_tiles_items::board_tiles,
                                                        bn::bg_palette_items::board_palette, map_item).create_bg(8, 48);
            bn::bg_tiles::set_allow_offset(true);
            return bg;
        }

        constexpr bn::color rainbow[12] = {
            bn::color(31, 8, 8), bn::color(31, 18, 4), bn::color(31, 28, 4), bn::color(18, 31, 6),
            bn::color(6, 31, 10), bn::color(4, 31, 24), bn::color(6, 24, 31), bn::color(8, 12, 31),
            bn::color(18, 8, 31), bn::color(28, 8, 31), bn::color(31, 8, 24), bn::color(31, 8, 14)
        };
    }

    board_view::board_view(layout_kind kind, int bg_priority) :
        _layout(make_layout(kind)),
        _map(new map_data()),
        _map_item(_map->cells[0], bn::size(32, 32)),
        _bg(create_board_bg(_map_item)),
        _map_ptr(_bg.map()),
        _palette(_bg.palette())
    {
        const bn::span<const bn::color> base = bn::bg_palette_items::board_palette.colors_ref();
        for(int i = 0; i < _colors.size(); ++i)
        {
            _colors[i] = i < base.size() ? base[i] : bn::color();
        }
        _bg.set_priority(bg_priority);
        bn::memory::clear(_map->cells);
        _draw_static(1);
        _map_ptr.reload_cells_ref();
    }

    int board_view::cell_screen_y(int board, int y) const
    {
        return (_layout.boards[board].map_y + y - cfg::board_hidden_rows) * 8;
    }

    void board_view::_draw_panel(int x, int y, int w, int h)
    {
        for(int yy = 0; yy < h; ++yy)
        {
            for(int xx = 0; xx < w; ++xx)
            {
                int t = tile::panel;
                const bool top = yy == 0;
                const bool bottom = yy == h - 1;
                const bool left = xx == 0;
                const bool right = xx == w - 1;
                if(top && left) t = tile::panel_tl;
                else if(top && right) t = tile::panel_tr;
                else if(bottom && left) t = tile::panel_bl;
                else if(bottom && right) t = tile::panel_br;
                else if(top) t = tile::panel_t;
                else if(bottom) t = tile::panel_b;
                else if(left) t = tile::panel_l;
                else if(right) t = tile::panel_r;
                _put(x + xx, y + yy, t, bank_frame);
            }
        }
    }

    void board_view::_draw_frame(int board, int cols, int rows)
    {
        const board_slot& s = _layout.boards[board];
        const int visible = rows - cfg::board_hidden_rows;
        for(int r = -1; r < visible; ++r)
        {
            _put(s.map_x - 1, s.map_y + r, tile::frame_l, bank_frame);
            _put(s.map_x + cols, s.map_y + r, tile::frame_r, bank_frame);
        }
        _put(s.map_x - 1, s.map_y + visible, tile::frame_bl, bank_frame);
        _put(s.map_x + cols, s.map_y + visible, tile::frame_br, bank_frame);
        for(int c = 0; c < cols; ++c)
        {
            _put(s.map_x + c, s.map_y + visible, tile::frame_b, bank_frame);
        }
    }

    void board_view::_draw_static(int world)
    {
        bn::memory::clear(_map->cells);
        _drawn_revision[0] = _drawn_revision[1] = 0xFFFFFFFFu;
        _overlay_count[0] = _overlay_count[1] = 0;
        const bool dual = _layout.kind == layout_kind::dual || _layout.kind == layout_kind::dual_boss;
        const int cols = dual ? cfg::dual_board_cols : cfg::board_cols;
        const int rows = dual ? cfg::dual_board_rows : cfg::board_rows;
        for(int b = 0; b < (dual ? 2 : 1); ++b)
        {
            if(b == 1 && ! _board_b_visible)
            {
                continue;
            }
            _draw_frame(b, cols, rows);
        }
        _draw_panel(_layout.hold_x, _layout.hold_y, 6, 5);
        _draw_panel(_layout.next_x, _layout.next_y, 6, 2 + _layout.next_count * 3);
        _draw_bridge(0, 0);
        set_world_tint(world);
    }

    void board_view::start_trail(int board, int columns, int highest_row, int distance)
    {
        trail_state& t = _trail[board & 1];
        t.columns = uint16_t(columns);
        t.to = int8_t(highest_row);
        t.from = int8_t(highest_row - distance);
        t.timer = 10;
    }

    void board_view::pulse_bridge(int from_board)
    {
        _bridge_hot = bridge_pulse_frames;
        _bridge_dir = from_board == 0 ? 1 : -1;
        _frame_pulse = bridge_pulse_frames;
    }

    void board_view::_draw_bridge(int frame, int active_board)
    {
        // Only the relay layout has the bridge: a conduit between the boards' bottom frames.
        if(_layout.kind != layout_kind::dual || ! _board_b_visible)
        {
            return;
        }
        const int row = _layout.boards[0].map_y + cfg::dual_board_rows - cfg::board_hidden_rows;
        const int left = _layout.boards[0].map_x + cfg::dual_board_cols + 1;
        const int span = _layout.boards[1].map_x - 2 - left + 1;

        // At rest the energy streams towards the board that is waiting for its turn.
        const int flow = active_board == 0 ? 1 : -1;
        const int phase = (flow > 0 ? (frame >> 2) : -(frame >> 2)) & 3;

        // CROSS OVER: three energy packets run across, two frames per segment, a glow behind each one.
        int packet = -1;
        if(_bridge_hot > 0)
        {
            const int step = ((bridge_pulse_frames - _bridge_hot) % 16) / 2;
            packet = _bridge_dir > 0 ? step : span - 1 - step;
            ++profile::bridge_draws;
        }
        for(int i = 0; i < span; ++i)
        {
            int t;
            if(i == packet)
            {
                t = tile::bridge_hot;
            }
            else if(packet >= 0 && i == packet - _bridge_dir)
            {
                t = tile::bridge_glow;
            }
            else if(i == 0)
            {
                t = tile::bridge_l;
            }
            else if(i == span - 1)
            {
                t = tile::bridge_r;
            }
            else
            {
                t = tile::bridge_0 + phase;
            }
            _put(left + i, row, t, bank_steel);
        }
    }

    void board_view::set_board_b_visible(bool visible)
    {
        if(visible == _board_b_visible)
        {
            return;
        }
        _board_b_visible = visible;
        _draw_static(_world);
        _map_ptr.reload_cells_ref();
    }

    void board_view::_draw_telegraphs(const session& s, int frame)
    {
        for(int b = 0; b < s.board_count(); ++b)
        {
            if(b == 1 && ! _board_b_visible)
            {
                continue;
            }
            const board& g = s.board_player(b).grid();
            const board_slot& slot = _layout.boards[b];
            const int visible = g.rows() - g.hidden();

            // Restore the left frame column and the bottom frame row, then overlay warnings.
            for(int r = 0; r < visible; ++r)
            {
                _put(slot.map_x - 1, slot.map_y + r, tile::frame_l, bank_frame);
            }
            for(int c = 0; c < g.cols(); ++c)
            {
                _put(slot.map_x + c, slot.map_y + visible, tile::frame_b, bank_frame);
            }

            int rows = s.incoming_rows(b);
            int gap = rows > 0 ? s.garbage(b, 0).gap : -1;
            bool imminent = rows > 0;
            const pending_action& ba = s.boss_action();
            if(ba.source && ba.board == b)
            {
                const attack_kind k = attack_kind(ba.kind);
                if(k == attack_kind::garbage || k == attack_kind::punch || k == attack_kind::cross_garbage)
                {
                    rows += ba.strength;
                    gap = ba.gap;
                    imminent = imminent || ba.countdown <= 1;
                }
            }
            const pending_action& ha = s.hazard_action();
            if(ha.source && ha.board == b)
            {
                const hazard_kind k = hazard_kind(ha.kind);
                if(k == hazard_kind::garbage_drip || k == hazard_kind::lava_pressure)
                {
                    rows += ha.strength;
                    gap = ha.gap;
                    imminent = imminent || ha.countdown <= 1;
                }
            }
            if(b == s.active_board() && s.overheat_active() && ! s.overheat_secured() &&
               s.overheat_timer() < cfg::overheat_garbage_warning_frames)
            {
                rows += s.overheat_fail_rows();
                imminent = true;
            }
            if(rows > visible)
            {
                rows = visible;
            }
            const bool lit = ! imminent || (frame & 8);
            for(int r = 0; r < rows && lit; ++r)
            {
                _put(slot.map_x - 1, slot.map_y + visible - 1 - r, tile::warn_row, bank_frame);
            }
            if(gap >= 0 && gap < g.cols() && rows > 0)
            {
                _put(slot.map_x + gap, slot.map_y + visible, tile::gap_mark, bank_frame);
            }

            // Column targets (crystal shards, molten rain, fire breath, frozen wall) shown in the chute row.
            const pending_action* actions[2] = { &ba, &ha };
            for(const pending_action* a : actions)
            {
                if(! a->source || a->board != b)
                {
                    continue;
                }
                for(int8_t col : a->columns)
                {
                    if(col >= 0 && col < g.cols() && ((frame >> 3) & 1))
                    {
                        _put(slot.map_x + col, slot.map_y - 1, tile::target, bank_frame);
                    }
                }
            }
        }
    }

    void board_view::_draw_hp(const session& s)
    {
        if(_layout.hp_x < 0 || ! s.has_boss())
        {
            return;
        }
        const int x = _layout.hp_x;
        const int y = _layout.hp_y;
        const int n = _layout.hp_tiles;
        _put(x, y, tile::hp_left, bank_meter);
        _put(x + n + 1, y, tile::hp_right, bank_meter);
        const int total = n * 8;
        const int fill = s.boss_max_hp() ? s.boss_hp() * total / s.boss_max_hp() : 0;
        for(int k = 0; k < n; ++k)
        {
            int level = fill - k * 8;
            level = level < 0 ? 0 : (level > 8 ? 8 : level);
            _put(x + 1 + k, y, tile::hp_0 + level, bank_meter);
        }
    }

    void board_view::_set_color(int index, bn::color color)
    {
        if(index >= 0 && index < _colors.size() && _colors[index] != color)
        {
            _colors[index] = color;
            _colors_dirty = true;
        }
    }

    void board_view::_commit_colors()
    {
        if(_colors_dirty)
        {
            _colors_dirty = false;
            _palette.set_colors(_colors);
        }
    }

    void board_view::set_world_tint(int world)
    {
        _world = world < 0 || world > 6 ? 0 : world;
        const world_tint& t = tints[_world];
        const bool themed = _frame_theme > 0 && _frame_theme < frame_theme_count;
        _metal = themed ? frame_metal[_frame_theme] : t.metal;
        _accent = themed ? frame_accent[_frame_theme] : t.accent;
        const bn::color black(1, 1, 3);
        const bn::color white(31, 31, 31);
        _set_color(2, mix(_metal, black, 11, 20));
        _set_color(3, mix(_metal, black, 5, 20));
        _set_color(4, _metal);
        _set_color(5, mix(_metal, white, 9, 20));
        _set_color(6, t.well);
        _set_color(7, mix(t.well, white, 3, 20));
        _set_color(8, t.panel);
        _set_color(9, mix(t.panel, white, 7, 20));
        _set_color(10, _accent);
        _set_color(11, mix(_accent, bn::color(31, 8, 0), 10, 20));
        _commit_colors();
    }

    void board_view::set_cosmetics(int block_theme, int frame_theme)
    {
        _block_theme = block_theme >= 0 && block_theme < 5 ? block_theme : 0;
        _frame_theme = frame_theme >= 0 && frame_theme < frame_theme_count ? frame_theme : 0;
        if(_block_theme)
        {
            for(int shape = 0; shape < 7; ++shape)
            {
                const rgb8 base = block_themes[_block_theme][shape];
                const rgb8 outline = mix8(base, 8, 8, 24, 80);
                const rgb8 shadow = mix8(base, 16, 16, 40, 45);
                const rgb8 light = mix8(base, 255, 255, 255, 35);
                const rgb8 high = mix8(base, 255, 255, 255, 70);
                const int bank = (shape + 1) * 16;
                _set_color(bank + 1, to_color(outline.r, outline.g, outline.b));
                _set_color(bank + 2, to_color(shadow.r, shadow.g, shadow.b));
                _set_color(bank + 3, to_color(base.r, base.g, base.b));
                _set_color(bank + 4, to_color(light.r, light.g, light.b));
                _set_color(bank + 5, to_color(high.r, high.g, high.b));
            }
        }
        set_world_tint(_world);
    }

    void board_view::set_challenge_view(bool shadow_stack, bool one_next)
    {
        _shadow_stack = shadow_stack;
        _one_next = one_next;
        _drawn_revision[0] = _drawn_revision[1] = 0xFFFFFFFFu;     // repaint the stacks
    }

    void board_view::flash_rows(int board, uint32_t rows, int frames)
    {
        clear_flash& f = _flash[board & 1];
        f.rows = rows;
        f.board = int8_t(board);
        f.timer = int8_t(frames);
        f.length = int8_t(frames);
    }

    void board_view::_draw_preview(int mx, int my, const queued_piece& piece, bool dim)
    {
        for(int yy = 0; yy < 2; ++yy)
        {
            for(int xx = 0; xx < 4; ++xx)
            {
                _put(mx + xx, my + yy, tile::panel, bank_frame);
            }
        }
        if(piece.type == piece_type::none)
        {
            return;
        }
        const piece_shape& shape = shape_of(piece.type, 0);
        const int oy = piece.type == piece_type::i ? -1 : 0;
        for(int m = 0; m < 4; ++m)
        {
            const point p = shape.minos[m];
            cell c;
            c.color = uint8_t(piece.type);
            if(m == piece.special_mino)
            {
                c.special = piece.special;
            }
            int bank = 0;
            int t = _cell_tile(c, bank);
            if(dim)
            {
                t = tile::ghost;
            }
            _put(mx + p.x, my + p.y + oy, t, bank);
        }
    }

    void board_view::_draw_meter(const session& s, int frame)
    {
        const int x = _layout.meter_x;
        const int top = _layout.meter_y;
        const int n = _layout.meter_tiles;
        if(! s.config().heat)
        {
            return;     // Classic has no heat at all
        }
        if(_layout.heat_bar_x >= 0)
        {
            // Horizontal bar (boss and dual layouts).
            const int hx = _layout.heat_bar_x;
            const int hy = _layout.heat_bar_y;
            const int hn = _layout.heat_bar_tiles;
            _put(hx, hy, tile::heath_left, bank_meter);
            _put(hx + hn + 1, hy, tile::heath_right, bank_meter);
            const int htotal = hn * 8;
            int hfill = s.overheat_active() ? s.overheat_timer() * htotal / cfg::overheat_frames
                                            : s.heat() * htotal / cfg::heat_max;
            if(s.overheat_active() && s.overheat_timer() < cfg::overheat_garbage_warning_frames &&
               ! s.overheat_secured() && (frame & 8))
            {
                hfill = htotal;
            }
            for(int k = 0; k < hn; ++k)
            {
                int level = hfill - k * 8;
                level = level < 0 ? 0 : (level > 8 ? 8 : level);
                _put(hx + 1 + k, hy, tile::heath_0 + level, bank_meter);
            }
            return;
        }
        if(x < 0)
        {
            return;
        }
        _put(x, top, tile::meter_top, bank_meter);
        _put(x, top + n + 1, tile::meter_bottom, bank_meter);
        const int total = n * 8;
        int fill;
        if(s.overheat_active())
        {
            fill = s.overheat_timer() * total / cfg::overheat_frames;
            if(s.overheat_timer() < cfg::overheat_garbage_warning_frames && ! s.overheat_secured() && (frame & 8))
            {
                fill = total;
            }
        }
        else
        {
            fill = s.heat() * total / cfg::heat_max;
        }
        for(int k = 0; k < n; ++k)
        {
            int level = fill - k * 8;
            level = level < 0 ? 0 : (level > 8 ? 8 : level);
            _put(x, top + n - k, tile::meter_0 + level, bank_meter);
        }
    }

    void board_view::_animate_palette(const session& s, int frame)
    {
        // Special-cell cores pulse, sparks crackle, rainbows cycle: every value changes on a fixed beat,
        // and all edits of a frame reach the hardware in one palette upload.
        {
            const bn::color core = hot_core[(frame >> 2) & 7];
            const bn::color bright = (frame & 8) ? bn::color(31, 31, 31) : bn::color(31, 30, 15);
            const uint32_t h = uint32_t(frame >> 2) * 2654435761u;
            const bn::color elec = electric[(h >> 28) & 3];
            const bn::color rb = rainbow[(frame / 6) % 12];
            for(int b = 1; b <= 10; ++b)
            {
                _set_color(b * 16 + 7, core);
                _set_color(b * 16 + 8, bright);
                _set_color(b * 16 + 10, elec);
                _set_color(b * 16 + 13, rb);
            }
        }

        // Heat meter colour follows the SPICE intensity scale.
        int heat_color;
        if(s.overheat_active())
        {
            heat_color = 6 + ((frame >> 2) & 1);
        }
        else
        {
            heat_color = s.heat() / 20;     // 0 MILD .. 5 at 100
        }
        if(heat_color != _last_heat_color)
        {
            _last_heat_color = heat_color;
            static constexpr bn::color fills[8] = {
                bn::color(16, 28, 10), bn::color(28, 28, 6), bn::color(31, 20, 4), bn::color(31, 12, 3),
                bn::color(31, 5, 3), bn::color(31, 5, 3), bn::color(31, 31, 31), bn::color(31, 10, 4)
            };
            _set_color(bank_meter * 16 + 5, fills[heat_color]);
            _set_color(bank_meter * 16 + 6, mix(fills[heat_color], bn::color(0, 0, 0), 7, 20));
        }

        // Danger: the frame pulses red.
        bool danger = false;
        for(int b = 0; b < s.board_count(); ++b)
        {
            danger = danger || s.danger(b);
        }
        if(_frame_pulse > 0)
        {
            // CROSS OVER: both board frames glow white-violet.
            const int k = 8 + ((_frame_pulse >> 1) & 7);
            _set_color(4, mix(_metal, bn::color(28, 20, 31), k, 20));
            _set_color(5, mix(_metal, bn::color(31, 31, 31), k, 20));
        }
        else if(danger)
        {
            const int k = 6 + ((frame >> 2) & 7);
            _set_color(4, mix(_metal, bn::color(31, 4, 4), k, 20));
            _set_color(5, mix(mix(_metal, bn::color(31, 31, 31), 9, 20), bn::color(31, 10, 8), k, 20));
        }
        else
        {
            _set_color(4, _metal);
            _set_color(5, mix(_metal, bn::color(31, 31, 31), 9, 20));
        }
        _commit_colors();
    }

    void board_view::update(const session& s, bool ghost_enabled, int frame)
    {
        bn::timer t;
        for(int b = 0; b < s.board_count(); ++b)
        {
            if(b == 1 && ! _board_b_visible)
            {
                continue;
            }
            _draw_board(s, b, ghost_enabled, frame);
            if(_flash[b].timer > 0)
            {
                --_flash[b].timer;
            }
        }
        profile::slots[0] = profile::per_mille(t.elapsed_ticks_with_restart());
        _draw_telegraphs(s, frame);
        _draw_hp(s);
        profile::slots[1] = profile::per_mille(t.elapsed_ticks_with_restart());

        // Hold (the active board's hold slot) and next previews.
        const player& p = s.board_player(s.active_board());
        _draw_preview(_layout.hold_x + 1, _layout.hold_y + 2, p.held_piece(), ! p.can_hold());
        for(int i = 0; i < _layout.next_count; ++i)
        {
            const bool shown = ! _one_next || i == 0;
            _draw_preview(_layout.next_x + 1, _layout.next_y + 2 + i * 3, shown ? s.queue().peek(i) : queued_piece(),
                          false);
        }
        _draw_meter(s, frame);
        if(_bridge_hot > 0 || (frame & 3) == 0)
        {
            _draw_bridge(frame, s.active_board());
            _bridge_hot -= _bridge_hot > 0 ? 1 : 0;
        }
        if(_frame_pulse > 0)
        {
            --_frame_pulse;
        }
        profile::slots[2] = profile::per_mille(t.elapsed_ticks_with_restart());
        _animate_palette(s, frame);
        profile::slots[3] = profile::per_mille(t.elapsed_ticks_with_restart());
        _map_ptr.reload_cells_ref();
        profile::slots[4] = profile::per_mille(t.elapsed_ticks_with_restart());
    }
}
