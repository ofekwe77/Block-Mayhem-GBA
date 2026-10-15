/*
 * BLOCK MAYHEM - the six bosses (PART 14-20, 51).
 *
 * Every phase lists a repeating attack pattern. Attacks are scheduled by
 * locked pieces and telegraphed at least two locks ahead (more for heavy ones).
 * Each boss introduces at least two mechanically distinct attacks tied to its
 * world's mechanics (PART 51.4). HP values: BALANCE.md.
 */
#include "bm_content.h"

namespace bm
{
    namespace
    {
        constexpr attack_def atk(attack_kind kind, int strength = 1, int telegraph = 2, int param = 0)
        {
            attack_def a;
            a.kind = kind;
            a.strength = uint8_t(strength);
            a.telegraph = uint8_t(telegraph);
            a.param = uint8_t(param);
            return a;
        }

        template<int N>
        constexpr boss_phase_def phase(int hp_percent, int interval, const attack_def (&pattern)[N])
        {
            boss_phase_def p;
            p.hp_percent = uint8_t(hp_percent);
            p.interval = uint8_t(interval);
            p.pattern_length = uint8_t(N);
            for(int i = 0; i < N && i < 6; ++i)
            {
                p.pattern[i] = pattern[i];
            }
            return p;
        }

        using a = attack_kind;

        // ---------------------------------------------------------------- 1. THE BLOCK MONSTER
        constexpr attack_def m1[] = { atk(a::punch, 1, 3), atk(a::garbage, 1, 2), atk(a::punch, 1, 3) };
        constexpr attack_def m2[] = { atk(a::punch, 2, 3), atk(a::armor_inject, 2, 2), atk(a::garbage, 1, 2) };
        constexpr attack_def m3[] = { atk(a::punch, 2, 3), atk(a::armor_inject, 3, 2), atk(a::garbage, 1, 2),
                                      atk(a::punch, 1, 2) };

        // ---------------------------------------------------------------- 2. THE CRYO TITAN
        constexpr attack_def c1[] = { atk(a::ice_inject, 3, 2), atk(a::crystal_shards, 2, 2), atk(a::chill, 2, 2) };
        constexpr attack_def c2[] = { atk(a::frozen_wall, 4, 3), atk(a::ice_inject, 4, 2), atk(a::crystal_shards, 3, 2) };
        constexpr attack_def c3[] = { atk(a::frozen_wall, 5, 3), atk(a::chill, 3, 2), atk(a::crystal_shards, 3, 2),
                                      atk(a::ice_inject, 4, 2) };

        // ---------------------------------------------------------------- 3. THE MAGMA HYDRA (one head per phase)
        constexpr attack_def h1[] = { atk(a::fire_breath, 2, 2), atk(a::heat_drain, 2, 2) };
        constexpr attack_def h2[] = { atk(a::molten_rain, 3, 2), atk(a::fire_breath, 3, 2) };
        constexpr attack_def h3[] = { atk(a::core_combo, 1, 3, 1), atk(a::molten_rain, 3, 3), atk(a::fire_breath, 3, 3) };
        constexpr attack_def h4[] = { atk(a::molten_rain, 4, 2), atk(a::fire_breath, 3, 2), atk(a::heat_drain, 4, 2) };

        // ---------------------------------------------------------------- 4. THE ELECTRIC EMPEROR (dual board)
        constexpr attack_def e1[] = { atk(a::surge, 3, 2), atk(a::cross_garbage, 1, 2) };
        constexpr attack_def e2[] = { atk(a::forced_switch, 1, 2), atk(a::garbage, 2, 3), atk(a::surge, 4, 2) };
        constexpr attack_def e3[] = { atk(a::cross_garbage, 2, 2), atk(a::surge, 4, 2), atk(a::forced_switch, 1, 2),
                                      atk(a::garbage, 2, 3) };

        // ---------------------------------------------------------------- 5. THE GRAVITY KING
        constexpr attack_def g1[] = { atk(a::gravity_left, 1, 3), atk(a::gravity_right, 1, 3) };
        constexpr attack_def g2[] = { atk(a::gravity_up, 1, 3), atk(a::garbage, 1, 2) };
        constexpr attack_def g3[] = { atk(a::gravity_flip, 1, 3), atk(a::gravity_up, 1, 3), atk(a::garbage, 1, 2) };
        constexpr attack_def g4[] = { atk(a::core_combo, 1, 3, 2), atk(a::gravity_up, 1, 3), atk(a::garbage, 2, 3) };

        // ---------------------------------------------------------------- 6. THE CORE OF MAYHEM
        constexpr attack_def k1[] = { atk(a::punch, 1, 3), atk(a::garbage, 1, 2), atk(a::armor_inject, 2, 2) };
        constexpr attack_def k2[] = { atk(a::ice_inject, 3, 2), atk(a::fire_breath, 2, 2), atk(a::surge, 3, 2) };
        constexpr attack_def k3[] = { atk(a::gravity_left, 1, 3), atk(a::gravity_up, 1, 3), atk(a::gravity_right, 1, 3) };
        constexpr attack_def k4[] = { atk(a::cross_garbage, 2, 2), atk(a::forced_switch, 1, 2), atk(a::crystal_shards, 2, 2) };
        constexpr attack_def k5[] = { atk(a::core_combo, 1, 3, 0), atk(a::core_combo, 1, 3, 1), atk(a::core_combo, 1, 3, 2),
                                      atk(a::core_combo, 1, 3, 3) };

        struct boss_table
        {
            boss_def b[6];
        };

        constexpr boss_table build()
        {
            boss_table t{};
            boss_def* d = &t.b[0];
            d->name = "THE BLOCK MONSTER";
            d->title = "FACTORY BRUTE";
            d->hp = cfg::boss_hp[0];
            d->phase_count = 3;
            d->phases[0] = phase(100, 7, m1);
            d->phases[1] = phase(70, 6, m2);
            d->phases[2] = phase(35, 5, m3);

            d = &t.b[1];
            d->name = "THE CRYO TITAN";
            d->title = "WARDEN OF THE DEEP COLD";
            d->hp = cfg::boss_hp[1];
            d->phase_count = 3;
            d->phases[0] = phase(100, 6, c1);
            d->phases[1] = phase(65, 5, c2);
            d->phases[2] = phase(30, 5, c3);

            d = &t.b[2];
            d->name = "THE MAGMA HYDRA";
            d->title = "THREE HEADS, ONE FURNACE";
            d->hp = cfg::boss_hp[2];
            d->phase_count = 4;
            d->phases[0] = phase(100, 6, h1);
            d->phases[1] = phase(72, 5, h2);
            d->phases[2] = phase(45, 6, h3);
            d->phases[3] = phase(18, 4, h4);

            d = &t.b[3];
            d->name = "THE ELECTRIC EMPEROR";
            d->title = "SOVEREIGN OF THE GRID";
            d->hp = cfg::boss_hp[3];
            d->phase_count = 3;
            d->dual = true;
            d->phases[0] = phase(100, 6, e1);
            d->phases[1] = phase(65, 5, e2);
            d->phases[2] = phase(30, 5, e3);

            d = &t.b[4];
            d->name = "THE GRAVITY KING";
            d->title = "LORD OF FALLING THINGS";
            d->hp = cfg::boss_hp[4];
            d->phase_count = 4;
            d->phases[0] = phase(100, 6, g1);
            d->phases[1] = phase(75, 6, g2);
            d->phases[2] = phase(45, 5, g3);
            d->phases[3] = phase(20, 4, g4);

            d = &t.b[5];
            d->name = "THE CORE OF MAYHEM";
            d->title = "HEART OF THE SPICE PROTOCOL";
            d->hp = cfg::boss_hp[5];
            d->phase_count = 5;
            d->dual = true;
            d->dual_phase = 4;
            d->phases[0] = phase(100, 6, k1);
            d->phases[1] = phase(80, 5, k2);
            d->phases[2] = phase(60, 6, k3);
            d->phases[3] = phase(40, 5, k4);
            d->phases[4] = phase(15, 4, k5);
            return t;
        }

        constexpr boss_table table = build();
    }

    const boss_def& boss(int id)
    {
        id = id < 1 ? 1 : (id > 6 ? 6 : id);
        return table.b[id - 1];
    }
}
