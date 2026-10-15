/*
 * BLOCK MAYHEM - game flow helpers: builds session configurations from the
 * player's choices and records results into the save.
 */
#ifndef BM_FLOW_H
#define BM_FLOW_H

#include "bm_app.h"

namespace bm
{
    // Builds the session configuration for the mode / stage selected in the flow state.
    [[nodiscard]] session_config make_session_config(const app& a);

    // Display names.
    [[nodiscard]] const char* mode_name(game_mode mode);
    [[nodiscard]] const char* ability_name(int ability);
    [[nodiscard]] const char* ability_short(int ability);
    [[nodiscard]] const char* heat_label(int heat, bool overheat);

    // Applies the finished session to the save (records, unlocks, statistics). Returns true if a record was set.
    bool record_session(app& a);

    // Stage rank (1 C .. 5 S+) from transparent criteria.
    [[nodiscard]] int stage_rank(const session& s);
}

#endif
