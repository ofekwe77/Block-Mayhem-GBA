#include "bm_resolver.h"

namespace bm
{
    namespace
    {
        constexpr int dir_dx[4] = { 0, 1, 0, -1 };     // up, right, down, left
        constexpr int dir_dy[4] = { -1, 0, 1, 0 };

        int min_int(int a, int b)
        {
            return a < b ? a : b;
        }
    }

    void resolver::_reset(const effect_mods& mods)
    {
        _mods = mods;
        if(_mods.spark_range > cfg::spark_range_max)
        {
            _mods.spark_range = cfg::spark_range_max;
        }
        if(_mods.rainbow_targets > 4)
        {
            _mods.rainbow_targets = 4;
        }
        if(_mods.magnet_cols > 4)
        {
            _mods.magnet_cols = 4;
        }
        _result = resolution_result();
        for(int y = 0; y < board::max_rows; ++y)
        {
            _removed[y] = 0;
            _activated[y] = 0;
        }
        _cleared_rows = 0;
        _current_count = 0;
        _next_count = 0;
        _active_trigger = -1;
        _thaw_pulses = 0;
        _sparks_this_resolution = 0;
        _double_spark_used = false;
    }

    void resolver::begin(const effect_mods& mods)
    {
        _reset(mods);
        _phase = phase::scan;
    }

    void resolver::begin_external(const effect_mods& mods)
    {
        _reset(mods);
        _result.waves = 1;
        _phase = phase::waves;
    }

    void resolver::begin_compaction(board& b, gravity_dir dir, const effect_mods& mods, event_queue& events,
                                    int board_index)
    {
        _reset(mods);
        compact(b, dir);
        events.push(event_type::gravity_shift, board_index, int(dir));
        _phase = phase::scan;
    }

    int resolver::step(board& b, event_queue& events, int board_index)
    {
        switch(_phase)
        {
        case phase::scan:
            return _scan(b, events, board_index);

        case phase::waves:
            return _wave(b, events, board_index);

        case phase::settle:
            return _settle(b, events, board_index);

        case phase::finish:
            _finish(b, events, board_index);
            return 0;

        case phase::idle:
            return 0;

        default:
            return 0;
        }
    }

    int resolver::_scan(board& b, event_queue& events, int board_index)
    {
        uint32_t full = 0;
        int count = 0;
        for(int y = 0; y < b.rows(); ++y)
        {
            if(b.row_full(y))
            {
                full |= 1u << y;
                ++count;
            }
        }
        if(! count)
        {
            _phase = phase::finish;
            return 0;
        }

        bool coated = false;
        for(int y = 0; y < b.rows() && ! coated; ++y)
        {
            if((full >> y) & 1)
            {
                for(int x = 0; x < b.cols(); ++x)
                {
                    if(b.at(x, y).coated())
                    {
                        coated = true;
                        break;
                    }
                }
            }
        }

        if(coated)
        {
            const bool force = _thaw_pulses >= cfg::max_thaw_pulses;
            ++_thaw_pulses;
            for(int y = 0; y < b.rows(); ++y)
            {
                if(! ((full >> y) & 1))
                {
                    continue;
                }
                for(int x = 0; x < b.cols(); ++x)
                {
                    cell& c = b.attributes(x, y);
                    if(! c.coated())
                    {
                        continue;
                    }
                    const coat_kind kind = c.coat;
                    const int hp = force ? 0 : int(c.coat_hp) - (1 + _mods.coat_bonus);
                    ++_result.coat_hits;
                    if(hp > 0)
                    {
                        c.coat_hp = uint8_t(hp);
                        events.push(event_type::coat_hit, board_index, x, y, int(kind), hp);
                    }
                    else
                    {
                        c.coat = coat_kind::none;
                        c.coat_hp = 0;
                        if(kind == coat_kind::ice)
                        {
                            ++_result.ice_broken;
                        }
                        else
                        {
                            ++_result.armor_broken;
                        }
                        events.push(event_type::coat_hit, board_index, x, y, int(kind), 0);
                    }
                }
            }
            events.push(event_type::thaw_pulse, board_index, int(full & 0xFFFF), int(full >> 16));
            return cfg::thaw_delay;     // rescan: rows are still full and clear once no coat remains
        }

        // Line wave: clear complete rows in place, queue the specials they contain.
        ++_result.waves;
        ++_result.line_waves;
        _result.any_line = true;
        if(_result.line_waves == 1)
        {
            _result.first_rows = count;
        }
        else
        {
            _result.extra_rows += count;
        }
        _result.rows_total += count;
        if(_result.line_waves <= 8)
        {
            _result.line_wave_rows[_result.line_waves - 1] = uint8_t(count);
        }
        if(_result.waves > 1)
        {
            events.push(event_type::chain_wave, board_index, _result.waves);
        }
        events.push(event_type::rows_full, board_index, int(full & 0xFFFF), int(full >> 16), count);

        _cleared_rows = full;
        for(int y = 0; y < b.rows(); ++y)
        {
            if(! ((full >> y) & 1))
            {
                continue;
            }
            bool garbage_row = false;
            for(int x = 0; x < b.cols(); ++x)
            {
                const cell c = b.at(x, y);
                if(c.color == color_garbage)
                {
                    garbage_row = true;
                }
                if(c.is_special() && ! (c.flags & flag_disabled) && ! ((_activated[y] >> x) & 1))
                {
                    _activated[y] = uint16_t(_activated[y] | (1u << x));
                    _enqueue(x, y, trigger_kind(int(c.special)));
                }
                if(c.flags & flag_charged)
                {
                    _enqueue(x, y, trigger_kind::pulse);
                }
                b.clear(x, y);
            }
            if(garbage_row)
            {
                ++_result.garbage_rows_cleared;
            }
        }
        events.push(event_type::lines_cleared, board_index, count, 0, 0, _result.waves);
        _phase = phase::waves;
        return cfg::line_clear_delay[min_int(count, 4)];
    }

    int resolver::_wave(board& b, event_queue& events, int board_index)
    {
        if(! _next_count)
        {
            _phase = phase::settle;
            return 0;
        }
        if(_result.waves >= cfg::max_resolution_waves)
        {
            _next_count = 0;    // safety bound: discard remaining activations
            _phase = phase::settle;
            return 0;
        }

        ++_result.waves;
        events.push(event_type::chain_wave, board_index, _result.waves);

        _current_count = _next_count;
        for(int i = 0; i < _next_count; ++i)
        {
            _current[i] = _next[i];
        }
        _next_count = 0;

        for(int i = 0; i < _current_count; ++i)
        {
            _active_trigger = i;
            _run_trigger(b, _current[i], events, board_index);
        }
        _active_trigger = -1;

        for(int i = 0; i < _current_count; ++i)
        {
            _result.special_points += min_int(_current[i].destroyed * cfg::special_cell_points,
                                              cfg::special_cell_points_cap);
        }
        _current_count = 0;
        return cfg::wave_delay;
    }

    int resolver::_settle(board& b, event_queue& events, int board_index)
    {
        bool changed = false;
        if(_cleared_rows)
        {
            b.remove_rows(_cleared_rows);
            // Keep the effect-gap marks aligned with the rows that moved.
            uint16_t shifted[board::max_rows] = {};
            int write = b.rows() - 1;
            for(int read = b.rows() - 1; read >= 0; --read)
            {
                if((_cleared_rows >> read) & 1)
                {
                    continue;
                }
                shifted[write--] = _removed[read];
            }
            for(int y = 0; y < board::max_rows; ++y)
            {
                _removed[y] = shifted[y];
            }
            _cleared_rows = 0;
            changed = true;
        }

        bool gaps = false;
        for(int y = 0; y < b.rows(); ++y)
        {
            if(_removed[y])
            {
                gaps = true;
                break;
            }
        }

        if(gaps)
        {
            const int cols = b.cols();
            const int rows = b.rows();
            cell line[board::max_rows > board::max_cols ? board::max_rows : board::max_cols];

            switch(_mods.settle_dir)
            {
            case gravity_dir::down:
                for(int x = 0; x < cols; ++x)
                {
                    for(int y = 0; y < rows; ++y)
                    {
                        line[y] = cell();
                    }
                    int shift = 0;
                    for(int y = rows - 1; y >= 0; --y)
                    {
                        if((_removed[y] >> x) & 1)
                        {
                            ++shift;
                            continue;
                        }
                        line[y + shift] = b.at(x, y);
                    }
                    for(int y = 0; y < rows; ++y)
                    {
                        b.set(x, y, line[y]);
                    }
                }
                break;

            case gravity_dir::up:
                for(int x = 0; x < cols; ++x)
                {
                    for(int y = 0; y < rows; ++y)
                    {
                        line[y] = cell();
                    }
                    int shift = 0;
                    for(int y = 0; y < rows; ++y)
                    {
                        if((_removed[y] >> x) & 1)
                        {
                            ++shift;
                            continue;
                        }
                        line[y - shift] = b.at(x, y);
                    }
                    for(int y = 0; y < rows; ++y)
                    {
                        b.set(x, y, line[y]);
                    }
                }
                break;

            case gravity_dir::left:
                for(int y = 0; y < rows; ++y)
                {
                    for(int x = 0; x < cols; ++x)
                    {
                        line[x] = cell();
                    }
                    int shift = 0;
                    for(int x = 0; x < cols; ++x)
                    {
                        if((_removed[y] >> x) & 1)
                        {
                            ++shift;
                            continue;
                        }
                        line[x - shift] = b.at(x, y);
                    }
                    for(int x = 0; x < cols; ++x)
                    {
                        b.set(x, y, line[x]);
                    }
                }
                break;

            case gravity_dir::right:
                for(int y = 0; y < rows; ++y)
                {
                    for(int x = 0; x < cols; ++x)
                    {
                        line[x] = cell();
                    }
                    int shift = 0;
                    for(int x = cols - 1; x >= 0; --x)
                    {
                        if((_removed[y] >> x) & 1)
                        {
                            ++shift;
                            continue;
                        }
                        line[x + shift] = b.at(x, y);
                    }
                    for(int x = 0; x < cols; ++x)
                    {
                        b.set(x, y, line[x]);
                    }
                }
                break;

            default:
                break;
            }
            events.push(event_type::settle, board_index, int(_mods.settle_dir));
            changed = true;
        }

        for(int y = 0; y < board::max_rows; ++y)
        {
            _removed[y] = 0;
            _activated[y] = 0;
        }
        _phase = phase::scan;
        return changed ? cfg::settle_delay : 0;
    }

    void resolver::_finish(board& b, event_queue& events, int board_index)
    {
        _result.perfect_clear = _result.any_line && b.empty();
        if(_result.perfect_clear)
        {
            events.push(event_type::perfect_clear, board_index);
        }
        events.push(event_type::resolution_done, board_index, _result.waves, _result.rows_total, _result.destroyed);
        _phase = phase::idle;
    }

    void resolver::_enqueue(int x, int y, trigger_kind kind)
    {
        if(_next_count >= max_triggers)
        {
            return;     // bounded queue (cannot happen with <= 200 cells, kept as a safety net)
        }
        trigger& t = _next[_next_count++];
        t.x = int8_t(x);
        t.y = int8_t(y);
        t.kind = kind;
        t.destroyed = 0;
    }

    bool resolver::hit(board& b, int x, int y, int power, int cause, event_queue& events, int board_index)
    {
        if(! b.inside(x, y))
        {
            return false;
        }
        const cell& c = b.at(x, y);
        if(c.empty() || c.color == color_stone || power <= 0)
        {
            return false;
        }
        if(c.coated())
        {
            cell& m = b.attributes(x, y);
            const coat_kind kind = m.coat;
            const int before = m.coat_hp;
            const int after = before - (power + _mods.coat_bonus);
            ++_result.coat_hits;
            if(after > 0)
            {
                m.coat_hp = uint8_t(after);
                events.push(event_type::coat_hit, board_index, x, y, int(kind), after);
                return true;
            }
            m.coat = coat_kind::none;
            m.coat_hp = 0;
            if(kind == coat_kind::ice)
            {
                ++_result.ice_broken;
            }
            else
            {
                ++_result.armor_broken;
            }
            events.push(event_type::coat_hit, board_index, x, y, int(kind), 0);
            // Only the base power left after breaking the coat carries through to the cell.
            if(power - before <= 0)
            {
                return true;
            }
        }
        _destroy(b, x, y, cause, events, board_index);
        return true;
    }

    void resolver::remove(board& b, int x, int y, int cause, event_queue& events, int board_index)
    {
        if(! b.inside(x, y) || b.at(x, y).empty())
        {
            return;
        }
        const int color = b.at(x, y).color;
        b.clear(x, y);
        _removed[y] = uint16_t(_removed[y] | (1u << x));
        events.push(event_type::cell_destroyed, board_index, x, y, color, cause);
    }

    void resolver::_destroy(board& b, int x, int y, int cause, event_queue& events, int board_index)
    {
        const cell c = b.at(x, y);
        if(c.is_special() && ! (c.flags & flag_disabled) && ! ((_activated[y] >> x) & 1))
        {
            _activated[y] = uint16_t(_activated[y] | (1u << x));
            _enqueue(x, y, trigger_kind(int(c.special)));
        }
        if(c.flags & flag_charged)
        {
            _enqueue(x, y, trigger_kind::pulse);
        }
        b.clear(x, y);
        _removed[y] = uint16_t(_removed[y] | (1u << x));
        ++_result.destroyed;
        if(_active_trigger >= 0)
        {
            ++_current[_active_trigger].destroyed;
        }
        events.push(event_type::cell_destroyed, board_index, x, y, c.color, cause);
    }

    void resolver::_run_trigger(board& b, trigger& t, event_queue& events, int board_index)
    {
        ++_result.triggers[int(t.kind)];
        events.push(event_type::special_triggered, board_index, t.x, t.y, int(t.kind), _result.waves);

        switch(t.kind)
        {
        case trigger_kind::bomb:
            _bomb(b, t.x, t.y, events, board_index);
            break;

        case trigger_kind::fire:
            _fire(b, t.x, t.y, events, board_index);
            break;

        case trigger_kind::spark:
            ++_sparks_this_resolution;
            _arcs(b, t.x, t.y, _mods.spark_range, cause_spark, events, board_index);
            if(_mods.double_spark && ! _double_spark_used && _sparks_this_resolution >= 2)
            {
                // DOUBLE SPARK: a qualified electrical chain (2+ sparks) discharges once more.
                _double_spark_used = true;
                _enqueue(t.x, t.y, trigger_kind::pulse);
            }
            break;

        case trigger_kind::magnet:
            _magnet(b, t.x, t.y, events, board_index);
            break;

        case trigger_kind::rainbow:
            _rainbow(b, t.x, t.y, events, board_index);
            break;

        case trigger_kind::pulse:
            events.push(event_type::charge_pulse, board_index, t.x, t.y);
            _arcs(b, t.x, t.y, cfg::chain_spark_pulse_range, cause_pulse, events, board_index);
            break;

        default:
            break;
        }
    }

    void resolver::_bomb(board& b, int x, int y, event_queue& events, int board_index)
    {
        const int r = cfg::bomb_radius;
        events.push(event_type::bomb_blast, board_index, x, y, r + _mods.bomb_plus);
        for(int dy = -r; dy <= r; ++dy)
        {
            for(int dx = -r; dx <= r; ++dx)
            {
                if(dx || dy)
                {
                    hit(b, x + dx, y + dy, _mods.bomb_power, cause_bomb, events, board_index);
                }
            }
        }
        for(int k = r + 1; k <= r + _mods.bomb_plus; ++k)
        {
            for(int d = 0; d < 4; ++d)
            {
                hit(b, x + dir_dx[d] * k, y + dir_dy[d] * k, _mods.bomb_power, cause_bomb, events, board_index);
            }
        }
    }

    void resolver::_fire(board& b, int x, int y, event_queue& events, int board_index)
    {
        events.push(event_type::fire_burst, board_index, x, y, _mods.fire_reach);
        for(int d = 0; d < 4; ++d)
        {
            for(int k = 1; k <= _mods.fire_reach; ++k)
            {
                const int nx = x + dir_dx[d] * k;
                const int ny = y + dir_dy[d] * k;
                if(! b.inside(nx, ny))
                {
                    break;
                }
                if(b.at(nx, ny).occupied())
                {
                    hit(b, nx, ny, 1, cause_fire, events, board_index);
                    break;
                }
            }
        }
    }

    void resolver::_arcs(board& b, int x, int y, int range, int cause, event_queue& events, int board_index)
    {
        for(int d = 0; d < 4; ++d)
        {
            int length = 0;
            for(int k = 1; k <= range; ++k)
            {
                const int nx = x + dir_dx[d] * k;
                const int ny = y + dir_dy[d] * k;
                if(! b.inside(nx, ny))
                {
                    break;
                }
                length = k;
                const cell& c = b.at(nx, ny);
                if(c.empty())
                {
                    continue;
                }
                if(c.color == color_stone)
                {
                    break;      // stone insulates
                }
                const bool armored = c.coated() && c.coat == coat_kind::armor;
                hit(b, nx, ny, 1, cause, events, board_index);
                if(armored)
                {
                    break;      // armor blocks the arc (and takes the hit)
                }
            }
            events.push(event_type::spark_arc, board_index, x, y, d, length);
        }
    }

    void resolver::_magnet(board& b, int x, int y, event_queue& events, int board_index)
    {
        const int cols = _mods.magnet_cols;
        const int rows = _mods.magnet_rows;

        // Gap search order: column distance, then own row / below / above (alternating outward), then left first.
        for(int dist = 0; dist <= cols; ++dist)
        {
            for(int r = 0; r <= rows * 2; ++r)
            {
                const int dy = (r == 0) ? 0 : ((r & 1) ? (r + 1) / 2 : -(r / 2));
                const int gy = y + dy;
                if(gy >= 0 && gy < 32 && ((_cleared_rows >> gy) & 1))
                {
                    continue;   // rows being cleared this wave are not gaps
                }
                for(int side = 0; side < (dist ? 2 : 1); ++side)
                {
                    const int gx = side == 0 ? x - dist : x + dist;
                    if(! b.inside(gx, gy) || b.at(gx, gy).occupied() || ((_removed[gy] >> gx) & 1))
                    {
                        continue;
                    }
                    // The gap must be supported (floor or a surviving cell below) so the block does not float.
                    const bool supported = gy == b.rows() - 1 ||
                            (b.at(gx, gy + 1).occupied() && ! ((_removed[gy + 1] >> gx) & 1));
                    if(! supported)
                    {
                        continue;
                    }
                    for(int s = 0; s < 2; ++s)
                    {
                        const int sx = s == 0 ? gx - 1 : gx + 1;
                        if(! b.inside(sx, gy))
                        {
                            continue;
                        }
                        const cell& src = b.at(sx, gy);
                        if(! src.ordinary() || ((_removed[gy] >> sx) & 1) || (src.flags & flag_charged))
                        {
                            continue;
                        }
                        const cell moved = src;
                        b.clear(sx, gy);
                        _removed[gy] = uint16_t(_removed[gy] | (1u << sx));     // column above the source settles
                        b.set(gx, gy, moved);
                        ++_result.magnet_moves;
                        events.push(event_type::magnet_pull, board_index, sx, gy, gx, gy);
                        return;
                    }
                }
            }
        }
        ++_result.magnet_fizzles;
        events.push(event_type::magnet_fizzle, board_index, x, y);
    }

    void resolver::_rainbow(board& b, int x, int y, event_queue& events, int board_index)
    {
        static constexpr int8_t offsets[8][2] = {
            { -1, -1 }, { 0, -1 }, { 1, -1 }, { -1, 0 }, { 1, 0 }, { -1, 1 }, { 0, 1 }, { 1, 1 }
        };
        // Priority: bomb, spark, fire, magnet, rainbow.
        static constexpr int priority[special_kind_count] = { 9, 0, 2, 1, 3, 4 };

        int chosen = 0;
        bool used[8] = {};
        while(chosen < _mods.rainbow_targets)
        {
            int best = -1;
            int best_priority = 99;
            for(int i = 0; i < 8; ++i)
            {
                if(used[i])
                {
                    continue;
                }
                const int nx = x + offsets[i][0];
                const int ny = y + offsets[i][1];
                if(! b.inside(nx, ny))
                {
                    continue;
                }
                const cell& c = b.at(nx, ny);
                if(! c.is_special() || c.coated() || (c.flags & flag_disabled) || ((_activated[ny] >> nx) & 1))
                {
                    continue;
                }
                const int p = priority[int(c.special)];
                if(p < best_priority)
                {
                    best_priority = p;
                    best = i;
                }
            }
            if(best < 0)
            {
                break;
            }
            used[best] = true;
            const int tx = x + offsets[best][0];
            const int ty = y + offsets[best][1];
            events.push(event_type::rainbow_link, board_index, x, y, tx, ty);
            _destroy(b, tx, ty, cause_rainbow, events, board_index);     // queues the target for the next wave
            ++chosen;
        }

        if(! chosen)
        {
            // No special neighbour: wild burst on the four orthogonal neighbours.
            for(int d = 0; d < 4; ++d)
            {
                hit(b, x + dir_dx[d], y + dir_dy[d], 1, cause_rainbow, events, board_index);
            }
        }
    }

    void resolver::compact(board& b, gravity_dir dir)
    {
        const int cols = b.cols();
        const int rows = b.rows();
        cell line[board::max_rows > board::max_cols ? board::max_rows : board::max_cols];

        switch(dir)
        {
        case gravity_dir::down:
            for(int x = 0; x < cols; ++x)
            {
                int n = 0;
                for(int y = 0; y < rows; ++y)
                {
                    if(b.at(x, y).occupied())
                    {
                        line[n++] = b.at(x, y);
                    }
                }
                for(int y = 0; y < rows; ++y)
                {
                    const int k = y - (rows - n);
                    b.set(x, y, k >= 0 ? line[k] : cell());
                }
            }
            break;

        case gravity_dir::up:
            for(int x = 0; x < cols; ++x)
            {
                int top = -1;
                int n = 0;
                for(int y = 0; y < rows; ++y)
                {
                    if(b.at(x, y).occupied())
                    {
                        if(top < 0)
                        {
                            top = y;
                        }
                        line[n++] = b.at(x, y);
                    }
                }
                if(top < 0)
                {
                    continue;
                }
                for(int y = top; y < rows; ++y)
                {
                    const int k = y - top;
                    b.set(x, y, k < n ? line[k] : cell());
                }
            }
            break;

        case gravity_dir::left:
            for(int y = 0; y < rows; ++y)
            {
                int n = 0;
                for(int x = 0; x < cols; ++x)
                {
                    if(b.at(x, y).occupied())
                    {
                        line[n++] = b.at(x, y);
                    }
                }
                for(int x = 0; x < cols; ++x)
                {
                    b.set(x, y, x < n ? line[x] : cell());
                }
            }
            break;

        case gravity_dir::right:
            for(int y = 0; y < rows; ++y)
            {
                int n = 0;
                for(int x = 0; x < cols; ++x)
                {
                    if(b.at(x, y).occupied())
                    {
                        line[n++] = b.at(x, y);
                    }
                }
                for(int x = 0; x < cols; ++x)
                {
                    const int k = x - (cols - n);
                    b.set(x, y, k >= 0 ? line[k] : cell());
                }
            }
            break;

        default:
            break;
        }
    }
}
