/*
 * BLOCK MAYHEM - the thirty Adventure stages (PART 13-19, 50).
 *
 * Built at compile time into ROM. Every stage has its own idea, starting board,
 * special-cell mix (with guarantees so objectives never depend on luck),
 * objectives, hazards and designed ability loadout. See STAGE_PASSPORTS.md.
 * Layout legend: '#' block, 'i' ice-coated, 'A' armored, 'x' stone,
 * 'b' bomb, 'f' fire, 'e' spark, 'm' magnet, 'r' rainbow, '.' empty (bottom aligned).
 */
#include "bm_content.h"

namespace bm
{
    namespace
    {
        using cfg::ability_laser;
        using cfg::ability_time_freeze;
        using cfg::ability_gravity_pulse;
        using cfg::ability_shape_shift;
        using cfg::ability_nuclear_burst;
        using cfg::ability_shield_matrix;
        using cfg::ability_chain_spark;
        using cfg::ability_emergency_vent;

        constexpr special_config specials(int bomb, int fire, int spark, int magnet, int rainbow, int imin, int imax)
        {
            special_config c;
            c.weights[0] = uint8_t(bomb);
            c.weights[1] = uint8_t(fire);
            c.weights[2] = uint8_t(spark);
            c.weights[3] = uint8_t(magnet);
            c.weights[4] = uint8_t(rainbow);
            c.interval_min = uint8_t(imin);
            c.interval_max = uint8_t(imax);
            return c;
        }

        constexpr special_config with(special_config c, int slot, special_kind kind, int every, int max_count = 0)
        {
            c.guarantees[slot].kind = kind;
            c.guarantees[slot].every = uint8_t(every);
            c.guarantees[slot].max_count = uint8_t(max_count);
            return c;
        }

        constexpr objective_def obj(objective_kind kind, int target, int param = 0)
        {
            objective_def o;
            o.kind = kind;
            o.target = uint16_t(target);
            o.param = uint8_t(param);
            return o;
        }

        constexpr hazard_def haz(hazard_kind kind, int interval, int strength, int telegraph, int param = 0)
        {
            hazard_def h;
            h.kind = kind;
            h.interval = uint8_t(interval);
            h.strength = uint8_t(strength);
            h.telegraph = uint8_t(telegraph);
            h.param = uint8_t(param);
            return h;
        }

        // ---------------------------------------------------------------- starting boards
        constexpr const char* l12[] = {
            "....#.....",
            "####.#####",
            "####.#####",
            "####.#####",
            "####.#####",
        };
        constexpr const char* l13[] = {
            "##b######.",
            "#########.",
            "######b##.",
        };
        constexpr const char* l14[] = {
            "#e#.......",
            "#b#f..##..",
            "#########.",
            "####b####.",
        };
        constexpr const char* l21[] = {
            "#i##.#i###",
            "##i#.##i##",
            "i###.###i#",
            "###i.#i###",
        };
        constexpr const char* l22[] = {
            "i#ii#.#i#i",
            "#ii#i#.ii#",
            "ii#.i#i#ii",
            "#i#i#ii.#i",
            "i.ii#i#i#i",
            "#i#ii#i.ii",
        };
        constexpr const char* l23[] = {
            "#i#.#i##i#",
            "i##i#.#i##",
        };
        constexpr const char* l24[] = {
            "iiieii.iii",
            "i#i#ie#.ii",
            "#iie#i#i.#",
        };
        constexpr const char* l31[] = {
            "#f##.##f##",
            "##f#.#f###",
        };
        constexpr const char* l32[] = {
            "x###.####x",
            "#x##.###x#",
            "##x#.##x##",
        };
        constexpr const char* l33[] = {
            "..f.......",
            ".#b#......",
            "####f.....",
            "#b###.####",
        };
        constexpr const char* l41[] = {
            "#e###e##.#",
            "##.#####e#",
            "#e###.#e##",
        };
        constexpr const char* l42[] = {
            "##m#.#####",
            "#.####m##.",
            "###.###m##",
        };
        constexpr const char* l43[] = {
            "###.####",
            "##.#####",
        };
        constexpr const char* l44[] = {
            "#.######",
            "######.#",
            "###.####",
        };
        constexpr const char* l51[] = {
            "#.#.#e#.#.",
            "##.#b#.##.",
        };
        constexpr const char* l52[] = {
            "#.#####.##",
            "###.####.#",
            "#.##.#####",
        };
        constexpr const char* l53[] = {
            "#b#..#e#..",
            "##.##.##.#",
        };
        constexpr const char* l54[] = {
            "#e#.##.#",
            "##.#e###",
        };
        constexpr const char* l61[] = {
            "iAe#.#bfi#",
            "#i#Ar.#e##",
            "f##i#A.#b#",
            "#bi#e#A.##",
        };
        constexpr const char* l62[] = {
            "x#.##x#.#x",
            "#.x#.##x.#",
        };
        constexpr const char* l63[] = {
            "#i#.A##e",
            "A#e##.#i",
        };
        constexpr const char* l64[] = {
            "####.#####",
            "#####.####",
            "###.######",
            "######.###",
            "##.#######",
            "#######.##",
        };

        struct stage_table
        {
            stage_def s[30];
        };

        constexpr void set_loadout(stage_def& d, int a = -1, int b = -1, int c = -1)
        {
            const int values[3] = { a, b, c };
            d.loadout_count = 0;
            for(int v : values)
            {
                if(v >= 0)
                {
                    d.loadout[d.loadout_count++] = uint8_t(v);
                }
            }
        }

        template<int N>
        constexpr void set_layout(stage_def& d, const char* const (&rows)[N])
        {
            d.layout = rows;
            d.layout_rows = uint8_t(N);
        }

        constexpr stage_table build()
        {
            stage_table t{};
            for(int w = 0; w < 6; ++w)
            {
                for(int i = 0; i < 5; ++i)
                {
                    stage_def& d = t.s[w * 5 + i];
                    d.world = uint8_t(w + 1);
                    d.index = uint8_t(i + 1);
                }
            }
            stage_def* d = nullptr;

            // ============================================================ WORLD 1 - CLASSIC CHAOS
            d = &t.s[0];
            d->name = "FIRST SPARK";
            d->idea = "LEARN THE MACHINE";
            d->base_level = 1; d->level_cap = 3; d->heat = false;
            d->objectives[0] = obj(objective_kind::lines, 8);
            d->secondary = obj(objective_kind::max_height, 10);
            d->tips[0] = "D-PAD MOVES  A/B ROTATE";
            d->tips[1] = "UP HARD DROPS  L HOLDS";
            d->tips[2] = "FILL A ROW TO CLEAR IT";
            d->par_seconds = 100; d->rank_score = 4000;

            d = &t.s[1];
            d->name = "HOT WIRES";
            d->idea = "HEAT POWERS THE LASER";
            d->base_level = 2; d->level_cap = 4; d->start_heat = 40;
            set_loadout(*d, ability_laser);
            set_layout(*d, l12);
            d->objectives[0] = obj(objective_kind::lines, 12);
            d->secondary = obj(objective_kind::time_limit, 150);
            d->tips[0] = "R: LASER (COSTS 40 HEAT)";
            d->tips[1] = "BLAST THE CAP OFF THE SHAFT";
            d->tips[2] = "CLEARS CHARGE THE HEAT METER";
            d->par_seconds = 130; d->rank_score = 9000;

            d = &t.s[2];
            d->name = "BOOM FACTORY";
            d->idea = "PLACE BOMBS ON PURPOSE";
            d->base_level = 2; d->level_cap = 5;
            set_loadout(*d, ability_laser);
            set_layout(*d, l13);
            d->specials = with(specials(100, 0, 0, 0, 0, 5, 7), 0, special_kind::bomb, 4, 12);
            d->objectives[0] = obj(objective_kind::lines, 12);
            d->objectives[1] = obj(objective_kind::bombs, 3);
            d->secondary = obj(objective_kind::max_height, 12);
            d->tips[0] = "BOMBS BLOW UP WHEN THEIR ROW CLEARS";
            d->tips[1] = "BLASTS DESTROY THE 3X3 AROUND THEM";
            d->tips[2] = "SET BOMBS NEXT TO YOUR MESS";
            d->par_seconds = 140; d->rank_score = 12000;

            d = &t.s[3];
            d->name = "CHAIN REACTION";
            d->idea = "ONE CLEAR, MANY BLASTS";
            d->base_level = 3; d->level_cap = 6;
            set_loadout(*d, ability_laser, ability_time_freeze);
            set_layout(*d, l14);
            d->specials = with(with(specials(34, 33, 33, 0, 0, 4, 6), 0, special_kind::fire, 6, 8), 1,
                               special_kind::spark, 6, 8);
            d->objectives[0] = obj(objective_kind::lines, 14);
            d->objectives[1] = obj(objective_kind::chains, 1, 2);
            d->secondary = obj(objective_kind::time_limit, 180);
            d->tips[0] = "FIRE AND SPARKS IGNITE OTHER SPECIALS";
            d->tips[1] = "CLUSTER SPECIALS FOR CHAIN WAVES";
            d->tips[2] = "TIME FREEZE STOPS THE FALL";
            d->par_seconds = 160; d->rank_score = 16000;

            d = &t.s[4];
            d->name = "THE BLOCK MONSTER";
            d->idea = "CLEAR ROWS TO PUNCH BACK";
            d->base_level = 3; d->level_cap = 5; d->boss = 1;
            set_loadout(*d, ability_laser, ability_time_freeze, ability_shield_matrix);
            d->specials = specials(40, 30, 30, 0, 0, 5, 7);
            d->secondary = obj(objective_kind::no_garbage, 0);
            d->tips[0] = "EVERY CLEAR DAMAGES THE MONSTER";
            d->tips[1] = "WATCH THE WIND-UP: GARBAGE INCOMING";
            d->tips[2] = "SHIELD MATRIX BLOCKS ONE ATTACK";
            d->par_seconds = 200; d->rank_score = 20000;

            // ============================================================ WORLD 2 - FROZEN FACTORY
            d = &t.s[5];
            d->name = "COLD START";
            d->idea = "ICE CRACKS, THEN CLEARS";
            d->base_level = 3; d->level_cap = 6;
            set_loadout(*d, ability_laser, ability_emergency_vent);
            set_layout(*d, l21);
            d->specials = specials(40, 60, 0, 0, 0, 6, 8);
            d->objectives[0] = obj(objective_kind::lines, 10);
            d->secondary = obj(objective_kind::max_height, 12);
            d->tips[0] = "FROZEN ROWS THAW BEFORE CLEARING";
            d->tips[1] = "FIRE MELTS ICE INSTANTLY";
            d->tips[2] = "VENT CLEARS THE TOP IN AN EMERGENCY";
            d->par_seconds = 130; d->rank_score = 14000;

            d = &t.s[6];
            d->name = "CRYSTAL PRESSURE";
            d->idea = "A WALL OF FROZEN ROWS";
            d->base_level = 4; d->level_cap = 6;
            set_loadout(*d, ability_laser, ability_emergency_vent, ability_shield_matrix);
            set_layout(*d, l22);
            d->specials = with(specials(30, 70, 0, 0, 0, 5, 7), 0, special_kind::fire, 6, 6);
            d->objectives[0] = obj(objective_kind::lines, 14);
            d->objectives[1] = obj(objective_kind::ice_broken, 12);
            d->secondary = obj(objective_kind::max_height, 13);
            d->tips[0] = "EACH ROW HAS ONE GAP: READ THE WALL";
            d->tips[1] = "BREAK ICE WITH CLEARS AND FIRE";
            d->tips[2] = nullptr;
            d->par_seconds = 170; d->rank_score = 18000;

            d = &t.s[7];
            d->name = "DEEP FREEZE";
            d->idea = "PLAN AROUND THE FROST";
            d->base_level = 4; d->level_cap = 7;
            set_loadout(*d, ability_laser, ability_shield_matrix, ability_emergency_vent);
            set_layout(*d, l23);
            d->specials = specials(40, 60, 0, 0, 0, 5, 7);
            d->hazards[0] = haz(hazard_kind::freeze_cells, 6, 3, 2);
            d->objectives[0] = obj(objective_kind::survive_pieces, 45);
            d->objectives[1] = obj(objective_kind::lines, 12);
            d->secondary = obj(objective_kind::max_height, 13);
            d->tips[0] = "BLUE CORNERS FREEZE SOON";
            d->tips[1] = "CLEAR MARKED BLOCKS BEFORE THE FROST";
            d->tips[2] = "SHIELD BLOCKS A FREEZING EVENT";
            d->par_seconds = 180; d->rank_score = 20000;

            d = &t.s[8];
            d->name = "ZERO POINT";
            d->idea = "LIGHTNING SHATTERS ICE";
            d->base_level = 5; d->level_cap = 7;
            set_loadout(*d, ability_chain_spark, ability_laser, ability_shield_matrix);
            set_layout(*d, l24);
            d->specials = with(specials(20, 20, 60, 0, 0, 4, 6), 0, special_kind::spark, 5, 10);
            d->hazards[0] = haz(hazard_kind::freeze_cells, 8, 2, 2);
            d->objectives[0] = obj(objective_kind::lines, 16);
            d->objectives[1] = obj(objective_kind::chains, 2, 2);
            d->secondary = obj(objective_kind::time_limit, 200);
            d->tips[0] = "SPARKS SHATTER ICE AND KEEP GOING";
            d->tips[1] = "CHAIN SPARK CHARGES A BLOCK";
            d->tips[2] = nullptr;
            d->par_seconds = 190; d->rank_score = 24000;

            d = &t.s[9];
            d->name = "THE CRYO TITAN";
            d->idea = "OUTLAST THE DEEP COLD";
            d->base_level = 4; d->level_cap = 6; d->boss = 2;
            set_loadout(*d, ability_laser, ability_shield_matrix, ability_emergency_vent);
            d->specials = specials(30, 40, 30, 0, 0, 5, 7);
            d->secondary = obj(objective_kind::max_height, 14);
            d->tips[0] = "FIRE AND SPARKS BEAT HIS ICE";
            d->tips[1] = "HE FREEZES YOUR LOWEST COLUMN";
            d->tips[2] = nullptr;
            d->par_seconds = 230; d->rank_score = 26000;

            // ============================================================ WORLD 3 - VOLCANO MADNESS
            d = &t.s[10];
            d->name = "IGNITION";
            d->idea = "FIRE EVERYWHERE";
            d->base_level = 5; d->level_cap = 8;
            set_loadout(*d, ability_laser, ability_shield_matrix, ability_emergency_vent);
            set_layout(*d, l31);
            d->specials = with(specials(25, 75, 0, 0, 0, 3, 5), 0, special_kind::fire, 4, 0);
            d->objectives[0] = obj(objective_kind::lines, 12);
            d->objectives[1] = obj(objective_kind::fires, 6);
            d->secondary = obj(objective_kind::time_limit, 140);
            d->tips[0] = "FIRE SPREADS TO ITS NEIGHBOURS";
            d->tips[1] = nullptr;
            d->tips[2] = nullptr;
            d->par_seconds = 130; d->rank_score = 22000;

            d = &t.s[11];
            d->name = "RISING HEAT";
            d->idea = "KEEP THE LAVA DOWN";
            d->base_level = 5; d->level_cap = 8;
            set_loadout(*d, ability_laser, ability_emergency_vent, ability_shield_matrix);
            set_layout(*d, l32);
            d->specials = specials(40, 60, 0, 0, 0, 5, 7);
            d->hazards[0] = haz(hazard_kind::lava_pressure, 0, 1, 2, 8);
            d->objectives[0] = obj(objective_kind::lines, 14);
            d->secondary = obj(objective_kind::no_garbage, 0);
            d->tips[0] = "LAVA PRESSURE RISES WHEN YOU DON'T CLEAR";
            d->tips[1] = "FULL PRESSURE = A ROW OF MAGMA";
            d->tips[2] = "STONE ONLY BREAKS IN FULL ROWS";
            d->par_seconds = 160; d->rank_score = 24000;

            d = &t.s[12];
            d->name = "INFERNO COMBO";
            d->idea = "FIRE + BOMBS = CHAINS";
            d->base_level = 6; d->level_cap = 8;
            set_loadout(*d, ability_nuclear_burst, ability_laser, ability_time_freeze);
            set_layout(*d, l33);
            d->specials = with(with(specials(50, 50, 0, 0, 0, 3, 5), 0, special_kind::fire, 5, 0), 1,
                               special_kind::bomb, 5, 0);
            d->objectives[0] = obj(objective_kind::lines, 16);
            d->objectives[1] = obj(objective_kind::chains, 2, 3);
            d->secondary = obj(objective_kind::time_limit, 200);
            d->tips[0] = "NUCLEAR BURST: 5X5 FOR 80 HEAT";
            d->tips[1] = "3-WAVE CHAINS: FIRE INTO BOMBS";
            d->tips[2] = nullptr;
            d->par_seconds = 190; d->rank_score = 30000;

            d = &t.s[13];
            d->name = "MELTDOWN";
            d->idea = "RIDE THE OVERHEAT";
            d->base_level = 7; d->level_cap = 10; d->start_heat = 60;
            set_loadout(*d, ability_laser, ability_time_freeze, ability_shield_matrix);
            d->specials = specials(40, 40, 20, 0, 0, 3, 5);
            d->objectives[0] = obj(objective_kind::lines, 18);
            d->objectives[1] = obj(objective_kind::overheat_success, 1);
            d->secondary = obj(objective_kind::time_limit, 170);
            d->tips[0] = "AT 100 HEAT YOU OVERHEAT";
            d->tips[1] = "CLEAR 2 ROWS IN 12 SECONDS";
            d->tips[2] = "FAIL AND MAGMA ROWS RISE";
            d->par_seconds = 170; d->rank_score = 36000;

            d = &t.s[14];
            d->name = "THE MAGMA HYDRA";
            d->idea = "THREE HEADS, ONE FURNACE";
            d->base_level = 6; d->level_cap = 7; d->boss = 3;
            set_loadout(*d, ability_nuclear_burst, ability_laser, ability_shield_matrix);
            d->specials = specials(40, 40, 20, 0, 0, 4, 6);
            d->secondary = obj(objective_kind::max_height, 14);
            d->tips[0] = "EACH HEAD HAS ITS OWN ATTACK";
            d->tips[1] = "COLUMNS GLOW BEFORE THE MAGMA FALLS";
            d->tips[2] = nullptr;
            d->par_seconds = 260; d->rank_score = 34000;

            // ============================================================ WORLD 4 - ELECTRIC CITY
            d = &t.s[15];
            d->name = "POWER GRID";
            d->idea = "BUILD A SPARK NETWORK";
            d->base_level = 6; d->level_cap = 9;
            set_loadout(*d, ability_chain_spark, ability_laser, ability_time_freeze);
            set_layout(*d, l41);
            d->specials = with(specials(15, 0, 85, 0, 0, 3, 5), 0, special_kind::spark, 4, 0);
            d->objectives[0] = obj(objective_kind::lines, 14);
            d->objectives[1] = obj(objective_kind::sparks, 8);
            d->secondary = obj(objective_kind::time_limit, 160);
            d->tips[0] = "SPARKS 4 APART LINK INTO CHAINS";
            d->tips[1] = "ARMOR AND STONE STOP AN ARC";
            d->tips[2] = nullptr;
            d->par_seconds = 150; d->rank_score = 30000;

            d = &t.s[16];
            d->name = "MAGNETIC STORM";
            d->idea = "PULL BLOCKS INTO PLACE";
            d->base_level = 6; d->level_cap = 9;
            set_loadout(*d, ability_shape_shift, ability_laser, ability_chain_spark);
            set_layout(*d, l42);
            d->specials = with(specials(10, 0, 40, 45, 5, 3, 5), 0, special_kind::magnet, 5, 0);
            d->objectives[0] = obj(objective_kind::lines, 16);
            d->objectives[1] = obj(objective_kind::magnets, 3);
            d->secondary = obj(objective_kind::time_limit, 190);
            d->tips[0] = "MAGNETS SLIDE A BLOCK INTO A NEARBY GAP";
            d->tips[1] = "SHAPE SHIFT: CHANGE YOUR FALLING SHAPE";
            d->tips[2] = nullptr;
            d->par_seconds = 180; d->rank_score = 32000;

            d = &t.s[17];
            d->name = "SPLIT CIRCUIT";
            d->idea = "TWO BOARDS, ONE BRAIN";
            d->base_level = 5; d->level_cap = 8; d->dual = true;
            set_loadout(*d, ability_laser);
            set_layout(*d, l43);
            d->specials = specials(40, 20, 40, 0, 0, 5, 7);
            d->objectives[0] = obj(objective_kind::lines, 16);
            d->secondary = obj(objective_kind::time_limit, 200);
            d->tips[0] = "SELECT SWITCHES BOARDS";
            d->tips[1] = "EVERY 4 LOCKS THE BOARD SWITCHES";
            d->tips[2] = "BOTH BOARDS SHARE ONE HEAT METER";
            d->par_seconds = 190; d->rank_score = 26000;

            d = &t.s[18];
            d->name = "CROSS OVERLOAD";
            d->idea = "CLEAR BOTH SIDES TOGETHER";
            d->base_level = 6; d->level_cap = 9; d->dual = true;
            set_loadout(*d, ability_shield_matrix);
            set_layout(*d, l44);
            d->specials = specials(30, 20, 50, 0, 0, 4, 6);
            d->hazards[0] = haz(hazard_kind::garbage_drip, 10, 1, 3);
            d->objectives[0] = obj(objective_kind::lines, 20);
            d->objectives[1] = obj(objective_kind::cross_combos, 2);
            d->secondary = obj(objective_kind::no_garbage, 0);
            d->tips[0] = "ROWS ON BOTH BOARDS WITHIN 4 LOCKS";
            d->tips[1] = "= CROSS OVER BONUS";
            d->tips[2] = nullptr;
            d->par_seconds = 230; d->rank_score = 34000;

            d = &t.s[19];
            d->name = "THE ELECTRIC EMPEROR";
            d->idea = "HE RULES BOTH GRIDS";
            d->base_level = 6; d->level_cap = 7; d->boss = 4; d->dual = true;
            set_loadout(*d, ability_shield_matrix);
            d->specials = specials(30, 20, 50, 0, 0, 4, 6);
            d->secondary = obj(objective_kind::max_height, 13);
            d->tips[0] = "SURGES DISABLE YOUR SPECIALS";
            d->tips[1] = "HE ATTACKS THE BOARD YOU LEAVE";
            d->tips[2] = nullptr;
            d->par_seconds = 280; d->rank_score = 38000;

            // ============================================================ WORLD 5 - GRAVITY DIMENSION
            d = &t.s[20];
            d->name = "SIDEWAYS";
            d->idea = "BLOCKS FALL LEFT AND RIGHT";
            d->base_level = 6; d->level_cap = 9;
            set_loadout(*d, ability_gravity_pulse, ability_laser, ability_shield_matrix);
            set_layout(*d, l51);
            d->specials = with(specials(50, 0, 50, 0, 0, 4, 6), 0, special_kind::bomb, 5, 0);
            d->hazards[0] = haz(hazard_kind::settle_cycle, 6, 0, 2, 0);
            d->objectives[0] = obj(objective_kind::lines, 14);
            d->secondary = obj(objective_kind::time_limit, 170);
            d->tips[0] = "BLASTED GAPS SETTLE SIDEWAYS";
            d->tips[1] = "THE ARROW SHOWS THE NEXT DIRECTION";
            d->tips[2] = "GRAVITY PULSE: COMPACT THE BOARD";
            d->par_seconds = 160; d->rank_score = 32000;

            d = &t.s[21];
            d->name = "REVERSE FIELD";
            d->idea = "HOLES SINK, BLOCKS RISE";
            d->base_level = 7; d->level_cap = 9;
            set_loadout(*d, ability_gravity_pulse, ability_time_freeze, ability_emergency_vent);
            set_layout(*d, l52);
            d->specials = specials(50, 20, 30, 0, 0, 4, 6);
            d->hazards[0] = haz(hazard_kind::settle_cycle, 7, 0, 2, 1);
            d->hazards[1] = haz(hazard_kind::gravity_shift, 11, 0, 3, 1);
            d->objectives[0] = obj(objective_kind::lines, 16);
            d->secondary = obj(objective_kind::max_height, 13);
            d->tips[0] = "UPWARD GRAVITY SINKS HOLES TO THE FLOOR";
            d->tips[1] = "IT NEVER RAISES YOUR STACK";
            d->tips[2] = nullptr;
            d->par_seconds = 190; d->rank_score = 34000;

            d = &t.s[22];
            d->name = "ROTATION ENGINE";
            d->idea = "FOUR DIRECTIONS IN TURN";
            d->base_level = 7; d->level_cap = 10;
            set_loadout(*d, ability_gravity_pulse, ability_laser, ability_shield_matrix);
            set_layout(*d, l53);
            d->specials = specials(40, 20, 40, 0, 0, 4, 6);
            d->hazards[0] = haz(hazard_kind::settle_cycle, 5, 0, 2, 2);
            d->objectives[0] = obj(objective_kind::lines, 18);
            d->secondary = obj(objective_kind::time_limit, 210);
            d->tips[0] = "SETTLING ROTATES: LEFT, UP, RIGHT, DOWN";
            d->tips[1] = nullptr;
            d->tips[2] = nullptr;
            d->par_seconds = 200; d->rank_score = 38000;

            d = &t.s[23];
            d->name = "FRACTURED REALITY";
            d->idea = "TWO BOARDS, BENT LAWS";
            d->base_level = 6; d->level_cap = 9; d->dual = true;
            set_loadout(*d, ability_gravity_pulse);
            set_layout(*d, l54);
            d->specials = specials(30, 10, 60, 0, 0, 4, 6);
            d->hazards[0] = haz(hazard_kind::settle_cycle, 6, 0, 2, 0);
            d->objectives[0] = obj(objective_kind::lines, 20);
            d->secondary = obj(objective_kind::time_limit, 260);
            d->tips[0] = "BOTH BOARDS FOLLOW THE SAME ARROW";
            d->tips[1] = nullptr;
            d->tips[2] = nullptr;
            d->par_seconds = 240; d->rank_score = 34000;

            d = &t.s[24];
            d->name = "THE GRAVITY KING";
            d->idea = "HIS CROWN BENDS SPACE";
            d->base_level = 6; d->level_cap = 7; d->boss = 5;
            set_loadout(*d, ability_gravity_pulse, ability_shield_matrix, ability_laser);
            d->specials = specials(40, 20, 40, 0, 0, 4, 6);
            d->secondary = obj(objective_kind::max_height, 14);
            d->tips[0] = "ARROWS WARN BEFORE EVERY SHIFT";
            d->tips[1] = "THE FALLING PIECE IS NEVER MOVED";
            d->tips[2] = nullptr;
            d->par_seconds = 300; d->rank_score = 42000;

            // ============================================================ WORLD 6 - THE FINAL MELTDOWN
            d = &t.s[25];
            d->name = "SYSTEM FAILURE";
            d->idea = "EVERY ELEMENT AT ONCE";
            d->base_level = 8; d->level_cap = 10;
            set_loadout(*d, ability_nuclear_burst, ability_laser, ability_chain_spark);
            set_layout(*d, l61);
            d->specials = specials(30, 25, 25, 10, 10, 3, 5);
            d->hazards[0] = haz(hazard_kind::freeze_cells, 10, 2, 2);
            d->objectives[0] = obj(objective_kind::lines, 20);
            d->objectives[1] = obj(objective_kind::chains, 2, 3);
            d->secondary = obj(objective_kind::time_limit, 220);
            d->tips[0] = "ICE, ARMOR, FIRE AND SPARKS COMBINED";
            d->tips[1] = nullptr;
            d->tips[2] = nullptr;
            d->par_seconds = 210; d->rank_score = 46000;

            d = &t.s[26];
            d->name = "TOTAL INSTABILITY";
            d->idea = "THE WORLD WON'T HOLD STILL";
            d->base_level = 8; d->level_cap = 11;
            set_loadout(*d, ability_gravity_pulse, ability_emergency_vent, ability_shield_matrix);
            set_layout(*d, l62);
            d->specials = specials(35, 25, 30, 5, 5, 4, 6);
            d->hazards[0] = haz(hazard_kind::gravity_shift, 8, 0, 3, 2);
            d->hazards[1] = haz(hazard_kind::lava_pressure, 0, 1, 2, 9);
            d->objectives[0] = obj(objective_kind::lines, 22);
            d->secondary = obj(objective_kind::no_garbage, 0);
            d->tips[0] = "GRAVITY SHIFTS AND LAVA PRESSURE";
            d->tips[1] = nullptr;
            d->tips[2] = nullptr;
            d->par_seconds = 240; d->rank_score = 48000;

            d = &t.s[27];
            d->name = "DOUBLE TROUBLE";
            d->idea = "TWO BOARDS UNDER SIEGE";
            d->base_level = 7; d->level_cap = 10; d->dual = true;
            set_loadout(*d, ability_shield_matrix);
            set_layout(*d, l63);
            d->specials = specials(30, 20, 40, 5, 5, 4, 6);
            d->hazards[0] = haz(hazard_kind::garbage_drip, 9, 1, 3);
            d->hazards[1] = haz(hazard_kind::surge, 12, 3, 2);
            d->objectives[0] = obj(objective_kind::lines, 24);
            d->objectives[1] = obj(objective_kind::cross_combos, 2);
            d->secondary = obj(objective_kind::time_limit, 300);
            d->tips[0] = "SURGES DISABLE SPECIALS FOR A WHILE";
            d->tips[1] = nullptr;
            d->tips[2] = nullptr;
            d->par_seconds = 280; d->rank_score = 44000;

            d = &t.s[28];
            d->name = "POINT OF NO RETURN";
            d->idea = "THE LONG ROAD TO THE CORE";
            d->base_level = 8; d->level_cap = 12;
            set_loadout(*d, ability_emergency_vent, ability_shield_matrix, ability_laser);
            set_layout(*d, l64);
            d->specials = specials(35, 25, 25, 10, 5, 4, 6);
            d->hazards[0] = haz(hazard_kind::mixed, 7, 2, 2);
            d->objectives[0] = obj(objective_kind::lines, 30);
            d->secondary = obj(objective_kind::max_height, 14);
            d->tips[0] = "EVERY HAZARD RETURNS - ONE AT A TIME";
            d->tips[1] = "VENT AND SHIELD ARE YOUR LIFELINES";
            d->tips[2] = nullptr;
            d->par_seconds = 330; d->rank_score = 56000;

            d = &t.s[29];
            d->name = "THE CORE OF MAYHEM";
            d->idea = "END THE SPICE PROTOCOL";
            d->base_level = 7; d->level_cap = 8; d->boss = 6; d->dual = true;
            set_loadout(*d, ability_shield_matrix);
            d->specials = specials(30, 25, 30, 10, 5, 4, 6);
            d->secondary = obj(objective_kind::max_height, 13);
            d->tips[0] = "IT USES EVERYTHING YOU HAVE LEARNED";
            d->tips[1] = "PHASE 4 OPENS THE SECOND BOARD";
            d->tips[2] = nullptr;
            d->par_seconds = 420; d->rank_score = 60000;

            return t;
        }

        constexpr stage_table table = build();

        constexpr const char* world_names[7] = {
            "", "CLASSIC CHAOS", "FROZEN FACTORY", "VOLCANO MADNESS", "ELECTRIC CITY", "GRAVITY DIMENSION",
            "THE FINAL MELTDOWN"
        };
    }

    const stage_def& stage(int world, int index)
    {
        return table.s[stage_number(world, index)];
    }

    int stage_number(int world, int index)
    {
        world = world < 1 ? 1 : (world > 6 ? 6 : world);
        index = index < 1 ? 1 : (index > 5 ? 5 : index);
        return (world - 1) * 5 + index - 1;
    }

    const char* world_name(int world)
    {
        return world >= 1 && world <= 6 ? world_names[world] : "";
    }
}
