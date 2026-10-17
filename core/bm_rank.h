/*
 * BLOCK MAYHEM - stage ranks C..S+ (PART 27.2). Transparent criteria shown on the stage card and the results:
 * score against the stage's S target (3 / 2 / 1 points at 100% / 67% / 33%), time against par (2 points within
 * par, 1 within 1.5 x par), no garbage received (1 point) and the bonus goal (2 points).
 * 8 points = S+, 6-7 = S, 4-5 = A, 2-3 = B, otherwise C.
 */
#ifndef BM_RANK_H
#define BM_RANK_H

#include <stdint.h>

namespace bm
{
    class session;

    struct rank_input
    {
        uint32_t score = 0;
        uint32_t s_target = 1;
        uint32_t seconds = 0;
        uint32_t par_seconds = 1;
        bool no_garbage = false;
        bool bonus_goal = false;
    };

    // Rank points 0..8 and the rank 1 (C) .. 5 (S+).
    [[nodiscard]] int rank_points(const rank_input& in);
    [[nodiscard]] int rank_from_points(int points);

    // The rank of a finished Adventure stage (0 when the session is not a won stage).
    [[nodiscard]] int stage_rank(const session& s);
}

#endif
