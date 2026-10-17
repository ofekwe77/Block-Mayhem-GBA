/*
 * BLOCK MAYHEM - board renderer hot path, compiled as ARM code into IWRAM.
 * Every frame rewrites up to two boards cell by cell; running from IWRAM instead of
 * the cartridge bus makes this loop several times faster.
 */
#include "bm_board_view.h"
#include "bm_profile.h"

#include "bm_tile_ids.h"

namespace bm
{
    void board_view::_put(int mx, int my, int tile, int bank, bool hflip, bool vflip)
    {
        if(mx < 0 || mx >= 32 || my < 0 || my >= 32)
        {
            return;
        }
        _map->cells[my * 32 + mx] = bn::regular_bg_map_cell(tile | (hflip ? 1 << 10 : 0) | (vflip ? 1 << 11 : 0) |
                                                            (bank << 12));
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

        trail_state& tr = _trail[b];
        const bool trail = tr.timer > 0;
        const bool full = g.revision() != _drawn_revision[b] || flashing || _flash_drawn[b] || trail ||
                          tr.columns != 0;
        if(full)
        {
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
                        int t = _cell_tile(c, bank);
                        if(_shadow_stack && t == tile::block)
                        {
                            t = tile::shadow;
                        }
                        _put(mx, my, t, bank);
                    }
                }
            }
            _drawn_revision[b] = g.revision();
        }
        else
        {
            // Nothing on the board changed: restore only the cells the piece and ghost covered last frame.
            for(int i = 0; i < _overlay_count[b]; ++i)
            {
                const int x = _overlay_x[b][i];
                const int y = _overlay_y[b][i];
                const cell& c = g.at(x, y);
                const int mx = slot.map_x + x;
                const int my = slot.map_y + y - hidden;
                if(c.empty())
                {
                    _put(mx, my, y < hidden ? tile::chute : tile::well, bank_frame);
                }
                else
                {
                    int bank = 0;
                    int t = _cell_tile(c, bank);
                    if(_shadow_stack && t == tile::block)
                    {
                        t = tile::shadow;
                    }
                    _put(mx, my, t, bank);
                }
            }
        }
        if(trail)
        {
            // Streak over the empty cells the piece fell through; it shrinks towards the landing point.
            const int length = tr.to - tr.from;
            const int visible_rows = length * tr.timer / 10;
            for(int y = tr.to - visible_rows; y < tr.to; ++y)
            {
                if(y < hidden)
                {
                    continue;
                }
                const int depth = tr.to - y;     // 1 = right above the piece
                const int t = depth * 3 <= length ? tile::trail1 : (depth * 3 <= length * 2 ? tile::trail2 : tile::trail3);
                for(int x = 0; x < g.cols(); ++x)
                {
                    if(((tr.columns >> x) & 1) && g.at(x, y).empty())
                    {
                        _put(slot.map_x + x, slot.map_y + y - hidden, t, bank_frame);
                    }
                }
            }
            --tr.timer;
            ++profile::trail_draws;
        }
        else
        {
            tr.columns = 0;     // one more full repaint after the trail ends clears it
        }
        _flash_drawn[b] = flashing;
        _overlay_count[b] = 0;
        auto remember = [this, b, &g](int x, int y)
        {
            if(_overlay_count[b] < 8 && x >= 0 && x < g.cols() && y >= 0 && y < g.rows())
            {
                _overlay_x[b][_overlay_count[b]] = int8_t(x);
                _overlay_y[b][_overlay_count[b]] = int8_t(y);
                ++_overlay_count[b];
            }
        };

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
                        remember(c.x, c.y);
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
            remember(c.x, c.y);
        }
    }

}
