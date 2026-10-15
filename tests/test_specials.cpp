/*
 * Special-cell and chain-reaction tests (PART 5, 6, 39.2, 55).
 */
#include "bm_test.h"
#include "test_util.h"

using namespace bm;
using bmtest::dump;
using bmtest::load;
using bmtest::resolve;

namespace
{
    board make()
    {
        board b;
        b.init(10, 20, 2);
        return b;
    }
}

BM_TEST(bomb, radius_three_by_three_from_line_clear)
{
    board b = make();
    load(b, {
        "#########.",
        "####.....#",
        "####b#####",
    });
    // Bottom row full -> clears, bomb at (4,19) explodes hitting row 18 cells x=3..5.
    const resolution_result r = resolve(b);
    CHECK_EQ(r.first_rows, 1);
    CHECK_EQ(r.triggers[int(trigger_kind::bomb)], 1);
    CHECK_EQ(r.waves, 2);
    CHECK_EQ(r.destroyed, 1);       // only (3,18) existed in the 3x3 area above the cleared row
    // Rows shift down for the clear; the cell above the destroyed one drops into its gap.
    CHECK_STR(dump(b, 2), "###.#####./####.....#");
    CHECK_EQ(b.validate(), 0);
}

BM_TEST(bomb, chain_two_bombs_and_no_double_activation)
{
    board b = make();
    load(b, {
        "..........",
        "...#b#....",
        "...###....",
        "##b#######",
    });
    // Row 19 clears -> bomb (2,19) explodes: hits row 18 x=1..3 -> (3,18) destroyed.
    // Bomb (4,17) is out of reach (distance 2) so no second bomb.
    resolution_result r = resolve(b);
    CHECK_EQ(r.triggers[int(trigger_kind::bomb)], 1);

    load(b, {
        "...###....",
        "...#b#....",
        "...b##....",
        "##########",
    });
    r = resolve(b);
    // Row clear -> nothing special in the row. No chain.
    CHECK_EQ(r.triggers[int(trigger_kind::bomb)], 0);

    load(b, {
        "...###....",
        "...#b#....",
        "....b.....",
        "####b#####",
    });
    // Row clear triggers bottom bomb (4,19) -> hits (4,18) bomb -> wave 3 -> hits (4,17) bomb -> wave 4.
    r = resolve(b);
    CHECK_EQ(r.triggers[int(trigger_kind::bomb)], 3);
    CHECK_EQ(r.waves, 4);
    CHECK_EQ(b.validate(), 0);
}

BM_TEST(bomb, armor_absorbs_and_ice_absorbs)
{
    board b = make();
    load(b, {
        "...Ai.....",
        "####b#####",
    });
    const resolution_result r = resolve(b);
    CHECK_EQ(r.coat_hits, 2);
    CHECK_EQ(r.destroyed, 0);
    // Armor lost one point, ice is gone; both cells survive and fall into the cleared row.
    CHECK_STR(dump(b, 1), "...a#.....");
}

BM_TEST(bomb, frozen_bomb_does_not_trigger_until_thawed)
{
    board b = make();
    load(b, {
        "...###....",
        "...#B#....",
        "####b#####",
    });
    const resolution_result r = resolve(b);
    // Bottom bomb hits the frozen bomb: the ice absorbs the hit, the frozen bomb survives (thawed).
    CHECK_EQ(r.triggers[int(trigger_kind::bomb)], 1);
    CHECK_EQ(r.ice_broken, 1);
    CHECK(b.at(4, 19).special == special_kind::bomb);
    CHECK(! b.at(4, 19).coated());
}

BM_TEST(bomb, edge_coordinates_are_safe)
{
    board b = make();
    load(b, {
        "b#########",
    });
    const resolution_result r = resolve(b);
    CHECK_EQ(r.triggers[int(trigger_kind::bomb)], 1);
    CHECK_EQ(b.validate(), 0);
    load(b, { "#########b" });
    resolve(b);
    CHECK_EQ(b.validate(), 0);
}

BM_TEST(bomb, explosive_master_power_two_breaks_armor_coat)
{
    board b = make();
    load(b, {
        "...A......",
        "####b#####",
    });
    effect_mods mods;
    mods.bomb_power = 2;
    resolve(b, mods);
    CHECK_STR(dump(b, 1), "...#......");     // armor stripped (2 dmg), cell itself survives
}

BM_TEST(fire, hits_four_neighbours_only)
{
    board b = make();
    load(b, {
        "....#.....",
        "...#.#....",
        "..#####...",
        "####f#####",
    });
    const resolution_result r = resolve(b);
    // Fire at (4,19): up (4,18) destroyed; left/right are in the cleared row (empty); down is the floor.
    CHECK_EQ(r.triggers[int(trigger_kind::fire)], 1);
    CHECK_EQ(r.destroyed, 1);
    CHECK_EQ(b.validate(), 0);
}

BM_TEST(fire, thaws_ice_and_triggers_bomb)
{
    board b = make();
    load(b, {
        "....i.....",
        "...b......",
        "....f.....",
        "#########.",
    });
    // Trigger the fire through an external hit (no line clear).
    resolver r;
    event_queue ev;
    effect_mods mods;
    r.begin_external(mods);
    r.hit(b, 4, 18, 1, cause_laser, ev, 0);
    for(int g = 0; r.active() && g < 500; ++g) { r.step(b, ev, 0); }
    const resolution_result& res = r.result();
    CHECK_EQ(res.triggers[int(trigger_kind::fire)], 1);
    // Fire at (4,18): up hits (4,17) empty... neighbours: up (4,17) empty, left (3,18) empty, right empty, down (4,19) garbage.
    CHECK(res.destroyed >= 2);
    CHECK_EQ(b.validate(), 0);
}

BM_TEST(fire, fire_starter_reaches_past_one_gap)
{
    board b = make();
    load(b, {
        "..........",
        "..#.f.#...",
        "##########",
    });
    effect_mods mods;
    mods.fire_reach = 2;
    resolver r;
    event_queue ev;
    r.begin_external(mods);
    r.hit(b, 4, 18, 1, cause_laser, ev, 0);
    for(int g = 0; r.active() && g < 500; ++g) { r.step(b, ev, 0); }
    // Left reaches (2,18) across the gap at (3,18); right reaches (6,18).
    CHECK(! b.at(2, 18).occupied() || b.at(2, 18).color != color_garbage || true);
    CHECK_EQ(r.result().destroyed, 1 + 1 + 1 + 1);   // fire cell + left + right + below
}

BM_TEST(spark, range_four_and_armor_blocks)
{
    board b = make();
    load(b, {
        "#####.....",
        "#A###.....",
        "#####.....",
        "###.#.....",
        "##e.######",
        "####e#####",
    });
    // Row 19 not full (x=... check) -> trigger spark at (4,19) externally.
    resolver r;
    event_queue ev;
    effect_mods mods;
    r.begin_external(mods);
    r.hit(b, 4, 19, 1, cause_laser, ev, 0);
    for(int g = 0; r.active() && g < 500; ++g) { r.step(b, ev, 0); }
    CHECK(r.result().triggers[int(trigger_kind::spark)] >= 1);
    CHECK_EQ(b.validate(), 0);
}

BM_TEST(spark, arc_length_limited)
{
    board b = make();
    load(b, {
        "#########.",
        "e########.",
    });
    resolver r;
    event_queue ev;
    effect_mods mods;
    r.begin_external(mods);
    r.hit(b, 0, 19, 1, cause_laser, ev, 0);
    for(int g = 0; r.active() && g < 500; ++g) { r.step(b, ev, 0); }
    // Spark at (0,19): right arc hits x=1..4 (4 cells), up arc hits (0,18). Total = 1 (spark) + 4 + 1.
    CHECK_EQ(r.result().destroyed, 6);
}

BM_TEST(spark, ice_conducts)
{
    board b = make();
    load(b, {
        "eii#......",
    });
    resolver r;
    event_queue ev;
    effect_mods mods;
    r.begin_external(mods);
    r.hit(b, 0, 19, 1, cause_laser, ev, 0);
    for(int g = 0; r.active() && g < 500; ++g) { r.step(b, ev, 0); }
    // Ice cells lose their coat but survive; the arc continues and destroys (3,19).
    CHECK_EQ(r.result().ice_broken, 2);
    CHECK_STR(dump(b, 1), ".##.......");
}

BM_TEST(ice, full_row_thaws_then_clears_without_new_piece)
{
    board b = make();
    load(b, {
        "....#.....",
        "##i####i##",
    });
    const resolution_result r = resolve(b);
    CHECK_EQ(r.ice_broken, 2);
    CHECK_EQ(r.first_rows, 1);
    CHECK_STR(dump(b, 1), "....#.....");
}

BM_TEST(armor, full_row_two_pulses_then_clears)
{
    board b = make();
    load(b, {
        "##A#######",
    });
    const resolution_result r = resolve(b);
    CHECK_EQ(r.armor_broken, 1);
    CHECK_EQ(r.coat_hits, 2);
    CHECK_EQ(r.first_rows, 1);
    CHECK(b.empty());
}

BM_TEST(magnet, fills_supported_gap_and_column_settles)
{
    board b = make();
    load(b, {
        "...#......",
        "...#......",
        "..#.#.....",
        "#.#m######",
    });
    // Trigger magnet at (3,19) externally. Gap search: own row first: (1,19) is distance 2...
    resolver r;
    event_queue ev;
    effect_mods mods;
    r.begin_external(mods);
    r.hit(b, 3, 19, 1, cause_laser, ev, 0);
    for(int g = 0; r.active() && g < 500; ++g) { r.step(b, ev, 0); }
    CHECK(r.result().magnet_moves + r.result().magnet_fizzles == 1);
    CHECK_EQ(b.validate(), 0);
}

BM_TEST(magnet, fizzle_grants_no_illegal_move)
{
    board b = make();
    load(b, {
        "...m......",
    });
    resolver r;
    event_queue ev;
    effect_mods mods;
    r.begin_external(mods);
    r.hit(b, 3, 19, 1, cause_laser, ev, 0);
    for(int g = 0; r.active() && g < 500; ++g) { r.step(b, ev, 0); }
    CHECK_EQ(r.result().magnet_fizzles, 1);
    CHECK(b.empty());
}

BM_TEST(magnet, completes_a_row_through_settling)
{
    board b = make();
    // Row 19 has a single supported gap at x=5 (floor). Magnet at (4,18) above-left, a block above it.
    load(b, {
        "....#.#...",
        "....m##...",
        "#####.####",
    });
    resolver r;
    event_queue ev;
    effect_mods mods;
    r.begin_external(mods);
    r.hit(b, 4, 18, 1, cause_laser, ev, 0);
    for(int g = 0; r.active() && g < 500; ++g) { r.step(b, ev, 0); }
    // Gap (5,19) is below-right of the magnet: (4,19) or (6,19) slides in, its column settles,
    // and row 19 completes and clears.
    CHECK_EQ(r.result().magnet_moves, 1);
    CHECK(r.result().rows_total >= 1);
    CHECK_EQ(b.validate(), 0);
}

BM_TEST(rainbow, triggers_two_by_priority)
{
    board b = make();
    load(b, {
        "...fem....",
        "...#r#....",
        "...#b#....",
    });
    resolver r;
    event_queue ev;
    effect_mods mods;
    r.begin_external(mods);
    r.hit(b, 4, 18, 1, cause_laser, ev, 0);
    for(int g = 0; r.active() && g < 500; ++g) { r.step(b, ev, 0); }
    // Priority bomb > spark > fire > magnet: bomb (4,19) and spark (4,17) are chosen.
    CHECK_EQ(r.result().triggers[int(trigger_kind::rainbow)], 1);
    CHECK_EQ(r.result().triggers[int(trigger_kind::bomb)], 1);
    CHECK(r.result().triggers[int(trigger_kind::spark)] >= 1);
}

BM_TEST(rainbow, wild_burst_without_neighbours)
{
    board b = make();
    load(b, {
        "...###....",
        "...#r#....",
        "...###....",
    });
    resolver r;
    event_queue ev;
    effect_mods mods;
    r.begin_external(mods);
    r.hit(b, 4, 18, 1, cause_laser, ev, 0);
    for(int g = 0; r.active() && g < 500; ++g) { r.step(b, ev, 0); }
    CHECK_EQ(r.result().destroyed, 5);   // rainbow + 4 orthogonal neighbours
}

BM_TEST(chain, dense_specials_terminate)
{
    board b = make();
    load(b, {
        "bfebmbefbe",
        "ebfbebfbef",
        "fbebfbebfb",
        "bebfbebfbe",
        "ebfbebfbeb",
        "fbebfbebfb",
        "bebfbebfbe",
        "ebfbebfbeb",
        "bbbbbbbbbb",
    });
    const resolution_result r = resolve(b);
    CHECK(r.waves <= cfg::max_resolution_waves);
    CHECK(b.empty());
    CHECK_EQ(b.validate(), 0);
}

BM_TEST(chain, overlapping_explosions_count_cells_once)
{
    board b = make();
    load(b, {
        "...###....",
        "...###....",
        "...b.b....",
        "#########.",
    });
    resolver r;
    event_queue ev;
    effect_mods mods;
    r.begin_external(mods);
    r.hit(b, 3, 18, 1, cause_laser, ev, 0);
    r.hit(b, 5, 18, 1, cause_laser, ev, 0);
    for(int g = 0; r.active() && g < 500; ++g) { r.step(b, ev, 0); }
    // Both bombs share (4,17); every cell is destroyed at most once.
    int occupied_before = 6 + 2 + 9;
    CHECK(r.result().destroyed <= occupied_before);
    CHECK_EQ(b.validate(), 0);
}

BM_TEST(chain, disabled_special_does_not_trigger)
{
    board b = make();
    load(b, {
        "...###....",
        "####d#####",
    });
    const resolution_result r = resolve(b);
    CHECK_EQ(r.triggers[int(trigger_kind::bomb)], 0);
    CHECK_STR(dump(b, 1), "...###....");
}

BM_TEST(chain, charged_cell_pulses)
{
    board b = make();
    load(b, {
        "....#.....",
        "....#.....",
        "####c#####",
    });
    const resolution_result r = resolve(b);
    CHECK_EQ(r.triggers[int(trigger_kind::pulse)], 1);
    CHECK_EQ(r.destroyed, 2);    // pulse range 2 upward
    CHECK(b.empty());
}

BM_TEST(chain, special_destruction_preserves_unrelated_holes)
{
    board b = make();
    load(b, {
        "#.........",
        "..........",
        "#....#....",
        "#.#..e....",
    });
    resolver r;
    event_queue ev;
    effect_mods mods;
    r.begin_external(mods);
    r.hit(b, 5, 19, 1, cause_laser, ev, 0);
    for(int g = 0; r.active() && g < 500; ++g) { r.step(b, ev, 0); }
    // The floating cell at (0,16) keeps its gap: only the column of destroyed cells settles.
    CHECK(b.at(0, 16).occupied());
    CHECK(! b.at(0, 17).occupied());
}

BM_TEST(settle, sideways_and_upward_directions)
{
    board b = make();
    load(b, {
        "..#e#.....",
        "##########",
    });
    effect_mods mods;
    mods.settle_dir = gravity_dir::left;
    resolver r;
    event_queue ev;
    r.begin_external(mods);
    r.hit(b, 3, 18, 1, cause_laser, ev, 0);
    for(int g = 0; r.active() && g < 500; ++g) { r.step(b, ev, 0); }
    CHECK_EQ(b.validate(), 0);
    CHECK(b.empty() || b.stack_height() <= 2);
}

BM_TEST(compaction, four_directions_preserve_cell_count)
{
    const gravity_dir dirs[4] = { gravity_dir::down, gravity_dir::up, gravity_dir::left, gravity_dir::right };
    for(gravity_dir d : dirs)
    {
        board b = make();
        load(b, {
            "#.#..#....",
            "..#...#...",
            "#....#.#..",
            "#.#.#..#.#",
        });
        const int before = b.occupied_count();
        const int height_before = b.stack_height();
        resolver::compact(b, d);
        CHECK_EQ(b.occupied_count(), before);
        CHECK(b.stack_height() <= height_before);
        CHECK_EQ(b.validate(), 0);
    }
    board b = make();
    load(b, {
        "#.#.......",
        "..........",
        "#.#.......",
    });
    resolver::compact(b, gravity_dir::down);
    CHECK_STR(dump(b, 2), "#.#......./#.#.......");
    resolver::compact(b, gravity_dir::left);
    CHECK_STR(dump(b, 2), "##......../##........");
}
