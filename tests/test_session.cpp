/*
 * Session-level tests: scoring, heat, Overheat, abilities, determinism and
 * long automated games with invariant checks every frame (PART 39.3, 55).
 */
#include "bm_test.h"
#include "test_util.h"

#include "bm_bot.h"
#include "bm_session.h"

using namespace bm;

namespace
{
    session_config classic_config(uint32_t seed)
    {
        session_config c;
        c.mode = game_mode::classic;
        c.seed = seed;
        c.specials = false;
        c.heat = false;
        c.loadout_count = 0;
        c.skip_intro = true;
        return c;
    }

    session_config arcade_config(uint32_t seed)
    {
        session_config c;
        c.mode = game_mode::arcade;
        c.seed = seed;
        c.specials = true;
        c.heat = true;
        c.loadout[0] = cfg::ability_laser;
        c.loadout[1] = cfg::ability_emergency_vent;
        c.loadout[2] = cfg::ability_shield_matrix;
        c.loadout_count = 3;
        c.skip_intro = true;
        return c;
    }

    struct play_result
    {
        uint32_t score = 0;
        int lines = 0;
        int pieces = 0;
        int frames = 0;
        int invalid = 0;
        bool lost = false;
    };

    play_result play(const session_config& cfg, bot_strategy strategy, int max_frames, uint32_t bot_seed = 7)
    {
        static session s;
        static bot b;
        s.start(cfg);
        b.reset(strategy, bot_seed);
        play_result r;
        for(int f = 0; f < max_frames && s.state() != session_state::lost && s.state() != session_state::won; ++f)
        {
            s.update(b.next(s));
            event e;
            while(s.events().pop(e)) {}
            const int v = s.validate();
            if(v && ! r.invalid)
            {
                r.invalid = v;
                bmtest::note("invalid state %d at frame %d", v, f);
            }
            r.frames = f;
        }
        r.score = s.score();
        r.lines = s.lines();
        r.pieces = s.stats().pieces;
        r.lost = s.state() == session_state::lost;
        return r;
    }

    // Plays one frame with explicit keys (edge detection handled here).
    struct driver
    {
        session s;
        uint16_t prev = 0;

        void frame(uint16_t held = 0)
        {
            input_frame in;
            in.held = held;
            in.pressed = uint16_t(held & ~prev);
            prev = held;
            s.update(in);
            event e;
            while(s.events().pop(e)) {}
        }

        void tap(uint16_t k)
        {
            frame(k);
            frame(0);
        }

        void until_falling()
        {
            for(int i = 0; i < 300 && s.board_player(s.active_board()).state() != player_state::falling &&
                s.state() == session_state::playing; ++i)
            {
                frame(0);
            }
        }
    };
}

BM_TEST(session, classic_bot_game_is_valid_and_scores)
{
    const play_result r = play(classic_config(11), bot_strategy::hole_min, 60 * 60 * 3);
    bmtest::note("classic hole_min: score %u lines %d pieces %d frames %d lost %d", r.score, r.lines, r.pieces,
                 r.frames, int(r.lost));
    CHECK_EQ(r.invalid, 0);
    CHECK(r.lines > 20);
    CHECK(r.score > 0);
}

BM_TEST(session, deterministic_replay)
{
    const play_result a = play(arcade_config(1234), bot_strategy::resource_aware, 60 * 60 * 2);
    const play_result b = play(arcade_config(1234), bot_strategy::resource_aware, 60 * 60 * 2);
    CHECK_EQ(a.score, b.score);
    CHECK_EQ(a.lines, b.lines);
    CHECK_EQ(a.pieces, b.pieces);
    const play_result c = play(arcade_config(1235), bot_strategy::resource_aware, 60 * 60 * 2);
    CHECK(c.score != a.score || c.pieces != a.pieces);
}

BM_TEST(session, single_line_scores_100_at_level_1)
{
    driver d;
    session_config c = classic_config(5);
    d.s.start(c);
    // Prepare a board with a 4-wide gap at the right of the bottom row: I piece (horizontal) completes it.
    board& b = d.s.board_player(0).grid();
    auto prepare = [&b]()
    {
        b.clear_all();
        for(int x = 0; x < 6; ++x) { b.set(x, 19, bmtest::cell_from_char('#')); }
        b.set(0, 18, bmtest::cell_from_char('#'));     // keeps the clear from being a perfect clear
    };
    prepare();
    // Find an I piece: drop pieces until the active piece is I.
    for(int k = 0; k < 14 && d.s.board_player(0).active().type != piece_type::i; ++k)
    {
        d.tap(key_up);
        d.until_falling();
        prepare();
    }
    CHECK(d.s.board_player(0).active().type == piece_type::i);
    const uint32_t before = d.s.score();
    for(int i = 0; i < 5; ++i) { d.tap(key_right); }
    d.tap(key_up);
    d.until_falling();
    const uint32_t gained = d.s.score() - before;
    // 100 for the single plus 2 points per hard-dropped row (17 rows).
    CHECK_EQ(int(gained), 100 + 2 * 17);
    CHECK_EQ(d.s.lines(), 1);
}

BM_TEST(heat, overheat_consumes_meter_and_caps_gain)
{
    driver d;
    d.s.start(arcade_config(3));
    d.s.debug_add_heat(60);
    CHECK_EQ(d.s.heat(), 60);
    CHECK(! d.s.overheat_active());
    d.s.debug_add_heat(60);     // reaches 100 -> Overheat, meter consumed
    CHECK(d.s.overheat_active());
    CHECK_EQ(d.s.heat(), 0);
    d.s.debug_add_heat(100);    // gain during Overheat is capped
    CHECK_EQ(d.s.heat(), cfg::overheat_heat_keep_cap);
    CHECK(d.s.overheat_active());
    CHECK_EQ(d.s.validate(), 0);
}

BM_TEST(heat, overheat_failure_queues_visible_garbage)
{
    driver d;
    session_config c = arcade_config(9);
    c.specials = false;
    d.s.start(c);
    d.s.debug_add_heat(100);
    CHECK(d.s.overheat_active());
    // Do nothing useful (pieces fall naturally); the timer must expire without two rows.
    for(int f = 0; f < cfg::overheat_frames + 200 && d.s.overheat_active(); ++f)
    {
        d.frame(0);
    }
    CHECK(! d.s.overheat_active());
    CHECK_EQ(d.s.stats().overheat_fail, 1);
    // The garbage is queued (or already inserted at the next lock).
    CHECK(d.s.incoming_rows(0) > 0 || d.s.stats().garbage_received > 0);
}

BM_TEST(heat, overheat_timer_stops_while_paused)
{
    driver d;
    d.s.start(arcade_config(4));
    d.s.debug_add_heat(100);
    const int t = d.s.overheat_timer();
    // "Pausing" = the game does not call update(). Nothing may change.
    CHECK_EQ(d.s.overheat_timer(), t);
    d.frame(0);
    CHECK_EQ(d.s.overheat_timer(), t - 1);
}

BM_TEST(abilities, denied_without_heat_and_exact_deduction)
{
    driver d;
    d.s.start(arcade_config(21));
    d.until_falling();
    d.tap(key_r);   // laser selected, no heat
    CHECK(! d.s.targeting());
    d.s.debug_add_heat(45);
    d.tap(key_r);
    CHECK(d.s.targeting());
    CHECK_EQ(d.s.heat(), 45);   // nothing spent until confirmed
    d.tap(key_b);               // cancel
    CHECK(! d.s.targeting());
    CHECK_EQ(d.s.heat(), 45);
    d.tap(key_r);
    d.tap(key_a);               // confirm
    CHECK(! d.s.targeting());
    CHECK_EQ(d.s.heat(), 45 - cfg::ability_cost[cfg::ability_laser]);
    CHECK_EQ(d.s.cooldown(cfg::ability_laser), cfg::ability_cooldown[cfg::ability_laser]);
    CHECK_EQ(d.s.stats().abilities_used, 1);
    // On cooldown: denied even with heat.
    d.s.debug_add_heat(50);
    d.until_falling();
    d.tap(key_r);
    CHECK(! d.s.targeting());
}

BM_TEST(abilities, cooldown_counts_locked_pieces_and_never_negative)
{
    driver d;
    d.s.start(arcade_config(22));
    d.until_falling();
    d.s.debug_add_heat(60);
    d.tap(key_r);
    d.tap(key_a);
    const int cd = d.s.cooldown(cfg::ability_laser);
    CHECK(cd > 0);
    for(int i = 0; i < cd + 5; ++i)
    {
        d.until_falling();
        d.tap(key_up);
        d.until_falling();
        CHECK(d.s.cooldown(cfg::ability_laser) >= 0);
    }
    CHECK_EQ(d.s.cooldown(cfg::ability_laser), 0);
}

BM_TEST(abilities, targeting_timeout_cancels_without_cost)
{
    driver d;
    d.s.start(arcade_config(23));
    d.until_falling();
    d.s.debug_add_heat(50);
    d.tap(key_r);
    CHECK(d.s.targeting());
    for(int i = 0; i < cfg::targeting_timeout_frames + 5; ++i)
    {
        d.frame(0);
    }
    CHECK(! d.s.targeting());
    CHECK_EQ(d.s.heat(), 50);
}

BM_TEST(abilities, vent_removes_highest_cells)
{
    driver d;
    session_config c = arcade_config(24);
    c.loadout[0] = cfg::ability_emergency_vent;
    c.loadout_count = 1;
    d.s.start(c);
    d.until_falling();
    board& b = d.s.board_player(0).grid();
    for(int y = 10; y < 20; ++y)
    {
        for(int x = 0; x < 9; ++x) { b.set(x, y, bmtest::cell_from_char('#')); }
    }
    const int before = b.occupied_count();
    d.s.debug_add_heat(60);
    d.tap(key_r);
    for(int i = 0; i < 60; ++i) { d.frame(0); }
    CHECK_EQ(b.occupied_count(), before - cfg::vent_cells);
    CHECK_EQ(d.s.validate(), 0);
}

BM_TEST(bots, all_strategies_run_without_invalid_states)
{
    const bot_strategy strategies[] = { bot_strategy::random_legal, bot_strategy::height_min, bot_strategy::hole_min,
                                        bot_strategy::line_clear, bot_strategy::resource_aware };
    for(bot_strategy st : strategies)
    {
        for(uint32_t seed = 1; seed <= 3; ++seed)
        {
            const play_result r = play(arcade_config(seed * 31), st, 60 * 60 * 2);
            CHECK_EQ(r.invalid, 0);
        }
    }
}
