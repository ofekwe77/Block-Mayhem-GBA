/*
 * BLOCK MAYHEM - automated players.
 *
 * Used by host-side simulations (balance analysis, exploit hunting, stage
 * validation) and by the ROM's diagnostic autoplay test path. The bot only
 * ever presses buttons through the same input_frame interface as a human, so
 * it can never do anything a player could not.
 *
 * Strategies (PART 31.3): random legal placement, height minimizing, hole
 * minimizing, line-clear focused, and resource aware (also spends heat).
 */
#ifndef BM_BOT_H
#define BM_BOT_H

#include "bm_session.h"

namespace bm
{
    enum class bot_strategy : uint8_t
    {
        random_legal,
        height_min,
        hole_min,
        line_clear,
        resource_aware
    };

    // Human pacing for balance runs (PART 54): reaction time per piece, frames between button taps,
    // and the chance of placing a piece one column off. All zero = the instant test bot.
    struct bot_pace
    {
        uint8_t think_frames = 0;
        uint8_t tap_interval = 0;
        uint8_t error_percent = 0;
    };

    struct bot_plan
    {
        bool valid = false;
        bool use_hold = false;
        int8_t rotation = 0;
        int8_t x = 0;
        int score = 0;
    };

    class bot
    {
    public:
        void reset(bot_strategy strategy, uint32_t seed);
        void set_pace(const bot_pace& pace) { _pace = pace; }
        [[nodiscard]] bot_strategy strategy() const { return _strategy; }

        // Chooses buttons for this frame. Call once per frame before session::update().
        input_frame next(const session& s);

        // Best placement for the active piece (exposed for tests).
        [[nodiscard]] bot_plan plan(const session& s);

    private:
        bot_strategy _strategy = bot_strategy::hole_min;
        rng _rng;
        bot_plan _plan;
        uint16_t _prev = 0;
        int _piece_serial = -1;
        int _last_type = 0;
        int _last_y = 99;
        int _last_board = 0;
        bool _hold_done = false;
        int _wait = 0;
        int _stall = 0;
        bot_pace _pace;
        int _think = 0;
        int _tap_cooldown = 0;

        [[nodiscard]] int _evaluate(const uint16_t* rows, int cols, int nrows, int lines_cleared, int max_drop_row) const;
        bool _try_plan(const session& s, piece_type type, bool hold, bot_plan& best) const;
        [[nodiscard]] uint16_t _ability_keys(const session& s);
    };
}

#endif
