/*
 * Challenge modifiers and unlockable cosmetics (PART 27.3, 30.7, 35.4): definitions, unlock
 * rules from real save progress, and the rule changes of every challenge on the real session.
 */
#include "bm_test.h"

#include "bm_bot.h"
#include "bm_challenges.h"
#include "bm_content.h"
#include "bm_cosmetics.h"
#include "bm_session.h"

#include <cstring>

using namespace bm;

namespace
{
    session_config classic(uint32_t seed, uint8_t challenges)
    {
        session_config c;
        c.mode = game_mode::classic;
        c.seed = seed;
        c.specials = false;
        c.heat = false;
        c.loadout_count = 0;
        c.skip_intro = true;
        c.challenges = challenges;
        return c;
    }

    session_config arcade(uint32_t seed, uint8_t challenges)
    {
        session_config c;
        c.mode = game_mode::arcade;
        c.seed = seed;
        c.specials = true;
        c.heat = true;
        c.loadout_count = 3;
        c.endless = true;
        c.skip_intro = true;
        c.challenges = challenges;
        return c;
    }

    struct run_result
    {
        uint32_t score = 0;
        uint64_t expected_scaled = 0;   // sum of every score event scaled by 'percent'
        int pieces = 0;
        int special_pieces = 0;
        int garbage_rows = 0;
        int invalid = 0;
        int frames = 0;
        int state = 0;
        int cause = 0;
    };

    run_result run(const session_config& c, int frames, int percent = 100, uint32_t bot_seed = 7)
    {
        static session s;
        static bot b;
        s.start(c);
        b.reset(bot_strategy::resource_aware, bot_seed);
        run_result r;
        for(int f = 0; f < frames && s.state() == session_state::playing; ++f)
        {
            s.update(b.next(s));
            event e;
            while(s.events().pop(e))
            {
                if(e.type == event_type::score)
                {
                    const uint32_t points = uint32_t(e.a & 0xFFFF) | (uint32_t(e.b) << 16);
                    r.expected_scaled += uint64_t(points) * uint64_t(percent) / 100u;
                }
                else if(e.type == event_type::piece_spawned &&
                        s.board_player(e.board).active().special != special_kind::none)
                {
                    ++r.special_pieces;
                }
            }
            if(s.validate() && ! r.invalid)
            {
                r.invalid = s.validate();
            }
            r.frames = f + 1;
        }
        r.score = s.score();
        r.state = int(s.state());
        r.cause = int(s.cause());
        r.pieces = s.stats().pieces;
        r.garbage_rows = s.stats().garbage_received;
        return r;
    }

    void clear_world(save_data& sv, int world)
    {
        sv.stages[stage_number(world, stages_per_world)].cleared = 1;
    }
}

BM_TEST(challenges, definitions)
{
    CHECK_EQ(int(challenge_total), 6);
    uint8_t worlds = 0;
    for(int i = 0; i < challenge_total; ++i)
    {
        const challenge_def& d = challenge(i);
        CHECK(d.name && std::strlen(d.name) <= 12);
        CHECK(d.desc && std::strlen(d.desc) <= 28);
        CHECK(d.unlock_world >= 1 && d.unlock_world <= 6);
        CHECK(! ((worlds >> d.unlock_world) & 1));      // one challenge per world
        worlds = uint8_t(worlds | (1u << d.unlock_world));
        CHECK(d.score_percent >= 50 && d.score_percent <= 200);
    }
}

BM_TEST(challenges, unlocked_by_world_boss_clears)
{
    save_data sv;
    CHECK_EQ(int(challenges_unlocked(sv)), 0);
    sv.stages[stage_number(1, 4)].cleared = 1;      // a non-boss stage unlocks nothing
    CHECK_EQ(int(challenges_unlocked(sv)), 0);
    clear_world(sv, 1);
    CHECK_EQ(int(challenges_unlocked(sv)), 1 << challenge_turbo);
    for(int w = 2; w <= 6; ++w)
    {
        clear_world(sv, w);
    }
    CHECK_EQ(int(challenges_unlocked(sv)), (1 << challenge_total) - 1);
}

BM_TEST(challenges, score_multipliers_combine)
{
    CHECK_EQ(challenge_score_percent(0), 100);
    CHECK_EQ(challenge_score_percent(1 << challenge_turbo), cfg::challenge_turbo_percent);
    CHECK_EQ(challenge_score_percent(1 << challenge_frenzy), cfg::challenge_frenzy_percent);
    const int both = challenge_score_percent(uint8_t((1 << challenge_turbo) | (1 << challenge_shadow)));
    CHECK_EQ(both, cfg::challenge_turbo_percent * cfg::challenge_shadow_percent / 100);
    const int all = challenge_score_percent(uint8_t((1 << challenge_total) - 1));
    bmtest::note("all six challenges: score x%d%%", all);
    CHECK(all > 200 && all < 300);
}

BM_TEST(challenges, turbo_starts_faster)
{
    static session s;
    s.start(classic(5, 0));
    const int base_level = s.level();
    const int base_gravity = s.gravity_frames();
    s.start(classic(5, 1 << challenge_turbo));
    CHECK_EQ(s.level(), base_level + cfg::challenge_turbo_levels);
    CHECK(s.gravity_frames() < base_gravity);
    CHECK_EQ(s.score_percent(), cfg::challenge_turbo_percent);
}

BM_TEST(challenges, no_hold_locks_the_hold_slot)
{
    static session s;
    s.start(classic(9, 1 << challenge_no_hold));
    input_frame idle;
    for(int i = 0; i < 5; ++i)
    {
        s.update(idle);
    }
    const piece_type before = s.board_player(0).active().type;
    input_frame l;
    l.held = key_l;
    l.pressed = key_l;
    s.update(l);
    CHECK(s.board_player(0).held() == piece_type::none);
    CHECK(s.board_player(0).active().type == before);
    CHECK(! s.board_player(0).can_hold());

    s.start(classic(9, 0));
    for(int i = 0; i < 5; ++i)
    {
        s.update(idle);
    }
    s.update(l);
    CHECK(s.board_player(0).held() != piece_type::none);
}

BM_TEST(challenges, presentation_challenges_only_scale_the_score)
{
    // SHADOW STACK and ONE NEXT change nothing in the rules: the same seed plays the same game,
    // and every score event pays exactly the scaled amount.
    const int percent = challenge_score_percent(uint8_t((1 << challenge_shadow) | (1 << challenge_one_next)));
    const run_result base = run(classic(21, 0), 60 * 60, percent);
    const run_result shadow = run(classic(21, uint8_t((1 << challenge_shadow) | (1 << challenge_one_next))), 60 * 60);
    bmtest::note("base score %u, shadow+one next %u (expected %llu)", base.score, shadow.score,
                 static_cast<unsigned long long>(base.expected_scaled));
    CHECK_EQ(base.pieces, shadow.pieces);
    CHECK(base.score > 1000);
    CHECK_EQ(uint64_t(shadow.score), base.expected_scaled);
    CHECK_EQ(shadow.invalid, 0);
}

BM_TEST(challenges, frenzy_doubles_special_pieces)
{
    const run_result normal = run(arcade(33, 0), 60 * 90);
    const run_result frenzy = run(arcade(33, 1 << challenge_frenzy), 60 * 90);
    const run_result classic_frenzy = run(classic(33, 1 << challenge_frenzy), 60 * 90);
    const run_result classic_plain = run(classic(33, 0), 60 * 90);
    bmtest::note("special pieces per 100: arcade %d, arcade frenzy %d, classic frenzy %d",
                 normal.special_pieces * 100 / (normal.pieces + 1), frenzy.special_pieces * 100 / (frenzy.pieces + 1),
                 classic_frenzy.special_pieces * 100 / (classic_frenzy.pieces + 1));
    CHECK_EQ(classic_plain.special_pieces, 0);
    CHECK(classic_frenzy.special_pieces > 0);
    // Rate per piece at least 1.6x the normal rate.
    CHECK(frenzy.special_pieces * 10 * (normal.pieces + 1) >= normal.special_pieces * 16 * (frenzy.pieces + 1));
    CHECK_EQ(frenzy.invalid, 0);
    CHECK_EQ(classic_frenzy.invalid, 0);
}

BM_TEST(challenges, rising_tide_sends_a_row_every_interval)
{
    static session s;
    s.start(classic(11, 1 << challenge_rising_tide));
    CHECK_EQ(s.tide_frames_left(), cfg::challenge_tide_frames);
    const run_result r = run(classic(11, 1 << challenge_rising_tide), cfg::challenge_tide_frames * 3 + 600);
    const run_result plain = run(classic(11, 0), cfg::challenge_tide_frames * 3 + 600);
    bmtest::note("rising tide: %d junk rows in %d frames (plain run %d frames, %d pieces / %d, state %d cause %d)",
                 r.garbage_rows, r.frames, plain.frames, plain.pieces, r.pieces, plain.state, plain.cause);
    CHECK_EQ(plain.garbage_rows, 0);
    CHECK_EQ(r.frames, cfg::challenge_tide_frames * 3 + 600);    // the bot survives the whole run
    CHECK_EQ(r.garbage_rows, 3);
    CHECK_EQ(r.invalid, 0);
    s.start(classic(11, 0));
    CHECK_EQ(s.tide_frames_left(), 0);
}

BM_TEST(cosmetics, definitions_and_defaults)
{
    save_data sv;
    for(int k = 0; k < cosmetic_kinds; ++k)
    {
        const cosmetic_kind kind = cosmetic_kind(k);
        CHECK(cosmetic_count(kind) >= 4 && cosmetic_count(kind) <= cosmetic_max_per_kind);
        CHECK(cosmetic_unlocked(sv, kind, 0));
        CHECK(! cosmetic_unlocked(sv, kind, cosmetic_count(kind)));
        for(int i = 0; i < cosmetic_count(kind); ++i)
        {
            const cosmetic_def& d = cosmetic(kind, i);
            CHECK(d.name && std::strlen(d.name) <= 10);
            CHECK(std::strlen(d.unlock) <= 30);
            CHECK(i == 0 || (kind == cosmetic_kind::blocks && i == 1) || d.unlock[0]);
        }
        CHECK_EQ(selected_cosmetic(sv, kind), 0);
    }
    // The colour-blind friendly theme is an accessibility option: never locked.
    CHECK(cosmetic_unlocked(sv, cosmetic_kind::blocks, 1));
    CHECK_EQ(update_cosmetics(sv), 0u);
}

BM_TEST(cosmetics, unlocked_by_progress_and_reported_once)
{
    save_data sv;
    sv.settings.block_theme = 2;    // a locked selection falls back to the default
    CHECK_EQ(selected_cosmetic(sv, cosmetic_kind::blocks), 0);

    clear_world(sv, 1);
    clear_world(sv, 2);
    uint32_t fresh = update_cosmetics(sv);
    CHECK(fresh & cosmetic_bit(cosmetic_kind::title, 1));        // MIDNIGHT: world 1
    CHECK(fresh & cosmetic_bit(cosmetic_kind::particles, 2));    // SNOW: world 2
    CHECK(! (fresh & cosmetic_bit(cosmetic_kind::blocks, 2)));   // NEON needs world 4
    CHECK_EQ(update_cosmetics(sv), 0u);                          // reported only once

    clear_world(sv, 4);
    fresh = update_cosmetics(sv);
    CHECK(fresh & cosmetic_bit(cosmetic_kind::blocks, 2));
    CHECK_EQ(selected_cosmetic(sv, cosmetic_kind::blocks), 2);

    sv.achievements = 0x3FF;        // 10 achievements: STARS and CANDY
    sv.stats.overheat_success = 25; // INFERNO
    sv.runs_completed = 1;          // CARBON
    sv.boss_rush_best_frames = 100; // GOLD
    sv.stages[0].best_rank = 5;     // HEARTS
    sv.campaign_complete = 1;       // CHILI, GOLDEN
    for(int i = 0; i < 15; ++i)
    {
        sv.stages[i].cleared = 1;   // PASTEL
    }
    clear_world(sv, 3);
    clear_world(sv, 5);
    update_cosmetics(sv);
    for(int k = 0; k < cosmetic_kinds; ++k)
    {
        for(int i = 0; i < cosmetic_count(cosmetic_kind(k)); ++i)
        {
            CHECK(cosmetic_unlocked(sv, cosmetic_kind(k), i));
        }
    }
}

BM_TEST(cosmetics, save_round_trip_keeps_choices)
{
    save_data sv;
    reset_defaults(sv);
    sv.settings.block_theme = 1;
    sv.settings.particle_style = 3;
    sv.settings.challenges = 0x21;
    sv.cosmetics_unlocked = 0x00FF00FFu;
    seal(sv, 1);
    CHECK(validate(sv) == save_status::ok);
    save_data copy;
    std::memcpy(static_cast<void*>(&copy), &sv, sizeof(sv));
    CHECK(validate(copy) == save_status::ok);
    CHECK_EQ(int(copy.settings.block_theme), 1);
    CHECK_EQ(int(copy.settings.particle_style), 3);
    CHECK_EQ(int(copy.settings.challenges), 0x21);
    CHECK_EQ(copy.cosmetics_unlocked, 0x00FF00FFu);
}
