/*
 * Automated stage + boss validation (PART 50.10, 51.5, 56.4, 56.5).
 *
 * Every Adventure stage is played by several bot strategies over several seeds.
 * Each frame validates all session invariants. A stage passes when at least one
 * run reaches the objective (proving a legal route to completion exists under the
 * stage's real rules and randomization), and no run softlocks.
 * Results are also written to tests/build/stage_report.md.
 */
#include "bm_test.h"

#include "bm_bot.h"
#include "bm_session.h"

#include <cstdio>

using namespace bm;

namespace
{
    struct stage_run
    {
        bool won = false;
        bool lost = false;
        bool timeout = false;
        int invalid = 0;
        uint32_t frames = 0;
        uint32_t score = 0;
        int lines = 0;
        int boss_phase = 0;
        int boss_hp = 0;
        int boss_attacks = 0;
        int max_height = 0;
        int hazards = 0;
    };

    stage_run run_stage(int world, int index, bot_strategy strategy, uint32_t seed, int max_frames)
    {
        static session s;
        static bot b;
        const stage_def& st = stage(world, index);
        s.start(make_stage_config(st, difficulty::standard, seed));
        b.reset(strategy, seed * 7 + 1);
        stage_run r;
        int hazards = 0;
        for(int f = 0; f < max_frames; ++f)
        {
            const session_state state = s.state();
            if(state == session_state::won || state == session_state::lost)
            {
                break;
            }
            s.update(b.next(s));
            event e;
            while(s.events().pop(e))
            {
                if(e.type == event_type::hazard_fired)
                {
                    ++hazards;
                }
            }
            const int v = s.validate();
            if(v && ! r.invalid)
            {
                r.invalid = v;
            }
            r.frames = uint32_t(f);
        }
        r.won = s.state() == session_state::won;
        r.lost = s.state() == session_state::lost;
        r.timeout = ! r.won && ! r.lost;
        r.score = s.score();
        r.lines = s.lines();
        r.boss_phase = s.boss_phase();
        r.boss_hp = s.boss_hp();
        r.boss_attacks = s.boss_attacks();
        r.max_height = s.stats().max_height;
        r.hazards = hazards;
        return r;
    }
}

BM_TEST(stages, all_thirty_stages_have_a_route_to_completion)
{
    const bot_strategy strategies[] = { bot_strategy::resource_aware, bot_strategy::hole_min, bot_strategy::line_clear };
    const char* strategy_names[] = { "resource", "hole", "lines" };
    constexpr int seeds = 4;
    constexpr int max_frames = 60 * 60 * 15;   // 15 minutes of game time per run

    FILE* report = std::fopen("tests/build/stage_report.md", "w");
    if(report)
    {
        std::fprintf(report, "| Stage | Name | Wins | Losses | Timeouts | Best time (s) | Avg score (wins) | Notes |\n");
        std::fprintf(report, "|-------|------|-----:|-------:|---------:|--------------:|-----------------:|-------|\n");
    }

    for(int w = 1; w <= 6; ++w)
    {
        for(int i = 1; i <= 5; ++i)
        {
            const stage_def& st = stage(w, i);
            int wins = 0;
            int losses = 0;
            int timeouts = 0;
            uint32_t best = 0xFFFFFFFFu;
            uint64_t score_sum = 0;
            int invalid = 0;
            int max_phase = 0;
            int attacks = 0;
            int hazards = 0;
            for(int sidx = 0; sidx < 3; ++sidx)
            {
                for(uint32_t seed = 1; seed <= seeds; ++seed)
                {
                    const stage_run r = run_stage(w, i, strategies[sidx], seed * 101 + uint32_t(w * 10 + i), max_frames);
                    if(r.invalid && ! invalid)
                    {
                        invalid = r.invalid;
                        bmtest::note("stage %d-%d %s seed %u invalid state %d", w, i, strategy_names[sidx], seed, r.invalid);
                    }
                    wins += r.won;
                    losses += r.lost;
                    timeouts += r.timeout;
                    if(r.won)
                    {
                        score_sum += r.score;
                        if(r.frames < best)
                        {
                            best = r.frames;
                        }
                    }
                    if(r.boss_phase > max_phase)
                    {
                        max_phase = r.boss_phase;
                    }
                    attacks += r.boss_attacks;
                    hazards += r.hazards;
                }
            }
            bmtest::note("stage %d-%d %-22s wins %2d/%d losses %2d timeouts %2d best %4us boss-phase %d attacks %d hazards %d",
                         w, i, st.name, wins, 3 * seeds, losses, timeouts, best == 0xFFFFFFFFu ? 0u : best / 60,
                         max_phase, attacks, hazards);
            if(report)
            {
                std::fprintf(report, "| %d-%d | %s | %d | %d | %d | %u | %llu | %s |\n", w, i, st.name, wins, losses,
                             timeouts, best == 0xFFFFFFFFu ? 0u : best / 60,
                             (unsigned long long) (wins ? score_sum / uint64_t(wins) : 0),
                             st.boss ? "boss" : (st.dual ? "dual" : ""));
            }
            CHECK(wins > 0);
            CHECK_EQ(invalid, 0);
            CHECK_EQ(timeouts, 0);
            if(st.hazards[0].kind != hazard_kind::none)
            {
                CHECK(hazards > 0);
            }
            if(st.boss)
            {
                CHECK(attacks > 0);
            }
        }
    }
    if(report)
    {
        std::fclose(report);
    }
}
