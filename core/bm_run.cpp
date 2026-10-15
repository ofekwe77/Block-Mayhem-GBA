#include "bm_run.h"

namespace bm
{
    namespace
    {
        constexpr int mini(int a, int b) { return a < b ? a : b; }
        constexpr int maxi(int a, int b) { return a > b ? a : b; }

        using namespace cfg;

        // ------------------------------------------------------------------ upgrade pool (PART 21.3)
        constexpr upgrade_def upgrades[upgrade_count] = {
            { "EXPLOSIVE MASTER", { "BOMBS REACH 1 CELL FURTHER", "LV2: BOMBS HIT 2X HARDER" },
              rarity::common, 2, tag_explosive, -1, -1 },
            { "FIRE STARTER", { "FIRE SPREADS 1 CELL", "FURTHER PER LEVEL" },
              rarity::common, 2, tag_explosive | tag_chain, -1, -1 },
            { "CHAIN KING", { "+25% CHAIN BONUS SCORE", "PER LEVEL" },
              rarity::common, 3, tag_chain, -1, -1 },
            { "HEAT ENGINE", { "+10% HEAT GAIN", "PER LEVEL" },
              rarity::common, 3, tag_heat, -1, -1 },
            { "SECOND CHANCE", { "SURVIVE ONE TOP-OUT: THE", "TOP 4 ROWS ARE VENTED" },
              rarity::rare, 1, tag_defense, -1, -1 },
            { "LASER FOCUS", { "LASER BEAM COSTS 5 HEAT", "LESS PER LEVEL" },
              rarity::common, 2, tag_precision, ability_laser, -1 },
            { "CRYSTAL BREAKER", { "+1 DAMAGE AGAINST ICE", "AND ARMOR COATS" },
              rarity::common, 1, tag_explosive, -1, -1 },
            { "ELECTRIC SURGE", { "SPARK ARCS REACH 1 CELL", "FURTHER PER LEVEL" },
              rarity::common, 2, tag_electric, -1, -1 },
            { "MAGNETIC FIELD", { "MAGNETS SEARCH 1 COLUMN", "WIDER, LV2: 1 ROW DEEPER" },
              rarity::common, 2, tag_chain, -1, -1 },
            { "TIME ENGINEER", { "TIME FREEZE LASTS 1 S", "LONGER PER LEVEL" },
              rarity::common, 2, tag_precision, ability_time_freeze, -1 },
            { "COMBO BATTERY", { "+2 HEAT FOR EVERY COMBO", "STEP PER LEVEL" },
              rarity::common, 2, tag_heat | tag_chain, -1, -1 },
            { "OVERHEAT EXPERT", { "+50% OVERHEAT SUCCESS", "BONUS PER LEVEL" },
              rarity::rare, 2, tag_heat, -1, -1 },
            { "SAFE VENT", { "EMERGENCY VENT REMOVES", "2 MORE CELLS PER LEVEL" },
              rarity::common, 2, tag_defense, ability_emergency_vent, -1 },
            { "IRON DEFENSE", { "WEAKENS ONE GARBAGE ATTACK", "PER ENCOUNTER PER LEVEL" },
              rarity::rare, 2, tag_defense, -1, -1 },
            { "RAINBOW MASTER", { "RAINBOWS TRIGGER ONE MORE", "NEIGHBOURING SPECIAL" },
              rarity::rare, 1, tag_chain, -1, -1 },
            { "PRECISION ENGINE", { "+20% SCORE FOR TRIPLES,", "QUADS AND T-TWISTS" },
              rarity::common, 2, tag_precision, -1, -1 },
            { "FAST CHARGER", { "ABILITY COOLDOWNS 1 PIECE", "SHORTER PER LEVEL (MIN 1)" },
              rarity::rare, 2, tag_precision | tag_defense, -1, -1 },
            { "REACTIVE ARMOR", { "ABSORBS THE FIRST HAZARD", "OF EVERY ENCOUNTER" },
              rarity::rare, 1, tag_defense, -1, -1 },
            { "DOUBLE SPARK", { "BIG SPARK CHAINS FIRE ONE", "EXTRA PULSE PER CHAIN" },
              rarity::legendary, 1, tag_electric | tag_chain, -1, -1 },
            { "FINAL GAMBIT", { "X1.5 SCORE WHILE YOUR", "STACK IS 70% HIGH OR MORE" },
              rarity::legendary, 1, tag_precision, -1, -1 },
            { "BOMB SUPPLY", { "MORE BOMBS ARRIVE IN", "EVERY ENCOUNTER" },
              rarity::common, 2, tag_explosive, -1, -1 },
            { "STORM SUPPLY", { "MORE SPARKS ARRIVE IN", "EVERY ENCOUNTER" },
              rarity::common, 2, tag_electric, -1, -1 },
            { "HOT START", { "START EVERY ENCOUNTER", "WITH 20 HEAT PER LEVEL" },
              rarity::common, 2, tag_heat, -1, -1 },
            { "ARMORY: LASER", { "EQUIP LASER BEAM:", "CUT THROUGH A COLUMN" },
              rarity::common, 1, tag_arsenal | tag_precision, -1, ability_laser },
            { "ARMORY: FREEZE", { "EQUIP TIME FREEZE:", "3 S WITHOUT GRAVITY" },
              rarity::common, 1, tag_arsenal | tag_precision, -1, ability_time_freeze },
            { "ARMORY: GRAVITY", { "EQUIP GRAVITY PULSE:", "COMPACT THE WHOLE BOARD" },
              rarity::rare, 1, tag_arsenal | tag_chain, -1, ability_gravity_pulse },
            { "ARMORY: SHIFT", { "EQUIP SHAPE SHIFT:", "CHANGE THE FALLING PIECE" },
              rarity::common, 1, tag_arsenal | tag_precision, -1, ability_shape_shift },
            { "ARMORY: NUKE", { "EQUIP NUCLEAR BURST:", "A 5X5 POWER-2 BLAST" },
              rarity::legendary, 1, tag_arsenal | tag_explosive, -1, ability_nuclear_burst },
            { "ARMORY: SHIELD", { "EQUIP SHIELD MATRIX:", "BLOCK THE NEXT ATTACK" },
              rarity::common, 1, tag_arsenal | tag_defense, -1, ability_shield_matrix },
            { "ARMORY: SPARK", { "EQUIP CHAIN SPARK:", "CHARGE ANY CELL" },
              rarity::rare, 1, tag_arsenal | tag_electric, -1, ability_chain_spark },
            { "ARMORY: VENT", { "EQUIP EMERGENCY VENT:", "REMOVE THE 8 HIGHEST CELLS" },
              rarity::common, 1, tag_arsenal | tag_defense, -1, ability_emergency_vent },
        };

        // ------------------------------------------------------------------ synergies (PART 21.5)
        constexpr synergy_def synergies[synergy_count] = {
            { "DEMOLITION", up_explosive_master, up_chain_king, "CHAIN BONUS +25% MORE" },
            { "MELTDOWN", up_heat_engine, up_overheat_expert, "OVERHEAT BONUS +50% MORE" },
            { "STORM CELL", up_electric_surge, up_double_spark, "SPARKS ARC 1 CELL FURTHER" },
            { "CLOCKWORK", up_time_engineer, up_precision_engine, "PRECISION SCORE +20% MORE" },
            { "BUNKER", up_iron_defense, up_safe_vent, "+1 IRON DEFENSE PER FIGHT" },
            { "WILDFIRE", up_fire_starter, up_crystal_breaker, "FIRE SPREADS 1 CELL FURTHER" },
            { "CAPACITOR", up_combo_battery, up_fast_charger, "+2 HEAT PER COMBO STEP" },
        };

        // ------------------------------------------------------------------ encounter templates (PART 21.1, 60.5)
        enum layout_kind : uint8_t
        {
            layout_none, layout_garbage, layout_ice, layout_armor, layout_holes
        };

        struct encounter_template
        {
            const char* name;
            const char* idea;
            const char* tip;
            uint8_t min_act;
            objective_kind objective;
            uint8_t param;
            uint8_t target[3];
            objective_kind objective2;
            uint8_t param2;
            uint8_t target2[3];
            uint8_t weights[5];         // bomb, fire, spark, magnet, rainbow
            uint8_t interval_min, interval_max;
            special_kind guarantee;
            uint8_t guarantee_every;
            hazard_kind hazard;
            uint8_t hazard_interval[3]; // per act; 0 = no hazard in that act
            uint8_t hazard_strength;
            uint8_t hazard_param;
            uint8_t layout;
            uint8_t start_heat;
        };

        constexpr encounter_template templates[run_template_count] = {
            { "DEMOLITION YARD", "BOMBS EVERYWHERE", "LOCK BOMBS INTO FULL ROWS TO DETONATE", 0,
              objective_kind::bombs, 0, { 4, 6, 8 }, objective_kind::none, 0, { 0, 0, 0 },
              { 55, 10, 15, 10, 10 }, 3, 5, special_kind::bomb, 6,
              hazard_kind::garbage_drip, { 0, 14, 11 }, 1, 0, layout_garbage, 0 },
            { "SPARK GRID", "LINK THE SPARKS", "SPARKS ARC TO OTHER SPARKS IN RANGE", 0,
              objective_kind::sparks, 0, { 5, 8, 11 }, objective_kind::none, 0, { 0, 0, 0 },
              { 10, 10, 60, 10, 10 }, 3, 5, special_kind::spark, 5,
              hazard_kind::surge, { 0, 12, 10 }, 2, 0, layout_none, 0 },
            { "FIRE PIT", "BURN IT DOWN", "FIRE BURNS ITS FOUR NEIGHBOURS", 0,
              objective_kind::fires, 0, { 4, 6, 8 }, objective_kind::none, 0, { 0, 0, 0 },
              { 10, 55, 15, 10, 10 }, 3, 5, special_kind::fire, 6,
              hazard_kind::molten_rain, { 0, 10, 8 }, 1, 0, layout_garbage, 0 },
            { "ICE FIELD", "THAW THE GRID", "CLEAR ROWS OR BURN ICE TO BREAK IT", 0,
              objective_kind::ice_broken, 0, { 6, 9, 12 }, objective_kind::none, 0, { 0, 0, 0 },
              { 15, 40, 25, 10, 10 }, 3, 5, special_kind::fire, 7,
              hazard_kind::freeze_cells, { 12, 10, 8 }, 2, 0, layout_ice, 0 },
            { "MAGNET MINE", "PULL THE PIECES IN", "MAGNETS SLIDE BLOCKS INTO NEARBY GAPS", 0,
              objective_kind::magnets, 0, { 3, 4, 6 }, objective_kind::none, 0, { 0, 0, 0 },
              { 15, 10, 15, 50, 10 }, 3, 5, special_kind::magnet, 6,
              hazard_kind::none, { 0, 0, 0 }, 0, 0, layout_holes, 0 },
            { "LINE RUSH", "CLEAR, CLEAR, CLEAR", "EVERY ROW COUNTS - KEEP MOVING", 0,
              objective_kind::lines, 0, { 10, 13, 16 }, objective_kind::none, 0, { 0, 0, 0 },
              { 30, 25, 25, 10, 10 }, 5, 7, special_kind::none, 0,
              hazard_kind::garbage_drip, { 0, 0, 10 }, 1, 0, layout_none, 0 },
            { "CHAIN LAB", "BUILD CHAIN REACTIONS", "SPECIALS TRIGGER SPECIALS: 2+ WAVES", 0,
              objective_kind::chains, 2, { 2, 3, 4 }, objective_kind::none, 0, { 0, 0, 0 },
              { 35, 25, 25, 5, 10 }, 2, 4, special_kind::none, 0,
              hazard_kind::none, { 0, 0, 0 }, 0, 0, layout_none, 0 },
            { "QUAD QUARRY", "FOUR AT ONCE", "STACK CLEAN AND WAIT FOR THE LONG BAR", 0,
              objective_kind::quads, 0, { 1, 1, 2 }, objective_kind::lines, 0, { 6, 8, 10 },
              { 25, 15, 25, 15, 20 }, 6, 9, special_kind::none, 0,
              hazard_kind::none, { 0, 0, 0 }, 0, 0, layout_none, 0 },
            { "SURVIVAL SHAFT", "HOLD THE LINE", "GARBAGE KEEPS RISING: STAY LOW", 0,
              objective_kind::survive_pieces, 0, { 35, 45, 55 }, objective_kind::none, 0, { 0, 0, 0 },
              { 30, 25, 25, 10, 10 }, 5, 7, special_kind::none, 0,
              hazard_kind::garbage_drip, { 10, 8, 7 }, 1, 0, layout_garbage, 0 },
            { "PRESSURE COOKER", "MIND THE LAVA", "CLEAR ROWS TO VENT THE PRESSURE", 1,
              objective_kind::lines, 0, { 0, 11, 14 }, objective_kind::none, 0, { 0, 0, 0 },
              { 20, 45, 15, 10, 10 }, 4, 6, special_kind::none, 0,
              hazard_kind::lava_pressure, { 0, 1, 1 }, 1, 9, layout_none, 0 },
            { "GRAVITY WELL", "THE FLOOR KEEPS MOVING", "BLASTED GAPS SETTLE THE ARROW'S WAY", 1,
              objective_kind::lines, 0, { 0, 10, 13 }, objective_kind::none, 0, { 0, 0, 0 },
              { 40, 15, 20, 15, 10 }, 3, 5, special_kind::none, 0,
              hazard_kind::settle_cycle, { 0, 6, 5 }, 0, 2, layout_none, 0 },
            { "HEAT WAVE", "MASTER THE OVERHEAT", "REACH 100 HEAT, THEN CLEAR 2 ROWS", 1,
              objective_kind::overheat_success, 0, { 1, 1, 1 }, objective_kind::lines, 0, { 0, 6, 8 },
              { 30, 25, 25, 10, 10 }, 3, 5, special_kind::none, 0,
              hazard_kind::none, { 0, 0, 0 }, 0, 0, layout_none, 50 },
            { "ARMOR DEPOT", "CRACK THE PLATES", "ARMOR TAKES TWO HITS TO BREAK", 1,
              objective_kind::lines, 0, { 0, 10, 12 }, objective_kind::none, 0, { 0, 0, 0 },
              { 50, 15, 15, 10, 10 }, 3, 5, special_kind::bomb, 8,
              hazard_kind::armor_plates, { 0, 10, 8 }, 2, 0, layout_armor, 0 },
            { "CHAOS STORM", "EVERYTHING AT ONCE", "EVERY HAZARD TAKES A TURN", 2,
              objective_kind::lines, 0, { 0, 0, 14 }, objective_kind::chains, 2, { 0, 0, 2 },
              { 30, 25, 25, 10, 10 }, 3, 5, special_kind::none, 0,
              hazard_kind::mixed, { 0, 0, 8 }, 2, 0, layout_garbage, 0 },
        };

        // Act boss candidates (single-board bosses; dual fights are an Adventure / Boss Rush feature).
        constexpr uint8_t act_bosses[3][2] = { { 1, 2 }, { 2, 3 }, { 3, 5 } };

        constexpr const char* boss_tips[7] = {
            nullptr,
            "EVERY CLEAR DAMAGES THE MONSTER",
            "ICE ATTACKS: BURN OR CLEAR THEM",
            "THREE HEADS, THREE ATTACKS",
            nullptr,
            "HE TILTS THE WORLD - STAY FLAT",
            nullptr,
        };

        enum run_stream : uint32_t
        {
            stream_run_encounter = 0x52554E31u,
            stream_run_offers = 0x52554E32u,
            stream_run_start = 0x52554E33u,
        };
    }

    const upgrade_def& upgrade(int id)
    {
        return upgrades[id >= 0 && id < upgrade_count ? id : 0];
    }

    const synergy_def& synergy(int id)
    {
        return synergies[id >= 0 && id < synergy_count ? id : 0];
    }

    const char* run_template_name(int id)
    {
        return templates[id >= 0 && id < run_template_count ? id : 0].name;
    }

    objective_kind run_template_objective(int id)
    {
        return templates[id >= 0 && id < run_template_count ? id : 0].objective;
    }

    // ================================================================== build

    bool run_build::has_ability(int ability) const
    {
        for(int i = 0; i < loadout_count; ++i)
        {
            if(loadout[i] == ability)
            {
                return true;
            }
        }
        return false;
    }

    bool run_build::synergy_active(int s) const
    {
        const synergy_def& d = synergy(s);
        return owns(d.a) && owns(d.b);
    }

    int run_build::upgrades_owned() const
    {
        int n = 0;
        for(uint8_t s : stacks)
        {
            n += s ? 1 : 0;
        }
        return n;
    }

    int run_build::synergies_active() const
    {
        int n = 0;
        for(int s = 0; s < synergy_count; ++s)
        {
            n += synergy_active(s) ? 1 : 0;
        }
        return n;
    }

    modifiers build_modifiers(const run_build& b)
    {
        modifiers m;
        auto st = [&b](int id) { return int(b.stacks[id]); };

        // Explosive.
        m.effects.bomb_plus = int8_t(mini(st(up_explosive_master) >= 1 ? 1 : 0, run_bomb_plus_max));
        m.effects.bomb_power = int8_t(mini(st(up_explosive_master) >= 2 ? 2 : 1, run_bomb_power_max));
        m.effects.fire_reach = int8_t(mini(fire_range + st(up_fire_starter) + (b.synergy_active(5) ? 1 : 0),
                                           run_fire_reach_max));
        m.effects.coat_bonus = int8_t(mini(st(up_crystal_breaker), 1));

        // Electric.
        m.effects.spark_range = int8_t(mini(spark_range + st(up_electric_surge) + (b.synergy_active(2) ? 1 : 0),
                                            spark_range_max));
        m.effects.double_spark = b.owns(up_double_spark);

        // Chain.
        m.effects.magnet_cols = int8_t(mini(magnet_columns + (st(up_magnetic_field) >= 1 ? 1 : 0), run_magnet_cols_max));
        m.effects.magnet_rows = int8_t(mini(1 + (st(up_magnetic_field) >= 2 ? 1 : 0), run_magnet_rows_max));
        m.effects.rainbow_targets = int8_t(mini(rainbow_targets + st(up_rainbow_master), run_rainbow_targets_max));
        m.chain_score_percent = int16_t(mini(100 + 25 * st(up_chain_king) + (b.synergy_active(0) ? 25 : 0),
                                             run_chain_score_percent_max));

        // Heat.
        m.heat_gain_percent = int16_t(mini(100 + 10 * st(up_heat_engine), run_heat_gain_percent_max));
        m.overheat_bonus_percent = int16_t(mini(100 + 50 * st(up_overheat_expert) + (b.synergy_active(1) ? 50 : 0),
                                                run_overheat_bonus_percent_max));
        m.combo_battery_heat = int8_t(mini(2 * st(up_combo_battery) + (b.synergy_active(6) ? 2 : 0),
                                           run_combo_battery_max));

        // Precision.
        m.precision_percent = int16_t(mini(100 + 20 * st(up_precision_engine) + (b.synergy_active(3) ? 20 : 0),
                                           run_precision_percent_max));
        m.gambit_percent = int16_t(b.owns(up_final_gambit) ? run_gambit_percent : 100);
        m.ability_cost_delta[ability_laser] = int8_t(-mini(5 * st(up_laser_focus), run_cost_reduction_max));
        m.freeze_bonus_frames = int16_t(mini(60 * st(up_time_engineer), run_freeze_bonus_max));
        const int cooldown_cut = mini(st(up_fast_charger), run_cooldown_reduction_max);
        for(int a = 0; a < ability_count; ++a)
        {
            m.ability_cooldown_delta[a] = int8_t(-cooldown_cut);     // the session floors cooldowns at 1
        }

        // Defense.
        m.vent_bonus_cells = int8_t(mini(2 * st(up_safe_vent), run_vent_bonus_max));
        m.iron_defense = int8_t(mini(st(up_iron_defense) + (b.synergy_active(4) ? 1 : 0), run_iron_defense_max));
        m.reactive_armor = int8_t(mini(st(up_reactive_armor), 1));
        m.second_chances = 0;       // granted per encounter from the run's remaining count
        return m;
    }

    int build_start_heat(const run_build& b)
    {
        return mini(20 * int(b.stacks[up_hot_start]), run_start_heat_max);
    }

    void apply_supply(const run_build& b, special_config& specials)
    {
        if(! specials.interval_min)
        {
            return;
        }
        const int bombs = mini(20 * int(b.stacks[up_bomb_supply]), run_supply_weight_max);
        const int sparks = mini(20 * int(b.stacks[up_storm_supply]), run_supply_weight_max);
        specials.weights[0] = uint8_t(mini(specials.weights[0] + bombs, 255));
        specials.weights[2] = uint8_t(mini(specials.weights[2] + sparks, 255));
    }

    bool upgrade_eligible(const run_build& b, int id)
    {
        if(id < 0 || id >= upgrade_count)
        {
            return false;
        }
        const upgrade_def& u = upgrade(id);
        if(b.stacks[id] >= u.max_stacks)
        {
            return false;
        }
        if(u.requires_ability >= 0 && ! b.has_ability(u.requires_ability))
        {
            return false;       // e.g. LASER FOCUS without a laser would do nothing
        }
        if(u.grants_ability >= 0 && (b.loadout_count >= 3 || b.has_ability(u.grants_ability)))
        {
            return false;
        }
        if(id == up_fast_charger && b.loadout_count == 0)
        {
            return false;
        }
        return true;
    }

    int synergy_completed_by(const run_build& b, int id)
    {
        if(b.owns(id))
        {
            return -1;
        }
        for(int s = 0; s < synergy_count; ++s)
        {
            const synergy_def& d = synergy(s);
            if((d.a == id && b.owns(d.b)) || (d.b == id && b.owns(d.a)))
            {
                return s;
            }
        }
        return -1;
    }

    // ================================================================== run

    void chaos_run::start(uint32_t seed)
    {
        *this = chaos_run();
        _seed = seed ? seed : 1;
        rng r(rng::derive(_seed, stream_run_start));

        // Starting resources vary by seed: one ability from a starter set.
        static constexpr uint8_t starters[6] = {
            ability_laser, ability_shield_matrix, ability_emergency_vent, ability_time_freeze, ability_chain_spark,
            ability_gravity_pulse
        };
        _build.loadout[0] = starters[r.below(6)];
        _build.loadout_count = 1;

        // Boss line-up: one per act, never repeated within a run.
        for(int a = 0; a < acts; ++a)
        {
            uint8_t pick = act_bosses[a][r.below(2)];
            for(int k = 0; k < a; ++k)
            {
                if(_bosses[k] == pick)
                {
                    pick = act_bosses[a][0] == pick ? act_bosses[a][1] : act_bosses[a][0];
                }
            }
            for(int k = 0; k < a; ++k)
            {
                if(_bosses[k] == pick)
                {
                    pick = 5;       // fallback that can never collide with acts 1-2 candidates
                }
            }
            _bosses[a] = pick;
        }

        // Encounter line-up: three different templates per act, never repeated within the run.
        uint16_t used = 0;
        for(int a = 0; a < acts; ++a)
        {
            for(int k = 0; k < slots_per_act - 1; ++k)
            {
                int candidates[run_template_count];
                int count = 0;
                for(int pass = 0; pass < 2 && ! count; ++pass)
                {
                    for(int t = 0; t < run_template_count; ++t)
                    {
                        if(template_available(t, a) && (pass == 1 || ! (used & (1u << t))))
                        {
                            candidates[count++] = t;
                        }
                    }
                }
                const int t = candidates[r.below(count)];
                used = uint16_t(used | (1u << t));
                _templates[a * (slots_per_act - 1) + k] = uint8_t(t);
            }
        }
        _phase = run_phase::encounter;
        _generate_encounter();
    }

    bool template_available(int template_id, int act)
    {
        return template_id >= 0 && template_id < run_template_count && act >= 0 && act < 3 &&
               templates[template_id].min_act <= act;
    }

    void generate_encounter(run_encounter& e, const run_build& build, int act, int slot, int template_id,
                            uint8_t boss_id, uint32_t seed)
    {
        rng r(seed);
        e = run_encounter();
        stage_def& d = e.def;
        const int a = act < 0 ? 0 : (act > 2 ? 2 : act);
        d.world = uint8_t(a == 0 ? 1 : (a == 1 ? 3 : 6));
        d.index = uint8_t(slot + 1);
        d.level_progression = true;
        d.heat = true;
        d.dual = false;
        for(int i = 0; i < 3; ++i)
        {
            d.loadout[i] = build.loadout[i];
        }
        d.loadout_count = build.loadout_count;
        d.par_seconds = uint16_t(100 + a * 20);
        d.rank_score = uint32_t(15000 + a * 15000);

        if(boss_id)
        {
            d.boss = boss_id;
            d.name = boss(boss_id).name;
            d.idea = boss(boss_id).title;
            d.base_level = uint8_t(3 + a * 2);
            d.level_cap = uint8_t(d.base_level + 2);
            d.specials.weights[0] = 35;
            d.specials.weights[1] = 25;
            d.specials.weights[2] = 25;
            d.specials.weights[3] = 10;
            d.specials.weights[4] = 5;
            d.specials.interval_min = uint8_t(special_interval_normal_min);
            d.specials.interval_max = uint8_t(special_interval_normal_max);
            d.tips[0] = boss_tips[boss_id <= 6 ? boss_id : 0];
            d.secondary.kind = objective_kind::max_height;
            d.secondary.target = 13;
            e.boss_hp_percent = uint16_t(run_boss_hp_percent[a]);
            e.template_id = 0xFF;
            apply_supply(build, d.specials);
            return;
        }

        const int tid = template_id >= 0 && template_id < run_template_count ? template_id : 0;
        const encounter_template& tp = templates[tid];
        e.template_id = uint8_t(tid);
        d.name = tp.name;
        d.idea = tp.idea;
        d.tips[0] = tp.tip;
        d.base_level = uint8_t(2 + a * 2 + (slot >= 2 ? 1 : 0));
        d.level_cap = uint8_t(d.base_level + 3);
        d.start_heat = tp.start_heat;

        int n = 0;
        if(tp.objective != objective_kind::none && tp.target[a])
        {
            d.objectives[n].kind = tp.objective;
            d.objectives[n].param = tp.param;
            d.objectives[n].target = tp.target[a];
            ++n;
        }
        if(tp.objective2 != objective_kind::none && tp.target2[a])
        {
            d.objectives[n].kind = tp.objective2;
            d.objectives[n].param = tp.param2;
            d.objectives[n].target = tp.target2[a];
            ++n;
        }
        if(! n)
        {
            d.objectives[0].kind = objective_kind::lines;       // safety net: never an encounter without a goal
            d.objectives[0].target = uint16_t(10 + a * 2);
        }
        for(int i = 0; i < 5; ++i)
        {
            d.specials.weights[i] = tp.weights[i];
        }
        d.specials.interval_min = tp.interval_min;
        d.specials.interval_max = tp.interval_max;
        if(tp.guarantee != special_kind::none)
        {
            d.specials.guarantees[0].kind = tp.guarantee;
            d.specials.guarantees[0].every = tp.guarantee_every;
        }
        if(tp.hazard != hazard_kind::none && tp.hazard_interval[a])
        {
            hazard_def& h = d.hazards[0];
            h.kind = tp.hazard;
            h.strength = tp.hazard_strength;
            h.param = tp.hazard_param;
            h.telegraph = uint8_t(tp.hazard == hazard_kind::garbage_drip ? 3 : 2);
            if(tp.hazard == hazard_kind::lava_pressure)
            {
                h.interval = 0;                                         // driven by the pressure meter
                h.param = uint8_t(tp.hazard_param - (a >= 2 ? 1 : 0));  // the meter fills faster in act 3
            }
            else
            {
                h.interval = tp.hazard_interval[a];
            }
        }

        // Bonus goal: take no garbage when something attacks, stay low while surviving, else beat the clock.
        const bool attacked = d.hazards[0].kind == hazard_kind::garbage_drip ||
                              d.hazards[0].kind == hazard_kind::lava_pressure;
        if(attacked && r.below(2))
        {
            d.secondary.kind = objective_kind::no_garbage;
        }
        else if(tp.objective == objective_kind::survive_pieces)
        {
            d.secondary.kind = objective_kind::max_height;
            d.secondary.target = 12;
        }
        else
        {
            d.secondary.kind = objective_kind::time_limit;
            d.secondary.target = d.par_seconds;
        }

        // Starting rows: every row keeps at least one gap, so none is ever complete.
        int rows = 0;
        switch(tp.layout)
        {
        case layout_garbage: rows = 1 + a; break;
        case layout_ice: rows = 2 + a; break;
        case layout_armor: rows = 2 + a; break;
        case layout_holes: rows = 2 + (a >= 1 ? 1 : 0); break;
        default: break;
        }
        rows = mini(rows, run_layout_rows);
        for(int y = 0; y < rows; ++y)
        {
            char* row = e.rows[y];
            const int gap = r.below(board_cols);
            const int gap2 = tp.layout == layout_holes ? r.below(board_cols) : gap;
            for(int x = 0; x < board_cols; ++x)
            {
                char c = '#';
                if(x == gap || x == gap2)
                {
                    c = '.';
                }
                else if(tp.layout == layout_ice && r.below(3) == 0)
                {
                    c = 'i';
                }
                else if(tp.layout == layout_armor && r.below(4) == 0)
                {
                    c = 'A';
                }
                row[x] = c;
            }
            row[board_cols] = 0;
            e.row_ptrs[y] = row;
        }
        d.layout = rows ? e.row_ptrs : nullptr;
        d.layout_rows = uint8_t(rows);
        apply_supply(build, d.specials);
    }

    void chaos_run::_generate_encounter()
    {
        rng r(rng::derive(_seed ^ (uint32_t(_index) * 0x9E3779B9u), stream_run_encounter));
        const int a = act();
        if(boss_slot())
        {
            generate_encounter(_encounter, _build, a, slot(), -1, _bosses[a], r.next());
            return;
        }
        generate_encounter(_encounter, _build, a, slot(), template_at(a, slot()), 0, r.next());
    }

    session_config chaos_run::make_config(difficulty diff) const
    {
        session_config c = make_stage_config(_encounter.def, diff, rng::derive(_seed, uint32_t(_index) + 1u));
        c.mode = game_mode::chaos_run;
        c.mods = build_modifiers(_build);
        c.mods.second_chances = int8_t(_second_chances);
        c.start_score = _score;
        c.start_heat = uint8_t(maxi(int(_encounter.def.start_heat), build_start_heat(_build)));
        c.boss_hp_percent = _encounter.boss_hp_percent;
        c.loadout_count = _build.loadout_count;
        for(int i = 0; i < 3; ++i)
        {
            c.loadout[i] = _build.loadout[i];
        }
        return c;
    }

    uint32_t chaos_run::finish_encounter(const session& s)
    {
        encounter_result r;
        r.won = s.state() == session_state::won;
        r.score = s.score();
        r.frames = s.stats().frames;
        r.lines = uint16_t(s.lines());
        r.max_chain = s.stats().max_chain;
        r.second_chances_used = s.stats().second_chances_used;
        r.secondary_done = s.secondary().done;
        return record_result(r);
    }

    uint32_t chaos_run::record_result(const encounter_result& r)
    {
        if(_phase != run_phase::encounter)
        {
            return 0;
        }
        _frames += r.frames;
        _lines += r.lines;
        _max_chain = uint16_t(maxi(_max_chain, r.max_chain));
        _second_chances = maxi(_second_chances - int(r.second_chances_used), 0);
        _last_bonus = 0;
        if(r.score > _score)
        {
            _score = r.score;       // the session started from the carried run score
        }
        if(! r.won)
        {
            _phase = run_phase::defeat;
            return 0;
        }
        const uint32_t act_number = uint32_t(act() + 1);
        uint32_t bonus = uint32_t(boss_slot() ? run_boss_bonus : run_encounter_bonus) * act_number;
        if(r.secondary_done)
        {
            bonus += uint32_t(run_secondary_bonus) * act_number;
        }
        _last_bonus = bonus;
        _score += bonus;
        ++_cleared;
        if(_index + 1 >= total_encounters)
        {
            _phase = run_phase::victory;
            return bonus;
        }
        _phase = run_phase::choose;
        _roll_offers();
        return bonus;
    }

    void chaos_run::_roll_offers()
    {
        rng r(rng::derive(_seed ^ (uint32_t(_index) * 0x85EBCA6Bu), stream_run_offers));
        _offer_count = 0;
        const int a = act();
        // Rarity odds (percent) by act; after a boss at least one card is rare or better.
        static constexpr int rare_odds[3] = { 22, 28, 34 };
        static constexpr int legendary_odds[3] = { 4, 8, 12 };
        for(int card = 0; card < offer_size; ++card)
        {
            const int roll = r.below(100);
            rarity want = roll < legendary_odds[a] ? rarity::legendary :
                          (roll < legendary_odds[a] + rare_odds[a] ? rarity::rare : rarity::common);
            if(card == 0 && boss_slot() && want == rarity::common)     // a boss always drops something good
            {
                want = rarity::rare;
            }
            // Eligible upgrades of the wanted rarity; fallbacks: any rare-or-better (for the guaranteed card), any.
            const bool guaranteed = card == 0 && boss_slot();
            int pool[upgrade_count];
            int count = 0;
            for(int pass = 0; pass < 3 && ! count; ++pass)
            {
                if(pass == 1 && ! guaranteed)
                {
                    continue;
                }
                for(int id = 0; id < upgrade_count; ++id)
                {
                    bool taken = false;
                    for(int k = 0; k < _offer_count; ++k)
                    {
                        taken = taken || _offers[k] == id;
                    }
                    if(taken || ! upgrade_eligible(_build, id))
                    {
                        continue;
                    }
                    const rarity rr = upgrade(id).rare;
                    if((pass == 0 && rr != want) || (pass == 1 && rr == rarity::common))
                    {
                        continue;
                    }
                    pool[count++] = id;
                }
            }
            if(! count)
            {
                break;      // the whole pool is exhausted (every upgrade maxed)
            }
            _offers[_offer_count++] = uint8_t(pool[r.below(count)]);
        }
    }

    void chaos_run::abandon()
    {
        if(_phase == run_phase::encounter || _phase == run_phase::choose)
        {
            _offer_count = 0;
            _phase = run_phase::defeat;
        }
    }

    void chaos_run::choose(int i)
    {
        if(_phase != run_phase::choose)
        {
            return;
        }
        if(i >= 0 && i < _offer_count)
        {
            const int id = _offers[i] < upgrade_count ? _offers[i] : 0;
            const upgrade_def& u = upgrade(id);
            if(_build.stacks[id] < u.max_stacks)
            {
                ++_build.stacks[id];
            }
            if(u.grants_ability >= 0 && _build.loadout_count < 3 && ! _build.has_ability(u.grants_ability))
            {
                _build.loadout[_build.loadout_count++] = uint8_t(u.grants_ability);
            }
            if(id == up_second_chance)
            {
                _second_chances = mini(_second_chances + 1, run_second_chances_max);
            }
        }
        _offer_count = 0;
        ++_index;
        _phase = run_phase::encounter;
        _generate_encounter();
    }
}
