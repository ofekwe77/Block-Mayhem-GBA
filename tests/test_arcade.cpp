/*
 * CHAOS ARCADE controlled environmental events (PART 22.2): telegraphed events of
 * several kinds that arrive more often as the level rises; endless (never "won").
 */
#include "bm_test.h"

#include "bm_bot.h"
#include "bm_session.h"

using namespace bm;

BM_TEST(arcade, events_rotate_escalate_and_stay_valid)
{
    static session s;
    static bot b;
    session_config c;
    c.mode = game_mode::arcade;
    c.seed = 2024;
    c.loadout_count = 3;
    c.endless = true;
    c.stage = &arcade_stage();
    s.start(c);
    b.reset(bot_strategy::resource_aware, 99);
    int warnings = 0;
    int fired = 0;
    uint32_t kinds = 0;
    int first_gap = -1;
    int last_gap = -1;
    int last_lock = -1;
    int invalid = 0;
    for(int f = 0; f < 60 * 60 * 6 && s.state() != session_state::lost; ++f)
    {
        s.update(b.next(s));
        event e;
        while(s.events().pop(e))
        {
            if(e.type == event_type::hazard_warning)
            {
                ++warnings;
                CHECK(e.b >= 1);        // always telegraphed at least one lock ahead
            }
            if(e.type == event_type::hazard_fired)
            {
                ++fired;
                kinds |= 1u << (e.a & 31);
                const int locks = s.stats().pieces;
                if(last_lock >= 0)
                {
                    const int gap = locks - last_lock;
                    if(first_gap < 0)
                    {
                        first_gap = gap;
                    }
                    last_gap = gap;
                }
                last_lock = locks;
            }
        }
        invalid += s.validate() ? 1 : 0;
        CHECK(s.state() != session_state::won);
    }
    bmtest::note("arcade: level %d, %d events fired, first gap %d locks, last gap %d locks", s.level(), fired,
                 first_gap, last_gap);
    CHECK_EQ(invalid, 0);
    CHECK(fired >= 10);
    CHECK(warnings >= fired);
    int distinct = 0;
    for(int k = 0; k < 32; ++k)
    {
        distinct += (kinds >> k) & 1;
    }
    CHECK(distinct >= 5);
    CHECK(last_gap > 0 && last_gap <= first_gap);
    CHECK(last_gap >= cfg::arcade_event_min_interval - 1);
}
