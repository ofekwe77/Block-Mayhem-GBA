/*
 * Achievement validation (PART 28, 35.2, 58.2): definitions, real triggers only,
 * progress tracking, no double counting after the session is merged, campaign flags.
 */
#include "bm_test.h"

#include "bm_achievements.h"
#include "bm_bot.h"

#include <cstring>

using namespace bm;

namespace
{
    void play_bot(session& s, const session_config& c, int frames)
    {
        static bot b;
        s.start(c);
        b.reset(bot_strategy::resource_aware, c.seed);
        for(int f = 0; f < frames && s.state() != session_state::won && s.state() != session_state::lost; ++f)
        {
            s.update(b.next(s));
            event e;
            while(s.events().pop(e))
            {
            }
        }
    }
}

BM_TEST(achievements, definitions_are_complete)
{
    CHECK(achievement_total >= 25);
    int hidden = 0;
    for(int i = 0; i < achievement_total; ++i)
    {
        const achievement_def& d = achievement(i);
        CHECK(d.name && d.name[0]);
        CHECK(std::strlen(d.name) <= 16);
        CHECK(std::strlen(d.desc) <= 29);
        CHECK(d.target >= 1);
        CHECK(d.icon <= 16);
        hidden += d.hidden ? 1 : 0;
        for(int j = 0; j < i; ++j)
        {
            CHECK(std::strcmp(d.name, achievement(j).name) != 0);
        }
    }
    CHECK(hidden >= 4);
}

BM_TEST(achievements, nothing_unlocks_without_events)
{
    save_data sv;
    reset_defaults(sv);
    static session s;
    session_config c;
    c.mode = game_mode::classic;
    c.specials = false;
    c.heat = false;
    s.start(c);
    achievement_input in;
    in.live = &s;
    CHECK_EQ(update_achievements(sv, in), 0u);
    CHECK_EQ(sv.achievements, 0u);
    in.live = nullptr;
    CHECK_EQ(update_achievements(sv, in), 0u);
}

BM_TEST(achievements, live_play_unlocks_and_tracks_progress_without_double_counting)
{
    save_data sv;
    reset_defaults(sv);
    static session s;
    session_config c;
    c.mode = game_mode::arcade;
    c.seed = 4242;
    c.loadout_count = 3;
    play_bot(s, c, 60 * 90);
    CHECK(s.lines() > 0);

    achievement_input in;
    in.live = &s;
    const uint32_t fresh = update_achievements(sv, in);
    CHECK(fresh & (1u << ach_first_line));
    CHECK(sv.achievements & (1u << ach_first_line));
    // Progress never exceeds the target and reflects the session.
    const uint32_t bombs = s.stats().bombs;
    CHECK_EQ(uint32_t(sv.achievement_progress[ach_bomb_squad]), bombs > 10 ? 10u : bombs);
    // A second evaluation reports nothing new.
    CHECK_EQ(update_achievements(sv, in), 0u);

    // After record_session merges the statistics, a merged evaluation must not count them twice.
    sv.stats.bombs += s.stats().bombs;
    in.merged = true;
    in.session_over = true;
    update_achievements(sv, in);
    CHECK_EQ(uint32_t(sv.achievement_progress[ach_bomb_squad]), sv.stats.bombs > 10 ? 10u : sv.stats.bombs);
}

BM_TEST(achievements, campaign_run_and_rush_flags)
{
    save_data sv;
    reset_defaults(sv);
    achievement_input in;
    CHECK_EQ(update_achievements(sv, in), 0u);
    sv.stages[stage_number(1, 5)].cleared = 1;
    CHECK(update_achievements(sv, in) & (1u << ach_factory_hero));
    sv.stages[stage_number(2, 5)].cleared = 1;
    CHECK(! (update_achievements(sv, in) & (1u << ach_world_traveler)));
    CHECK_EQ(int(sv.achievement_progress[ach_world_traveler]), 2);
    sv.stages[stage_number(3, 5)].cleared = 1;
    CHECK(update_achievements(sv, in) & (1u << ach_world_traveler));
    sv.campaign_complete = 1;
    CHECK(update_achievements(sv, in) & (1u << ach_mayhem_champion));
    sv.runs_completed = 1;
    CHECK(update_achievements(sv, in) & (1u << ach_chaos_survivor));
    sv.boss_rush_best_frames = 12345;
    CHECK(update_achievements(sv, in) & (1u << ach_rush_hour));
    in.stage_rank = 5;
    CHECK(update_achievements(sv, in) & (1u << ach_puzzle_genius));
}

BM_TEST(achievements, master_needs_twenty_others_and_arsenal_accumulates)
{
    save_data sv;
    reset_defaults(sv);
    achievement_input in;
    for(int i = 0; i < 19; ++i)
    {
        sv.achievements |= 1u << i;
    }
    CHECK(! (update_achievements(sv, in) & (1u << ach_master_of_mayhem)));
    sv.achievements |= 1u << 19;
    CHECK(update_achievements(sv, in) & (1u << ach_master_of_mayhem));

    // ARSENAL: abilities used in different sessions add up.
    save_data sv2;
    reset_defaults(sv2);
    sv2.achievement_progress[ach_arsenal] = 0x7F;      // seven abilities already used
    CHECK(! (update_achievements(sv2, in) & (1u << ach_arsenal)));
    sv2.achievement_progress[ach_arsenal] = 0xFF;
    CHECK(update_achievements(sv2, in) & (1u << ach_arsenal));
}
