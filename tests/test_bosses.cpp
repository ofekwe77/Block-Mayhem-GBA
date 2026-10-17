/*
 * Boss-specific verification entries (PART 51.5, 56.5): one entry per boss. Each plays the real boss stage
 * with the test bot at a skilled human pace, four seeds with the stage's abilities and four without (so no
 * Shield Matrix hides an attack), and audits the event stream of the fight:
 *   - initial HP equals the definition,
 *   - phases advance one at a time and only once HP is at or below the phase threshold,
 *   - attacks follow the current phase's pattern in order (scheduling),
 *   - every attack is telegraphed at least boss_min_telegraph_pieces locks ahead,
 *   - every attack kind of the boss runs and shows its effect on the board or the session,
 *   - damage is consistent (HP falls by exactly the damage reported, never rises, per-hit cap),
 *   - death: HP 0 raises boss_defeated and wins the stage.
 * Unlock progression after a boss (next world, Boss Rush, challenges, music) is checked by
 * challenges.unlocked_by_world_boss_clears and on the ROM by tests/emu/stages_w1..6.txt (worlds_unlocked).
 */
#include "bm_test.h"

#include "bm_bot.h"
#include "bm_session.h"

using namespace bm;

namespace
{
    struct boss_audit
    {
        int runs = 0;
        int wins = 0;
        int wins_with_abilities = 0;
        int attacks = 0;
        int telegraph_violations = 0;
        int schedule_violations = 0;
        int phase_violations = 0;
        int damage_violations = 0;
        int max_phase = 0;
        int initial_hp_errors = 0;
        int death_errors = 0;
        uint32_t kinds_seen = 0;        // bit per attack_kind executed
        uint32_t kinds_effect = 0;      // bit per attack_kind that showed an effect
    };

    bool effect_visible(attack_kind k, int affected, const session& s, bool gravity_event, bool heat_drop)
    {
        switch(k)
        {
        case attack_kind::chill: return s.chill_pieces() > 0;
        case attack_kind::surge: return s.surge_pieces() > 0;
        case attack_kind::heat_drain: return heat_drop || s.heat() == 0;
        case attack_kind::gravity_left:
        case attack_kind::gravity_right:
        case attack_kind::gravity_up:
        case attack_kind::gravity_flip: return gravity_event;
        case attack_kind::forced_switch: return true;      // checked through board_switched below
        case attack_kind::core_combo: return affected > 0 || gravity_event || s.surge_pieces() > 0;
        default: return affected > 0;
        }
    }

    void audit_fight(int world, uint32_t seed, bool abilities, boss_audit& out)
    {
        static session s;
        static bot b;
        const stage_def& st = stage(world, 5);
        const boss_def& def = boss(st.boss);
        session_config config = make_stage_config(st, difficulty::standard, seed);
        if(! abilities)
        {
            config.loadout_count = 0;   // no Shield Matrix: every attack lands and must show its effect
        }
        s.start(config);
        b.reset(bot_strategy::resource_aware, seed);
        bot_pace pace;
        pace.think_frames = 14;     // the "skilled" balance profile (BALANCE.md)
        pace.tap_interval = 4;
        pace.error_percent = 4;
        b.set_pace(pace);
        ++out.runs;
        if(s.boss_hp() != def.hp || s.boss_max_hp() != def.hp)
        {
            ++out.initial_hp_errors;
        }
        int phase = 0;
        int pattern_index = 0;
        int hp = s.boss_hp();
        int locks_since_telegraph = 0;
        int telegraph_kind = -1;
        int telegraph_board = -1;
        bool switch_expected = false;
        for(int f = 0; f < 60 * 60 * 20 && s.state() != session_state::won && s.state() != session_state::lost; ++f)
        {
            const int heat_before = s.heat();
            s.update(b.next(s));
            event e;
            bool gravity_event = false;
            bool heat_drop = false;
            int attack_kind_now = -1;
            int affected = 0;
            while(s.events().pop(e))
            {
                switch(e.type)
                {
                case event_type::piece_locked:
                    ++locks_since_telegraph;
                    break;
                case event_type::boss_telegraph:
                    telegraph_kind = e.a;
                    telegraph_board = e.board;
                    locks_since_telegraph = 0;
                    if(e.b < cfg::boss_min_telegraph_pieces)
                    {
                        ++out.telegraph_violations;
                    }
                    {
                        // Scheduling: the telegraphed attack is the next one of the current phase's pattern.
                        const boss_phase_def& ph = def.phases[phase];
                        if(ph.pattern_length && e.a != int(ph.pattern[pattern_index % ph.pattern_length].kind))
                        {
                            ++out.schedule_violations;
                        }
                    }
                    break;
                case event_type::boss_attack:
                    ++out.attacks;
                    ++pattern_index;
                    if(e.a != telegraph_kind || locks_since_telegraph < cfg::boss_min_telegraph_pieces)
                    {
                        ++out.telegraph_violations;
                    }
                    (void) telegraph_board;
                    telegraph_kind = -1;
                    attack_kind_now = e.a;
                    affected = e.b;
                    out.kinds_seen |= 1u << e.a;
                    if(attack_kind(e.a) == attack_kind::forced_switch)
                    {
                        switch_expected = true;
                    }
                    break;
                case event_type::shield_block:
                    if(e.a < 100 && e.b == 0)
                    {
                        // A shielded boss attack is consumed: telegraphed, scheduled, but without effect.
                        ++pattern_index;
                        if(e.a != telegraph_kind || locks_since_telegraph < cfg::boss_min_telegraph_pieces)
                        {
                            ++out.telegraph_violations;
                        }
                        telegraph_kind = -1;
                        out.kinds_seen |= 1u << e.a;
                    }
                    break;
                case event_type::gravity_shift:
                    gravity_event = true;
                    break;
                case event_type::heat_changed:
                    heat_drop = heat_drop || e.b < 0;
                    break;
                case event_type::board_switched:
                    if(switch_expected)
                    {
                        out.kinds_effect |= 1u << int(attack_kind::forced_switch);
                        switch_expected = false;
                    }
                    break;
                case event_type::boss_phase:
                    if(e.a != phase + 1 || s.boss_hp() * 100 > s.boss_max_hp() * def.phases[e.a].hp_percent)
                    {
                        ++out.phase_violations;
                    }
                    phase = e.a;
                    pattern_index = 0;
                    telegraph_kind = -1;      // the old phase's telegraph is cancelled
                    out.max_phase = phase > out.max_phase ? phase : out.max_phase;
                    break;
                case event_type::boss_damage:
                    if(e.a <= 0 || e.b != (hp - e.a > 0 ? hp - e.a : 0) || e.a > cfg::boss_chain_damage_cap + 40)
                    {
                        ++out.damage_violations;
                    }
                    hp = e.b;
                    break;
                case event_type::boss_defeated:
                    if(hp != 0)
                    {
                        ++out.death_errors;
                    }
                    break;
                default:
                    break;
                }
            }
            (void) heat_before;
            if(attack_kind_now >= 0 && attack_kind(attack_kind_now) != attack_kind::forced_switch &&
               effect_visible(attack_kind(attack_kind_now), affected, s, gravity_event, heat_drop))
            {
                out.kinds_effect |= 1u << attack_kind_now;
            }
            if(s.boss_hp() > hp)
            {
                ++out.damage_violations;    // HP never rises
            }
        }
        if(s.state() == session_state::won)
        {
            ++out.wins;
            out.wins_with_abilities += abilities ? 1 : 0;
            if(s.boss_hp() != 0 || ! s.boss_defeated())
            {
                ++out.death_errors;
            }
        }
    }

    void verify_boss(int world)
    {
        const stage_def& st = stage(world, 5);
        const boss_def& def = boss(st.boss);
        boss_audit a;
        for(uint32_t seed = 1; seed <= 8; ++seed)
        {
            audit_fight(world, seed * 977u + uint32_t(world), seed > 4, a);
        }
        uint32_t expected = 0;
        for(int p = 0; p < def.phase_count; ++p)
        {
            for(int k = 0; k < def.phases[p].pattern_length; ++k)
            {
                expected |= 1u << int(def.phases[p].pattern[k].kind);
            }
        }
        bmtest::note("%s: %d/%d wins (%d/4 with its abilities, %d/4 without), %d attacks, phases %d/%d, kinds %08x "
                     "seen %08x with effect %08x", def.name, a.wins, a.runs, a.wins_with_abilities,
                     a.wins - a.wins_with_abilities, a.attacks, a.max_phase + 1, def.phase_count, expected, a.kinds_seen,
                     a.kinds_effect);
        CHECK(a.wins > 0);
        CHECK_EQ(a.initial_hp_errors, 0);
        CHECK_EQ(a.max_phase + 1, int(def.phase_count));
        CHECK_EQ(a.phase_violations, 0);
        CHECK_EQ(a.schedule_violations, 0);
        CHECK_EQ(a.telegraph_violations, 0);
        CHECK_EQ(a.damage_violations, 0);
        CHECK_EQ(a.death_errors, 0);
        CHECK_EQ(a.kinds_seen & expected, expected);        // every attack of every phase ran
        CHECK_EQ(a.kinds_effect & expected, expected);      // and showed its effect
    }
}

BM_TEST(boss1, block_monster_full_verification) { verify_boss(1); }
BM_TEST(boss2, cryo_titan_full_verification) { verify_boss(2); }
BM_TEST(boss3, magma_hydra_full_verification) { verify_boss(3); }
BM_TEST(boss4, electric_emperor_full_verification) { verify_boss(4); }
BM_TEST(boss5, gravity_king_full_verification) { verify_boss(5); }
BM_TEST(boss6, core_of_mayhem_full_verification) { verify_boss(6); }
