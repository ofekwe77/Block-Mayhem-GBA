/*
 * BLOCK MAYHEM - chain-reaction resolution engine.
 *
 * A deterministic, non-recursive state machine. After a piece locks (or an
 * ability / hazard hits the board) it repeats:
 *
 *   SCAN    find complete rows; rows holding ice/armor first get a finite
 *           "thaw pulse" (each pulse removes one coat point) and are rescanned.
 *   LINES   complete rows are cleared in place (one wave); specials inside them
 *           are queued for the next wave.
 *   WAVES   queued specials activate at their original positions, one wave at
 *           a time; specials they hit are queued for the following wave.
 *   SETTLE  cleared rows are removed (standard behaviour) and the gaps left by
 *           special effects are closed in the settle direction - each cell moves
 *           by the number of cells destroyed beyond it, so pre-existing holes are
 *           preserved and a plain line clear behaves exactly like a classic one.
 *   ...then SCAN again until the board is stable.
 *
 * Safety: every special activates at most once per resolution, queues are
 * bounded, coordinates are validated, and the number of waves is capped.
 * Each step returns the presentation delay (frames) before the next step.
 */
#ifndef BM_RESOLVER_H
#define BM_RESOLVER_H

#include "bm_board.h"
#include "bm_events.h"

namespace bm
{
    // Upgrade / stage modifiers for special effects. Defaults = base rules.
    struct effect_mods
    {
        int8_t bomb_power = 1;          // EXPLOSIVE MASTER: 2
        int8_t bomb_plus = 0;           // EXPLOSIVE MASTER: +1 orthogonal reach
        int8_t fire_reach = 1;          // FIRE STARTER: 2 (skips one empty cell)
        int8_t spark_range = cfg::spark_range;     // ELECTRIC SURGE: +1 (max spark_range_max)
        int8_t magnet_cols = cfg::magnet_columns;  // MAGNETIC FIELD: 3
        int8_t magnet_rows = 1;                    // MAGNETIC FIELD: 2
        int8_t rainbow_targets = cfg::rainbow_targets; // RAINBOW MASTER: 3
        int8_t coat_bonus = 0;          // CRYSTAL BREAKER: +1 damage against ice/armor
        bool double_spark = false;      // DOUBLE SPARK
        gravity_dir settle_dir = gravity_dir::down;
    };

    enum class trigger_kind : uint8_t
    {
        bomb = 1, fire, spark, magnet, rainbow,
        pulse,          // charged cell (Chain Spark) or Double Spark discharge
    };

    // Causes reported in cell_destroyed events (d field) for abilities.
    enum destroy_cause : int16_t
    {
        cause_line = 0,
        cause_bomb = 1, cause_fire, cause_spark, cause_magnet, cause_rainbow, cause_pulse,
        cause_laser = 10, cause_nuke, cause_vent, cause_hazard
    };

    struct resolution_result
    {
        int waves = 0;              // total waves (line waves + special waves)
        int line_waves = 0;
        int first_rows = 0;         // rows cleared by the first line wave
        int rows_total = 0;
        int extra_rows = 0;         // rows cleared by cascade line waves
        int destroyed = 0;          // cells destroyed by special effects / abilities
        int special_points = 0;     // already capped per effect
        int triggers[7] = {};       // indexed by trigger_kind
        int magnet_moves = 0;
        int magnet_fizzles = 0;
        int coat_hits = 0;
        int ice_broken = 0;
        int armor_broken = 0;
        int garbage_rows_cleared = 0;
        bool perfect_clear = false;
        bool any_line = false;
    };

    class resolver
    {
    public:
        static constexpr int max_triggers = 96;

        // Start resolving after a piece lock (scan rows first).
        void begin(const effect_mods& mods);

        // Start with external hits already applied through hit() (abilities / hazards).
        void begin_external(const effect_mods& mods);

        // Full compaction in a direction (Gravity Pulse, gravity hazards), then cascades.
        void begin_compaction(board& b, gravity_dir dir, const effect_mods& mods, event_queue& events, int board_index);

        // Applies one hit (used by abilities while setting up begin_external). Returns true if a cell changed.
        bool hit(board& b, int x, int y, int power, int cause, event_queue& events, int board_index);

        // Marks a cell destroyed outright (no coat check) - used by Emergency Vent.
        void remove(board& b, int x, int y, int cause, event_queue& events, int board_index);

        [[nodiscard]] bool active() const { return _phase != phase::idle; }

        // Performs the next resolution step. Returns the delay (frames) before calling again.
        int step(board& b, event_queue& events, int board_index);

        [[nodiscard]] const resolution_result& result() const { return _result; }

        // Full compaction helper usable without a resolution (tests, boss attacks).
        static void compact(board& b, gravity_dir dir);

    private:
        enum class phase : uint8_t { idle, scan, waves, settle, finish };

        struct trigger
        {
            int8_t x = 0;
            int8_t y = 0;
            trigger_kind kind = trigger_kind::bomb;
            uint8_t destroyed = 0;      // cells destroyed by this trigger (for capped points)
        };

        effect_mods _mods;
        resolution_result _result;
        phase _phase = phase::idle;
        uint16_t _removed[board::max_rows] = {};    // cells destroyed by effects this step (gaps to close)
        uint16_t _activated[board::max_rows] = {};  // specials already activated this resolution
        uint32_t _cleared_rows = 0;                 // rows cleared by the current line wave
        trigger _current[max_triggers];
        trigger _next[max_triggers];
        int _current_count = 0;
        int _next_count = 0;
        int _active_trigger = -1;                   // index into _current while processing
        int _thaw_pulses = 0;
        int _sparks_this_resolution = 0;
        bool _double_spark_used = false;

        void _reset(const effect_mods& mods);
        int _scan(board& b, event_queue& events, int board_index);
        int _wave(board& b, event_queue& events, int board_index);
        int _settle(board& b, event_queue& events, int board_index);
        void _finish(board& b, event_queue& events, int board_index);

        void _enqueue(int x, int y, trigger_kind kind);
        void _destroy(board& b, int x, int y, int cause, event_queue& events, int board_index);
        void _run_trigger(board& b, trigger& t, event_queue& events, int board_index);
        void _bomb(board& b, int x, int y, event_queue& events, int board_index);
        void _fire(board& b, int x, int y, event_queue& events, int board_index);
        void _arcs(board& b, int x, int y, int range, int cause, event_queue& events, int board_index);
        void _magnet(board& b, int x, int y, event_queue& events, int board_index);
        void _rainbow(board& b, int x, int y, event_queue& events, int board_index);
    };
}

#endif
