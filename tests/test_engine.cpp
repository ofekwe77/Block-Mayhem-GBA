/*
 * Core falling-block engine tests: shapes, rotation, kicks, collision,
 * seven-bag, movement, lock delay, hold, line clears and top-out.
 */
#include "bm_test.h"
#include "test_util.h"

#include <algorithm>
#include <set>
#include <vector>

using namespace bm;
using bmtest::load;

namespace
{
    struct rig
    {
        std::vector<piece_type> script;
        piece_queue queue;
        event_queue events;
        player p;
        frame_params params;
        effect_mods mods;
        uint16_t prev = 0;

        explicit rig(std::initializer_list<piece_type> pieces, uint32_t seed = 1) :
            script(pieces)
        {
            queue.init(seed, special_config(), 99);
            if(! script.empty())
            {
                queue.script(script.data(), int(script.size()));
            }
            p.init(0, cfg::board_cols, cfg::board_rows, cfg::board_hidden_rows, &queue, &events);
            params.gravity_frames = 1000;   // effectively no gravity unless a test sets it
        }

        // Runs one frame with the given held keys (pressed = newly held).
        bool frame(uint16_t held = 0)
        {
            input_frame in;
            in.held = held;
            in.pressed = uint16_t(held & ~prev);
            prev = held;
            return p.update(in, params, mods);
        }

        void tap(uint16_t key)
        {
            frame(key);
            frame(0);
        }

        // Runs frames until the board is stable and waiting for a spawn (max 600 frames).
        void settle()
        {
            for(int i = 0; i < 600 && p.state() != player_state::waiting_spawn &&
                p.state() != player_state::topped_out; ++i)
            {
                frame(0);
                if(p.resolution_finished())
                {
                    p.acknowledge_resolution();
                }
            }
        }
    };

    std::set<std::pair<int, int>> cells_of(const piece_state& s)
    {
        std::set<std::pair<int, int>> out;
        for(int m = 0; m < 4; ++m)
        {
            const point c = s.cell(m);
            out.insert({ c.x, c.y });
        }
        return out;
    }
}

BM_TEST(shapes, four_distinct_minos_every_rotation)
{
    for(int t = 1; t <= 7; ++t)
    {
        for(int r = 0; r < 4; ++r)
        {
            piece_state s;
            s.type = piece_type(t);
            s.rotation = int8_t(r);
            CHECK_EQ(int(cells_of(s).size()), 4);
        }
    }
}

BM_TEST(shapes, four_rotations_return_to_spawn)
{
    for(int t = 1; t <= 7; ++t)
    {
        const piece_shape& a = shape_of(piece_type(t), 0);
        const piece_shape& b = shape_of(piece_type(t), 4);
        for(int m = 0; m < 4; ++m)
        {
            CHECK_EQ(a.minos[m].x, b.minos[m].x);
            CHECK_EQ(a.minos[m].y, b.minos[m].y);
        }
    }
}

BM_TEST(shapes, rotation_states_match_reference)
{
    // I in state R occupies the third column of its 4x4 box; state 2 the third row; state L the second column.
    piece_state s;
    s.type = piece_type::i;
    s.rotation = 1;
    for(int m = 0; m < 4; ++m) { CHECK_EQ(int(s.cell(m).x), 2); }
    s.rotation = 2;
    for(int m = 0; m < 4; ++m) { CHECK_EQ(int(s.cell(m).y), 2); }
    s.rotation = 3;
    for(int m = 0; m < 4; ++m) { CHECK_EQ(int(s.cell(m).x), 1); }

    // T state R: vertical bar in column 1 with the nub on the right.
    s.type = piece_type::t;
    s.rotation = 1;
    std::set<std::pair<int, int>> expect = { { 1, 0 }, { 1, 1 }, { 1, 2 }, { 2, 1 } };
    CHECK(cells_of(s) == expect);
    // J spawn: corner at top-left.
    s.type = piece_type::j;
    s.rotation = 0;
    expect = { { 0, 0 }, { 0, 1 }, { 1, 1 }, { 2, 1 } };
    CHECK(cells_of(s) == expect);
    // L state 2 (180): flat row with the corner at bottom-left.
    s.type = piece_type::l;
    s.rotation = 2;
    expect = { { 0, 1 }, { 1, 1 }, { 2, 1 }, { 0, 2 } };
    CHECK(cells_of(s) == expect);
}

BM_TEST(shapes, o_never_rotates)
{
    rig r({ piece_type::o });
    CHECK(r.p.spawn_now());
    const auto before = cells_of(r.p.active());
    r.tap(key_a);
    r.tap(key_b);
    CHECK(cells_of(r.p.active()) == before);
}

BM_TEST(rotation, spawn_positions_centered)
{
    rig r({ piece_type::t, piece_type::i, piece_type::o });
    CHECK(r.p.spawn_now());
    // T: columns 3..5, lowest row 2 (first visible row) after the immediate one-row drop.
    auto c = cells_of(r.p.active());
    std::set<std::pair<int, int>> expect = { { 4, 1 }, { 3, 2 }, { 4, 2 }, { 5, 2 } };
    CHECK(c == expect);
    r.tap(key_up);
    r.settle();
    CHECK(r.p.spawn_now());
    c = cells_of(r.p.active());
    expect = { { 3, 2 }, { 4, 2 }, { 5, 2 }, { 6, 2 } };
    CHECK(c == expect);
    r.tap(key_up);
    r.settle();
    CHECK(r.p.spawn_now());
    c = cells_of(r.p.active());
    expect = { { 4, 1 }, { 5, 1 }, { 4, 2 }, { 5, 2 } };
    CHECK(c == expect);
}

BM_TEST(rotation, i_wall_kick_from_left_wall)
{
    rig r({ piece_type::i });
    CHECK(r.p.spawn_now());
    r.tap(key_a);                       // vertical (column 5)
    for(int i = 0; i < 10; ++i) { r.tap(key_left); }
    // Leftmost vertical I occupies column 0.
    int minx = 99;
    for(int m = 0; m < 4; ++m) { minx = std::min(minx, int(r.p.active().cell(m).x)); }
    CHECK_EQ(minx, 0);
    // Rotate back to horizontal: needs a kick to the right to stay inside the board.
    r.tap(key_a);
    CHECK_EQ(int(r.p.active().rotation), 2);
    for(int m = 0; m < 4; ++m) { CHECK(r.p.active().cell(m).x >= 0); }
}

BM_TEST(rotation, never_overlaps_cells)
{
    rig r({ piece_type::t, piece_type::s, piece_type::z, piece_type::j, piece_type::l, piece_type::i });
    for(int k = 0; k < 6; ++k)
    {
        bmtest::load(r.p.grid(), {
            "#.#.#.#.#.",
            ".#.#.#.#.#",
            "#.#.#.#.#.",
            ".#.#.#.#.#",
        });
        CHECK(r.p.spawn_now());
        for(int i = 0; i < 40; ++i)
        {
            r.tap((i % 3) ? key_a : key_b);
            r.frame(key_down);
            CHECK(r.p.grid().fits(r.p.active()) || r.p.state() != player_state::falling);
            if(r.p.state() != player_state::falling)
            {
                break;
            }
        }
        r.tap(key_up);
        r.settle();
        CHECK_EQ(r.p.grid().validate(), 0);
    }
}

BM_TEST(rotation, t_twist_kick_into_slot)
{
    // A classic slot: rotating T clockwise from state 0 at the right spot kicks it into the hole.
    rig r({ piece_type::t });
    bmtest::load(r.p.grid(), {
        "..........",
        "##.#######",
        "#...######",
        "##.#######",
    });
    CHECK(r.p.spawn_now());
    // Move T so its box sits over columns 1..3, drop it to rest on the ledge, then rotate.
    r.tap(key_left);
    r.tap(key_left);
    while(! r.p.grounded()) { r.frame(key_down); r.frame(0); }
    r.tap(key_b);     // counter-clockwise (state L), stem pointing into the slot via kicks
    r.tap(key_b);     // state 2: T pointing down into the slot
    // The piece must still be legal regardless of kicks.
    CHECK(r.p.grid().fits(r.p.active()));
}

BM_TEST(bag, every_bag_contains_all_seven)
{
    bag_randomizer bag;
    bag.seed(12345);
    for(int b = 0; b < 500; ++b)
    {
        bool seen[8] = {};
        for(int i = 0; i < 7; ++i)
        {
            const int t = int(bag.next());
            CHECK(t >= 1 && t <= 7);
            CHECK(! seen[t]);
            seen[t] = true;
        }
    }
}

BM_TEST(bag, deterministic_per_seed)
{
    bag_randomizer a;
    bag_randomizer b;
    bag_randomizer c;
    a.seed(777);
    b.seed(777);
    c.seed(778);
    bool differs = false;
    for(int i = 0; i < 140; ++i)
    {
        const piece_type pa = a.next();
        CHECK(pa == b.next());
        if(pa != c.next())
        {
            differs = true;
        }
    }
    CHECK(differs);
}

BM_TEST(bag, max_gap_between_same_shape_is_twelve)
{
    bag_randomizer bag;
    bag.seed(4242);
    int last[8];
    for(int& l : last) { l = -1; }
    for(int i = 0; i < 7000; ++i)
    {
        const int t = int(bag.next());
        if(last[t] >= 0)
        {
            CHECK(i - last[t] <= 13);
        }
        last[t] = i;
    }
}

BM_TEST(movement, walls_block_horizontal_moves)
{
    rig r({ piece_type::o });
    CHECK(r.p.spawn_now());
    for(int i = 0; i < 20; ++i) { r.tap(key_right); }
    int maxx = 0;
    for(int m = 0; m < 4; ++m) { maxx = std::max(maxx, int(r.p.active().cell(m).x)); }
    CHECK_EQ(maxx, 9);
    for(int i = 0; i < 20; ++i) { r.tap(key_left); }
    int minx = 99;
    for(int m = 0; m < 4; ++m) { minx = std::min(minx, int(r.p.active().cell(m).x)); }
    CHECK_EQ(minx, 0);
}

BM_TEST(movement, das_and_arr_timing)
{
    rig r({ piece_type::t });
    CHECK(r.p.spawn_now());
    const int x0 = r.p.active().x;
    r.frame(key_left);                      // immediate move on press
    CHECK_EQ(int(r.p.active().x), x0 - 1);
    for(int i = 0; i < cfg::das_frames - 1; ++i) { r.frame(key_left); }
    CHECK_EQ(int(r.p.active().x), x0 - 1);   // still waiting for the repeat delay
    r.frame(key_left);                      // DAS reached: repeat
    CHECK_EQ(int(r.p.active().x), x0 - 2);
    r.frame(key_left);
    CHECK_EQ(int(r.p.active().x), x0 - 2);
    r.frame(key_left);                      // every arr_frames
    CHECK_EQ(int(r.p.active().x), x0 - 3);
}

BM_TEST(movement, hard_drop_lands_on_ghost)
{
    rig r({ piece_type::i });
    load(r.p.grid(), { "####..####" });
    CHECK(r.p.spawn_now());
    const int ghost = r.p.ghost_y();
    r.tap(key_up);
    CHECK_EQ(r.p.last_lock().hard_drop_rows, ghost - 1);
    CHECK(r.p.grid().at(3, cfg::board_rows - 2).occupied());
    CHECK(r.p.grid().at(6, cfg::board_rows - 2).occupied());
}

BM_TEST(movement, soft_drop_rows_counted_once)
{
    rig r({ piece_type::o });
    CHECK(r.p.spawn_now());
    const int y0 = r.p.active().y;
    for(int i = 0; i < 10; ++i) { r.frame(key_down); }
    const int moved = r.p.active().y - y0;
    CHECK(moved >= 4 && moved <= 6);
}

BM_TEST(lock, lock_delay_thirty_frames)
{
    rig r({ piece_type::o });
    CHECK(r.p.spawn_now());
    while(! r.p.grounded()) { r.frame(key_down); }
    r.frame(0);
    int frames = 1;
    while(r.p.state() == player_state::falling && frames < 100)
    {
        r.frame(0);
        ++frames;
    }
    CHECK(frames <= cfg::lock_delay_frames + 1);
    CHECK(frames >= cfg::lock_delay_frames - 1);
}

BM_TEST(lock, resets_are_limited)
{
    rig r({ piece_type::t });
    CHECK(r.p.spawn_now());
    while(! r.p.grounded()) { r.frame(key_down); }
    // Keep rotating forever: the piece must still lock within (resets + 1) * delay frames.
    int frames = 0;
    while(r.p.state() == player_state::falling && frames < 2000)
    {
        r.frame((frames % 20 == 10) ? key_a : 0);
        ++frames;
    }
    CHECK(r.p.state() != player_state::falling);
    CHECK(frames < (cfg::max_lock_resets + 2) * cfg::lock_delay_frames);
}

BM_TEST(hold, once_per_piece_and_spawn_orientation)
{
    rig r({ piece_type::t, piece_type::i, piece_type::o, piece_type::s });
    CHECK(r.p.spawn_now());
    r.tap(key_a);                                   // rotate T
    r.tap(key_l);                                   // hold T, I comes out
    CHECK(r.p.held() == piece_type::t);
    CHECK(r.p.active().type == piece_type::i);
    CHECK_EQ(int(r.p.active().rotation), 0);
    r.tap(key_l);                                   // second hold before lock: ignored
    CHECK(r.p.active().type == piece_type::i);
    CHECK(r.p.held() == piece_type::t);
    r.tap(key_up);
    r.settle();
    CHECK(r.p.spawn_now());                         // O
    r.tap(key_l);                                   // swap: T returns in spawn orientation
    CHECK(r.p.active().type == piece_type::t);
    CHECK_EQ(int(r.p.active().rotation), 0);
    CHECK(r.p.held() == piece_type::o);
}

BM_TEST(clear, single_double_triple_quad)
{
    for(int n = 1; n <= 4; ++n)
    {
        board b;
        b.init(10, 20, 2);
        std::vector<const char*> rows;
        for(int i = 0; i < 4 - n; ++i) { rows.push_back("#.#.#.#.#."); }
        for(int i = 0; i < n; ++i) { rows.push_back("##########"); }
        b.clear_all();
        int y = 20 - int(rows.size());
        for(const char* row : rows)
        {
            for(int x = 0; x < 10; ++x) { b.set(x, y, bmtest::cell_from_char(row[x])); }
            ++y;
        }
        const resolution_result res = bmtest::resolve(b);
        CHECK_EQ(res.first_rows, n);
        CHECK_EQ(res.waves, 1);
        CHECK_EQ(b.occupied_count(), (4 - n) * 5);
        CHECK_EQ(b.validate(), 0);
    }
}

BM_TEST(clear, standard_clear_preserves_holes_and_overhangs)
{
    board b;
    b.init(10, 20, 2);
    bmtest::load(b, {
        "#.........",
        "..........",
        "##.#######",
        "##########",
    });
    bmtest::resolve(b);
    // Only the full bottom row disappears; the floating cell keeps its gap below it.
    CHECK_STR(bmtest::dump(b, 3), "#........./........../##.#######");
}

BM_TEST(topout, block_out_when_spawn_is_blocked)
{
    rig r({ piece_type::t });
    board& b = r.p.grid();
    for(int y = 0; y < 2; ++y)
    {
        for(int x = 3; x < 6; ++x) { b.set(x, y, bmtest::cell_from_char('#')); }
    }
    CHECK(! r.p.spawn_now());
    CHECK(r.p.state() == player_state::topped_out);
}

BM_TEST(topout, lock_out_in_hidden_rows)
{
    rig r({ piece_type::o });
    board& b = r.p.grid();
    // Fill columns 4-5 up to row 2 (first visible row), leaving the spawn area free.
    for(int y = 2; y < 20; ++y)
    {
        b.set(4, y, bmtest::cell_from_char('#'));
        b.set(5, y, bmtest::cell_from_char('#'));
    }
    CHECK(r.p.spawn_now());
    r.tap(key_up);
    CHECK(r.p.state() == player_state::topped_out);
}

BM_TEST(board, garbage_push_has_one_gap)
{
    board b;
    b.init(10, 20, 2);
    const int8_t gaps[2] = { 3, 7 };
    CHECK(b.push_garbage(2, gaps));
    CHECK(! b.at(3, 18).occupied());
    CHECK(! b.at(7, 19).occupied());
    CHECK_EQ(b.occupied_count(), 18);
    CHECK_EQ(b.validate(), 0);
}

BM_TEST(board, garbage_push_overflow_reports_topout)
{
    board b;
    b.init(10, 20, 2);
    b.set(0, 0, bmtest::cell_from_char('#'));
    const int8_t gaps[1] = { 0 };
    CHECK(! b.push_garbage(1, gaps));
}
