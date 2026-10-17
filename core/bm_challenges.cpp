#include "bm_challenges.h"

#include "bm_config.h"
#include "bm_content.h"

namespace bm
{
    namespace
    {
        constexpr challenge_def defs[challenge_total] = {
            { "TURBO", "START 4 LEVELS FASTER", cfg::challenge_turbo_percent, 1 },
            { "NO HOLD", "THE HOLD SLOT IS LOCKED", cfg::challenge_no_hold_percent, 2 },
            { "ONE NEXT", "ONLY ONE PIECE PREVIEWED", cfg::challenge_one_next_percent, 3 },
            { "FRENZY", "SPECIALS TWICE AS OFTEN", cfg::challenge_frenzy_percent, 4 },
            { "RISING TIDE", "A JUNK ROW RISES EVERY 20 S", cfg::challenge_tide_percent, 5 },
            { "SHADOW STACK", "LANDED BLOCKS GO DARK", cfg::challenge_shadow_percent, 6 },
        };
    }

    const challenge_def& challenge(int id)
    {
        return defs[id >= 0 && id < challenge_total ? id : 0];
    }

    uint8_t challenges_unlocked(const save_data& sv)
    {
        uint8_t mask = 0;
        for(int i = 0; i < challenge_total; ++i)
        {
            if(sv.stages[stage_number(defs[i].unlock_world, stages_per_world)].cleared)
            {
                mask = uint8_t(mask | (1u << i));
            }
        }
        return mask;
    }

    int challenge_score_percent(uint8_t mask)
    {
        int percent = 100;
        for(int i = 0; i < challenge_total; ++i)
        {
            if((mask >> i) & 1)
            {
                percent = percent * defs[i].score_percent / 100;
            }
        }
        return percent;
    }
}
