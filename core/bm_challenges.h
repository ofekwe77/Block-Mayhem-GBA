/*
 * BLOCK MAYHEM - challenge modifiers (PART 27.3 rewards, PART 30.7 difficulty modifiers).
 *
 * Optional rules for Classic, Chaos Arcade and Dual Board. Each one is unlocked by
 * clearing a world of the Adventure (its boss stage) and scales the score: the
 * harder a challenge makes play, the more it pays; the one that makes scoring
 * easier (FRENZY) pays less. Stage, Chaos Run and Boss Rush balance never uses them.
 */
#ifndef BM_CHALLENGES_H
#define BM_CHALLENGES_H

#include <stdint.h>

#include "bm_save.h"

namespace bm
{
    enum challenge_id : uint8_t
    {
        challenge_turbo,        // start several levels higher (faster gravity)
        challenge_no_hold,      // the hold slot is locked
        challenge_one_next,     // only the next piece is previewed
        challenge_frenzy,       // special blocks twice as often (Classic gains specials)
        challenge_rising_tide,  // a garbage row rises at a fixed interval
        challenge_shadow,       // landed blocks show only a dim outline
        challenge_total
    };

    struct challenge_def
    {
        const char* name;
        const char* desc;
        uint16_t score_percent;
        uint8_t unlock_world;   // clearing this world's boss stage unlocks the challenge
    };

    [[nodiscard]] const challenge_def& challenge(int id);

    // Bit per challenge_id unlocked by the campaign progress in the save.
    [[nodiscard]] uint8_t challenges_unlocked(const save_data& sv);

    // Combined score multiplier of a set of challenges, in percent (100 = none).
    [[nodiscard]] int challenge_score_percent(uint8_t mask);
}

#endif
