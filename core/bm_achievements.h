/*
 * BLOCK MAYHEM - achievements (PART 28, 35.2).
 *
 * Every achievement has a trigger evaluated from real game data only (session
 * statistics, lifetime statistics, campaign / run / rush records), optional
 * progress, a saved unlock bit, an icon and a short description. Hidden
 * achievements keep their name and description secret until unlocked.
 */
#ifndef BM_ACHIEVEMENTS_H
#define BM_ACHIEVEMENTS_H

#include "bm_save.h"
#include "bm_session.h"

namespace bm
{
    enum achievement_id : uint8_t
    {
        ach_first_line, ach_double_trouble, ach_triple_threat, ach_perfect_four, ach_hot_stuff, ach_heat_master,
        ach_bomb_squad, ach_chain_rookie, ach_chain_master, ach_meltdown, ach_electrician, ach_firestarter,
        ach_ice_breaker, ach_laser_expert, ach_gravity_wizard, ach_cross_over, ach_factory_hero, ach_world_traveler,
        ach_mayhem_champion, ach_chaos_survivor, ach_speed_demon, ach_untouchable, ach_puzzle_genius,
        ach_last_stand, ach_master_of_mayhem,
        // hidden
        ach_clean_sweep, ach_arsenal, ach_iron_wall, ach_rush_hour, ach_spice_lord, ach_spice_detective,
        achievement_total
    };

    static_assert(achievement_total <= achievement_count, "save data holds 32 achievement slots");

    struct achievement_def
    {
        const char* name;
        const char* desc;
        uint16_t target;        // progress target (1 = simple unlock)
        uint8_t icon;           // spr_goals frame
        bool hidden;
    };

    [[nodiscard]] const achievement_def& achievement(int id);

    // Context for one evaluation. 'live' is the session being played or just finished; its statistics
    // are added to the lifetime totals unless 'merged' says record_session already folded them in.
    struct achievement_input
    {
        const session* live = nullptr;
        bool merged = false;
        bool session_over = false;      // the session ended (won / lost) and its result is final
        uint8_t stage_rank = 0;         // Adventure rank of the finished stage (5 = S+)
    };

    // Updates progress and unlock bits in the save. Returns the mask of newly unlocked achievements.
    uint32_t update_achievements(save_data& sv, const achievement_input& in);

    [[nodiscard]] int achievements_unlocked(const save_data& sv);
}

#endif
