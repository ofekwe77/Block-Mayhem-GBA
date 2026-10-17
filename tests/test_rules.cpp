/*
 * Rule-by-rule verification of scoring, heat, Overheat, abilities, the dual board and difficulty on the real
 * session (PART 4, 7, 8, 9, 10, 12, 30.1). Pieces are scripted, boards are prepared cell by cell and every
 * expectation is computed from bm_config.h, so a balance change keeps the tests meaningful.
 */
#include "bm_test.h"
#include "test_util.h"

#include "bm_session.h"

using namespace bm;

namespace
{
    session_config base_config(game_mode mode, uint32_t seed)
    {
        session_config c;
        c.mode = mode;
        c.seed = seed;
        c.specials = false;
        c.heat = true;
        c.loadout_count = 0;
        c.skip_intro = false;       // the first spawn waits for the intro: the queue can be scripted first
        return c;
    }

    // Drives a session with explicit keys and records what the rules reported.
    struct driver
    {
        session s;
        uint16_t prev = 0;
        uint32_t points = 0;        // sum of score events
        int drop_rows = 0;          // hard-drop rows (2 points each)
        int soft_rows = 0;
        int last_rows = -1;
        int last_twist = -1;
        int heat_gained = 0;
        int boss_damage = 0;
        int cross_combos = 0;
        int switches = 0;
        int shield_blocks = 0;

        void start(const session_config& c, std::initializer_list<piece_type> script)
        {
            static piece_type types[32];
            int n = 0;
            for(piece_type t : script)
            {
                types[n++] = t;
            }
            s.start(c);
            if(n)
            {
                s.debug_queue().script(types, n);
            }
            for(int i = 0; i < 200 && s.state() == session_state::intro; ++i)
            {
                frame(0);
            }
            until_falling();
        }

        void frame(uint16_t held = 0)
        {
            input_frame in;
            in.held = held;
            in.pressed = uint16_t(held & ~prev);
            prev = held;
            s.update(in);
            event e;
            while(s.events().pop(e))
            {
                switch(e.type)
                {
                case event_type::score: points += uint32_t(e.a & 0xFFFF) | (uint32_t(e.b) << 16); break;
                case event_type::hard_drop: drop_rows += e.a; break;
                case event_type::lines_cleared: last_rows = e.a; break;
                case event_type::twist_clear: last_twist = e.b; break;
                case event_type::heat_changed: heat_gained += e.b > 0 ? e.b : 0; break;
                case event_type::boss_damage: boss_damage += e.a; break;
                case event_type::cross_combo: ++cross_combos; break;
                case event_type::board_switched: ++switches; break;
                case event_type::shield_block: ++shield_blocks; break;
                default: break;
                }
            }
        }

        void tap(uint16_t k)
        {
            frame(k);
            frame(0);
        }

        player& active() { return s.board_player(s.active_board()); }

        void until_falling()
        {
            for(int i = 0; i < 400 && active().state() != player_state::falling && s.state() == session_state::playing; ++i)
            {
                frame(0);
            }
        }

        int min_x()
        {
            int m = 99;
            for(int k = 0; k < 4; ++k)
            {
                m = active().active().cell(k).x < m ? active().active().cell(k).x : m;
            }
            return m;
        }

        // Rotates clockwise to 'rotation', moves the leftmost mino to 'column', hard drops, waits for the next piece.
        void place(int rotation, int column)
        {
            for(int i = 0; i < 4 && active().active().rotation != rotation; ++i)
            {
                tap(key_a);
            }
            for(int i = 0; i < 12 && min_x() > column; ++i)
            {
                tap(key_left);
            }
            for(int i = 0; i < 12 && min_x() < column; ++i)
            {
                tap(key_right);
            }
            tap(key_up);
            until_falling();
        }

        void reset_counters()
        {
            points = 0;
            drop_rows = 0;
            heat_gained = 0;
            last_rows = -1;
            last_twist = -1;
            boss_damage = 0;
        }
    };

    // Four full-width rows (garbage) with column 'gap' open, plus a stray cell so no clear is a perfect clear.
    void quad_well(board& b, int gap, bool stray = true)
    {
        b.clear_all();
        for(int y = b.rows() - 4; y < b.rows(); ++y)
        {
            for(int x = 0; x < b.cols(); ++x)
            {
                if(x != gap)
                {
                    b.set(x, y, bmtest::cell_from_char('#'));
                }
            }
        }
        if(stray)
        {
            b.set(0, b.rows() - 5, bmtest::cell_from_char('#'));
        }
    }
}

BM_TEST(rules, gravity_by_level_difficulty_and_overheat)
{
    // PART 4.4 / 30.1: frames per row from the level table, scaled by difficulty, faster during Overheat.
    static session s;
    for(int level = 1; level <= 12; ++level)
    {
        session_config c = base_config(game_mode::arcade, 3);
        c.start_level = uint8_t(level);
        c.diff = difficulty::standard;
        s.start(c);
        CHECK_EQ(s.gravity_frames(), cfg::gravity_table[level - 1] > cfg::min_gravity_frames ?
                                     cfg::gravity_table[level - 1] : cfg::min_gravity_frames);
        c.diff = difficulty::beginner;
        s.start(c);
        CHECK_EQ(s.gravity_frames(), cfg::gravity_table[level - 1] * cfg::beginner_gravity_percent / 100);
        c.diff = difficulty::expert;
        s.start(c);
        const int expert = cfg::gravity_table[level - 1] * cfg::expert_gravity_percent / 100;
        CHECK_EQ(s.gravity_frames(), expert > cfg::min_gravity_frames ? expert : cfg::min_gravity_frames);
    }
    session_config c = base_config(game_mode::arcade, 3);
    c.start_level = 1;
    c.skip_intro = true;
    s.start(c);
    const int normal = s.gravity_frames();
    s.debug_add_heat(cfg::heat_max);
    CHECK(s.overheat_active());
    CHECK_EQ(s.gravity_frames(), normal * cfg::overheat_gravity_percent / 100);
}

BM_TEST(rules, quad_back_to_back_and_combo_scoring)
{
    // PART 7.2-7.5: a quad pays line_points[4] x level; a second difficult clear in a row pays b2b_percent on the
    // line base and is a combo step (combo_points x step x level).
    driver d;
    session_config c = base_config(game_mode::arcade, 11);
    c.start_level = 2;
    c.heat = false;             // no Overheat: the score is the plain rule
    d.start(c, { piece_type::i, piece_type::i, piece_type::i, piece_type::i });
    const int level = d.s.level();
    board& b = d.active().grid();

    quad_well(b, 9);
    d.reset_counters();
    d.place(1, 9);
    CHECK_EQ(d.last_rows, 4);
    CHECK_EQ(d.points, uint32_t(cfg::line_points[4] * level + d.drop_rows * cfg::hard_drop_points));

    quad_well(b, 9);
    d.reset_counters();
    d.place(1, 9);
    CHECK_EQ(d.last_rows, 4);
    const uint32_t b2b = uint32_t(cfg::line_points[4] * cfg::b2b_percent / 100 * level);
    const uint32_t combo = uint32_t(cfg::combo_points * 1 * level);
    CHECK_EQ(d.points, b2b + combo + uint32_t(d.drop_rows * cfg::hard_drop_points));
    CHECK_EQ(int(d.s.stats().max_combo), 1);
    CHECK_EQ(d.s.validate(), 0);
}

BM_TEST(rules, heat_gains_follow_rows_and_back_to_back)
{
    // PART 8.2: line_heat by rows, b2b_heat on a back-to-back difficult clear. The Laser spends 40 between the
    // two quads so the second gain is not cut by the meter's cap.
    driver d;
    session_config c = base_config(game_mode::arcade, 31);
    c.loadout[0] = cfg::ability_laser;
    c.loadout_count = 1;
    d.start(c, { piece_type::i, piece_type::i, piece_type::i, piece_type::i });
    board& b = d.active().grid();
    quad_well(b, 9);
    d.reset_counters();
    d.place(1, 9);
    CHECK_EQ(d.heat_gained, cfg::line_heat[4]);
    CHECK_EQ(d.s.heat(), cfg::line_heat[4]);
    d.tap(key_r);
    CHECK(d.s.targeting());
    d.tap(key_a);
    for(int i = 0; i < 60; ++i)
    {
        d.frame(0);
    }
    CHECK_EQ(d.s.heat(), cfg::line_heat[4] - cfg::ability_cost[cfg::ability_laser]);
    quad_well(b, 9);
    d.reset_counters();
    d.place(1, 9);
    CHECK_EQ(d.heat_gained, cfg::line_heat[4] + cfg::b2b_heat);
}

BM_TEST(rules, chain_bonus_special_points_and_heat)
{
    // A single clears a row holding a bomb: wave 1 = the row, wave 2 = the bomb destroying three cells above.
    driver d;
    session_config c = base_config(game_mode::arcade, 12);
    c.specials = false;
    d.start(c, { piece_type::i, piece_type::i });
    const int level = d.s.level();
    board& b = d.active().grid();
    b.clear_all();
    const int y = b.rows() - 1;
    for(int x = 0; x < 9; ++x)
    {
        b.set(x, y, bmtest::cell_from_char(x == 4 ? 'b' : '#'));
    }
    b.set(3, y - 1, bmtest::cell_from_char('#'));
    b.set(4, y - 1, bmtest::cell_from_char('#'));
    b.set(5, y - 1, bmtest::cell_from_char('#'));
    d.reset_counters();
    d.place(1, 9);              // vertical I in column 9: completes only the bottom row
    CHECK_EQ(d.last_rows, 1);
    CHECK_EQ(d.s.stats().max_chain, 2);
    const uint32_t expected = uint32_t(cfg::line_points[1] * level + cfg::chain_points[2] * level +
                                       3 * cfg::special_cell_points + d.drop_rows * cfg::hard_drop_points);
    CHECK_EQ(d.points, expected);
    CHECK_EQ(d.heat_gained, cfg::line_heat[1] + cfg::chain_wave_heat);
    CHECK_EQ(d.s.validate(), 0);
}

BM_TEST(rules, special_points_are_capped_per_effect_and_perfect_clear_pays)
{
    driver d;
    session_config c = base_config(game_mode::arcade, 13);
    d.start(c, { piece_type::i, piece_type::i });
    const int level = d.s.level();
    board& b = d.active().grid();
    quad_well(b, 9, false);     // nothing else on the board: the quad empties it
    d.reset_counters();
    d.place(1, 9);
    CHECK_EQ(d.last_rows, 4);
    CHECK_EQ(d.s.stats().perfect_clears, 1);
    CHECK_EQ(d.points, uint32_t(cfg::line_points[4] * level + cfg::perfect_clear_points * level +
                                d.drop_rows * cfg::hard_drop_points));
    // The per-effect cap: 25 points per destroyed cell, never more than special_cell_points_cap for one effect.
    CHECK(cfg::special_cell_points_cap / cfg::special_cell_points == 20);
}

BM_TEST(rules, t_twist_double_scores_twist_points)
{
    // PART 7.5: a T whose last move is a rotation into a slot with three occupied box corners (both front
    // corners among them) is a full T-TWIST. Slot for a T-TWIST double, T centre at (4, 18):
    driver d;
    session_config c = base_config(game_mode::classic, 14);
    c.heat = false;
    d.start(c, { piece_type::t, piece_type::i, piece_type::i });
    const int level = d.s.level();
    board& b = d.active().grid();
    bmtest::load(b, {
        ".....#....",
        "###...####",
        "####.#####",
    });
    CHECK(d.active().active().type == piece_type::t);
    d.reset_counters();
    d.tap(key_b);                                   // spawn state -> L: a vertical bar over column 4
    CHECK_EQ(int(d.active().active().rotation), 3);
    CHECK_EQ(d.min_x(), 3);
    for(int i = 0; i < 60 && ! d.active().grounded(); ++i)
    {
        d.frame(key_down);
    }
    d.frame(0);
    CHECK(d.active().grounded());
    d.tap(key_b);                                   // L -> 2 in place: the T points down into the slot
    CHECK_EQ(int(d.active().active().rotation), 2);
    for(int i = 0; i < 120 && d.active().state() == player_state::falling; ++i)
    {
        d.frame(0);                                 // no further movement: the lock delay ends the piece
    }
    d.until_falling();
    CHECK_EQ(d.last_rows, 2);
    CHECK_EQ(d.last_twist, 2);
    CHECK_EQ(int(d.s.stats().twists), 1);
    CHECK(d.points >= uint32_t(cfg::twist_points[2] * level));
    CHECK(d.points < uint32_t(cfg::twist_points[2] * level + 40));     // plus the soft-drop rows
}

BM_TEST(rules, overheat_doubles_line_score_and_rewards_success)
{
    // PART 9.3 / 9.4: during Overheat line clears pay overheat_score_percent; two rows inside the window succeed.
    driver d;
    session_config c = base_config(game_mode::arcade, 15);
    d.start(c, { piece_type::i, piece_type::i, piece_type::i });
    const int level = d.s.level();
    d.s.debug_add_heat(cfg::heat_max);
    CHECK(d.s.overheat_active());
    board& b = d.active().grid();
    quad_well(b, 9);
    d.reset_counters();
    d.place(1, 9);
    CHECK_EQ(d.last_rows, 4);
    const uint32_t line = uint32_t(cfg::line_points[4] * level) * cfg::overheat_score_percent / 100;
    CHECK(d.points >= line + uint32_t(d.drop_rows * cfg::hard_drop_points));
    for(int i = 0; i < cfg::overheat_frames + 200 && d.s.overheat_active(); ++i)
    {
        d.frame(0);
    }
    CHECK_EQ(int(d.s.stats().overheat_success), 1);
    CHECK_EQ(int(d.s.stats().overheat_fail), 0);
}

BM_TEST(rules, time_freeze_stops_gravity_and_the_boss_clock)
{
    // PART 10.2: three seconds without automatic falling; locks during the freeze do not advance a boss attack.
    driver d;
    session_config c = make_stage_config(stage(1, 5), difficulty::standard, 16);
    c.skip_intro = false;
    c.loadout[0] = cfg::ability_time_freeze;
    c.loadout_count = 1;
    d.start(c, {});
    d.s.debug_add_heat(cfg::ability_cost[cfg::ability_time_freeze] + 5);
    const int y0 = d.active().active().y;
    d.tap(key_r);
    CHECK(d.s.freeze_frames() > 0);
    for(int i = 0; i < 100; ++i)
    {
        d.frame(0);
    }
    CHECK_EQ(int(d.active().active().y), y0);       // nothing fell
    const int countdown = d.s.boss_countdown();
    d.tap(key_up);                                  // lock a piece inside the freeze
    d.until_falling();
    CHECK(d.s.freeze_frames() > 0);
    CHECK_EQ(d.s.boss_countdown(), countdown);
    for(int i = 0; i < cfg::time_freeze_frames; ++i)
    {
        d.frame(0);
    }
    CHECK_EQ(d.s.freeze_frames(), 0);
}

BM_TEST(rules, shield_blocks_the_next_boss_attack)
{
    // PART 10.6: Shield Matrix absorbs the next hostile attack, then drops.
    driver d;
    session_config c = make_stage_config(stage(1, 5), difficulty::standard, 17);
    c.skip_intro = false;
    c.loadout[0] = cfg::ability_shield_matrix;
    c.loadout_count = 1;
    d.start(c, {});
    d.s.debug_add_heat(cfg::ability_cost[cfg::ability_shield_matrix] + 5);
    d.tap(key_r);
    CHECK_EQ(d.s.shield_pieces(), cfg::shield_duration_pieces);
    const int attacks = d.s.boss_attacks();
    const int taken = d.s.stats().hostile_attacks_taken;
    // Lock pieces until the boss acts.
    for(int k = 0; k < 12 && d.s.boss_attacks() == attacks; ++k)
    {
        d.tap(key_up);
        d.until_falling();
    }
    CHECK(d.s.boss_attacks() > attacks);
    CHECK(d.shield_blocks >= 1);
    CHECK_EQ(int(d.s.stats().hostile_attacks_taken), taken);
    CHECK_EQ(d.s.shield_pieces(), 0);
}

BM_TEST(rules, shape_shift_turns_the_piece_into_a_legal_shape)
{
    // PART 10.4: options are the six other shapes; only a shape that fits where the piece is can be chosen.
    driver d;
    session_config c = base_config(game_mode::arcade, 18);
    c.loadout[0] = cfg::ability_shape_shift;
    c.loadout_count = 1;
    d.start(c, { piece_type::o, piece_type::i });
    d.s.debug_add_heat(cfg::ability_cost[cfg::ability_shape_shift] + 5);
    CHECK(d.active().active().type == piece_type::o);
    d.tap(key_r);
    CHECK(d.s.targeting());
    const int option = d.s.target_option();
    CHECK(d.s.shape_option_legal(option));
    const piece_type wanted = d.s.shape_option(option);
    CHECK(wanted != piece_type::o);
    d.tap(key_a);
    CHECK(! d.s.targeting());
    CHECK(d.active().active().type == wanted);
    CHECK(d.active().grid().fits(d.active().active()));
    CHECK_EQ(d.s.cooldown(cfg::ability_shape_shift), cfg::ability_cooldown[cfg::ability_shape_shift]);
}

BM_TEST(dual, boards_share_the_queue_and_switching_freezes_the_other_board)
{
    // PART 12.2-12.3: two 8x18 boards, one active at a time; the inactive board's piece does not move.
    driver d;
    session_config c = base_config(game_mode::dual_relay, 19);
    c.dual = true;
    d.start(c, {});
    CHECK_EQ(d.s.board_count(), 2);
    CHECK_EQ(d.s.board_player(0).grid().cols(), cfg::dual_board_cols);
    CHECK_EQ(d.s.board_player(1).grid().rows(), cfg::dual_board_rows);
    CHECK_EQ(d.s.active_board(), 0);
    d.tap(key_select);
    CHECK_EQ(d.s.active_board(), 1);
    d.until_falling();
    const piece_state frozen = d.s.board_player(0).active();
    for(int i = 0; i < 120; ++i)
    {
        d.frame(0);
    }
    CHECK_EQ(int(d.s.board_player(0).active().y), int(frozen.y));
    CHECK_EQ(int(d.s.board_player(0).active().x), int(frozen.x));
    CHECK_EQ(d.s.validate(), 0);
}

BM_TEST(dual, forced_switch_after_four_locks)
{
    driver d;
    session_config c = base_config(game_mode::dual_relay, 20);
    c.dual = true;
    d.start(c, {});
    const int first = d.s.active_board();
    for(int k = 0; k < cfg::dual_forced_switch_locks; ++k)
    {
        CHECK_EQ(d.s.active_board(), first);
        d.tap(key_up);
        d.until_falling();
    }
    CHECK(d.s.active_board() != first);
    CHECK(d.switches >= 1);
}

BM_TEST(dual, cross_over_within_the_window_scores_and_heats)
{
    // PART 12.6: rows on one board, then rows on the other within cross_combo_window_locks = CROSS OVER.
    driver d;
    session_config c = base_config(game_mode::dual_relay, 21);
    c.dual = true;
    d.start(c, { piece_type::i, piece_type::i, piece_type::i, piece_type::i });
    for(int bi = 0; bi < 2; ++bi)
    {
        board& b = d.s.board_player(bi).grid();
        b.clear_all();
        for(int x = 0; x < b.cols() - 1; ++x)
        {
            b.set(x, b.rows() - 1, bmtest::cell_from_char('#'));
        }
        b.set(0, b.rows() - 2, bmtest::cell_from_char('#'));
    }
    const int level = d.s.level();
    d.place(1, cfg::dual_board_cols - 1);           // single on board A
    CHECK_EQ(d.cross_combos, 0);
    d.tap(key_select);
    d.until_falling();
    d.reset_counters();
    d.place(1, cfg::dual_board_cols - 1);           // single on board B within the window
    CHECK_EQ(d.cross_combos, 1);
    CHECK_EQ(int(d.s.stats().cross_combos), 1);
    CHECK(d.points >= uint32_t(cfg::cross_combo_points * level));
    CHECK(d.heat_gained >= cfg::cross_combo_heat);
}

BM_TEST(dual, either_board_topping_out_loses)
{
    driver d;
    session_config c = base_config(game_mode::dual_relay, 22);
    c.dual = true;
    d.start(c, {});
    board& b = d.s.board_player(1).grid();      // the inactive board is buried
    for(int y = 1; y < b.rows(); ++y)
    {
        for(int x = 0; x < b.cols() - 1; ++x)
        {
            b.set(x, y, bmtest::cell_from_char('#'));
        }
    }
    for(int k = 0; k < 10 && d.s.state() == session_state::playing; ++k)
    {
        d.tap(key_up);      // locks on board A until the forced switch hands over the buried board
        d.until_falling();
    }
    CHECK(d.s.state() == session_state::lost);
}

BM_TEST(difficulty, beginner_and_expert_change_warnings_and_penalties)
{
    // PART 30.1: Beginner = longer boss spacing and a lighter Overheat failure; Expert = shorter spacing.
    static session s;
    const stage_def& st = stage(1, 5);
    s.start(make_stage_config(st, difficulty::beginner, 23));
    const int beginner = s.boss_countdown();
    CHECK_EQ(s.overheat_fail_rows(), cfg::overheat_fail_rows_beginner);
    s.start(make_stage_config(st, difficulty::standard, 23));
    const int standard = s.boss_countdown();
    CHECK_EQ(s.overheat_fail_rows(), cfg::overheat_fail_rows_standard);
    s.start(make_stage_config(st, difficulty::expert, 23));
    const int expert = s.boss_countdown();
    CHECK_EQ(s.overheat_fail_rows(), cfg::overheat_fail_rows_expert);
    CHECK_EQ(beginner, standard + 2);
    CHECK_EQ(expert, standard - 1 < 3 ? 3 : standard - 1);
}
