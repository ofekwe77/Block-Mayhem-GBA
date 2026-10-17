/*
 * BLOCK MAYHEM - game flow helpers: builds session configurations from the
 * player's choices and records results into the save.
 */
#ifndef BM_FLOW_H
#define BM_FLOW_H

#include "bm_app.h"
#include "bm_rank.h"

#include "bn_string.h"

namespace bm
{
    // Builds the session configuration for the mode / stage selected in the flow state.
    [[nodiscard]] session_config make_session_config(const app& a);

    // Display names.
    [[nodiscard]] const char* mode_name(game_mode mode);
    [[nodiscard]] const char* ability_name(int ability);
    [[nodiscard]] const char* ability_short(int ability);
    [[nodiscard]] const char* ability_desc(int ability);
    // First Adventure stage (0..29) whose loadout teaches the ability, -1 if none.
    [[nodiscard]] int ability_unlock_stage(int ability);
    // Sound test: a track is listed once the player has reached the place it plays.
    [[nodiscard]] bool track_unlocked(const save_data& sv, int music_id);
    [[nodiscard]] const char* heat_label(int heat, bool overheat);

    // Applies the finished session to the save (records, unlocks, statistics). Returns true if a record was set.
    bool record_session(app& a);

    // CHAOS RUN ended (victory, defeat or abandoned): best score / act, completions. Returns true on a new best.
    bool record_run_end(app& a);

    // BOSS RUSH: every boss already defeated in Adventure, in world order (PART 22.6).
    constexpr int boss_rush_lives = 2;
    [[nodiscard]] int boss_rush_available(const save_data& sv);    // number of bosses that would join
    bool begin_boss_rush(app& a);
    // Ends the rush (complete or given up) and stores best score / time. Returns true on a new record.
    bool end_boss_rush(app& a, bool complete);

    // Stage rank (1 C .. 5 S+) from transparent criteria.
    // stage_rank(): core/bm_rank.h

    // Human-readable objective ("CLEAR 12 ROWS"), written into 'out'.
    void objective_text(const objective_def& o, bn::istring_base& out);

    // True when the stage can be played (first stage of an unlocked world, or previous stage cleared).
    [[nodiscard]] bool stage_unlocked(const save_data& sv, int world, int index);
    [[nodiscard]] int world_cleared_stages(const save_data& sv, int world);
}

#endif
