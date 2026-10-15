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
