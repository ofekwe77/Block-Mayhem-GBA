#include "bm_achievements.h"

namespace bm
{
    namespace
    {
        // icon = spr_goals frame: 0 star, 1 rows, 2 bomb, 3 chain, 4 magnet, 5 thermometer, 6 cross, 7 T piece,
        // 8 skull, 9 bolt, 10 flame, 11 ice, 12 four, 13 twist, 14 limit, 15 shield, 16 clock.
        constexpr achievement_def defs[achievement_total] = {
            { "FIRST LINE", "CLEAR YOUR FIRST ROW", 1, 1, false },
            { "DOUBLE TROUBLE", "CLEAR 2 ROWS AT ONCE", 1, 1, false },
            { "TRIPLE THREAT", "CLEAR 3 ROWS AT ONCE", 1, 1, false },
            { "PERFECT FOUR", "CLEAR 4 ROWS AT ONCE", 1, 12, false },
            { "HOT STUFF", "ACTIVATE OVERHEAT", 1, 5, false },
            { "HEAT MASTER", "COMPLETE AN OVERHEAT", 1, 5, false },
            { "BOMB SQUAD", "TRIGGER 10 BOMBS", 10, 2, false },
            { "CHAIN ROOKIE", "TRIGGER A 2-WAVE CHAIN", 2, 3, false },
            { "CHAIN MASTER", "TRIGGER A 5-WAVE CHAIN", 5, 3, false },
            { "MELTDOWN", "REACH AN 8-WAVE CHAIN", 8, 3, false },
            { "ELECTRICIAN", "TRIGGER 20 SPARK CELLS", 20, 9, false },
            { "FIRESTARTER", "TRIGGER 15 FIRE CELLS", 15, 10, false },
            { "ICE BREAKER", "BREAK 30 ICE COATS", 30, 11, false },
            { "LASER EXPERT", "CLEAR A ROW WITH THE LASER", 1, 9, false },
            { "GRAVITY WIZARD", "WIN A FIGHT USING GRAVITY", 1, 13, false },
            { "CROSS OVER", "LAND A DUAL-BOARD COMBO", 1, 6, false },
            { "FACTORY HERO", "DEFEAT THE BLOCK MONSTER", 1, 8, false },
            { "WORLD TRAVELER", "CONQUER THREE WORLDS", 3, 0, false },
            { "MAYHEM CHAMPION", "COMPLETE THE CAMPAIGN", 1, 0, false },
            { "CHAOS SURVIVOR", "FINISH A CHAOS RUN", 1, 7, false },
            { "SPEED DEMON", "CLEAR A STAGE UNDER PAR", 1, 16, false },
            { "UNTOUCHABLE", "BEAT A BOSS UNHARMED", 1, 15, false },
            { "PUZZLE GENIUS", "EARN AN S+ STAGE RANK", 1, 0, false },
            { "LAST STAND", "RECOVER FROM CRITICAL", 1, 14, false },
            { "MASTER OF MAYHEM", "UNLOCK 20 ACHIEVEMENTS", 20, 0, false },
            { "CLEAN SWEEP", "EMPTY THE WHOLE BOARD", 1, 1, true },
            { "ARSENAL", "USE ALL EIGHT ABILITIES", 8, 7, true },
            { "IRON WALL", "BLOCK 5 ATTACKS IN A GAME", 5, 15, true },
            { "RUSH HOUR", "BEAT ALL SIX BOSSES IN A RUSH", 1, 8, true },
            { "SPICE LORD", "3 OVERHEATS IN ONE GAME", 3, 5, true },
            { "SPICE DETECTIVE", "WITNESS 4 SECRET EVENTS", 4, 0, true },
        };

        int popcount8(uint32_t v)
        {
            int n = 0;
            for(int i = 0; i < 8; ++i)
            {
                n += (v >> i) & 1;
            }
            return n;
        }

        bool stage_mode(game_mode m)
        {
            return m == game_mode::adventure || m == game_mode::chaos_run || m == game_mode::boss_rush;
        }
    }

    const achievement_def& achievement(int id)
    {
        return defs[id >= 0 && id < achievement_total ? id : 0];
    }

    int achievements_unlocked(const save_data& sv)
    {
        int n = 0;
        for(int i = 0; i < achievement_total; ++i)
        {
            n += (sv.achievements >> i) & 1;
        }
        return n;
    }

    uint32_t update_achievements(save_data& sv, const achievement_input& in)
    {
        const session* s = in.live;
        session_stats none;
        const session_stats& st = s ? s->stats() : none;
        const bool add = s && ! in.merged;
        const bool won = s && in.session_over && s->state() == session_state::won;
        auto life = [add](uint32_t lifetime, uint32_t live) { return lifetime + (add ? live : 0u); };

        // ARSENAL keeps the mask of abilities ever used in its progress slot.
        if(s)
        {
            uint32_t mask = sv.achievement_progress[ach_arsenal];
            for(int a = 0; a < cfg::ability_count; ++a)
            {
                if(st.ability_uses[a])
                {
                    mask |= 1u << a;
                }
            }
            sv.achievement_progress[ach_arsenal] = uint16_t(mask & 0xFF);
        }

        int bosses = 0;
        for(int w = 1; w <= 6; ++w)
        {
            bosses += sv.stages[stage_number(w, 5)].cleared ? 1 : 0;
        }
        const uint32_t max_chain = st.max_chain > sv.stats.max_chain ? st.max_chain : sv.stats.max_chain;

        uint32_t value[achievement_total] = {};
        value[ach_first_line] = life(sv.stats.lines, st.lines);
        value[ach_double_trouble] = uint32_t(st.doubles + st.triples + st.quads);
        value[ach_triple_threat] = uint32_t(st.triples + st.quads);
        value[ach_perfect_four] = life(sv.stats.quads, st.quads);
        value[ach_hot_stuff] = life(sv.stats.overheats, st.overheats);
        value[ach_heat_master] = life(sv.stats.overheat_success, st.overheat_success);
        value[ach_bomb_squad] = life(sv.stats.bombs, st.bombs);
        value[ach_chain_rookie] = max_chain;
        value[ach_chain_master] = max_chain;
        value[ach_meltdown] = max_chain;
        value[ach_electrician] = life(sv.stats.sparks, st.sparks);
        value[ach_firestarter] = life(sv.stats.fires, st.fires);
        value[ach_ice_breaker] = life(sv.stats.ice_broken, st.ice_broken);
        value[ach_laser_expert] = st.laser_clears;
        value[ach_gravity_wizard] = won && st.gravity_uses && stage_mode(s->config().mode) ? 1 : 0;
        value[ach_cross_over] = life(sv.stats.cross_combos, st.cross_combos);
        value[ach_factory_hero] = sv.stages[stage_number(1, 5)].cleared ? 1 : 0;
        value[ach_world_traveler] = uint32_t(bosses);
        value[ach_mayhem_champion] = sv.campaign_complete ? 1 : 0;
        value[ach_chaos_survivor] = sv.runs_completed;
        value[ach_speed_demon] = won && s->config().mode == game_mode::adventure && s->config().stage &&
                                 ! s->config().boss && st.frames / 60 <= s->config().stage->par_seconds ? 1 : 0;
        value[ach_untouchable] = won && s->config().boss && st.hostile_attacks_taken == 0 ? 1 : 0;
        value[ach_puzzle_genius] = in.stage_rank >= 5 ? 1 : 0;
        value[ach_last_stand] = st.last_stand;
        value[ach_clean_sweep] = st.perfect_clears;
        value[ach_arsenal] = uint32_t(popcount8(sv.achievement_progress[ach_arsenal]));
        value[ach_iron_wall] = st.shield_blocks;
        value[ach_rush_hour] = sv.boss_rush_best_frames ? 1 : 0;
        value[ach_spice_lord] = st.overheat_success;
        value[ach_spice_detective] = uint32_t(popcount8(sv.secrets_found));

        uint32_t fresh = 0;
        for(int pass = 0; pass < 2; ++pass)
        {
            if(pass == 1)
            {
                // MASTER OF MAYHEM counts the other unlocked achievements (after this update).
                int others = 0;
                for(int i = 0; i < achievement_total; ++i)
                {
                    others += i != ach_master_of_mayhem && ((sv.achievements | fresh) >> i) & 1 ? 1 : 0;
                }
                value[ach_master_of_mayhem] = uint32_t(others);
            }
            for(int i = 0; i < achievement_total; ++i)
            {
                if((pass == 0) == (i == ach_master_of_mayhem))
                {
                    continue;
                }
                const achievement_def& d = defs[i];
                if(i != ach_arsenal && d.target > 1)
                {
                    const uint32_t v = value[i] > d.target ? d.target : value[i];
                    if(v > sv.achievement_progress[i])
                    {
                        sv.achievement_progress[i] = uint16_t(v);
                    }
                }
                const uint32_t bit = 1u << i;
                if(! (sv.achievements & bit) && value[i] >= d.target)
                {
                    sv.achievements |= bit;
                    fresh |= bit;
                }
            }
        }
        return fresh;
    }
}
