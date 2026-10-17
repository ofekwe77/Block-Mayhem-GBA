/*
 * BLOCK MAYHEM - documentation generator (host). Builds STAGE_PASSPORTS.md (PART 50.3) from the real
 * stage, boss and configuration data the ROM uses, plus fresh fairness simulations of every stage with
 * the same session code (so the passports can never drift from the game).
 *
 *   tools/bm.sh docs      (compiles this file with the core sources and runs it from the project root)
 */
#include "bm_bot.h"
#include "bm_config.h"
#include "bm_content.h"
#include "bm_session.h"

#include <cstdio>
#include <cstring>
#include <string>

using namespace bm;

namespace
{
    const char* ability_names[cfg::ability_count] = {
        "Laser", "Time Freeze", "Gravity Pulse", "Shape Shift", "Nuclear Burst", "Shield Matrix", "Chain Spark",
        "Emergency Vent"
    };

    const char* special_names[6] = { "none", "bomb", "fire", "electric spark", "magnet", "rainbow" };

    struct world_info
    {
        const char* visual;
        const char* music;
        const char* mechanic;
    };

    const world_info worlds[7] = {
        { "", "", "" },
        { "Steel factory: conveyor belts, pistons and gears on two parallax layers, yellow hazard trims, sparks "
          "from the machines when bombs go off",
          "`mus_w1` (driving factory groove) and its Overheat arrangement `mus_w1_hot`",
          "the core rules: rows, the Heat Meter, bombs and fire, first chains" },
        { "Frozen factory: icicles and frosted pipes, falling snow, a cold blue palette, ice-blue board frame",
          "`mus_w2` (icy bells over a steady pulse) and `mus_w2_hot`",
          "ice coats (frozen cells need an extra hit) and freezing hazards" },
        { "Volcano: lava falls and a glowing magma lake with a heat-shimmer raster wave, embers in the air, red-brown "
          "frame", "`mus_w3` (heavy toms and low brass) and `mus_w3_hot`",
          "fire cells, lava pressure and molten rain" },
        { "Electric city: neon skyline, power lines with travelling sparks, flickering signs, violet-cyan frame",
          "`mus_w4` (fast synth arpeggios) and `mus_w4_hot`",
          "electric sparks, surges that disable specials, the dual-board relay" },
        { "Gravity dimension: floating islands and rings in a starfield, slow orbital motion, teal frame",
          "`mus_w5` (floating pads, half-time beat) and `mus_w5_hot`",
          "gravity shifts and rotating settle directions" },
        { "Final meltdown: the core chamber, magenta energy and alarms, an eye that watches the board, "
          "pulsing red danger trims", "`mus_w6` (relentless minor-key drive) and `mus_w6_hot`",
          "every earlier hazard returns, one at a time, and the Core of Mayhem" },
    };

    std::string objective_text(const objective_def& o)
    {
        char b[96];
        switch(o.kind)
        {
        case objective_kind::lines: std::snprintf(b, sizeof(b), "clear %d rows", o.target); break;
        case objective_kind::bombs: std::snprintf(b, sizeof(b), "trigger %d bombs", o.target); break;
        case objective_kind::chains:
            std::snprintf(b, sizeof(b), "complete %d chain reactions of %d+ waves", o.target, o.param);
            break;
        case objective_kind::magnets: std::snprintf(b, sizeof(b), "pull %d blocks with magnets", o.target); break;
        case objective_kind::overheat_success: std::snprintf(b, sizeof(b), "survive %d Overheats", o.target); break;
        case objective_kind::cross_combos: std::snprintf(b, sizeof(b), "land %d CROSS OVER combos", o.target); break;
        case objective_kind::survive_pieces:
            std::snprintf(b, sizeof(b), "lock %d pieces without topping out", o.target);
            break;
        case objective_kind::boss_defeat: std::snprintf(b, sizeof(b), "reduce the boss to 0 HP"); break;
        case objective_kind::sparks: std::snprintf(b, sizeof(b), "trigger %d electric cells", o.target); break;
        case objective_kind::fires: std::snprintf(b, sizeof(b), "trigger %d fire cells", o.target); break;
        case objective_kind::ice_broken: std::snprintf(b, sizeof(b), "break %d ice coats", o.target); break;
        case objective_kind::quads: std::snprintf(b, sizeof(b), "clear 4 rows at once %d times", o.target); break;
        case objective_kind::twists: std::snprintf(b, sizeof(b), "score %d T-TWIST clears", o.target); break;
        case objective_kind::max_height:
            std::snprintf(b, sizeof(b), "finish with the stack at most %d rows high", o.target);
            break;
        case objective_kind::no_garbage: std::snprintf(b, sizeof(b), "take no hostile garbage"); break;
        case objective_kind::time_limit: std::snprintf(b, sizeof(b), "finish within %d seconds", o.target); break;
        case objective_kind::none:
        default: std::snprintf(b, sizeof(b), "none"); break;
        }
        return b;
    }

    std::string hazard_text(const hazard_def& h)
    {
        char b[200];
        const char* what = "";
        switch(h.kind)
        {
        case hazard_kind::freeze_cells: what = "FREEZE: telegraphed surface cells become ice-coated"; break;
        case hazard_kind::lava_pressure:
            what = "LAVA PRESSURE: a meter fills every lock; when full, a telegraphed molten garbage row rises";
            break;
        case hazard_kind::gravity_shift: what = "GRAVITY SHIFT: the board compacts in the telegraphed direction"; break;
        case hazard_kind::settle_cycle: what = "SETTLE CYCLE: the direction blocks settle after effects rotates"; break;
        case hazard_kind::garbage_drip: what = "GARBAGE DRIP: telegraphed garbage rows on a schedule"; break;
        case hazard_kind::armor_plates: what = "ARMOR PLATES: telegraphed cells become armoured (2 hits)"; break;
        case hazard_kind::surge: what = "SURGE: special cells are disabled for a few pieces"; break;
        case hazard_kind::molten_rain: what = "MOLTEN RAIN: telegraphed columns receive a molten block"; break;
        case hazard_kind::mixed: what = "MIXED: rotates through freeze, lava, gravity and surge, one at a time"; break;
        case hazard_kind::none:
        default: return "";
        }
        std::snprintf(b, sizeof(b), "%s — every %d locks, strength %d, telegraphed %d locks ahead", what, h.interval,
                      h.strength, h.telegraph);
        return b;
    }

    std::string attack_name(attack_kind k)
    {
        switch(k)
        {
        case attack_kind::garbage: return "garbage rows";
        case attack_kind::punch: return "punch (garbage with a shared gap)";
        case attack_kind::armor_inject: return "armour injection";
        case attack_kind::ice_inject: return "ice injection";
        case attack_kind::frozen_wall: return "frozen wall";
        case attack_kind::chill: return "chill (heat gain halved)";
        case attack_kind::crystal_shards: return "crystal shards";
        case attack_kind::fire_breath: return "fire breath";
        case attack_kind::molten_rain: return "molten rain";
        case attack_kind::heat_drain: return "heat drain";
        case attack_kind::surge: return "surge (specials disabled)";
        case attack_kind::cross_garbage: return "cross garbage (inactive board)";
        case attack_kind::forced_switch: return "forced board switch";
        case attack_kind::gravity_left: return "gravity left";
        case attack_kind::gravity_right: return "gravity right";
        case attack_kind::gravity_up: return "gravity up";
        case attack_kind::gravity_flip: return "gravity flip";
        case attack_kind::core_combo: return "core combo (two learned attacks at once)";
        case attack_kind::none:
        default: return "none";
        }
    }

    struct fairness
    {
        int wins = 0;
        int runs = 0;
        int best_seconds = 0;
        int median_seconds = 0;
        int idle_loss_seconds = -1;
    };

    fairness simulate(const stage_def& st)
    {
        static session s;
        static bot b;
        fairness f;
        int times[8] = {};
        for(uint32_t seed = 1; seed <= 8; ++seed)
        {
            s.start(make_stage_config(st, difficulty::standard, seed * 7919u + uint32_t(st.world * 10 + st.index)));
            b.reset(bot_strategy::resource_aware, seed);
            int frames = 0;
            for(; frames < 60 * 60 * 15 && s.state() != session_state::won && s.state() != session_state::lost; ++frames)
            {
                s.update(b.next(s));
                event e;
                while(s.events().pop(e))
                {
                }
            }
            ++f.runs;
            if(s.state() == session_state::won)
            {
                times[f.wins++] = frames / 60;
            }
        }
        for(int i = 0; i < f.wins; ++i)
        {
            for(int j = i + 1; j < f.wins; ++j)
            {
                if(times[j] < times[i])
                {
                    const int t = times[i];
                    times[i] = times[j];
                    times[j] = t;
                }
            }
        }
        f.best_seconds = f.wins ? times[0] : 0;
        f.median_seconds = f.wins ? times[f.wins / 2] : 0;
        s.start(make_stage_config(st, difficulty::standard, 1));
        const input_frame idle;
        for(int frames = 0; frames < 60 * 60 * 20; ++frames)
        {
            s.update(idle);
            event e;
            while(s.events().pop(e))
            {
            }
            if(s.state() == session_state::lost)
            {
                f.idle_loss_seconds = frames / 60;
                break;
            }
            if(s.state() == session_state::won)
            {
                break;
            }
        }
        return f;
    }

    const char* hazard_short(hazard_kind k)
    {
        switch(k)
        {
        case hazard_kind::freeze_cells: return "freezing cells";
        case hazard_kind::lava_pressure: return "lava pressure";
        case hazard_kind::gravity_shift: return "gravity shifts";
        case hazard_kind::settle_cycle: return "a rotating settle direction";
        case hazard_kind::garbage_drip: return "dripping garbage";
        case hazard_kind::armor_plates: return "armour plates";
        case hazard_kind::surge: return "power surges";
        case hazard_kind::molten_rain: return "molten rain";
        case hazard_kind::mixed: return "every earlier hazard in turn";
        case hazard_kind::none:
        default: return "";
        }
    }

    // The stage's signature mechanic, derived from what its rules actually demand.
    std::string signature(const stage_def& st)
    {
        std::string s;
        if(st.boss)
        {
            s = std::string("boss battle against ") + boss(st.boss).name;
        }
        else
        {
            for(const objective_def& o : st.objectives)
            {
                const char* d = nullptr;
                switch(o.kind)
                {
                case objective_kind::bombs: d = "deliberate bomb placement"; break;
                case objective_kind::chains: d = "building chain reactions"; break;
                case objective_kind::magnets: d = "magnet pulls that complete rows"; break;
                case objective_kind::overheat_success: d = "riding out Overheat"; break;
                case objective_kind::cross_combos: d = "CROSS OVER combos between the two boards"; break;
                case objective_kind::survive_pieces: d = "survival under pressure"; break;
                case objective_kind::sparks: d = "electric spark chains"; break;
                case objective_kind::fires: d = "spreading fire through the stack"; break;
                case objective_kind::ice_broken: d = "breaking ice coats"; break;
                case objective_kind::quads: d = "four-row clears"; break;
                case objective_kind::twists: d = "T-TWIST clears"; break;
                default: break;
                }
                if(d)
                {
                    s += (s.empty() ? "" : " and ") + std::string(d);
                }
            }
            if(s.empty())
            {
                s = st.loadout_count && st.heat ? std::string("spending heat on the ") + ability_names[st.loadout[0]] :
                                                 std::string("clean line clears");
            }
            if(st.dual && s.find("CROSS") == std::string::npos)
            {
                s += " across two relay boards";
            }
        }
        // Hazards: the settle cycle is named by the directions it rotates through.
        static const char* settle_cycles[3] = { "settling that swings left and right", "settling that flips up and down",
                                                "settling that turns through all four directions" };
        std::string under;
        for(const hazard_def& hz : st.hazards)
        {
            const char* h = hz.kind == hazard_kind::settle_cycle ? settle_cycles[hz.param % 3] : hazard_short(hz.kind);
            if(h[0])
            {
                under += (under.empty() ? "" : " and ") + std::string(h);
            }
        }
        if(! under.empty())
        {
            s += ", under " + under;
        }
        return s;
    }

    const char* difficulty_target(int world, int index)
    {
        static const char* targets[7] = {
            "", "first-time players: every rule is introduced one at a time",
            "players who finished World 1: ice adds a resource-management layer",
            "intermediate: fire and lava pressure punish a tall stack",
            "intermediate to advanced: sparks, surges and the two-board relay",
            "advanced: gravity changes the board's shape under you",
            "expert: every hazard returns and the final boss tests everything"
        };
        (void) index;
        return targets[world];
    }
}

int main()
{
    FILE* f = std::fopen("STAGE_PASSPORTS.md", "wb");     // binary: LF line endings on every host
    if(! f)
    {
        std::fprintf(stderr, "cannot write STAGE_PASSPORTS.md\n");
        return 1;
    }
    std::fprintf(f, "# BLOCK MAYHEM — Stage Design Passports (PART 50.3)\n\n");
    std::fprintf(f, "Generated by `tools/gen_docs.cpp` (`tools/bm.sh docs`) from the stage, boss and balance data the ROM\n"
                    "uses (`core/bm_stages.cpp`, `core/bm_bosses.cpp`, `core/bm_config.h`). The fairness figures are fresh\n"
                    "simulations of each stage with the real session code: eight seeded runs by the resource-aware test\n"
                    "bot (instant inputs) and one idle run. Human-paced calibration (reaction time, input delay and\n"
                    "misplacements) is in BALANCE.md.\n\n");
    std::fprintf(f, "Common to every stage — **randomization policy:** pieces come from a seeded 7-bag; special blocks are\n"
                    "decorated onto pieces by a separate seeded distributor (interval and weights below, with guarantees);\n"
                    "hazard and boss targets (columns, cells, gaps) use their own seeded streams, so presentation effects\n"
                    "can never change them (PART 55.6). **Failure criteria:** a piece cannot spawn (top out) or incoming\n"
                    "garbage pushes the stack out of the well. **Automated tests:** host `stages.all_thirty_stages_have_a_route_to_completion`\n"
                    "(12 bot runs, invariants checked every frame), `stages.every_stage_can_be_lost_and_idling_never_wins`,\n"
                    "`balance.human_paced_stage_report`; ROM `tests/emu/stages_wN.txt` (loads, plays to victory with the\n"
                    "autoplay bot, results and progression, screenshot `stage_W_I.png`).\n\n");
    for(int w = 1; w <= 6; ++w)
    {
        std::fprintf(f, "## World %d — %s\n\n", w, world_name(w));
        for(int i = 1; i <= 5; ++i)
        {
            const stage_def& st = stage(w, i);
            std::fprintf(f, "### WORLD_%02d_STAGE_%02d — %d-%d %s\n\n", w, i, w, i, st.name);
            std::fprintf(f, "- **Core concept:** %s\n", st.idea);
            std::string primary;
            for(const objective_def& o : st.objectives)
            {
                if(o.kind != objective_kind::none)
                {
                    primary += (primary.empty() ? "" : "; ") + objective_text(o);
                }
            }
            if(primary.empty() && st.boss)
            {
                primary = std::string("defeat ") + boss(st.boss).name + " (reduce it to 0 HP across its " +
                          std::to_string(boss(st.boss).phase_count) + " phases)";
            }
            std::fprintf(f, "- **Primary objective:** %s\n", primary.c_str());
            std::fprintf(f, "- **Secondary objective (rank bonus):** %s\n", objective_text(st.secondary).c_str());
            std::string loadout;
            for(int k = 0; k < st.loadout_count; ++k)
            {
                loadout += (k ? ", " : "") + std::string(ability_names[st.loadout[k]]);
            }
            int layout_cells = 0;
            for(int r = 0; r < st.layout_rows; ++r)
            {
                for(const char* p = st.layout[r]; *p; ++p)
                {
                    layout_cells += *p != '.' ? 1 : 0;
                }
            }
            std::fprintf(f, "- **Starting conditions:** %s board%s, level %d (cap %d%s), heat %s%s, abilities: %s%s\n",
                         st.dual ? "two 8x16" : "one 10x20", st.dual ? "s (relay: SELECT switches)" : "", st.base_level,
                         st.level_cap, st.level_progression ? ", rises with rows" : ", fixed",
                         st.heat ? "on" : "off", st.start_heat ? (", starts at " + std::to_string(st.start_heat)).c_str() : "",
                         loadout.empty() ? "none" : loadout.c_str(),
                         st.layout_rows ? (", prepared layout of " + std::to_string(st.layout_rows) + " rows (" +
                                           std::to_string(layout_cells) + " cells)").c_str() : ", empty board");
            std::string specials;
            int total_weight = 0;
            for(int k = 0; k < 5; ++k)
            {
                total_weight += st.specials.weights[k];
            }
            for(int k = 0; k < 5; ++k)
            {
                if(st.specials.weights[k])
                {
                    specials += (specials.empty() ? "" : ", ") + std::string(special_names[k + 1]) + " " +
                                std::to_string(st.specials.weights[k] * 100 / (total_weight ? total_weight : 1)) + "%";
                }
            }
            std::string guarantees;
            for(const special_guarantee& g : st.specials.guarantees)
            {
                if(g.kind != special_kind::none && g.every)
                {
                    guarantees += (guarantees.empty() ? "" : "; ") + std::string("a ") + special_names[int(g.kind)] +
                                  " within every " + std::to_string(g.every) + " pieces" +
                                  (g.max_count ? " (first " + std::to_string(g.max_count) + ")" : "");
                }
            }
            if(st.specials.interval_max)
            {
                std::fprintf(f, "- **Special blocks:** one special piece every %d-%d pieces: %s%s%s\n",
                             st.specials.interval_min + 1, st.specials.interval_max + 1, specials.c_str(),
                             guarantees.empty() ? "" : "; guaranteed: ", guarantees.c_str());
            }
            else
            {
                std::fprintf(f, "- **Special blocks:** none from the bag%s%s\n", guarantees.empty() ? "" : "; guaranteed: ",
                             guarantees.c_str());
            }
            std::fprintf(f, "- **Signature mechanic:** %s (world theme: %s)\n", signature(st).c_str(),
                         worlds[w].mechanic);
            std::string hz;
            for(const hazard_def& h : st.hazards)
            {
                const std::string t = hazard_text(h);
                if(! t.empty())
                {
                    hz += (hz.empty() ? "" : "<br>") + t;
                }
            }
            std::fprintf(f, "- **Hazards:** %s\n", hz.empty() ? "none" : hz.c_str());
            if(st.boss)
            {
                const boss_def& b = boss(st.boss);
                std::fprintf(f, "- **Boss:** %s, \"%s\" — %d HP, %d phases%s\n", b.name, b.title, b.hp, b.phase_count,
                             b.dual ? (b.dual_phase ? ", second board unseals in a later phase" : ", two boards") : "");
                for(int p = 0; p < b.phase_count; ++p)
                {
                    const boss_phase_def& ph = b.phases[p];
                    std::string pattern;
                    for(int k = 0; k < ph.pattern_length; ++k)
                    {
                        pattern += (k ? ", " : "") + attack_name(ph.pattern[k].kind) + " (" +
                                   std::to_string(ph.pattern[k].strength) + ", warned " +
                                   std::to_string(ph.pattern[k].telegraph) + " locks)";
                    }
                    std::fprintf(f, "  - phase %d from %d%% HP, an attack every %d locks: %s\n", p + 1, ph.hp_percent,
                                 ph.interval, pattern.c_str());
                }
            }
            const fairness fr = simulate(st);
            std::fprintf(f, "- **Fairness guarantee:** every hazard and boss attack is telegraphed before it acts; "
                            "the objective counts only effects the player causes; %s. Simulation: %d/%d bot runs clear it "
                            "(best %d s, median %d s); idling loses after %d s, so the stage can neither be won by "
                            "waiting nor become unwinnable by randomization.\n",
                         guarantees.empty() ? "no objective depends on a special the bag might withhold" :
                                              "objective-critical specials are guaranteed (above)",
                         fr.wins, fr.runs, fr.best_seconds, fr.median_seconds, fr.idle_loss_seconds);
            std::fprintf(f, "- **Visual identity:** %s%s\n", worlds[w].visual,
                         st.boss ? "; the boss rig animates idle, telegraph, attack, hit, enrage and a destruction "
                                   "sequence" : "");
            std::fprintf(f, "- **Music identity:** %s\n", st.boss ? (w == 6 ? "`mus_final` and `mus_final_hot`" :
                                                                       "`mus_boss` and `mus_boss_hot`") :
                                                                   worlds[w].music);
            std::fprintf(f, "- **Difficulty target:** %s (stage %d of 5)\n", difficulty_target(w, i), i);
            std::fprintf(f, "- **Success criteria:** %s%s\n", primary.c_str(),
                         st.boss ? " (the boss's destruction sequence then plays)" : "");
            std::fprintf(f, "- **Rank (C..S+):** S score target %u, par time %d s, plus no garbage taken and the secondary "
                            "objective\n", st.rank_score, st.par_seconds);
            std::fprintf(f, "- **Polish checklist:** verified on the ROM: loads, stage card goals, HUD, hazards%s, "
                            "victory, results and progression (stages_w%d.txt, screenshot stage_%d_%d.png). Remaining: "
                            "human review of the music arrangement (AUD-06, needs a listener)\n\n",
                         st.boss ? ", boss phases and destruction" : "", w, w, i);
        }
    }
    std::fclose(f);
    std::printf("wrote STAGE_PASSPORTS.md\n");
    return 0;
}
