/*
 * CHAOS RUN validation (PART 21, 39.6, 60): upgrade pool, caps, deterministic
 * generation, offer validity, encounter completability and full bot runs.
 * Results are also written to tests/build/run_report.md.
 */
#include "bm_test.h"

#include "bm_bot.h"
#include "bm_run.h"

#include <cstdio>
#include <cstring>

using namespace bm;

namespace
{
    struct play_result
    {
        bool won = false;
        bool timeout = false;
        int invalid = 0;
        uint32_t frames = 0;
    };

    play_result play(const session_config& config, bot_strategy strategy, uint32_t seed, session& s)
    {
        static bot b;
        constexpr int max_frames = 60 * 60 * 12;
        s.start(config);
        b.reset(strategy, seed * 7 + 3);
        play_result r;
        for(int f = 0; f < max_frames; ++f)
        {
            if(s.state() == session_state::won || s.state() == session_state::lost)
            {
                break;
            }
            s.update(b.next(s));
            event e;
            while(s.events().pop(e))
            {
            }
            const int v = s.validate();
            if(v && ! r.invalid)
            {
                r.invalid = v;
            }
            r.frames = uint32_t(f);
        }
        r.won = s.state() == session_state::won;
        r.timeout = s.state() != session_state::won && s.state() != session_state::lost;
        return r;
    }

    encounter_result win_result(const chaos_run& run)
    {
        encounter_result r;
        r.won = true;
        r.score = run.score() + 1000;
        r.frames = 3600;
        r.lines = 10;
        return r;
    }

    bool same_encounter(const run_encounter& a, const run_encounter& b)
    {
        if(a.template_id != b.template_id || a.def.boss != b.def.boss || a.def.layout_rows != b.def.layout_rows ||
           std::strcmp(a.def.name, b.def.name) != 0)
        {
            return false;
        }
        for(int i = 0; i < 3; ++i)
        {
            if(a.def.objectives[i].kind != b.def.objectives[i].kind ||
               a.def.objectives[i].target != b.def.objectives[i].target)
            {
                return false;
            }
        }
        for(int y = 0; y < a.def.layout_rows; ++y)
        {
            if(std::strcmp(a.rows[y], b.rows[y]) != 0)
            {
                return false;
            }
        }
        return a.def.hazards[0].kind == b.def.hazards[0].kind && a.def.hazards[0].interval == b.def.hazards[0].interval;
    }
}

BM_TEST(run, upgrade_pool_is_large_and_well_formed)
{
    CHECK(upgrade_count >= 20);
    int rarities[3] = {};
    for(int i = 0; i < upgrade_count; ++i)
    {
        const upgrade_def& u = upgrade(i);
        CHECK(u.name && u.name[0]);
        CHECK(std::strlen(u.name) <= 18);
        CHECK(std::strlen(u.desc[0]) <= 26);
        CHECK(std::strlen(u.desc[1]) <= 26);
        CHECK(u.max_stacks >= 1 && u.max_stacks <= 3);
        CHECK(u.tags != 0);
        ++rarities[int(u.rare)];
        for(int j = 0; j < i; ++j)
        {
            CHECK(std::strcmp(u.name, upgrade(j).name) != 0);
        }
    }
    CHECK(rarities[0] > 0 && rarities[1] > 0 && rarities[2] > 0);
    for(int s = 0; s < synergy_count; ++s)
    {
        CHECK(synergy(s).a != synergy(s).b);
        CHECK(synergy(s).a < upgrade_count && synergy(s).b < upgrade_count);
    }
    // Every archetype of PART 60.1 is reachable.
    const uint8_t archetypes[] = { tag_explosive, tag_electric, tag_heat, tag_defense, tag_precision, tag_chain };
    for(uint8_t t : archetypes)
    {
        int n = 0;
        for(int i = 0; i < upgrade_count; ++i)
        {
            n += (upgrade(i).tags & t) ? 1 : 0;
        }
        CHECK(n >= 3);
    }
}

BM_TEST(run, maxed_build_respects_every_cap)
{
    run_build b;
    for(int i = 0; i < upgrade_count; ++i)
    {
        b.stacks[i] = upgrade(i).max_stacks;
    }
    b.loadout[0] = cfg::ability_laser;
    b.loadout[1] = cfg::ability_time_freeze;
    b.loadout[2] = cfg::ability_emergency_vent;
    b.loadout_count = 3;
    CHECK_EQ(b.synergies_active(), synergy_count);
    const modifiers m = build_modifiers(b);
    CHECK(m.heat_gain_percent <= cfg::run_heat_gain_percent_max);
    CHECK(m.chain_score_percent <= cfg::run_chain_score_percent_max);
    CHECK(m.overheat_bonus_percent <= cfg::run_overheat_bonus_percent_max);
    CHECK(m.precision_percent <= cfg::run_precision_percent_max);
    CHECK(m.combo_battery_heat <= cfg::run_combo_battery_max);
    CHECK(m.iron_defense <= cfg::run_iron_defense_max);
    CHECK(m.effects.fire_reach <= cfg::run_fire_reach_max);
    CHECK(m.effects.spark_range <= cfg::spark_range_max);
    CHECK(m.effects.magnet_cols <= cfg::run_magnet_cols_max);
    CHECK(m.effects.rainbow_targets <= cfg::run_rainbow_targets_max);
    CHECK(m.effects.bomb_plus <= cfg::run_bomb_plus_max);
    CHECK(m.effects.bomb_power <= cfg::run_bomb_power_max);
    CHECK(m.freeze_bonus_frames + cfg::time_freeze_frames <= cfg::time_freeze_max_frames);
    CHECK(m.vent_bonus_cells + cfg::vent_cells <= cfg::vent_cells_max);
    CHECK(m.reactive_armor <= 1);
    CHECK(build_start_heat(b) <= cfg::run_start_heat_max);

    // Through the real session rules: costs stay above the floor, cooldowns never reach zero.
    session_config c;
    c.mode = game_mode::chaos_run;
    c.mods = m;
    c.loadout_count = 3;
    static session s;
    s.start(c);
    for(int a = 0; a < cfg::ability_count; ++a)
    {
        CHECK(s.ability_cost(a) >= cfg::ability_min_cost);
        CHECK(s.ability_cost(a) >= cfg::ability_cost[a] - cfg::run_cost_reduction_max);
        CHECK(s.ability_cooldown_max(a) >= cfg::ability_min_cooldown);
    }
    special_config sp;
    sp.interval_min = 3;
    sp.interval_max = 5;
    sp.weights[0] = 10;
    sp.weights[2] = 10;
    apply_supply(b, sp);
    CHECK(sp.weights[0] <= 10 + cfg::run_supply_weight_max);
    CHECK(sp.weights[2] <= 10 + cfg::run_supply_weight_max);
}

BM_TEST(run, same_seed_same_run_and_seeds_differ)
{
    static chaos_run a;
    static chaos_run b;
    a.start(777);
    b.start(777);
    for(int step = 0; step < chaos_run::total_encounters; ++step)
    {
        CHECK(same_encounter(a.encounter(), b.encounter()));
        a.record_result(win_result(a));
        b.record_result(win_result(b));
        if(a.phase() != run_phase::choose)
        {
            break;
        }
        CHECK_EQ(a.offer_count(), b.offer_count());
        for(int i = 0; i < a.offer_count(); ++i)
        {
            CHECK_EQ(a.offer(i), b.offer(i));
        }
        a.choose(step % a.offer_count());
        b.choose(step % b.offer_count());
    }
    CHECK(a.phase() == run_phase::victory);
    CHECK_EQ(a.cleared(), chaos_run::total_encounters);

    // Different seeds give different runs (PART 60.5): compare the first act's line-up.
    int distinct = 0;
    uint32_t signatures[16] = {};
    for(uint32_t seed = 1; seed <= 16; ++seed)
    {
        static chaos_run r;
        r.start(seed * 7919u);
        uint32_t sig = 0;
        for(int k = 0; k < 3; ++k)
        {
            sig = sig * 31u + r.encounter().template_id;
            r.record_result(win_result(r));
            r.choose(0);
        }
        sig = sig * 31u + r.encounter().def.boss;
        bool seen = false;
        for(int i = 0; i < int(seed) - 1; ++i)
        {
            seen = seen || signatures[i] == sig;
        }
        signatures[seed - 1] = sig;
        distinct += seen ? 0 : 1;
    }
    CHECK(distinct >= 10);
}

BM_TEST(run, offers_are_always_valid_and_runs_never_repeat_content)
{
    for(uint32_t seed = 1; seed <= 300; ++seed)
    {
        static chaos_run r;
        r.start(seed * 2654435761u);
        rng pick(seed);
        uint16_t templates_seen = 0;
        uint8_t bosses[3] = {};
        for(int step = 0; step < chaos_run::total_encounters; ++step)
        {
            const run_encounter& e = r.encounter();
            if(e.def.boss)
            {
                bosses[r.act()] = e.def.boss;
                CHECK(e.boss_hp_percent == cfg::run_boss_hp_percent[r.act()]);
                CHECK(! boss(e.def.boss).dual);
            }
            else
            {
                CHECK(! (templates_seen & (1u << e.template_id)));     // no repeated encounter in a run
                templates_seen = uint16_t(templates_seen | (1u << e.template_id));
                CHECK(template_available(e.template_id, r.act()));
                // Starting rows never contain a complete row.
                for(int y = 0; y < e.def.layout_rows; ++y)
                {
                    CHECK(std::strchr(e.rows[y], '.') != nullptr);
                }
                CHECK(e.def.objectives[0].kind != objective_kind::none);
            }
            r.record_result(win_result(r));
            if(r.phase() != run_phase::choose)
            {
                break;
            }
            CHECK(r.offer_count() == chaos_run::offer_size);
            bool rare_or_better = false;
            for(int i = 0; i < r.offer_count(); ++i)
            {
                const int id = r.offer(i);
                CHECK(upgrade_eligible(r.build(), id));
                for(int j = 0; j < i; ++j)
                {
                    CHECK(r.offer(j) != id);
                }
                rare_or_better = rare_or_better || upgrade(id).rare != rarity::common;
            }
            if(r.boss_slot())
            {
                CHECK(rare_or_better);
            }
            const int before = r.build().loadout_count;
            const int choice = pick.below(r.offer_count());
            const int id = r.offer(choice);
            r.choose(choice);
            if(upgrade(id).grants_ability >= 0)
            {
                CHECK_EQ(r.build().loadout_count, before + 1);
            }
            CHECK(r.build().loadout_count <= 3);
        }
        CHECK(r.phase() == run_phase::victory);
        CHECK(bosses[0] && bosses[1] && bosses[2]);
        CHECK(bosses[0] != bosses[1] && bosses[1] != bosses[2] && bosses[0] != bosses[2]);
    }
}

BM_TEST(run, every_encounter_template_and_boss_is_completable)
{
    FILE* report = std::fopen("tests/build/run_report.md", "w");
    if(report)
    {
        std::fprintf(report, "# CHAOS RUN validation report (generated by tests/test_run.cpp)\n");
        std::fprintf(report, "\n## Encounter templates (bots: resource / hole / lines, 2 seeds each)\n\n");
        std::fprintf(report, "| Template | Act | Wins | Best time (s) |\n|---|---:|---:|---:|\n");
    }
    static session s;
    int quad_template = -1;
    for(int t = 0; t < run_template_count; ++t)
    {
        static run_encounter e;
        generate_encounter(e, run_build(), 1, 0, t, 0, 1);
        if(e.def.objectives[0].kind == objective_kind::quads)
        {
            quad_template = t;
        }
    }
    run_build build;
    build.loadout[0] = cfg::ability_laser;
    build.loadout[1] = cfg::ability_shield_matrix;
    build.loadout[2] = cfg::ability_emergency_vent;
    build.loadout_count = 3;
    const bot_strategy strategies[] = { bot_strategy::resource_aware, bot_strategy::hole_min, bot_strategy::line_clear };
    for(int t = 0; t < run_template_count; ++t)
    {
        for(int act = 0; act < 3; ++act)
        {
            if(! template_available(t, act))
            {
                continue;
            }
            int wins = 0;
            int invalid = 0;
            int timeouts = 0;
            uint32_t best = 0xFFFFFFFFu;
            for(bot_strategy st : strategies)
            {
                for(uint32_t seed = 1; seed <= 2; ++seed)
                {
                    static run_encounter e;
                    generate_encounter(e, build, act, 1, t, 0, seed * 97u + uint32_t(t));
                    session_config c = make_stage_config(e.def, difficulty::standard, seed * 31u + uint32_t(act));
                    c.mode = game_mode::chaos_run;
                    const play_result r = play(c, st, seed, s);
                    wins += r.won ? 1 : 0;
                    invalid += r.invalid ? 1 : 0;
                    timeouts += r.timeout ? 1 : 0;
                    if(r.won && r.frames < best)
                    {
                        best = r.frames;
                    }
                }
            }
            bmtest::note("run template %-16s act %d wins %d/6 timeouts %d", run_template_name(t), act + 1, wins,
                         timeouts);
            if(report)
            {
                std::fprintf(report, "| %s | %d | %d/6 | %u |\n", run_template_name(t), act + 1, wins,
                             best == 0xFFFFFFFFu ? 0u : best / 60u);
            }
            CHECK(wins > 0);
            CHECK_EQ(invalid, 0);
            // Skill objectives (QUADS) may outlast a bot that never builds a well; anything else must end.
            if(t != quad_template)
            {
                CHECK_EQ(timeouts, 0);
            }
        }
    }
    // Run bosses at every act they can appear in, with the act's HP scaling.
    constexpr uint8_t candidates[3][2] = { { 1, 2 }, { 2, 3 }, { 3, 5 } };
    for(int act = 0; act < 3; ++act)
    {
        for(uint8_t id : candidates[act])
        {
            int wins = 0;
            int invalid = 0;
            for(bot_strategy st : strategies)
            {
                static run_encounter e;
                generate_encounter(e, build, act, 3, -1, id, 11u + id);
                session_config c = make_stage_config(e.def, difficulty::standard, 5u + id);
                c.mode = game_mode::chaos_run;
                c.boss_hp_percent = e.boss_hp_percent;
                const play_result r = play(c, st, 3u + uint32_t(act), s);
                wins += r.won ? 1 : 0;
                invalid += r.invalid ? 1 : 0;
            }
            bmtest::note("run boss %-18s act %d wins %d/3", boss(id).name, act + 1, wins);
            if(report)
            {
                std::fprintf(report, "| BOSS %s | %d | %d/3 | |\n", boss(id).name, act + 1, wins);
            }
            CHECK(wins > 0);
            CHECK_EQ(invalid, 0);
        }
    }
    if(report)
    {
        std::fclose(report);
    }
}

BM_TEST(run, full_bot_runs_progress_through_acts)
{
    static session s;
    static chaos_run r;
    FILE* report = std::fopen("tests/build/run_report.md", "a");
    if(report)
    {
        std::fprintf(report, "\n## Full runs (resource-aware bot, first-offer policy)\n\n");
        std::fprintf(report, "| Seed | Cleared | Result | Score | Minutes | Build |\n|---:|---:|---|---:|---:|---|\n");
    }
    int victories = 0;
    int act_two = 0;
    for(uint32_t seed = 1; seed <= 6; ++seed)
    {
        r.start(seed * 104729u);
        int invalid = 0;
        while(r.phase() == run_phase::encounter)
        {
            const play_result p = play(r.make_config(difficulty::standard), bot_strategy::resource_aware, seed, s);
            invalid += p.invalid ? 1 : 0;
            r.finish_encounter(s);
            if(r.phase() == run_phase::choose)
            {
                // Prefer an upgrade that completes a synergy, else the first card.
                int choice = 0;
                for(int i = 0; i < r.offer_count(); ++i)
                {
                    if(synergy_completed_by(r.build(), r.offer(i)) >= 0)
                    {
                        choice = i;
                        break;
                    }
                }
                r.choose(choice);
            }
        }
        CHECK_EQ(invalid, 0);
        victories += r.phase() == run_phase::victory ? 1 : 0;
        act_two += r.cleared() >= chaos_run::slots_per_act ? 1 : 0;
        bmtest::note("run seed %u cleared %d/%d %s score %u", seed, r.cleared(), chaos_run::total_encounters,
                     r.phase() == run_phase::victory ? "VICTORY" : "defeat", r.score());
        if(report)
        {
            char build[160] = {};
            int len = 0;
            for(int i = 0; i < upgrade_count && len < 140; ++i)
            {
                if(r.build().stacks[i])
                {
                    len += std::snprintf(build + len, sizeof(build) - size_t(len), "%s%s x%d", len ? ", " : "",
                                         upgrade(i).name, int(r.build().stacks[i]));
                }
            }
            std::fprintf(report, "| %u | %d | %s | %u | %u | %s |\n", seed, r.cleared(),
                         r.phase() == run_phase::victory ? "victory" : "defeat", r.score(), r.frames() / 3600u, build);
        }
    }
    if(report)
    {
        std::fclose(report);
    }
    CHECK(act_two >= 3);        // most runs get past the first boss
    CHECK(victories >= 1);      // a complete run is achievable
}
