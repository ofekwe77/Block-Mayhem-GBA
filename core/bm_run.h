/*
 * BLOCK MAYHEM - CHAOS RUN roguelite (PART 21, 60).
 *
 * A run is three acts of three generated encounters plus a boss. Everything is
 * derived from the run seed, so the same seed always produces the same
 * encounters and the same upgrade offers for the same choices.
 *
 * Upgrades never touch the rules directly: build_modifiers() folds the owned
 * upgrades (and any completed synergies) into the session 'modifiers' struct,
 * enforcing the explicit stacking caps documented in BALANCE.md.
 */
#ifndef BM_RUN_H
#define BM_RUN_H

#include "bm_session.h"

namespace bm
{
    enum class rarity : uint8_t
    {
        common, rare, legendary
    };

    // Build archetypes (PART 60.1). An upgrade may carry several tags.
    enum upgrade_tag : uint8_t
    {
        tag_explosive = 1, tag_electric = 2, tag_heat = 4, tag_defense = 8, tag_precision = 16, tag_chain = 32,
        tag_arsenal = 64
    };

    enum upgrade_id : uint8_t
    {
        up_explosive_master, up_fire_starter, up_chain_king, up_heat_engine, up_second_chance, up_laser_focus,
        up_crystal_breaker, up_electric_surge, up_magnetic_field, up_time_engineer, up_combo_battery,
        up_overheat_expert, up_safe_vent, up_iron_defense, up_rainbow_master, up_precision_engine,
        up_fast_charger, up_reactive_armor, up_double_spark, up_final_gambit, up_bomb_supply, up_storm_supply,
        up_hot_start, up_armory_laser, up_armory_freeze, up_armory_gravity, up_armory_shift, up_armory_nuke,
        up_armory_shield, up_armory_spark, up_armory_vent,
        upgrade_count
    };

    struct upgrade_def
    {
        const char* name;
        const char* desc[2];        // two short lines (<= 26 characters each)
        rarity rare;
        uint8_t max_stacks;
        uint8_t tags;
        int8_t requires_ability;    // -1: none; otherwise the ability must be equipped
        int8_t grants_ability;      // -1: none; ARMORY upgrades add an ability (needs a free slot)
    };

    [[nodiscard]] const upgrade_def& upgrade(int id);

    struct synergy_def
    {
        const char* name;
        uint8_t a;
        uint8_t b;
        const char* desc;
    };

    constexpr int synergy_count = 7;
    [[nodiscard]] const synergy_def& synergy(int id);

    // Owned upgrades and the equipped abilities of a run.
    struct run_build
    {
        uint8_t stacks[upgrade_count] = {};
        uint8_t loadout[3] = { 0, 0, 0 };
        uint8_t loadout_count = 0;

        [[nodiscard]] bool owns(int id) const { return stacks[id] > 0; }
        [[nodiscard]] bool has_ability(int ability) const;
        [[nodiscard]] bool synergy_active(int s) const;
        [[nodiscard]] int upgrades_owned() const;     // distinct upgrades
        [[nodiscard]] int synergies_active() const;
    };

    // Folds a build into session modifiers with every cap applied (PART 21.6).
    [[nodiscard]] modifiers build_modifiers(const run_build& build);
    [[nodiscard]] int build_start_heat(const run_build& build);
    // Supply upgrades bias the special-cell distribution of every encounter.
    void apply_supply(const run_build& build, special_config& specials);
    // True when the upgrade can be offered: not maxed and it works with the current loadout.
    [[nodiscard]] bool upgrade_eligible(const run_build& build, int id);
    // Synergy completed by taking 'id' with the current build (-1 if none).
    [[nodiscard]] int synergy_completed_by(const run_build& build, int id);

    enum class run_phase : uint8_t
    {
        inactive,
        encounter,      // the next encounter is ready to play
        choose,         // an encounter was cleared; pick one of the offers
        victory,
        defeat
    };

    constexpr int run_layout_rows = 6;
    constexpr int run_template_count = 14;

    struct run_encounter
    {
        stage_def def;
        char rows[run_layout_rows][cfg::board_cols + 1] = {};
        const char* row_ptrs[run_layout_rows] = {};
        uint8_t template_id = 0;
        uint16_t boss_hp_percent = 100;
    };

    // Outcome of one encounter (taken from the finished session).
    struct encounter_result
    {
        bool won = false;
        bool secondary_done = false;
        uint32_t score = 0;         // session score (includes the carried run score)
        uint32_t frames = 0;
        uint16_t lines = 0;
        uint16_t max_chain = 0;
        uint8_t second_chances_used = 0;
    };

    // Encounter generation (also used directly by the host-side validation tests).
    [[nodiscard]] bool template_available(int template_id, int act);
    void generate_encounter(run_encounter& e, const run_build& build, int act, int slot, int template_id,
                            uint8_t boss_id, uint32_t seed);

    class chaos_run
    {
    public:
        static constexpr int acts = 3;
        static constexpr int slots_per_act = 4;         // three encounters + boss
        static constexpr int total_encounters = acts * slots_per_act;
        static constexpr int offer_size = 3;

        void start(uint32_t seed);

        [[nodiscard]] run_phase phase() const { return _phase; }
        [[nodiscard]] uint32_t seed() const { return _seed; }
        [[nodiscard]] int act() const { return _index / slots_per_act; }
        [[nodiscard]] int slot() const { return _index % slots_per_act; }
        [[nodiscard]] int index() const { return _index; }
        [[nodiscard]] bool boss_slot() const { return slot() == slots_per_act - 1; }
        [[nodiscard]] int cleared() const { return _cleared; }
        [[nodiscard]] uint32_t score() const { return _score; }
        [[nodiscard]] uint32_t frames() const { return _frames; }
        [[nodiscard]] int second_chances_left() const { return _second_chances; }
        [[nodiscard]] const run_encounter& encounter() const { return _encounter; }
        [[nodiscard]] const run_build& build() const { return _build; }
        [[nodiscard]] uint8_t boss_of_act(int a) const { return _bosses[a]; }
        // Template of a regular encounter (slot 0..2) - lets the run map preview the whole line-up.
        [[nodiscard]] int template_at(int a, int s) const { return _templates[a * (slots_per_act - 1) + s]; }
        [[nodiscard]] uint32_t last_bonus() const { return _last_bonus; }
        [[nodiscard]] uint16_t max_chain() const { return _max_chain; }
        [[nodiscard]] uint32_t lines() const { return _lines; }

        // Session configuration for the current encounter.
        [[nodiscard]] session_config make_config(difficulty diff) const;

        // Reports the finished encounter. Won: score + bonus, then offers (or victory after the last boss).
        // Lost: the run ends in defeat. Returns the bonus awarded.
        uint32_t finish_encounter(const session& s);
        uint32_t record_result(const encounter_result& r);

        [[nodiscard]] int offer_count() const { return _offer_count; }
        [[nodiscard]] int offer(int i) const { return _offers[i]; }

        // Takes offer 'i' (phase choose) and prepares the next encounter.
        void choose(int i);

        // Ends the run early (abandoned from the map or quit mid-encounter): counts as a defeat.
        void abandon();

    private:
        run_build _build;
        run_encounter _encounter;
        uint32_t _seed = 0;
        uint32_t _score = 0;
        uint32_t _frames = 0;
        uint32_t _last_bonus = 0;
        uint32_t _lines = 0;
        uint16_t _max_chain = 0;
        uint8_t _templates[acts * (slots_per_act - 1)] = {};    // encounter line-up chosen at start
        uint8_t _bosses[acts] = {};
        uint8_t _offers[offer_size] = {};
        int _offer_count = 0;
        int _index = 0;
        int _cleared = 0;
        int _second_chances = 0;
        run_phase _phase = run_phase::inactive;

        void _generate_encounter();
        void _roll_offers();
    };

    // Encounter templates: names and the primary objective (map icons, documentation, tests).
    [[nodiscard]] const char* run_template_name(int id);
    [[nodiscard]] objective_kind run_template_objective(int id);
}

#endif
