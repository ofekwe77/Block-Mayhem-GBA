#include "bm_flow.h"

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
            c.loadout_count = 3;
            c.endless = true;
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

        default:
            break;
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

    int stage_rank(const session& s)
    {
        const stage_def* st = s.config().stage;
        if(! st || s.state() != session_state::won)
        {
            return 0;
        }
        // Transparent criteria: score vs the stage's S target, time vs par, garbage received, secondary objective.
        int points = 0;
        const uint32_t target = st->rank_score;
        if(s.score() >= target) points += 3;
        else if(s.score() * 3 >= target * 2) points += 2;
        else if(s.score() * 3 >= target) points += 1;
        const uint32_t seconds = s.stats().frames / 60;
        if(seconds <= st->par_seconds) points += 2;
        else if(seconds <= st->par_seconds * 3 / 2u) points += 1;
        if(s.stats().garbage_received == 0) points += 1;
        if(s.secondary().done) points += 2;
        // 0..8 points -> C .. S+
        if(points >= 8) return 5;
        if(points >= 6) return 4;
        if(points >= 4) return 3;
        if(points >= 2) return 2;
        return 1;
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
        return record;
    }
}
