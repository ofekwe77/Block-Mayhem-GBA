/*
 * BLOCK MAYHEM - falling-piece controller for one board.
 *
 * Owns the board, the active piece, hold, gravity, lock delay and the board's
 * resolver. The session supplies frame parameters (gravity speed, whether
 * input is accepted) and the shared piece queue, and decides when the next
 * piece may spawn (the "between pieces" hook used for garbage and hazards).
 */
#ifndef BM_PLAYER_H
#define BM_PLAYER_H

#include "bm_board.h"
#include "bm_events.h"
#include "bm_randomizer.h"
#include "bm_resolver.h"

namespace bm
{
    // Shared upcoming-piece queue (one per session, shared by both boards in dual mode).
    class piece_queue
    {
    public:
        void init(uint32_t seed, const special_config& specials, uint32_t special_seed);
        [[nodiscard]] queued_piece pop();
        [[nodiscard]] const queued_piece& peek(int index) const { return _items[(_head + index) % cfg::queue_length]; }
        [[nodiscard]] bag_randomizer& bag() { return _bag; }
        [[nodiscard]] special_distributor& specials() { return _specials; }
        // Forces a special onto the next generated piece that has none (Shape Shift keeps specials, scripted events).
        void replace_peek(int index, const queued_piece& piece) { _items[(_head + index) % cfg::queue_length] = piece; }
        // Overrides the shape sequence (puzzle stages / tests). Pieces past the script return to the bag.
        void script(const piece_type* types, int count);

    private:
        bag_randomizer _bag;
        special_distributor _specials;
        queued_piece _items[cfg::queue_length];
        int _head = 0;
        const piece_type* _script = nullptr;
        int _script_count = 0;
        int _script_index = 0;

        [[nodiscard]] queued_piece _generate();
    };

    struct frame_params
    {
        int gravity_frames = 48;    // frames per row; <= 0 disables automatic falling (Time Freeze)
        bool accept_input = true;   // false while targeting / paused / inactive board
        bool soft_drop_allowed = true;
        bool frozen = false;        // piece held completely still (ability targeting): no gravity, no lock timer
    };

    enum class twist_kind : uint8_t
    {
        none,
        mini,
        full
    };

    // What happened at the most recent lock (consumed by the session for scoring).
    struct lock_info
    {
        piece_type type = piece_type::none;
        twist_kind twist = twist_kind::none;
        int hard_drop_rows = 0;
        int soft_drop_rows = 0;
        int lowest_row = 0;
        int highest_row = 0;
        bool topped_out = false;    // lock out: the piece locked entirely in the hidden area
    };

    enum class player_state : uint8_t
    {
        waiting_spawn,  // board stable, waiting for the session to allow the next spawn
        spawn_delay,
        falling,
        resolving,
        topped_out
    };

    class player
    {
    public:
        void init(int index, int cols, int rows, int hidden, piece_queue* queue, event_queue* events);

        [[nodiscard]] board& grid() { return _board; }
        [[nodiscard]] const board& grid() const { return _board; }
        [[nodiscard]] resolver& resolution() { return _resolver; }
        [[nodiscard]] player_state state() const { return _state; }
        [[nodiscard]] const piece_state& active() const { return _piece; }
        [[nodiscard]] bool has_piece() const { return _state == player_state::falling; }
        [[nodiscard]] piece_type held() const { return _hold.type; }
        [[nodiscard]] const queued_piece& held_piece() const { return _hold; }
        [[nodiscard]] bool can_hold() const { return _can_hold && _hold_enabled; }
        [[nodiscard]] bool hold_enabled() const { return _hold_enabled; }
        // NO HOLD challenge: the hold slot stays locked for the whole session.
        void set_hold_enabled(bool enabled) { _hold_enabled = enabled; }
        [[nodiscard]] int ghost_y() const;
        [[nodiscard]] int lock_timer() const { return _lock_timer; }
        [[nodiscard]] bool grounded() const;
        [[nodiscard]] int index() const { return _index; }

        // Per-frame update. Returns true when a resolution finished this frame (session must score it).
        bool update(const input_frame& input, const frame_params& params, const effect_mods& mods);

        // Session hooks.
        [[nodiscard]] bool resolution_finished() const { return _resolution_finished; }
        [[nodiscard]] const lock_info& last_lock() const { return _last_lock; }
        [[nodiscard]] bool lock_started_resolution() const { return _lock_resolution; }
        void acknowledge_resolution() { _resolution_finished = false; }
        void allow_spawn(int delay_frames);
        bool spawn_now();                   // returns false on block out (top-out)
        void start_external_resolution(const effect_mods& mods);   // after ability/hazard hits applied via resolution().hit()
        void start_compaction(gravity_dir dir, const effect_mods& mods);
        void set_topped_out();

        // Replaces the active piece's shape (Shape Shift). Returns false if illegal.
        bool transform_active(piece_type type, bool apply = true, piece_state* preview = nullptr);

        // SECOND CHANCE: leaves the topped-out state; the caller has cleared space on the board.
        void revive();

        // Combo / back-to-back bookkeeping (per board).
        int combo = 0;          // consecutive clearing locks (0 = none yet)
        int b2b = 0;            // consecutive difficult clears (0 = none)
        int last_lock_rows = 0;

    private:
        board _board;
        resolver _resolver;
        effect_mods _mods;
        piece_queue* _queue = nullptr;
        event_queue* _events = nullptr;
        piece_state _piece;
        piece_state _saved_piece;
        bool _resume_after_resolution = false;
        queued_piece _hold;
        lock_info _last_lock;
        player_state _state = player_state::waiting_spawn;
        int _index = 0;
        int _gravity_counter = 0;
        int _soft_counter = 0;
        int _das_dir = 0;
        int _das_timer = 0;
        int _lock_timer = 0;
        int _lock_resets = 0;
        int _lowest_y = 0;
        int _delay = 0;
        int _spawn_delay = 0;
        int _soft_rows = 0;
        bool _can_hold = true;
        bool _hold_enabled = true;
        bool _last_rotate = false;
        int _last_kick = 0;
        bool _resolution_finished = false;
        bool _lock_resolution = false;

        bool _spawn(const queued_piece& next);
        bool _try_move(int dx, int dy);
        bool _try_rotate(int dir);
        void _lock(bool hard);
        void _hold_piece();
        void _handle_horizontal(const input_frame& input);
        void _on_successful_action();
        [[nodiscard]] twist_kind _detect_twist() const;
    };
}

#endif
