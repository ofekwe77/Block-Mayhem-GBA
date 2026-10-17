#include "bm_flow.h"

#include "bm_achievements.h"
#include "bm_challenges.h"
#include "bm_cosmetics.h"
#include "bm_rank.h"
#include "bm_run.h"

namespace bm
{
    session_config make_session_config(const app& a)
    {
        session_config c;
        c.mode = a.flow.mode;
        c.diff = difficulty(a.save.settings.difficulty > 2 ? 1 : a.save.settings.difficulty);
        c.seed = a.flow.seed;
        switch(a.flow.mode)
        {
        case game_mode::classic:
            c.specials = false;
            c.heat = false;
            c.loadout_count = 0;
            c.start_level = 1;
            break;

        case game_mode::arcade:
            c.specials = true;
            c.heat = true;
            c.loadout[0] = a.flow.arcade_loadout[0];
            c.loadout[1] = a.flow.arcade_loadout[1];
            c.loadout[2] = a.flow.arcade_loadout[2];
            c.loadout_count = uint8_t(a.flow.arcade_count >= 1 && a.flow.arcade_count <= 3 ? a.flow.arcade_count : 3);
            c.endless = true;
            c.stage = &arcade_stage();      // controlled environmental events
            break;

        case game_mode::dual_relay:
            c.dual = true;
            c.specials = true;
            c.heat = true;
            c.loadout[0] = a.flow.dual_ability;
            c.loadout_count = 1;
            c.endless = true;
            break;

        case game_mode::adventure:
            c = make_stage_config(stage(a.flow.world, a.flow.stage), c.diff, c.seed);
            break;

        case game_mode::chaos_run:
            c = a.run->make_config(c.diff);
            break;

        case game_mode::boss_rush:
        {
            const int world = a.flow.boss_rush_worlds[a.flow.boss_rush_index < a.flow.boss_rush_count ?
                                                       a.flow.boss_rush_index : 0];
            c = make_stage_config(stage(world ? world : 1, 5), c.diff, c.seed);
            c.mode = game_mode::boss_rush;
            c.start_score = a.flow.boss_rush_score;
            break;
        }

        default:
            break;
        }
        if(c.mode == game_mode::classic || c.mode == game_mode::arcade || c.mode == game_mode::dual_relay)
        {
            c.challenges = uint8_t(a.save.settings.challenges & challenges_unlocked(a.save));
        }
        return c;
    }

    const char* mode_name(game_mode mode)
    {
        switch(mode)
        {
        case game_mode::classic: return "CLASSIC";
        case game_mode::arcade: return "CHAOS ARCADE";
        case game_mode::adventure: return "ADVENTURE";
        case game_mode::chaos_run: return "CHAOS RUN";
        case game_mode::dual_relay: return "DUAL BOARD";
        case game_mode::boss_rush: return "BOSS RUSH";
        default: return "";
        }
    }

    const char* ability_name(int ability)
    {
        static constexpr const char* names[] = {
            "LASER BEAM", "TIME FREEZE", "GRAVITY PULSE", "SHAPE SHIFT",
            "NUCLEAR BURST", "SHIELD MATRIX", "CHAIN SPARK", "EMERGENCY VENT"
        };
        return ability >= 0 && ability < cfg::ability_count ? names[ability] : "";
    }

    const char* ability_short(int ability)
    {
        static constexpr const char* names[] = { "LASER", "FREEZE", "GRAVITY", "SHIFT", "NUKE", "SHIELD", "SPARK", "VENT" };
        return ability >= 0 && ability < cfg::ability_count ? names[ability] : "";
    }

    const char* ability_desc(int ability)
    {
        static constexpr const char* descs[] = {
            "BEAM THROUGH ONE COLUMN", "3 SECONDS WITHOUT GRAVITY", "COMPACT THE BOARD ONE WAY",
            "CHANGE THE FALLING PIECE", "5X5 POWER-2 BLAST", "BLOCK THE NEXT ATTACK", "CHARGE A CELL TO PULSE",
            "REMOVE THE 8 HIGHEST CELLS"
        };
        return ability >= 0 && ability < cfg::ability_count ? descs[ability] : "";
    }

    int ability_unlock_stage(int ability)
    {
        for(int w = 1; w <= 6; ++w)
        {
            for(int i = 1; i <= 5; ++i)
            {
                const stage_def& st = stage(w, i);
                for(int k = 0; k < st.loadout_count; ++k)
                {
                    if(st.loadout[k] == ability)
                    {
                        return stage_number(w, i);
                    }
                }
            }
        }
        return -1;
    }

    bool track_unlocked(const save_data& sv, int music_id)
    {
        int cleared = 0;
        for(const stage_record& r : sv.stages)
        {
            cleared += r.cleared ? 1 : 0;
        }
        switch(music_id)
        {
        case 3: case 4: case 5: case 6: case 7: case 8:     // world themes: once the world is open
            return sv.worlds_unlocked >= music_id - 2;
        case 9:     // boss theme: after the first boss
            return sv.stages[stage_number(1, 5)].cleared != 0;
        case 10:    // final boss: once World 6 is open
            return sv.worlds_unlocked >= 6;
        case 11:    // stage clear: after any cleared stage
            return cleared > 0;
        case 13:    // ending: campaign complete
            return sv.campaign_complete != 0;
        default:    // title, menu, game over, classic
            return music_id >= 1 && music_id <= 14;
        }
    }

    const char* heat_label(int heat, bool overheat)
    {
        if(overheat)
        {
            return "OVERHEAT";
        }
        if(heat >= 80) return "INFERNO";
        if(heat >= 60) return "SPICY";
        if(heat >= 40) return "HOT";
        if(heat >= 20) return "WARM";
        return "MILD";
    }

    void objective_text(const objective_def& o, bn::istring_base& out)
    {
        bn::ostringstream s(out);
        switch(o.kind)
        {
        case objective_kind::lines: s << "CLEAR " << o.target << " ROWS"; break;
        case objective_kind::bombs: s << "TRIGGER " << o.target << " BOMBS"; break;
        case objective_kind::chains: s << o.target << " CHAINS OF " << int(o.param) << "+ WAVES"; break;
        case objective_kind::magnets: s << o.target << " MAGNET PULLS"; break;
        case objective_kind::overheat_success: s << "MASTER " << o.target << " OVERHEAT"; break;
        case objective_kind::cross_combos: s << o.target << " CROSS OVER COMBOS"; break;
        case objective_kind::survive_pieces: s << "SURVIVE " << o.target << " PIECES"; break;
        case objective_kind::boss_defeat: s << "DEFEAT THE BOSS"; break;
        case objective_kind::sparks: s << "TRIGGER " << o.target << " SPARKS"; break;
        case objective_kind::fires: s << "TRIGGER " << o.target << " FIRES"; break;
        case objective_kind::ice_broken: s << "BREAK " << o.target << " ICE"; break;
        case objective_kind::quads: s << o.target << " QUAD BLASTS"; break;
        case objective_kind::twists: s << o.target << " T-TWISTS"; break;
        case objective_kind::max_height: s << "STACK NEVER ABOVE " << o.target; break;
        case objective_kind::no_garbage: s << "TAKE NO GARBAGE"; break;
        case objective_kind::time_limit: s << "FINISH IN " << o.target << " SECONDS"; break;
        default: break;
        }
    }

    bool stage_unlocked(const save_data& sv, int world, int index)
    {
        if(world > sv.worlds_unlocked)
        {
            return false;
        }
        if(index <= 1)
        {
            return true;
        }
        return sv.stages[stage_number(world, index - 1)].cleared != 0;
    }

    int world_cleared_stages(const save_data& sv, int world)
    {
        int n = 0;
        for(int i = 1; i <= 5; ++i)
        {
            n += sv.stages[stage_number(world, i)].cleared ? 1 : 0;
        }
        return n;
    }

    bool record_session(app& a)
    {
        const session& s = *a.game;
        save_data& sv = a.save;
        bool record = false;
        const uint8_t challenges_before = challenges_unlocked(sv);

        auto insert_high = [&record](uint32_t* table, uint32_t score)
        {
            for(int i = 0; i < 5; ++i)
            {
                if(score > table[i])
                {
                    for(int j = 4; j > i; --j)
                    {
                        table[j] = table[j - 1];
                    }
                    table[i] = score;
                    record = i == 0;
                    return;
                }
            }
        };

        switch(s.config().mode)
        {
        case game_mode::classic:
            insert_high(sv.classic_high, s.score());
            break;
        case game_mode::arcade:
            insert_high(sv.arcade_high, s.score());
            break;
        case game_mode::dual_relay:
            insert_high(sv.dual_high, s.score());
            break;
        case game_mode::adventure:
        {
            const stage_def* st = s.config().stage;
            a.flow.rank = 0;
            if(st && s.state() == session_state::won)
            {
                stage_record& rec = sv.stages[stage_number(st->world, st->index)];
                const int rank = stage_rank(s);
                const uint16_t secs = uint16_t(s.stats().frames / 60 > 65535 ? 65535 : s.stats().frames / 60);
                a.flow.rank = uint8_t(rank);
                if(! rec.cleared || s.score() > rec.best_score)
                {
                    record = rec.cleared != 0;
                    rec.best_score = s.score();
                }
                if(! rec.cleared || secs < rec.best_seconds)
                {
                    rec.best_seconds = secs;
                }
                if(rank > rec.best_rank)
                {
                    rec.best_rank = uint8_t(rank);
                }
                rec.cleared = 1;
                // Abilities met in the campaign become available in Chaos Arcade.
                for(int i = 0; i < st->loadout_count; ++i)
                {
                    sv.abilities_unlocked = uint8_t(sv.abilities_unlocked | (1u << st->loadout[i]));
                }
                if(st->boss)
                {
                    ++sv.stats.bosses_defeated;
                    if(st->world < 6 && sv.worlds_unlocked <= st->world)
                    {
                        sv.worlds_unlocked = uint8_t(st->world + 1);
                    }
                    if(st->world >= 3)
                    {
                        sv.boss_rush_unlocked = 1;
                    }
                    if(st->world == 6)
                    {
                        sv.campaign_complete = 1;
                    }
                    sv.music_unlocked |= 1u << (st->world + 1);
                }
            }
            break;
        }
        case game_mode::boss_rush:
            if(a.flow.boss_rush_state == 0)
            {
                a.flow.boss_rush_frames += s.stats().frames;
                if(s.state() == session_state::won)
                {
                    ++sv.stats.bosses_defeated;
                    a.flow.boss_rush_score = s.score();
                    ++a.flow.boss_rush_index;
                    if(a.flow.boss_rush_index >= a.flow.boss_rush_count)
                    {
                        record = end_boss_rush(a, true);
                    }
                }
                else if(a.flow.boss_rush_lives > 0)
                {
                    --a.flow.boss_rush_lives;      // recovery: retry the same boss
                }
                else
                {
                    record = end_boss_rush(a, false);
                }
            }
            break;

        case game_mode::chaos_run:
            if(a.run->phase() == run_phase::encounter)
            {
                a.run->finish_encounter(s);
                if(a.run->phase() == run_phase::victory || a.run->phase() == run_phase::defeat)
                {
                    record = record_run_end(a);
                }
                if(s.config().boss && s.state() == session_state::won)
                {
                    ++sv.stats.bosses_defeated;
                }
            }
            break;

        default:
            break;
        }

        // Lifetime statistics.
        const session_stats& st = s.stats();
        sv.stats.lines += st.lines;
        sv.stats.pieces += st.pieces;
        sv.stats.bombs += st.bombs;
        sv.stats.fires += st.fires;
        sv.stats.sparks += st.sparks;
        sv.stats.ice_broken += st.ice_broken;
        sv.stats.overheats += st.overheats;
        sv.stats.overheat_success += st.overheat_success;
        sv.stats.chains += st.chains2;
        if(st.max_chain > sv.stats.max_chain)
        {
            sv.stats.max_chain = st.max_chain;
        }
        sv.stats.quads += st.quads;
        sv.stats.play_seconds += st.frames / 60;
        sv.stats.cross_combos += st.cross_combos;
        sv.stats.abilities_used += st.abilities_used;
        a.flow.new_record = record;

        achievement_input in;
        in.live = &s;
        in.merged = true;
        in.session_over = true;
        in.stage_rank = s.config().mode == game_mode::adventure ? a.flow.rank : 0;
        a.pending_achievements |= update_achievements(sv, in);
        // Rewards (PART 27.3): challenge modifiers and cosmetics earned by this session.
        a.flow.new_challenges = uint8_t(challenges_unlocked(sv) & ~challenges_before);
        a.flow.new_cosmetics = update_cosmetics(sv);
        return record;
    }

    int boss_rush_available(const save_data& sv)
    {
        int n = 0;
        for(int w = 1; w <= 6; ++w)
        {
            n += sv.stages[stage_number(w, 5)].cleared ? 1 : 0;
        }
        return sv.boss_rush_unlocked ? n : 0;
    }

    bool begin_boss_rush(app& a)
    {
        flow_state& f = a.flow;
        f.boss_rush_count = 0;
        for(int w = 1; w <= 6; ++w)
        {
            if(a.save.stages[stage_number(w, 5)].cleared)
            {
                f.boss_rush_worlds[f.boss_rush_count++] = uint8_t(w);
            }
        }
        f.mode = game_mode::boss_rush;
        f.boss_rush_index = 0;
        f.boss_rush_lives = uint8_t(boss_rush_lives);
        f.boss_rush_state = 0;
        f.boss_rush_score = 0;
        f.boss_rush_frames = 0;
        f.world = f.boss_rush_count ? f.boss_rush_worlds[0] : 1;
        return f.boss_rush_count > 0;
    }

    bool end_boss_rush(app& a, bool complete)
    {
        flow_state& f = a.flow;
        save_data& sv = a.save;
        f.boss_rush_state = complete ? 1 : 2;
        bool record = false;
        if(f.boss_rush_score > sv.boss_rush_best_score)
        {
            record = sv.boss_rush_best_score != 0 || complete;
            sv.boss_rush_best_score = f.boss_rush_score;
        }
        // Best time only for a full six-boss rush (shorter rushes are not comparable).
        if(complete && f.boss_rush_count == 6 &&
           (! sv.boss_rush_best_frames || f.boss_rush_frames < sv.boss_rush_best_frames))
        {
            record = true;
            sv.boss_rush_best_frames = f.boss_rush_frames;
        }
        a.flow.new_record = record;
        return record;
    }

    bool record_run_end(app& a)
    {
        save_data& sv = a.save;
        const chaos_run& r = *a.run;
        const bool won = r.phase() == run_phase::victory;
        bool record = false;
        if(r.score() > sv.run_best_score)
        {
            record = sv.run_best_score != 0 || won;
            sv.run_best_score = r.score();
        }
        const int reached = won ? 4 : r.act() + 1;     // 4 = complete run
        if(reached > sv.run_best_act)
        {
            sv.run_best_act = uint8_t(reached);
        }
        if(won && sv.runs_completed < 0xFFFF)
        {
            ++sv.runs_completed;
        }
        a.flow.new_record = record;
        return record;
    }
}
