/*
 * BLOCK MAYHEM - gameplay events.
 *
 * The simulation emits events; presentation (particles, sounds, shake, text)
 * and statistics consume them. Game rules never depend on whether an event was
 * displayed: dropping events can only lose effects, never change outcomes.
 */
#ifndef BM_EVENTS_H
#define BM_EVENTS_H

#include <stdint.h>

namespace bm
{
    enum class event_type : uint8_t
    {
        none,
        piece_spawned,      // a = piece type
        piece_moved,
        piece_rotated,      // a = 1 if a kick was used
        piece_held,
        piece_locked,       // a = piece type, b = lowest row, c = 1 if hard drop
        hard_drop,          // a = rows descended, b = column center x2, c = top row, d = bottom row
        rows_full,          // a = row mask (low 16), b = row mask (high), c = count
        thaw_pulse,         // a = row mask low, b = row mask high
        coat_hit,           // a = x, b = y, c = coat kind, d = hp left
        lines_cleared,      // a = rows, d = wave (twist clears also raise twist_clear)
        cell_destroyed,     // a = x, b = y, c = color, d = cause (special_kind or 10+ability)
        special_triggered,  // a = x, b = y, c = special_kind, d = wave
        spark_arc,          // a = x, b = y, c = direction (0 up 1 right 2 down 3 left), d = length
        fire_burst,         // a = x, b = y, c = reach
        bomb_blast,         // a = x, b = y, c = radius
        magnet_pull,        // a = from x, b = from y, c = to x, d = to y
        magnet_fizzle,      // a = x, b = y
        rainbow_link,       // a = from x, b = from y, c = to x, d = to y
        charge_pulse,       // a = x, b = y
        chain_wave,         // a = wave number (2+)
        settle,             // a = direction
        resolution_done,    // a = waves, b = rows total, c = cells destroyed
        score,              // a = points (low 16), b = points (high 16), c = x, d = y (popup anchor)
        combo,              // a = combo index
        b2b,                // a = streak
        level_up,           // a = new level
        heat_changed,       // a = new heat, b = delta (signed)
        overheat_start,
        overheat_warning,   // a = rows of garbage that will arrive
        overheat_success,
        overheat_fail,      // a = garbage rows
        overheat_end,
        ability_selected,   // a = ability id
        ability_denied,     // a = ability id, b = reason (1 heat, 2 cooldown, 3 illegal)
        ability_targeting,  // a = ability id
        ability_fired,      // a = ability id, b = x, c = y, d = extra
        ability_cancelled,  // a = ability id
        shield_up,
        shield_block,       // a = attack kind (100 + hazard kind for environmental events), b = 1: queued garbage stopped
        shield_down,
        garbage_warning,    // a = rows, b = board
        garbage_inserted,   // a = rows
        garbage_cancelled,  // a = rows
        hazard_warning,     // a = hazard kind, b = pieces until it fires
        hazard_fired,       // a = hazard kind, b = cells affected
        gravity_shift,      // a = direction
        boss_damage,        // a = damage, b = hp left
        boss_telegraph,     // a = attack kind, b = pieces until
        boss_attack,        // a = attack kind, b = strength
        boss_phase,         // a = new phase
        boss_defeated,
        board_switched,     // a = new active board
        forced_switch_warn, // a = locks remaining
        cross_combo,        // a = count
        objective_progress, // a = objective index, b = current, c = target
        objective_complete,
        stage_clear,
        game_over,          // a = cause
        danger_on,
        danger_off,
        perfect_clear,
        twist_no_lines,     // a = mini
        piece_trail,        // a = column mask, b = highest landed row, c = rows dropped (hard-drop trail)
        twist_clear         // a = rows, b = 1 mini / 2 full T-TWIST, c = 1 when it was back-to-back
    };

    struct event
    {
        event_type type = event_type::none;
        int8_t board = 0;
        int16_t a = 0;
        int16_t b = 0;
        int16_t c = 0;
        int16_t d = 0;
    };

    class event_queue
    {
    public:
        static constexpr int capacity = 256;

        void clear()
        {
            _head = 0;
            _count = 0;
            _dropped = 0;
        }

        void push(event_type type, int board = 0, int a = 0, int b = 0, int c = 0, int d = 0)
        {
            if(_count == capacity)
            {
                // Oldest presentation event is dropped; gameplay never depends on it.
                _head = (_head + 1) % capacity;
                --_count;
                ++_dropped;
            }
            event& e = _events[(_head + _count) % capacity];
            e.type = type;
            e.board = int8_t(board);
            e.a = int16_t(a);
            e.b = int16_t(b);
            e.c = int16_t(c);
            e.d = int16_t(d);
            ++_count;
        }

        [[nodiscard]] bool pop(event& out)
        {
            if(! _count)
            {
                return false;
            }
            out = _events[_head];
            _head = (_head + 1) % capacity;
            --_count;
            return true;
        }

        [[nodiscard]] int size() const { return _count; }
        [[nodiscard]] int dropped() const { return _dropped; }

    private:
        event _events[capacity];
        int _head = 0;
        int _count = 0;
        int _dropped = 0;
    };
}

#endif
