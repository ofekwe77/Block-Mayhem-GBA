/*
 * BLOCK MAYHEM - one play session (a Classic game, an Adventure stage, a Chaos
 * Run encounter, a Dual Board Relay game, a Boss Rush fight...).
 *
 * The session owns one or two boards, the shared piece queue, score, heat,
 * Overheat, abilities, garbage queues, objectives, environmental hazards and
 * the boss. It advances one frame per update() call; pausing simply means not
 * calling update(), so every timer and boss countdown stops with it.
 *
 * Order of rules after every lock ("between pieces"):
 *   resolution -> scoring/heat/boss damage -> counters -> Overheat-failure
 *   garbage -> incoming garbage -> boss attack -> stage hazard -> board switch
 *   -> spawn. Nothing spawns while any board effect is unresolved.
 */
#ifndef BM_SESSION_H
#define BM_SESSION_H

#include "bm_content.h"
#include "bm_events.h"
#include "bm_player.h"

namespace bm
{
    enum class session_state : uint8_t
    {
        intro,          // READY / GO countdown (non-interactive)
        playing,
        transition,     // boss phase change / boss entrance: non-interactive, timers frozen
        won,
        lost
    };

    enum class loss_cause : uint8_t
    {
        none, top_out, crushed_by_garbage, objective_failed
    };

    struct session_config
    {
        game_mode mode = game_mode::classic;
        difficulty diff = difficulty::standard;
        uint32_t seed = 1;
        uint8_t start_level = 1;
        const stage_def* stage = nullptr;       // Adventure / Chaos Run encounter / Boss Rush
        const boss_def* boss = nullptr;
        uint8_t boss_id = 0;                    // 1..6 when a boss is present
        bool dual = false;
        uint8_t loadout[3] = { cfg::ability_laser, cfg::ability_time_freeze, cfg::ability_shield_matrix };
        uint8_t loadout_count = 0;              // 0 = abilities disabled
        bool specials = true;
        bool heat = true;
        modifiers mods;
        uint32_t start_score = 0;               // carried score (Chaos Run / Boss Rush)
        bool endless = false;                   // Arcade / Dual relay endless variants
        uint16_t lines_goal = 0;                // optional line target for non-stage modes (0 = none)
        bool skip_intro = false;
    };

    // Telegraphed action shared by hazards and boss attacks.
    struct pending_action
    {
        uint8_t source = 0;         // 0 none, 1 hazard, 2 boss
        uint8_t kind = 0;           // hazard_kind or attack_kind
        uint8_t strength = 0;
        uint8_t param = 0;
        int8_t board = 0;
        int8_t countdown = 0;       // locks until it fires
        int8_t columns[4] = { -1, -1, -1, -1 };     // telegraphed target columns
        int8_t gap = -1;            // garbage gap column
        bool marked = false;        // target cells chosen / marked
    };

    struct garbage_entry
    {
        int8_t rows = 0;
        int8_t gap = 0;
        int8_t countdown = 0;       // locks until insertion (0 = next between-pieces)
        coat_kind coat = coat_kind::none;
        bool hostile = true;        // shield / defense can stop it
        bool overheat = false;
    };

    struct objective_state
    {
        objective_def def;
        uint16_t value = 0;
        bool done = false;
    };

    struct session_stats
    {
        uint32_t frames = 0;
        uint16_t pieces = 0;
        uint16_t lines = 0;
        uint16_t singles = 0, doubles = 0, triples = 0, quads = 0;
        uint16_t twists = 0;
        uint16_t max_combo = 0;
        uint16_t max_chain = 0;
        uint16_t chains2 = 0;       // resolutions with 2+ waves
        uint16_t chains5 = 0;
        uint16_t bombs = 0, fires = 0, sparks = 0, magnets = 0, rainbows = 0;
        uint16_t magnet_moves = 0;
        uint16_t ice_broken = 0, armor_broken = 0;
        uint16_t specials_destroyed = 0;
        uint16_t overheats = 0, overheat_success = 0, overheat_fail = 0;
        uint16_t abilities_used = 0;
        uint16_t ability_uses[cfg::ability_count] = {};
        uint16_t laser_clears = 0;  // laser shots whose resolution cleared a row
        uint16_t gravity_uses = 0;
        uint16_t garbage_received = 0;
        uint16_t hostile_attacks_taken = 0;
        uint16_t shield_blocks = 0;
        uint16_t cross_combos = 0;
        uint16_t perfect_clears = 0;
        uint16_t boss_damage = 0;
        uint16_t last_stand = 0;    // recovered from critical height
        uint8_t max_height = 0;
        uint8_t second_chances_used = 0;
    };

    class session
    {
    public:
        void start(const session_config& config);
        void update(const input_frame& input);

        // ---------------------------------------------------------- read access for presentation
        [[nodiscard]] session_state state() const { return _state; }
        [[nodiscard]] loss_cause cause() const { return _cause; }
        [[nodiscard]] const session_config& config() const { return _config; }
        [[nodiscard]] int board_count() const { return _config.dual ? 2 : 1; }
        [[nodiscard]] const player& board_player(int i) const { return _players[i]; }
        [[nodiscard]] player& board_player(int i) { return _players[i]; }
        [[nodiscard]] int active_board() const { return _active; }
        [[nodiscard]] const piece_queue& queue() const { return _queue; }
        [[nodiscard]] event_queue& events() { return _events; }
        [[nodiscard]] uint32_t score() const { return _score; }
        [[nodiscard]] int level() const { return _level; }
        [[nodiscard]] int lines() const { return _stats.lines; }
        [[nodiscard]] int heat() const { return _heat; }
        [[nodiscard]] bool overheat_active() const { return _overheat_timer > 0; }
        [[nodiscard]] int overheat_timer() const { return _overheat_timer; }
        [[nodiscard]] int overheat_rows() const { return _overheat_rows; }
        [[nodiscard]] bool overheat_secured() const { return _overheat_secured; }
        [[nodiscard]] int intro_timer() const { return _intro_timer; }
        [[nodiscard]] int transition_timer() const { return _transition_timer; }
        [[nodiscard]] const session_stats& stats() const { return _stats; }
        [[nodiscard]] int gravity_frames() const;
        [[nodiscard]] bool danger(int board) const;

        // Abilities.
        [[nodiscard]] int loadout_count() const { return _config.loadout_count; }
        [[nodiscard]] int loadout(int slot) const { return _config.loadout[slot]; }
        [[nodiscard]] int selected_slot() const { return _selected; }
        [[nodiscard]] int ability_cost(int ability) const;
        [[nodiscard]] int ability_cooldown_max(int ability) const;
        [[nodiscard]] int cooldown(int ability) const { return _cooldowns[ability]; }
        [[nodiscard]] bool ability_ready(int ability) const;
        [[nodiscard]] bool targeting() const { return _targeting >= 0; }
        [[nodiscard]] int targeting_ability() const { return _targeting; }
        [[nodiscard]] int target_x() const { return _target_x; }
        [[nodiscard]] int target_y() const { return _target_y; }
        [[nodiscard]] int target_option() const { return _target_option; }
        [[nodiscard]] int targeting_timer() const { return _targeting_timer; }
        [[nodiscard]] bool shape_option_legal(int option) const;
        // Where the active piece would end up if transformed into the option's shape (false = illegal).
        [[nodiscard]] bool shape_option_preview(int option, piece_state& out) const;
        [[nodiscard]] piece_type shape_option(int option) const;
        [[nodiscard]] int shield_pieces() const { return _shield_pieces; }
        [[nodiscard]] int freeze_frames() const { return _freeze_frames; }
        [[nodiscard]] int chill_pieces() const { return _chill_pieces; }
        [[nodiscard]] int surge_pieces() const { return _surge_pieces; }

        // Garbage, hazards and boss.
        [[nodiscard]] int garbage_count(int board) const { return _garbage_count[board]; }
        [[nodiscard]] const garbage_entry& garbage(int board, int i) const { return _garbage[board][i]; }
        [[nodiscard]] int incoming_rows(int board) const;
        [[nodiscard]] int overheat_fail_rows() const;
        [[nodiscard]] const pending_action& hazard_action() const { return _hazard; }
        [[nodiscard]] const pending_action& boss_action() const { return _boss_action; }
        [[nodiscard]] bool has_boss() const { return _config.boss != nullptr; }
        [[nodiscard]] int boss_hp() const { return _boss_hp; }
        [[nodiscard]] int boss_max_hp() const { return _boss_max_hp; }
        [[nodiscard]] int boss_phase() const { return _boss_phase; }
        [[nodiscard]] int boss_attacks() const { return _boss_attacks_done; }
        [[nodiscard]] int boss_countdown() const { return _boss_countdown; }
        [[nodiscard]] bool boss_defeated() const { return _boss_defeated; }
        [[nodiscard]] int lava_pressure() const { return _lava; }
        [[nodiscard]] gravity_dir settle_direction() const { return _settle_dir; }
        [[nodiscard]] int forced_switch_locks() const { return _locks_on_board; }
        [[nodiscard]] int cross_window(int board) const { return _cross_window[board]; }

        // Objectives.
        [[nodiscard]] int objective_count() const { return _objective_count; }
        [[nodiscard]] const objective_state& objective(int i) const { return _objectives[i]; }
        [[nodiscard]] const objective_state& secondary() const { return _secondary; }
        [[nodiscard]] bool objectives_complete() const;

        // Diagnostics / test interface.
        void debug_add_heat(int amount) { _gain_heat_raw(amount); }
        void debug_set_boss_hp(int hp);
        [[nodiscard]] int validate() const;

    private:
        session_config _config;
        player _players[2];
        piece_queue _queue;
        event_queue _events;
        session_stats _stats;
        session_state _state = session_state::intro;
        loss_cause _cause = loss_cause::none;
        rng _hazard_rng;
        rng _boss_rng;
        rng _garbage_rng;
        uint32_t _score = 0;
        int _level = 1;
        int _active = 0;
        int _intro_timer = 0;
        int _transition_timer = 0;
        int _gravity_percent = 100;

        // Heat / Overheat.
        int _heat = 0;
        int _overheat_timer = 0;
        int _overheat_rows = 0;
        bool _overheat_secured = false;
        bool _overheat_fail_pending = false;

        // Abilities.
        int _selected = 0;
        int _cooldowns[cfg::ability_count] = {};
        int _targeting = -1;
        int _target_x = 0;
        int _target_y = 0;
        int _target_option = 0;
        int _targeting_timer = 0;
        int _shield_pieces = 0;
        int _freeze_frames = 0;
        int _chill_pieces = 0;
        int _surge_pieces = 0;
        int _pending_ability_damage = 0;
        int _pending_ability = -1;
        int _charged_cells = 0;

        // Garbage.
        garbage_entry _garbage[2][8];
        int _garbage_count[2] = {};

        // Between-pieces sequencing.
        int _between_step = 0;
        int _between_board = -1;
        bool _needs_counters = false;
        bool _last_resolution_from_lock = false;
        bool _last_cleared = false;
        bool _switch_pending = false;
        int _iron_defense_left = 0;

        // Stage hazards.
        pending_action _hazard;
        int _hazard_counter[2] = {};
        int _hazard_cycle = 0;
        int _lava = 0;
        gravity_dir _settle_dir = gravity_dir::down;
        int _reactive_armor_left = 0;

        // Boss.
        int _boss_hp = 0;
        int _boss_max_hp = 0;
        int _boss_phase = 0;
        int _boss_pattern_index = 0;
        int _boss_countdown = 0;
        int _boss_attacks_done = 0;
        bool _boss_defeated = false;
        int _gravity_flip = 0;
        pending_action _boss_action;

        // Dual board.
        int _locks_on_board = 0;
        int _cross_window[2] = {};
        int _total_locks = 0;

        // Objectives.
        objective_state _objectives[3];
        int _objective_count = 0;
        objective_state _secondary;
        int _danger_state[2] = {};
        bool _critical_seen = false;

        // Internals.
        void _setup_stage();
        void _update_playing(const input_frame& input);
        void _handle_ability_input(const input_frame& input);
        void _update_targeting(const input_frame& input);
        bool _activate(int ability);
        void _confirm_target();
        void _cancel_targeting();
        void _fire_laser(int column);
        void _fire_nuke(int cx, int cy);
        void _fire_vent();
        void _fire_chain_spark(int x, int y);
        void _fire_gravity(gravity_dir dir);
        bool _fire_shape_shift(int option);

        void _on_resolution(player& p);
        void _between_pieces(player& p);
        void _advance_counters(player& p);
        bool _insert_due_garbage(player& p);
        bool _run_boss_attack(player& p);
        bool _run_hazard(player& p);
        void _schedule_boss_attack();
        void _schedule_hazard();
        void _prepare_action(pending_action& a, player& p);
        void _execute_action(pending_action& a, player& p, bool& started_resolution);
        void _queue_garbage(int board, int rows, int gap, int countdown, coat_kind coat, bool hostile, bool overheat);
        void _apply_boss_damage(int damage);
        void _update_boss_phase();
        void _check_objectives();
        void _bump_objective(objective_kind kind, int amount, int param = 0);
        void _lose(loss_cause cause);
        void _win();
        void _switch_board(bool forced);
        void _gain_heat(int amount);
        void _gain_heat_raw(int amount);
        void _spend_heat(int amount);
        void _update_overheat();
        void _end_overheat();
        void _add_score(uint32_t points, int x = 0, int y = 0);
        [[nodiscard]] effect_mods _effect_mods() const;
        [[nodiscard]] int _stack_rows(int board) const;
        void _update_danger();
        [[nodiscard]] bool _any_resolving() const;
    };
}

#endif
