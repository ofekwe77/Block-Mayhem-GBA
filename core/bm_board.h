/*
 * BLOCK MAYHEM - the logical puzzle board.
 *
 * Fixed-size grid (max 10 x 20) of compact cells plus one occupancy bitmask per
 * row for fast collision tests. Row 0 is the top. The top 'hidden' rows are the
 * spawn area. Every mutation goes through set()/clear() so the bitmasks and
 * cells never disagree (board invariant).
 */
#ifndef BM_BOARD_H
#define BM_BOARD_H

#include "bm_config.h"
#include "bm_piece.h"
#include "bm_types.h"

namespace bm
{
    class board
    {
    public:
        static constexpr int max_cols = cfg::board_cols;
        static constexpr int max_rows = cfg::board_rows;

        void init(int cols, int rows, int hidden);

        [[nodiscard]] int cols() const { return _cols; }
        [[nodiscard]] int rows() const { return _rows; }
        [[nodiscard]] int hidden() const { return _hidden; }
        [[nodiscard]] uint16_t full_mask() const { return _full_mask; }

        [[nodiscard]] bool inside(int x, int y) const
        {
            return x >= 0 && x < _cols && y >= 0 && y < _rows;
        }

        // Collision test: walls and floor are solid, space above row 0 is open.
        [[nodiscard]] bool blocked(int x, int y) const
        {
            if(x < 0 || x >= _cols || y >= _rows)
            {
                return true;
            }
            if(y < 0)
            {
                return false;
            }
            return (_bits[y] >> x) & 1;
        }

        [[nodiscard]] const cell& at(int x, int y) const { return _cells[y][x]; }
        [[nodiscard]] uint16_t row_bits(int y) const { return _bits[y]; }
        [[nodiscard]] bool row_full(int y) const { return _bits[y] == _full_mask; }
        [[nodiscard]] bool row_empty(int y) const { return _bits[y] == 0; }

        void set(int x, int y, const cell& c);
        void clear(int x, int y);
        void clear_all();

        // Mutable access to non-occupancy attributes (special, coat, flags). Never use it to change color.
        [[nodiscard]] cell& attributes(int x, int y)
        {
            ++_revision;
            return _cells[y][x];
        }

        // Changes whenever any cell may have changed (renderers redraw only when it moves).
        [[nodiscard]] uint32_t revision() const { return _revision; }

        [[nodiscard]] bool fits(const piece_state& p) const;
        [[nodiscard]] int drop_distance(const piece_state& p) const;

        // Column height measured from the floor (0 = empty column).
        [[nodiscard]] int column_height(int x) const;
        [[nodiscard]] int stack_height() const;
        [[nodiscard]] int holes() const;
        [[nodiscard]] int occupied_count() const;
        [[nodiscard]] int count_rows_in_hidden_area() const;
        [[nodiscard]] bool empty() const;

        // Pushes every row up by 'count' rows and fills the bottom with garbage rows that have one gap each.
        // Returns false if occupied cells were pushed above the top (top-out).
        bool push_garbage(int count, const int8_t* gap_columns, coat_kind coat = coat_kind::none);

        // Removes the given rows (bitmask over rows) and shifts everything above down (standard clear).
        void remove_rows(uint32_t row_mask);

        // Board invariant check used by tests and debug builds. Returns 0 if valid, else an error code.
        [[nodiscard]] int validate() const;

    private:
        cell _cells[max_rows][max_cols];
        uint16_t _bits[max_rows] = {};
        int _cols = cfg::board_cols;
        int _rows = cfg::board_rows;
        int _hidden = cfg::board_hidden_rows;
        uint16_t _full_mask = (1u << cfg::board_cols) - 1;
        uint32_t _revision = 0;
    };
}

#endif
