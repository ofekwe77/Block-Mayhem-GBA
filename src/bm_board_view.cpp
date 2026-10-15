#include "bm_board_view.h"

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
        constexpr int bank_frame = 0;
        constexpr int bank_garbage = 8;
        constexpr int bank_stone = 9;
        constexpr int bank_steel = 10;
        constexpr int bank_meter = 11;

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
                l.meter_x = 18; l.meter_y = 9; l.meter_tiles = 8;
                break;

            case layout_kind::dual:
                l.boards[0] = { 2, 3 };
                l.boards[1] = { 20, 3 };
                l.next_x = 12; l.next_y = 1; l.next_count = 2;
                l.hold_x = 12; l.hold_y = 9;
                l.meter_x = 14; l.meter_y = 15; l.meter_tiles = 2;
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
        _bg.set_priority(bg_priority);
        bn::memory::clear(_map->cells);
        _draw_static(1);
        _map_ptr.reload_cells_ref();
    }

    int board_view::cell_screen_y(int board, int y) const
    {
        return (_layout.boards[board].map_y + y - cfg::board_hidden_rows) * 8;
    }

    void board_view::_put(int mx, int my, int tile, int bank, bool hflip, bool vflip)
    {
        if(mx < 0 || mx >= 32 || my < 0 || my >= 32)
        {
            return;
        }
        _map->cells[my * 32 + mx] = bn::regular_bg_map_cell(tile | (hflip ? 1 << 10 : 0) | (vflip ? 1 << 11 : 0) |
                                                            (bank << 12));
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
        const bool dual = _layout.kind == layout_kind::dual;
        const int cols = dual ? cfg::dual_board_cols : cfg::board_cols;
        const int rows = dual ? cfg::dual_board_rows : cfg::board_rows;
        for(int b = 0; b < (dual ? 2 : 1); ++b)
        {
            _draw_frame(b, cols, rows);
        }
        _draw_panel(_layout.hold_x, _layout.hold_y, 6, 5);
        _draw_panel(_layout.next_x, _layout.next_y, 6, 2 + _layout.next_count * 3);
        set_world_tint(world);
    }

    void board_view::set_world_tint(int world)
    {
        _world = world < 0 || world > 6 ? 0 : world;
        const world_tint& t = tints[_world];
        const bn::color black(1, 1, 3);
        const bn::color white(31, 31, 31);
        _palette.set_color(2, mix(t.metal, black, 11, 20));
        _palette.set_color(3, mix(t.metal, black, 5, 20));
        _palette.set_color(4, t.metal);
        _palette.set_color(5, mix(t.metal, white, 9, 20));
        _palette.set_color(6, t.well);
        _palette.set_color(7, mix(t.well, white, 3, 20));
        _palette.set_color(8, t.panel);
        _palette.set_color(9, mix(t.panel, white, 7, 20));
        _palette.set_color(10, t.accent);
        _palette.set_color(11, mix(t.accent, bn::color(31, 8, 0), 10, 20));
    }

    void board_view::flash_rows(int board, uint32_t rows, int frames)
    {
        clear_flash& f = _flash[board & 1];
        f.rows = rows;
        f.board = int8_t(board);
        f.timer = int8_t(frames);
        f.length = int8_t(frames);
    }

    int board_view::_cell_tile(const cell& c, int& bank)
    {
        bank = c.color;
        if(c.color == color_garbage)
        {
            bank = bank_garbage;
        }
        else if(c.color == color_stone)
        {
            bank = bank_stone;
            return tile::stone;
        }
        if(c.coat == coat_kind::armor && c.coat_hp)
        {
            bank = bank_steel;
            return c.coat_hp >= 2 ? tile::armor2 : tile::armor1;
        }
        const bool iced = c.coat == coat_kind::ice && c.coat_hp;
        const bool off = c.flags & flag_disabled;
        switch(c.special)
        {
        case special_kind::bomb: return iced ? tile::ice_bomb : (off ? tile::off_bomb : tile::bomb);
        case special_kind::fire: return iced ? tile::ice_fire : (off ? tile::off_fire : tile::fire);
        case special_kind::spark: return iced ? tile::ice_spark : (off ? tile::off_spark : tile::spark);
        case special_kind::magnet: return iced ? tile::ice_magnet : (off ? tile::off_magnet : tile::magnet);
        case special_kind::rainbow: return iced ? tile::ice_rainbow : (off ? tile::off_rainbow : tile::rainbow);
        case special_kind::none:
        default:
            break;
        }
        if(iced)
        {
            return tile::ice;
        }
        if(c.flags & flag_charged)
        {
            return tile::charged;
        }
        if(c.flags & flag_frost_mark)
        {
            return tile::frost_mark;
        }
        if(c.flags & flag_armor_mark)
        {
            return tile::armor_mark;
        }
        return c.color == color_garbage ? tile::garbage : tile::block;
    }

    void board_view::_draw_board(const session& s, int b, bool ghost_enabled, int frame)
    {
        const player& p = s.board_player(b);
        const board& g = p.grid();
        const board_slot& slot = _layout.boards[b];
        const int hidden = g.hidden();
        clear_flash& fl = _flash[b];
        const bool flashing = fl.timer > 0;
        const int phase = flashing ? (fl.length - fl.timer) * 3 / (fl.length ? fl.length : 1) : 0;

        for(int y = hidden - 1; y < g.rows(); ++y)
        {
            const int my = slot.map_y + y - hidden;
            for(int x = 0; x < g.cols(); ++x)
            {
                const int mx = slot.map_x + x;
                if(flashing && ((fl.rows >> y) & 1) && y >= hidden)
                {
                    const int t = phase == 0 ? tile::flash1 : (phase == 1 ? tile::flash2 : tile::flash3);
                    _put(mx, my, t, bank_frame);
                    continue;
                }
                const cell& c = g.at(x, y);
                if(c.empty())
                {
                    _put(mx, my, y < hidden ? tile::chute : tile::well, bank_frame);
                }
                else
                {
                    int bank = 0;
                    const int t = _cell_tile(c, bank);
                    _put(mx, my, t, bank);
                }
            }
        }

        if(p.state() != player_state::falling)
        {
            return;
        }
        piece_state shifted;
        const bool active = b == s.active_board();
        const bool shape_preview = active && s.targeting() && s.targeting_ability() == cfg::ability_shape_shift &&
                                   s.shape_option_preview(s.target_option(), shifted);
        const piece_state& piece = shape_preview ? shifted : p.active();
        if(ghost_enabled && active)
        {
            piece_state ghost = piece;
            ghost.y = int8_t(piece.y + g.drop_distance(piece));
            if(ghost.y != piece.y)
            {
                for(int m = 0; m < 4; ++m)
                {
                    const point c = ghost.cell(m);
                    if(c.y >= hidden)
                    {
                        _put(slot.map_x + c.x, slot.map_y + c.y - hidden, tile::ghost, int(piece.type));
                    }
                }
            }
        }
        for(int m = 0; m < 4; ++m)
        {
            const point c = piece.cell(m);
            if(c.y < hidden - 1)
            {
                continue;
            }
            cell pc;
            pc.color = uint8_t(piece.type);
            if(m == piece.special_mino)
            {
                pc.special = piece.special;
            }
            int bank = 0;
            int t = _cell_tile(pc, bank);
            // The paused piece of the inactive board is drawn as a hollow outline.
            if(! active)
            {
                t = tile::ghost;
            }
            // Lock-delay pulse: the piece glints while it is about to lock.
            if(active && p.grounded() && p.lock_timer() > cfg::lock_delay_frames - 8 && (frame & 2))
            {
                t = tile::flash2;
            }
            _put(slot.map_x + c.x, slot.map_y + c.y - hidden, t, bank);
        }
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
        if(_layout.kind == layout_kind::dual || ! s.config().heat)
        {
            return;     // dual layout shows heat with sprites; Classic has no heat at all
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
        if(frame & 1)
        {
            const bn::color core = hot_core[(frame >> 2) & 7];
            const bn::color bright = (frame & 8) ? bn::color(31, 31, 31) : bn::color(31, 30, 15);
            uint32_t h = uint32_t(frame >> 1) * 2654435761u;
            const bn::color elec = electric[(h >> 28) & 3];
            const bn::color rb = rainbow[(frame / 6) % 12];
            for(int b = 1; b <= 10; ++b)
            {
                _palette.set_color(b * 16 + 7, core);
                _palette.set_color(b * 16 + 8, bright);
                _palette.set_color(b * 16 + 10, elec);
                _palette.set_color(b * 16 + 13, rb);
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
            _palette.set_color(bank_meter * 16 + 5, fills[heat_color]);
            _palette.set_color(bank_meter * 16 + 6, mix(fills[heat_color], bn::color(0, 0, 0), 7, 20));
        }

        // Danger: the frame pulses red.
        bool danger = false;
        for(int b = 0; b < s.board_count(); ++b)
        {
            danger = danger || s.danger(b);
        }
        const world_tint& t = tints[_world];
        if(danger)
        {
            const int k = 6 + ((frame >> 2) & 7);
            _palette.set_color(4, mix(t.metal, bn::color(31, 4, 4), k, 20));
            _palette.set_color(5, mix(mix(t.metal, bn::color(31, 31, 31), 9, 20), bn::color(31, 10, 8), k, 20));
        }
        else
        {
            _palette.set_color(4, t.metal);
            _palette.set_color(5, mix(t.metal, bn::color(31, 31, 31), 9, 20));
        }
    }

    void board_view::update(const session& s, bool ghost_enabled, int frame)
    {
        for(int b = 0; b < s.board_count(); ++b)
        {
            _draw_board(s, b, ghost_enabled, frame);
            if(_flash[b].timer > 0)
            {
                --_flash[b].timer;
            }
        }

        // Hold (the active board's hold slot) and next previews.
        const player& p = s.board_player(s.active_board());
        _draw_preview(_layout.hold_x + 1, _layout.hold_y + 2, p.held_piece(), ! p.can_hold());
        for(int i = 0; i < _layout.next_count; ++i)
        {
            _draw_preview(_layout.next_x + 1, _layout.next_y + 2 + i * 3, s.queue().peek(i), false);
        }
        _draw_meter(s, frame);
        _animate_palette(s, frame);
        _map_ptr.reload_cells_ref();
    }
}
