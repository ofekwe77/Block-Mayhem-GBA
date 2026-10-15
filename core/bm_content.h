/*
 * BLOCK MAYHEM - content data model: game modes, stages, objectives, hazards,
 * bosses and upgrade modifiers. The actual data lives in bm_stages.cpp,
 * bm_bosses.cpp and bm_upgrades.cpp.
 */
#ifndef BM_CONTENT_H
#define BM_CONTENT_H

#include "bm_config.h"
#include "bm_randomizer.h"
#include "bm_resolver.h"

namespace bm
{
    enum class game_mode : uint8_t
    {
        classic,
        arcade,
        adventure,
        chaos_run,
        dual_relay,
        boss_rush,
        count
    };

    // ------------------------------------------------------------------ objectives

    enum class objective_kind : uint8_t
    {
        none,
        lines,              // clear N rows (combined across boards)
        bombs,              // trigger N bombs
        chains,             // complete N resolutions with at least 'param' waves
        magnets,            // successful magnet effects (moves)
        overheat_success,   // complete N Overheats successfully
        cross_combos,       // CROSS OVER combos (dual board)
        survive_pieces,     // lock N pieces without topping out
        boss_defeat,        // reduce the boss to 0 HP
        sparks,             // trigger N electrical cells
        fires,              // trigger N fire cells
        ice_broken,         // break N ice coats
        quads,              // N four-row clears
        twists,             // N T-TWIST clears
        max_height,         // secondary: keep the stack at or below 'target' rows (checked at the end)
        no_garbage,         // secondary: receive no hostile garbage
        time_limit          // secondary: finish within 'target' seconds
    };

    struct objective_def
    {
        objective_kind kind = objective_kind::none;
        uint8_t param = 0;
        uint16_t target = 0;
    };

    // ------------------------------------------------------------------ hazards (stage environment)

    enum class hazard_kind : uint8_t
    {
        none,
        freeze_cells,       // telegraphed cells become ice-coated (World 2)
        lava_pressure,      // pressure meter fills per lock; at max a telegraphed molten garbage row (World 3)
        gravity_shift,      // telegraphed full compaction in a direction (World 5); param = direction cycle id
        settle_cycle,       // the special-effect settle direction rotates (World 5); param = cycle id
        garbage_drip,       // telegraphed garbage rows on a schedule
        armor_plates,       // telegraphed cells become armored
        surge,              // specials disabled for a few pieces (World 4)
        molten_rain,        // telegraphed columns receive a molten block on top (World 3)
        mixed               // World 6: rotates through freeze / lava / gravity / surge
    };

    struct hazard_def
    {
        hazard_kind kind = hazard_kind::none;
        uint8_t interval = 0;       // locked pieces between activations
        uint8_t strength = 0;       // cells / rows affected
        uint8_t telegraph = 2;      // locks of warning before it fires
        uint8_t param = 0;
    };

    // ------------------------------------------------------------------ bosses

    enum class attack_kind : uint8_t
    {
        none,
        garbage,            // strength rows of garbage (one gap, telegraphed preview)
        punch,              // Block Monster: 1-2 garbage rows with a shared gap, big wind-up
        armor_inject,       // strength surface cells become armored
        ice_inject,         // strength surface cells become ice-coated
        frozen_wall,        // a column of empty cells above the stack fills with ice blocks (strength tall)
        chill,              // heat gain halved for strength locks
        crystal_shards,     // strength telegraphed columns receive an ice block on top
        fire_breath,        // telegraphed column receives strength molten garbage blocks on top
        molten_rain,        // strength telegraphed columns receive one molten block each
        heat_drain,         // steals strength*5 heat
        surge,              // all specials disabled for strength locks
        cross_garbage,      // garbage aimed at the inactive board (dual)
        forced_switch,      // forces a board switch (dual)
        gravity_left,
        gravity_right,
        gravity_up,         // holes sink to the bottom (column up-compaction)
        gravity_flip,       // alternates left/right each use
        core_combo          // final boss: two previously learned attacks at once
    };

    struct attack_def
    {
        attack_kind kind = attack_kind::none;
        uint8_t strength = 1;
        uint8_t telegraph = cfg::boss_min_telegraph_pieces;    // locks of warning
        uint8_t param = 0;
    };

    struct boss_phase_def
    {
        uint8_t hp_percent = 100;   // phase begins when HP <= this percent of max
        uint8_t interval = 6;       // locks between the end of one attack and the next
        uint8_t pattern_length = 0;
        attack_def pattern[6];
    };

    struct boss_def
    {
        const char* name = "";
        const char* title = "";         // short epithet shown on the boss entrance card
        uint16_t hp = 100;
        uint8_t phase_count = 1;
        bool dual = false;
        uint8_t dual_phase = 0;         // dual fights: board B unseals when this phase starts (0 = from the start)
        boss_phase_def phases[5];
    };

    // ------------------------------------------------------------------ stages

    struct stage_def
    {
        uint8_t world = 1;              // 1..6
        uint8_t index = 1;              // 1..5 (5 = boss)
        const char* name = "";
        const char* idea = "";             // core concept line shown on the stage card
        uint8_t base_level = 1;
        uint8_t level_cap = 8;
        bool level_progression = true;
        bool dual = false;
        bool heat = true;
        uint8_t loadout[3] = { 0, 0, 0 };   // designed ability loadout for the stage
        uint8_t loadout_count = 0;
        uint8_t start_heat = 0;
        special_config specials;
        objective_def objectives[3];
        objective_def secondary;
        hazard_def hazards[2];
        uint8_t boss = 0;               // 1..6 for boss stages
        gravity_dir settle = gravity_dir::down;
        const char* const* layout = nullptr;   // initial board rows, top to bottom (bottom aligned)
        uint8_t layout_rows = 0;
        const char* tips[3] = { nullptr, nullptr, nullptr };
        uint16_t par_seconds = 180;     // time component of the rank
        uint32_t rank_score = 20000;    // score for an S rank
    };

    // ------------------------------------------------------------------ upgrade modifiers (CHAOS RUN)

    struct modifiers
    {
        effect_mods effects;
        int16_t heat_gain_percent = 100;            // HEAT ENGINE (capped)
        int8_t ability_cost_delta[cfg::ability_count] = {};
        int8_t ability_cooldown_delta[cfg::ability_count] = {};
        int16_t freeze_bonus_frames = 0;            // TIME ENGINEER
        int8_t vent_bonus_cells = 0;                // SAFE VENT
        int16_t chain_score_percent = 100;          // CHAIN KING
        int16_t precision_percent = 100;            // PRECISION ENGINE (3-4 row clears and twists)
        int16_t overheat_bonus_percent = 100;       // OVERHEAT EXPERT
        int16_t gambit_percent = 100;               // FINAL GAMBIT (score while the stack is high)
        int8_t combo_battery_heat = 0;              // COMBO BATTERY
        int8_t second_chances = 0;                  // SECOND CHANCE
        int8_t iron_defense = 0;                    // IRON DEFENSE (garbage severity reductions)
        int8_t reactive_armor = 0;                  // REACTIVE ARMOR (environmental absorbs per encounter)
    };

    // ------------------------------------------------------------------ content access (bm_stages.cpp / bm_bosses.cpp)

    constexpr int world_count = 6;
    constexpr int stages_per_world = 5;

    [[nodiscard]] const stage_def& stage(int world, int index);     // world 1..6, index 1..5
    [[nodiscard]] const boss_def& boss(int id);                     // 1..6
    [[nodiscard]] const char* world_name(int world);
    [[nodiscard]] int stage_number(int world, int index);           // 0..29
}

#endif
