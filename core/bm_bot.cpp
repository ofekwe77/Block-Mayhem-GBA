#include "bm_bot.h"

namespace bm
{
    namespace
    {
        bool fits_bits(const uint16_t* rows, int cols, int nrows, piece_type type, int rot, int x, int y)
        {
            const piece_shape& s = shape_of(type, rot);
            for(const point& m : s.minos)
            {
                const int cx = x + m.x;
                const int cy = y + m.y;
                if(cx < 0 || cx >= cols || cy >= nrows)
                {
                    return false;
                }
                if(cy >= 0 && ((rows[cy] >> cx) & 1))
                {
                    return false;
                }
            }
            return true;
        }
    }

    void bot::reset(bot_strategy strategy, uint32_t seed)
    {
        _strategy = strategy;
        _rng.reseed(seed);
        _plan = bot_plan();
        _prev = 0;
        _piece_serial = -1;
        _last_type = 0;
        _last_y = 99;
        _hold_done = false;
        _wait = 0;
        _stall = 0;
    }

    int bot::_evaluate(const uint16_t* rows, int cols, int nrows, int lines_cleared, int max_drop_row) const
    {
        int heights[board::max_cols] = {};
        int holes = 0;
        int covered_depth = 0;
        for(int x = 0; x < cols; ++x)
        {
            int top = -1;
            for(int y = 0; y < nrows; ++y)
            {
                if((rows[y] >> x) & 1)
                {
                    if(top < 0)
                    {
                        top = y;
                    }
                }
                else if(top >= 0)
                {
                    ++holes;
                    covered_depth += y - top;
                }
            }
            heights[x] = top < 0 ? 0 : nrows - top;
        }
        int aggregate = 0;
        int bump = 0;
        int max_h = 0;
        for(int x = 0; x < cols; ++x)
        {
            aggregate += heights[x];
            if(heights[x] > max_h)
            {
                max_h = heights[x];
            }
            if(x + 1 < cols)
            {
                const int d = heights[x] - heights[x + 1];
                bump += d < 0 ? -d : d;
            }
        }
        // Deep wells (more than one) are penalised; one well for long pieces is fine.
        int wells = 0;
        for(int x = 0; x < cols; ++x)
        {
            const int left = x == 0 ? 99 : heights[x - 1];
            const int right = x == cols - 1 ? 99 : heights[x + 1];
            const int depth = (left < right ? left : right) - heights[x];
            if(depth >= 3)
            {
                wells += depth;
            }
        }

        const int visible = nrows - 2;
        const bool high = max_h > visible - 7;
        switch(_strategy)
        {
        case bot_strategy::height_min:
            return -60 * aggregate - 300 * holes - 40 * bump + 50 * lines_cleared - 100 * max_h - 10 * wells;

        case bot_strategy::line_clear:
            return -30 * aggregate - 200 * holes - 20 * bump + (high ? 250 : 200) * lines_cleared * lines_cleared -
                   15 * wells;

        case bot_strategy::random_legal:
            return 0;

        case bot_strategy::hole_min:
        case bot_strategy::resource_aware:
        default:
            return -51 * aggregate - 360 * holes - 18 * bump + (high ? 160 : 76) * lines_cleared - 20 * covered_depth -
                   12 * wells + max_drop_row;
        }
    }

    bool bot::_try_plan(const session& s, piece_type type, bool hold, bot_plan& best) const
    {
        const player& p = s.board_player(s.active_board());
        const board& b = p.grid();
        const int cols = b.cols();
        const int nrows = b.rows();
        uint16_t base[board::max_rows];
        for(int y = 0; y < nrows; ++y)
        {
            base[y] = b.row_bits(y);
        }
        const uint16_t full = b.full_mask();
        const int box = box_size(type);
        const int spawn_x = (cols - (box == 4 ? 4 : 3)) / 2;
        bool found = false;
        const int rotations = type == piece_type::o ? 1 : 4;
        for(int rot = 0; rot < rotations; ++rot)
        {
            int start_y = 1;
            if(! fits_bits(base, cols, nrows, type, rot, spawn_x, start_y))
            {
                start_y = 0;
                if(! fits_bits(base, cols, nrows, type, rot, spawn_x, start_y))
                {
                    continue;
                }
            }
            for(int dir = -1; dir <= 1; dir += 2)
            {
                for(int x = (dir < 0 ? spawn_x : spawn_x + 1); ; x += dir)
                {
                    if(! fits_bits(base, cols, nrows, type, rot, x, start_y))
                    {
                        break;      // horizontal path blocked at spawn height
                    }
                    int y = start_y;
                    while(fits_bits(base, cols, nrows, type, rot, x, y + 1))
                    {
                        ++y;
                    }
                    uint16_t rows[board::max_rows];
                    for(int r = 0; r < nrows; ++r)
                    {
                        rows[r] = base[r];
                    }
                    bool above = false;
                    int low = 0;
                    for(const point& m : shape_of(type, rot).minos)
                    {
                        const int cy = y + m.y;
                        if(cy < 0)
                        {
                            above = true;
                            continue;
                        }
                        rows[cy] = uint16_t(rows[cy] | (1u << (x + m.x)));
                        if(cy > low)
                        {
                            low = cy;
                        }
                    }
                    if(above)
                    {
                        continue;
                    }
                    int lines = 0;
                    int write = nrows - 1;
                    for(int read = nrows - 1; read >= 0; --read)
                    {
                        if(rows[read] == full)
                        {
                            ++lines;
                            continue;
                        }
                        rows[write--] = rows[read];
                    }
                    for(; write >= 0; --write)
                    {
                        rows[write] = 0;
                    }
                    int score = _evaluate(rows, cols, nrows, lines, low);
                    if(_strategy == bot_strategy::random_legal)
                    {
                        score = int(const_cast<rng&>(_rng).below(1000));
                    }
                    if(! best.valid || score > best.score)
                    {
                        best.valid = true;
                        best.use_hold = hold;
                        best.rotation = int8_t(rot);
                        best.x = int8_t(x);
                        best.score = score;
                        found = true;
                    }
                }
            }
        }
        return found;
    }

    bot_plan bot::plan(const session& s)
    {
        bot_plan best;
        const player& p = s.board_player(s.active_board());
        if(p.state() != player_state::falling)
        {
            return best;
        }
        _try_plan(s, p.active().type, false, best);
        if(p.can_hold() && _strategy != bot_strategy::random_legal)
        {
            const piece_type alt = p.held() != piece_type::none ? p.held() : s.queue().peek(0).type;
            bot_plan hold_best;
            if(_try_plan(s, alt, true, hold_best) && (! best.valid || hold_best.score > best.score + 40))
            {
                best = hold_best;
            }
        }
        return best;
    }

    uint16_t bot::_ability_keys(const session& s)
    {
        if(_strategy != bot_strategy::resource_aware || ! s.loadout_count())
        {
            return 0;
        }
        const int active = s.active_board();
        const player& p = s.board_player(active);
        const board& b = p.grid();
        const int ability = s.loadout(s.selected_slot());

        if(s.targeting())
        {
            // Aim, then confirm.
            const int t = s.targeting_ability();
            if(t == cfg::ability_laser || t == cfg::ability_nuclear_burst || t == cfg::ability_chain_spark)
            {
                // Laser / nuke: aim at the column with the most cells above holes.
                int best_x = s.target_x();
                int best_score = -1;
                for(int x = 0; x < b.cols(); ++x)
                {
                    int score = 0;
                    bool hole_below = false;
                    for(int y = b.rows() - 1; y >= b.hidden(); --y)
                    {
                        if(! b.at(x, y).occupied())
                        {
                            hole_below = true;
                        }
                        else if(hole_below)
                        {
                            ++score;
                        }
                    }
                    score = score * 4 + b.column_height(x);
                    if(score > best_score)
                    {
                        best_score = score;
                        best_x = x;
                    }
                }
                if(s.target_x() < best_x) { return key_right; }
                if(s.target_x() > best_x) { return key_left; }
                if(t != cfg::ability_laser)
                {
                    const int want_y = b.rows() - b.column_height(best_x);
                    if(s.target_y() < want_y && s.target_y() < b.rows() - 1) { return key_down; }
                    if(s.target_y() > want_y) { return key_up; }
                }
            }
            else if(t == cfg::ability_gravity_pulse)
            {
                if(s.target_option() != 0) { return key_down; }
            }
            return key_a;
        }

        if(! s.ability_ready(ability))
        {
            // Rotate the selection to a ready ability when one exists.
            for(int i = 1; i < s.loadout_count(); ++i)
            {
                const int slot = (s.selected_slot() + i) % s.loadout_count();
                if(s.ability_ready(s.loadout(slot)) && ! s.config().dual)
                {
                    return key_select;
                }
            }
            return 0;
        }

        const int height = b.stack_height();
        const int visible = b.rows() - b.hidden();
        const int holes = b.holes();
        bool use = false;
        switch(ability)
        {
        case cfg::ability_emergency_vent:
            use = height >= visible - 4;
            break;
        case cfg::ability_laser:
        case cfg::ability_nuclear_burst:
            use = holes >= 3 || height >= visible - 5;
            break;
        case cfg::ability_gravity_pulse:
            use = holes >= 4;
            break;
        case cfg::ability_shield_matrix:
            use = s.boss_action().source != 0 || s.hazard_action().source != 0 || s.incoming_rows(active) > 0;
            break;
        case cfg::ability_time_freeze:
            use = height >= visible - 6;
            break;
        case cfg::ability_shape_shift:
            use = false;
            break;
        case cfg::ability_chain_spark:
            use = s.heat() >= 90;
            break;
        default:
            break;
        }
        return use ? key_r : 0;
    }

    input_frame bot::next(const session& s)
    {
        input_frame in;
        if(s.state() != session_state::playing)
        {
            _prev = 0;
            return in;
        }
        uint16_t want = 0;
        const player& p = s.board_player(s.active_board());

        const uint16_t ability = _ability_keys(s);
        if(ability || s.targeting())
        {
            want = ability;
        }
        else if(p.state() != player_state::falling)
        {
            want = 0;
        }
        else
        {
            const int type = int(p.active().type);
            if(type != _last_type || p.active().y < _last_y || ! _plan.valid)
            {
                _plan = plan(s);
                _hold_done = false;
                _stall = 0;
            }
            _last_type = type;
            _last_y = p.active().y;
            ++_stall;

            if(! _plan.valid || _stall > 90)
            {
                want = key_up;
            }
            else if(_plan.use_hold && ! _hold_done && p.can_hold())
            {
                want = key_l;
                _hold_done = true;
                _plan.valid = false;    // replan once the held piece is swapped
            }
            else if(p.active().rotation != _plan.rotation)
            {
                want = ((p.active().rotation + 3) & 3) == _plan.rotation ? key_b : key_a;
            }
            else if(p.active().x != _plan.x)
            {
                want = p.active().x < _plan.x ? key_right : key_left;
            }
            else
            {
                want = key_up;
            }
        }

        if(want & _prev)
        {
            want = 0;       // release first so the next frame registers a fresh press
        }
        in.held = want;
        in.pressed = uint16_t(want & ~_prev);
        _prev = want;
        return in;
    }
}
