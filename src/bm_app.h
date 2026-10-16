/*
 * BLOCK MAYHEM - application context shared by every scene.
 *
 * Owns the save data, the active play session (kept alive so result screens
 * can read it), the flow state (which mode / stage / run is in progress), and
 * the emulator test interface.
 */
#ifndef BM_APP_H
#define BM_APP_H

#include "bm_bot.h"
#include "bm_save.h"
#include "bm_scene.h"
#include "bm_session.h"
#include "test_interface.h"

#include "bn_fixed.h"

namespace bm
{
    class chaos_run;

    enum class session_outcome : uint8_t
    {
        none, won, lost, quit
    };

    struct flow_state
    {
        game_mode mode = game_mode::classic;
        uint8_t world = 1;              // adventure: 1..6
        uint8_t stage = 1;              // adventure: 1..5
        uint8_t boss_rush_index = 0;    // fight index in the sequence
        uint8_t boss_rush_lives = 2;    // recovery opportunities
        uint8_t boss_rush_worlds[6] = {};   // worlds whose boss joins the rush, in order
        uint8_t boss_rush_count = 0;
        uint8_t boss_rush_state = 0;    // 0 fighting, 1 complete, 2 over
        uint32_t boss_rush_score = 0;
        uint32_t boss_rush_frames = 0;
        uint32_t seed = 1;
        session_outcome outcome = session_outcome::none;
        uint8_t rank = 0;               // rank of the last finished stage
        bool new_record = false;
        bool autoplay = false;          // diagnostic test path only
        bool diagnostic = false;
        uint8_t arcade_loadout[3] = { 0, 1, 5 };
        uint8_t arcade_count = 3;
        uint8_t dual_ability = 0;
        bool credits_from_ending = false;   // the credits roll after the final ending returns to the title
    };

    class app
    {
    public:
        app();

        save_data save;
        flow_state flow;
        session* game = nullptr;        // allocated once (EWRAM)
        bot autoplayer;
        chaos_run* run = nullptr;       // Chaos Run state (allocated once)

        bm_test_interface* test = nullptr;
        int save_status = 0;
        uint32_t frame = 0;
        uint32_t visual_seed = 1;

        // Persists the save (both slots alternate). Returns true on success.
        bool write_save();
        void load_save();

        // Builds the input for this frame (joypad or diagnostic autoplay).
        [[nodiscard]] input_frame gameplay_input();

        // Per-frame bookkeeping (frame counter, CPU statistics, test mailbox).
        void tick();

        // Achievements unlocked but not yet announced (bit per achievement_id).
        uint32_t pending_achievements = 0;

        // Mailbox command waiting for the current scene to act on (diagnostics).
        uint32_t pending_command = 0;
        uint32_t pending_args[4] = {};

        [[nodiscard]] bn::fixed music_volume() const;
        [[nodiscard]] bn::fixed sfx_volume() const;

    private:
        int _slot = -1;
        int _bot_ticks = 0;     // autoplay planning time of this frame (excluded from the CPU statistics)
        uint32_t _sequence = 0;
    };

    app& the_app();
}

#endif
