/*
 * BLOCK MAYHEM - central balance configuration.
 *
 * Every numeric gameplay rule lives here so balance changes never require
 * hunting through unrelated source files. BALANCE.md documents these values.
 * All values are integers (frames, points, heat, HP) - no floating point.
 */
#ifndef BM_CONFIG_H
#define BM_CONFIG_H

#include <stdint.h>

namespace bm::cfg
{
    // ---------------------------------------------------------------- board
    constexpr int board_cols = 10;
    constexpr int board_rows = 20;          // logical rows including hidden spawn rows
    constexpr int board_hidden_rows = 2;
    constexpr int dual_board_cols = 8;
    constexpr int dual_board_rows = 18;     // 16 visible + 2 hidden

    // ---------------------------------------------------------------- input / movement (frames @ 60 Hz)
    constexpr int das_frames = 10;          // initial horizontal repeat delay
    constexpr int arr_frames = 2;           // automatic horizontal repeat interval
    constexpr int soft_drop_frames = 2;     // soft drop: one row every N frames
    constexpr int lock_delay_frames = 30;
    constexpr int max_lock_resets = 12;
    constexpr int queue_visible = 3;        // previews shown in standard modes
    constexpr int queue_length = 6;         // pieces generated ahead

    // Gravity: frames per row for displayed level 1..12 (12+ uses the last entry).
    constexpr int gravity_table[12] = { 48, 42, 36, 30, 26, 22, 18, 15, 12, 10, 8, 6 };
    constexpr int lines_per_level = 10;
    constexpr int max_level = 30;

    // Difficulty gravity scaling in percent of the table value (higher = slower).
    constexpr int beginner_gravity_percent = 140;
    constexpr int standard_gravity_percent = 100;
    constexpr int expert_gravity_percent = 75;
    constexpr int min_gravity_frames = 4;

    // ---------------------------------------------------------------- scoring
    // Base points by rows cleared together; multiplied by the displayed level
    // (displayed level = internal level index + 1, so "level + 1" with 0-based internal levels).
    constexpr int line_points[5] = { 0, 100, 300, 500, 800 };
    constexpr int twist_points[4] = { 400, 800, 1200, 1600 };   // T-TWIST with 0..3 rows
    constexpr int twist_mini_points = 100;                       // T-TWIST mini (no rows) / added to mini clears
    constexpr int soft_drop_points = 1;     // per manually advanced row
    constexpr int hard_drop_points = 2;     // per descended row
    constexpr int combo_points = 50;        // x combo index x level
    constexpr int combo_index_cap = 20;
    constexpr int b2b_percent = 150;        // back-to-back multiplier on the line-clear base
    // Chain bonus by resolution wave count (index = waves, capped at 8), x level.
    constexpr int chain_points[9] = { 0, 0, 100, 250, 450, 700, 1000, 1400, 2000 };
    constexpr int special_cell_points = 25; // per cell destroyed by a special effect
    constexpr int special_cell_points_cap = 500;   // per single effect (20 cells)
    constexpr int perfect_clear_points = 2000;      // board completely empty after a clear, x level

    // ---------------------------------------------------------------- heat
    constexpr int heat_max = 100;
    constexpr int line_heat[5] = { 0, 13, 30, 45, 65 };
    constexpr int twist_heat_bonus = 10;
    constexpr int chain_wave_heat = 5;      // per resolution wave after the first
    constexpr int chain_heat_cap = 25;      // max extra heat from waves in one resolution
    constexpr int b2b_heat = 10;
    constexpr int magnet_fizzle_heat = 3;   // magnet found no legal move
    constexpr int chill_heat_percent = 50;  // heat gain while chilled (Cryo Titan)

    // ---------------------------------------------------------------- overheat
    constexpr int overheat_frames = 720;            // 12 s of active piece-control time
    constexpr int overheat_success_rows = 2;
    constexpr int overheat_score_percent = 200;     // line-clear score doubled
    constexpr int overheat_boss_damage_percent = 125;
    constexpr int overheat_gravity_percent = 80;    // falling speed moderately faster
    constexpr int overheat_success_points = 1500;   // x level
    constexpr int overheat_heat_keep_cap = 50;      // heat that may accumulate during Overheat
    constexpr int overheat_garbage_warning_frames = 180; // last 3 s show the incoming garbage
    constexpr int overheat_fail_rows_beginner = 1;
    constexpr int overheat_fail_rows_standard = 2;
    constexpr int overheat_fail_rows_expert = 2;

    // ---------------------------------------------------------------- abilities
    enum ability_id : uint8_t
    {
        ability_laser,
        ability_time_freeze,
        ability_gravity_pulse,
        ability_shape_shift,
        ability_nuclear_burst,
        ability_shield_matrix,
        ability_chain_spark,
        ability_emergency_vent,
        ability_count
    };

    constexpr int ability_cost[ability_count] = { 40, 35, 45, 25, 80, 30, 45, 50 };
    constexpr int ability_cooldown[ability_count] = { 4, 5, 5, 3, 8, 5, 4, 6 };   // locked pieces
    constexpr int ability_min_cost = 15;            // upgrades can never reduce below this
    constexpr int ability_min_cooldown = 1;         // cooldowns never reach zero
    constexpr int targeting_timeout_frames = 300;   // 5 s, then the selection is cancelled (no cost)
    constexpr int laser_max_hits = 10;              // beam hit budget per activation
    constexpr int time_freeze_frames = 180;         // 3 s of no automatic falling
    constexpr int time_freeze_max_frames = 300;     // cap with upgrades
    constexpr int nuke_radius = 2;                  // 5x5
    constexpr int shield_duration_pieces = 10;      // shield lasts this many locks or until it blocks
    constexpr int chain_spark_max_charged = 2;
    constexpr int chain_spark_pulse_range = 2;
    constexpr int vent_cells = 8;                   // cells removed by Emergency Vent
    constexpr int vent_cells_max = 12;              // cap with upgrades

    // ---------------------------------------------------------------- special cells
    constexpr int bomb_radius = 1;                  // 3x3
    constexpr int fire_range = 1;
    constexpr int spark_range = 4;
    constexpr int spark_range_max = 6;
    constexpr int magnet_columns = 2;
    constexpr int rainbow_targets = 2;
    constexpr int ice_hp = 1;
    constexpr int armor_hp = 2;
    constexpr int max_resolution_waves = 32;        // hard safety bound
    constexpr int max_thaw_pulses = 3;

    // Resolution animation delays (simulation frames, deterministic).
    constexpr int line_clear_delay[5] = { 0, 16, 18, 20, 26 };
    constexpr int wave_delay = 10;
    constexpr int thaw_delay = 14;
    constexpr int settle_delay = 6;
    constexpr int spawn_delay = 2;                  // ARE after a lock with no clear

    // ---------------------------------------------------------------- boss combat
    constexpr int boss_line_damage[5] = { 0, 5, 12, 20, 35 };
    constexpr int boss_twist_damage_bonus = 6;
    constexpr int boss_chain_damage_per_wave = 4;
    constexpr int boss_chain_damage_cap = 20;
    constexpr int boss_ability_damage_laser = 6;
    constexpr int boss_ability_damage_nuke = 14;
    constexpr int boss_resolution_damage_cap_percent = 40;  // of max HP per resolution
    constexpr int boss_hp[6] = { 100, 140, 180, 220, 260, 350 };
    constexpr int boss_min_telegraph_pieces = 2;            // every attack warns at least this many locks ahead
    constexpr int boss_phase_transition_frames = 90;        // non-interactive, timers frozen

    // ---------------------------------------------------------------- dual board
    constexpr int dual_forced_switch_locks = 4;
    constexpr int cross_combo_window_locks = 4;
    constexpr int cross_combo_points = 1000;        // x level
    constexpr int cross_combo_heat = 15;
    constexpr int cross_double_heat = 6;            // 2+ rows on A grants heat
    constexpr int cross_quad_garbage_cancel = 2;    // 4 rows on A cancels pending garbage on B

    // ---------------------------------------------------------------- special-cell distribution
    constexpr int special_interval_normal_min = 5;  // about one special piece every 6 locks
    constexpr int special_interval_normal_max = 7;
    constexpr int special_interval_advanced_min = 3;
    constexpr int special_interval_advanced_max = 5;
}

#endif
