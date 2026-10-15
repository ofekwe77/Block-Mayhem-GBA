#include "bm_piece.h"

namespace bm
{
    namespace
    {
        // Spawn-state minos (y down) for I, O, T, S, Z, J, L.
        constexpr piece_shape spawn_shapes[7] = {
            { { { 0, 1 }, { 1, 1 }, { 2, 1 }, { 3, 1 } } },     // I (4x4 box)
            { { { 1, 0 }, { 2, 0 }, { 1, 1 }, { 2, 1 } } },     // O
            { { { 1, 0 }, { 0, 1 }, { 1, 1 }, { 2, 1 } } },     // T
            { { { 1, 0 }, { 2, 0 }, { 0, 1 }, { 1, 1 } } },     // S
            { { { 0, 0 }, { 1, 0 }, { 1, 1 }, { 2, 1 } } },     // Z
            { { { 0, 0 }, { 0, 1 }, { 1, 1 }, { 2, 1 } } },     // J
            { { { 2, 0 }, { 0, 1 }, { 1, 1 }, { 2, 1 } } },     // L
        };

        constexpr int box_sizes[7] = { 4, 4, 3, 3, 3, 3, 3 };

        struct shape_table
        {
            piece_shape shapes[7][4] = {};

            constexpr shape_table()
            {
                for(int t = 0; t < 7; ++t)
                {
                    shapes[t][0] = spawn_shapes[t];
                    for(int r = 1; r < 4; ++r)
                    {
                        if(t == 1)
                        {
                            shapes[t][r] = spawn_shapes[t];    // O never changes
                            continue;
                        }
                        const int n = box_sizes[t];
                        for(int m = 0; m < 4; ++m)
                        {
                            const point& p = shapes[t][r - 1].minos[m];
                            // Clockwise rotation inside an n x n box: (x, y) -> (n - 1 - y, x).
                            shapes[t][r].minos[m] = point{ int8_t(n - 1 - p.y), p.x };
                        }
                    }
                }
            }
        };

        constexpr shape_table table;

        // Kick tests, y down. Index [from rotation][test].
        constexpr point jlstz_cw[4][5] = {
            { { 0, 0 }, { -1, 0 }, { -1, -1 }, { 0, 2 }, { -1, 2 } },     // 0 -> R
            { { 0, 0 }, { 1, 0 }, { 1, 1 }, { 0, -2 }, { 1, -2 } },       // R -> 2
            { { 0, 0 }, { 1, 0 }, { 1, -1 }, { 0, 2 }, { 1, 2 } },        // 2 -> L
            { { 0, 0 }, { -1, 0 }, { -1, 1 }, { 0, -2 }, { -1, -2 } },    // L -> 0
        };

        constexpr point jlstz_ccw[4][5] = {
            { { 0, 0 }, { 1, 0 }, { 1, -1 }, { 0, 2 }, { 1, 2 } },        // 0 -> L
            { { 0, 0 }, { 1, 0 }, { 1, 1 }, { 0, -2 }, { 1, -2 } },       // R -> 0
            { { 0, 0 }, { -1, 0 }, { -1, -1 }, { 0, 2 }, { -1, 2 } },     // 2 -> R
            { { 0, 0 }, { -1, 0 }, { -1, 1 }, { 0, -2 }, { -1, -2 } },    // L -> 2
        };

        constexpr point i_cw[4][5] = {
            { { 0, 0 }, { -2, 0 }, { 1, 0 }, { -2, 1 }, { 1, -2 } },      // 0 -> R
            { { 0, 0 }, { -1, 0 }, { 2, 0 }, { -1, -2 }, { 2, 1 } },      // R -> 2
            { { 0, 0 }, { 2, 0 }, { -1, 0 }, { 2, -1 }, { -1, 2 } },      // 2 -> L
            { { 0, 0 }, { 1, 0 }, { -2, 0 }, { 1, 2 }, { -2, -1 } },      // L -> 0
        };

        constexpr point i_ccw[4][5] = {
            { { 0, 0 }, { -1, 0 }, { 2, 0 }, { -1, -2 }, { 2, 1 } },      // 0 -> L
            { { 0, 0 }, { 2, 0 }, { -1, 0 }, { 2, -1 }, { -1, 2 } },      // R -> 0
            { { 0, 0 }, { 1, 0 }, { -2, 0 }, { 1, 2 }, { -2, -1 } },      // 2 -> R
            { { 0, 0 }, { -2, 0 }, { 1, 0 }, { -2, 1 }, { 1, -2 } },      // L -> 2
        };

        constexpr point no_kick[1] = { { 0, 0 } };
    }

    const piece_shape& shape_of(piece_type type, int rotation)
    {
        return table.shapes[int(type) - 1][rotation & 3];
    }

    int box_size(piece_type type)
    {
        return box_sizes[int(type) - 1];
    }

    int kick_tests(piece_type type, int from, int to, const point*& tests)
    {
        from &= 3;
        to &= 3;
        if(type == piece_type::o)
        {
            tests = no_kick;
            return 1;
        }
        const bool cw = ((from + 1) & 3) == to;
        if(type == piece_type::i)
        {
            tests = cw ? i_cw[from] : i_ccw[from];
        }
        else
        {
            tests = cw ? jlstz_cw[from] : jlstz_ccw[from];
        }
        return 5;
    }
}
