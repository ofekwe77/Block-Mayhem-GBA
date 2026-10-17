#include "bm_rank.h"

#include "bm_session.h"

namespace bm
{
    int rank_points(const rank_input& in)
    {
        int points = 0;
        const uint64_t score = in.score;
        const uint64_t target = in.s_target ? in.s_target : 1;
        if(score >= target)
        {
            points += 3;
        }
        else if(score * 3 >= target * 2)
        {
            points += 2;
        }
        else if(score * 3 >= target)
        {
            points += 1;
        }
        if(in.seconds <= in.par_seconds)
        {
            points += 2;
        }
        else if(in.seconds <= in.par_seconds * 3 / 2u)
        {
            points += 1;
        }
        points += in.no_garbage ? 1 : 0;
        points += in.bonus_goal ? 2 : 0;
        return points;
    }

    int rank_from_points(int points)
    {
        if(points >= 8)
        {
            return 5;
        }
        if(points >= 6)
        {
            return 4;
        }
        if(points >= 4)
        {
            return 3;
        }
        if(points >= 2)
        {
            return 2;
        }
        return 1;
    }

    int stage_rank(const session& s)
    {
        const stage_def* st = s.config().stage;
        if(! st || s.state() != session_state::won)
        {
            return 0;
        }
        rank_input in;
        in.score = s.score();
        in.s_target = st->rank_score;
        in.seconds = s.stats().frames / 60;
        in.par_seconds = st->par_seconds;
        in.no_garbage = s.stats().garbage_received == 0;
        in.bonus_goal = s.secondary().done;
        return rank_from_points(rank_points(in));
    }
}
