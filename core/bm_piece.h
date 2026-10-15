/*
 * BLOCK MAYHEM - shape definitions and the rotation system.
 *
 * Rotation follows the standard modern rotation system ("SRS"): shapes rotate
 * inside their bounding box (3x3, or 4x4 for I) and five wall-kick offsets are
 * tested in order. Coordinates are x right, y DOWN (the board's row 0 is the top).
 * The O shape never rotates.
 */
#ifndef BM_PIECE_H
#define BM_PIECE_H

#include "bm_types.h"

namespace bm
{
    // Mino offsets inside the bounding box for one rotation state. Mino index
    // order is preserved across rotations so a special cell keeps its mino.
    struct piece_shape
    {
        point minos[4];
    };

    // Shape cells for type (1..7) and rotation (0 = spawn, 1 = R, 2 = 180, 3 = L).
    [[nodiscard]] const piece_shape& shape_of(piece_type type, int rotation);

    // Bounding box size (4 for I, 3 for the others; O uses a 4-wide box).
    [[nodiscard]] int box_size(piece_type type);

    // Kick offsets (y down) for rotating from 'from' to 'to'. Returns the number of tests (5, or 1 for O).
    [[nodiscard]] int kick_tests(piece_type type, int from, int to, const point*& tests);

    [[nodiscard]] constexpr char piece_letter(piece_type type)
    {
        constexpr char letters[] = { '-', 'I', 'O', 'T', 'S', 'Z', 'J', 'L' };
        return letters[int(type)];
    }

    // An active (falling) shape: type, box position, rotation, plus an optional special mino.
    struct piece_state
    {
        piece_type type = piece_type::none;
        int8_t x = 0;          // board column of the bounding box's left edge
        int8_t y = 0;          // board row of the bounding box's top edge
        int8_t rotation = 0;
        int8_t special_mino = -1;              // which mino (0..3) carries a special, -1 = none
        special_kind special = special_kind::none;

        [[nodiscard]] point cell(int mino) const
        {
            const point& p = shape_of(type, rotation).minos[mino];
            return point{ int8_t(x + p.x), int8_t(y + p.y) };
        }
    };

    // A generated upcoming shape (queue entry).
    struct queued_piece
    {
        piece_type type = piece_type::none;
        int8_t special_mino = -1;
        special_kind special = special_kind::none;
    };
}

#endif
