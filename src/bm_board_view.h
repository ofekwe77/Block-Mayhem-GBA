/*
 * BLOCK MAYHEM - board renderer.
 *
 * One dynamic 32x32 regular background holds the boards, frames, piece
 * previews, panels and the heat meter. It is rebuilt from the session state
 * every frame (cheap: ~1K map cells) and uploaded once. Special-cell symbols
 * animate through palette changes rather than extra tiles.
 */
#ifndef BM_BOARD_VIEW_H
#define BM_BOARD_VIEW_H

#include "bm_session.h"

#include "bn_bg_palette_ptr.h"
#include "bn_optional.h"
#include "bn_regular_bg_map_cell.h"
#include "bn_regular_bg_map_item.h"
#include "bn_regular_bg_map_ptr.h"
#include "bn_regular_bg_ptr.h"
#include "bn_unique_ptr.h"

namespace bm
{
    enum class layout_kind : uint8_t
    {
        single,     // board centred, panels on both sides
        boss,       // board on the left, boss on the right
        dual        // two boards with a shared centre column
    };

    struct board_slot
    {
        int8_t map_x = 10;      // map column of board column 0
        int8_t map_y = 1;       // map row of the first visible board row
    };

    struct view_layout
    {
        layout_kind kind = layout_kind::single;
        board_slot boards[2];
        int8_t hold_x = 1, hold_y = 1;          // hold panel (6 x 5 tiles)
        int8_t next_x = 22, next_y = 1;         // next panel (6 x N tiles)
        int8_t next_count = 3;
        int8_t meter_x = 7, meter_y = 7;        // heat meter column, 11 tiles tall
        int8_t meter_tiles = 9;
    };

    // Rows currently flashing because they were cleared (presentation state, set by the effects system).
    struct clear_flash
    {
        uint32_t rows = 0;
        int8_t board = 0;
        int8_t timer = 0;
        int8_t length = 0;
    };

    class board_view
    {
    public:
        board_view(layout_kind kind, int bg_priority);

        [[nodiscard]] const view_layout& layout() const { return _layout; }
        [[nodiscard]] bn::regular_bg_ptr& bg() { return _bg; }

        // Screen pixel position (top-left) of board cell (x, y) of the given board.
        [[nodiscard]] int cell_screen_x(int board, int x) const { return (_layout.boards[board].map_x + x) * 8; }
        [[nodiscard]] int cell_screen_y(int board, int y) const;

        void flash_rows(int board, uint32_t rows, int frames);
        void set_world_tint(int world);
        void update(const session& s, bool ghost_enabled, int frame);

    private:
        struct map_data
        {
            alignas(int) bn::regular_bg_map_cell cells[32 * 32];
        };

        view_layout _layout;
        bn::unique_ptr<map_data> _map;
        bn::regular_bg_map_item _map_item;
        bn::regular_bg_ptr _bg;
        bn::regular_bg_map_ptr _map_ptr;
        bn::bg_palette_ptr _palette;
        clear_flash _flash[2];
        int _world = 1;
        int _last_heat_color = -1;

        void _put(int mx, int my, int tile, int bank, bool hflip = false, bool vflip = false);
        void _draw_static(int world);
        void _draw_panel(int x, int y, int w, int h);
        void _draw_frame(int board, int cols, int rows);
        void _draw_board(const session& s, int board, bool ghost_enabled, int frame);
        void _draw_preview(int mx, int my, const queued_piece& piece, bool dim);
        void _draw_meter(const session& s, int frame);
        void _animate_palette(const session& s, int frame);
        [[nodiscard]] static int _cell_tile(const cell& c, int& bank);
    };
}

#endif
