#include "bm_player.h"

namespace bm
{
    // ------------------------------------------------------------------ piece_queue

    void piece_queue::init(uint32_t seed, const special_config& specials, uint32_t special_seed)
    {
        _bag.seed(seed);
        _specials.configure(specials, special_seed);
        _head = 0;
        _script = nullptr;
        _script_count = 0;
        _script_index = 0;
        for(int i = 0; i < cfg::queue_length; ++i)
        {
            _items[i] = _generate();
        }
    }

    void piece_queue::script(const piece_type* types, int count)
    {
        _script = types;
        _script_count = count;
        _script_index = 0;
        // Regenerate the visible queue so the script starts with the very next piece.
        for(int i = 0; i < cfg::queue_length; ++i)
        {
            _items[(_head + i) % cfg::queue_length] = _generate();
        }
    }

    queued_piece piece_queue::_generate()
    {
        queued_piece q;
        if(_script && _script_index < _script_count)
        {
            q.type = _script[_script_index++];
        }
        else
        {
            q.type = _bag.next();
        }
        _specials.decorate(q);
        return q;
    }

    queued_piece piece_queue::pop()
    {
        const queued_piece result = _items[_head];
        _items[_head] = _generate();
        _head = (_head + 1) % cfg::queue_length;
        return result;
    }

    // ------------------------------------------------------------------ player

    void player::init(int index, int cols, int rows, int hidden, piece_queue* queue, event_queue* events)
    {
        _index = index;
        _board.init(cols, rows, hidden);
        _queue = queue;
        _events = events;
        _piece = piece_state();
        _hold = queued_piece();
        _last_lock = lock_info();
        _state = player_state::waiting_spawn;
        _gravity_counter = 0;
        _soft_counter = 0;
        _das_dir = 0;
        _das_timer = 0;
        _lock_timer = 0;
        _lock_resets = 0;
        _lowest_y = 0;
        _delay = 0;
        _spawn_delay = 0;
        _soft_rows = 0;
        _can_hold = true;
        _last_rotate = false;
        _last_kick = 0;
        _resolution_finished = false;
        _lock_resolution = false;
        _resume_after_resolution = false;
        combo = 0;
        b2b = 0;
        last_lock_rows = 0;
    }

    int player::ghost_y() const
    {
        return _piece.y + _board.drop_distance(_piece);
    }

    bool player::grounded() const
    {
        piece_state probe = _piece;
        ++probe.y;
        return ! _board.fits(probe);
    }

    void player::allow_spawn(int delay_frames)
    {
        if(_state != player_state::waiting_spawn)
        {
            return;
        }
        _spawn_delay = delay_frames;
        _state = player_state::spawn_delay;
    }

    bool player::spawn_now()
    {
        if(_state != player_state::waiting_spawn && _state != player_state::spawn_delay)
        {
            return _state == player_state::falling;
        }
        return _spawn(_queue->pop());
    }

    void player::set_topped_out()
    {
        _state = player_state::topped_out;
    }

    bool player::_spawn(const queued_piece& next)
    {
        _piece.type = next.type;
        _piece.rotation = 0;
        _piece.special = next.special;
        _piece.special_mino = next.special_mino;
        const int box = box_size(next.type);
        _piece.x = int8_t((_board.cols() - (box == 4 ? 4 : 3)) / 2);
        _piece.y = 0;
        if(! _board.fits(_piece))
        {
            _state = player_state::topped_out;
            return false;
        }
        // Drop one row immediately so the piece becomes visible below the spawn chute.
        piece_state lower = _piece;
        lower.y = 1;
        if(_board.fits(lower))
        {
            _piece = lower;
        }
        _gravity_counter = 0;
        _lock_timer = 0;
        _lock_resets = 0;
        _lowest_y = _piece.y;
        _soft_rows = 0;
        _last_rotate = false;
        _last_kick = 0;
        _state = player_state::falling;
        _events->push(event_type::piece_spawned, _index, int(next.type));
        return true;
    }

    bool player::_try_move(int dx, int dy)
    {
        piece_state p = _piece;
        p.x = int8_t(p.x + dx);
        p.y = int8_t(p.y + dy);
        if(! _board.fits(p))
        {
            return false;
        }
        _piece = p;
        _last_rotate = false;
        if(dy > 0)
        {
            if(_piece.y > _lowest_y)
            {
                _lowest_y = _piece.y;
                _lock_resets = 0;
                _lock_timer = 0;
            }
        }
        else
        {
            _on_successful_action();
            _events->push(event_type::piece_moved, _index, dx);
        }
        return true;
    }

    void player::_on_successful_action()
    {
        if(_lock_timer > 0 && _lock_resets < cfg::max_lock_resets)
        {
            _lock_timer = 0;
            ++_lock_resets;
        }
    }

    bool player::_try_rotate(int dir)
    {
        if(_piece.type == piece_type::o)
        {
            return false;   // the O shape never rotates
        }
        const int from = _piece.rotation;
        const int to = (from + dir) & 3;
        const point* tests = nullptr;
        const int count = kick_tests(_piece.type, from, to, tests);
        for(int i = 0; i < count; ++i)
        {
            piece_state p = _piece;
            p.rotation = int8_t(to);
            p.x = int8_t(p.x + tests[i].x);
            p.y = int8_t(p.y + tests[i].y);
            if(_board.fits(p))
            {
                _piece = p;
                _last_rotate = true;
                _last_kick = i;
                _on_successful_action();
                if(_piece.y > _lowest_y)
                {
                    _lowest_y = _piece.y;
                    _lock_resets = 0;
                    _lock_timer = 0;
                }
                _events->push(event_type::piece_rotated, _index, i > 0 ? 1 : 0);
                return true;
            }
        }
        return false;
    }

    void player::_handle_horizontal(const input_frame& input)
    {
        const bool left_hit = input.hit(key_left);
        const bool right_hit = input.hit(key_right);
        const bool left = input.down(key_left);
        const bool right = input.down(key_right);

        if(left_hit || right_hit)
        {
            _das_dir = right_hit && ! left_hit ? 1 : (left_hit && ! right_hit ? -1 : (right ? 1 : -1));
            _das_timer = 0;
            _try_move(_das_dir, 0);
            return;
        }

        // Keep repeating in the held direction; if the repeating key was released fall back to the other one.
        if(_das_dir == -1 && ! left)
        {
            _das_dir = right ? 1 : 0;
            _das_timer = 0;
        }
        else if(_das_dir == 1 && ! right)
        {
            _das_dir = left ? -1 : 0;
            _das_timer = 0;
        }

        if(! _das_dir)
        {
            return;
        }
        ++_das_timer;
        if(_das_timer >= cfg::das_frames)
        {
            if((_das_timer - cfg::das_frames) % cfg::arr_frames == 0)
            {
                _try_move(_das_dir, 0);
            }
        }
    }

    void player::_hold_piece()
    {
        if(! _can_hold)
        {
            return;
        }
        queued_piece current;
        current.type = _piece.type;
        current.special = _piece.special;
        current.special_mino = _piece.special_mino;
        const queued_piece next = _hold.type == piece_type::none ? _queue->pop() : _hold;
        _hold = current;
        _can_hold = false;
        _events->push(event_type::piece_held, _index, int(current.type));
        _spawn(next);
    }

    twist_kind player::_detect_twist() const
    {
        if(_piece.type != piece_type::t || ! _last_rotate)
        {
            return twist_kind::none;
        }
        const int x = _piece.x;
        const int y = _piece.y;
        const bool corner[4] = {
            _board.blocked(x, y), _board.blocked(x + 2, y), _board.blocked(x, y + 2), _board.blocked(x + 2, y + 2)
        };
        const int count = int(corner[0]) + int(corner[1]) + int(corner[2]) + int(corner[3]);
        if(count < 3)
        {
            return twist_kind::none;
        }
        static constexpr int front[4][2] = { { 0, 1 }, { 1, 3 }, { 2, 3 }, { 0, 2 } };
        const int r = _piece.rotation & 3;
        if((corner[front[r][0]] && corner[front[r][1]]) || _last_kick == 4)
        {
            return twist_kind::full;
        }
        return twist_kind::mini;
    }

    void player::_lock(bool hard)
    {
        _last_lock = lock_info();
        _last_lock.type = _piece.type;
        _last_lock.twist = _detect_twist();
        _last_lock.soft_drop_rows = _soft_rows;
        int min_y = 99;
        int max_y = -99;
        bool above = false;
        bool all_hidden = true;
        for(int m = 0; m < 4; ++m)
        {
            const point p = _piece.cell(m);
            if(p.y < min_y)
            {
                min_y = p.y;
            }
            if(p.y > max_y)
            {
                max_y = p.y;
            }
            if(p.y >= _board.hidden())
            {
                all_hidden = false;
            }
            if(p.y < 0)
            {
                above = true;
                continue;
            }
            cell c;
            c.color = uint8_t(_piece.type);
            if(m == _piece.special_mino)
            {
                c.special = _piece.special;
                if(_mods.disable_new_specials && c.special != special_kind::none)
                {
                    c.flags = flag_disabled;
                }
            }
            _board.set(p.x, p.y, c);
        }
        _last_lock.lowest_row = max_y;
        _last_lock.highest_row = min_y;
        _last_lock.topped_out = above || all_hidden;
        _events->push(event_type::piece_locked, _index, int(_piece.type), max_y, hard ? 1 : 0);
        _can_hold = true;
        _piece.type = piece_type::none;

        if(_last_lock.topped_out)
        {
            _state = player_state::topped_out;
            return;
        }
        _resolver.begin(_mods);
        _lock_resolution = true;
        _resume_after_resolution = false;
        _state = player_state::resolving;
        _delay = 0;
    }

    void player::start_external_resolution(const effect_mods& mods)
    {
        (void) mods;    // modifiers were captured by resolver::begin_external before the hits
        _lock_resolution = false;
        _resume_after_resolution = _state == player_state::falling;
        _saved_piece = _piece;
        _state = player_state::resolving;
        _delay = 0;
    }

    void player::start_compaction(gravity_dir dir, const effect_mods& mods)
    {
        _resolver.begin_compaction(_board, dir, mods, *_events, _index);
        _lock_resolution = false;
        _resume_after_resolution = _state == player_state::falling;
        _saved_piece = _piece;
        _state = player_state::resolving;
        _delay = cfg::settle_delay;
    }

    void player::revive()
    {
        if(_state == player_state::topped_out)
        {
            _piece.type = piece_type::none;
            _state = player_state::waiting_spawn;
        }
    }

    bool player::transform_active(piece_type type, bool apply, piece_state* preview)
    {
        if(_state != player_state::falling || type == _piece.type || type == piece_type::none)
        {
            return false;
        }
        static constexpr int8_t offsets[6][2] = { { 0, 0 }, { -1, 0 }, { 1, 0 }, { 0, -1 }, { -1, -1 }, { 1, -1 } };
        for(const auto& o : offsets)
        {
            piece_state p = _piece;
            p.type = type;
            p.rotation = 0;
            p.x = int8_t(_piece.x + o[0]);
            p.y = int8_t(_piece.y + o[1]);
            if(_board.fits(p))
            {
                if(preview)
                {
                    *preview = p;
                }
                if(! apply)
                {
                    return true;
                }
                _piece = p;
                _last_rotate = false;
                if(_piece.y > _lowest_y)
                {
                    _lowest_y = _piece.y;
                }
                return true;
            }
        }
        return false;
    }

    bool player::update(const input_frame& input, const frame_params& params, const effect_mods& mods)
    {
        _mods = mods;
        switch(_state)
        {
        case player_state::spawn_delay:
            if(--_spawn_delay <= 0)
            {
                _spawn(_queue->pop());
            }
            return false;

        case player_state::resolving:
            if(_delay > 0)
            {
                --_delay;
                return false;
            }
            for(int guard = 0; guard < 64; ++guard)
            {
                _delay = _resolver.step(_board, *_events, _index);
                if(! _resolver.active())
                {
                    _resolution_finished = true;
                    if(_resume_after_resolution)
                    {
                        // An ability/hazard resolution ran under a falling piece: resume it, nudging it
                        // upward if settled cells now overlap it.
                        _piece = _saved_piece;
                        int bumps = 0;
                        while(! _board.fits(_piece) && bumps < 4)
                        {
                            --_piece.y;
                            ++bumps;
                        }
                        if(! _board.fits(_piece))
                        {
                            _state = player_state::topped_out;
                            return true;
                        }
                        _state = player_state::falling;
                    }
                    else
                    {
                        _state = player_state::waiting_spawn;
                    }
                    return true;
                }
                if(_delay > 0)
                {
                    break;
                }
            }
            return false;

        case player_state::falling:
            break;

        case player_state::waiting_spawn:
        case player_state::topped_out:
        default:
            return false;
        }

        if(params.accept_input)
        {
            if(input.hit(key_l))
            {
                _hold_piece();
                if(_state != player_state::falling)
                {
                    return false;
                }
            }
            if(input.hit(key_a))
            {
                _try_rotate(1);
            }
            if(input.hit(key_b))
            {
                _try_rotate(-1);
            }
            _handle_horizontal(input);

            if(input.hit(key_up))
            {
                const int distance = _board.drop_distance(_piece);
                if(distance > 0)
                {
                    _piece.y = int8_t(_piece.y + distance);
                    _last_rotate = false;
                }
                const point top = _piece.cell(0);
                _events->push(event_type::hard_drop, _index, distance, _piece.x * 2 + box_size(_piece.type),
                              top.y - distance, _piece.y);
                _lock(true);
                _last_lock.hard_drop_rows = distance;
                return false;
            }

            if(params.soft_drop_allowed && input.down(key_down))
            {
                if(input.hit(key_down) || ++_soft_counter >= cfg::soft_drop_frames)
                {
                    _soft_counter = 0;
                    if(_try_move(0, 1))
                    {
                        ++_soft_rows;
                        _gravity_counter = 0;
                    }
                }
            }
            else
            {
                _soft_counter = 0;
            }
        }
        else
        {
            _das_dir = 0;
            _das_timer = 0;
        }

        if(params.frozen)
        {
            return false;
        }

        if(params.gravity_frames > 0)
        {
            if(++_gravity_counter >= params.gravity_frames)
            {
                _gravity_counter = 0;
                _try_move(0, 1);
            }
        }

        if(grounded())
        {
            if(++_lock_timer >= cfg::lock_delay_frames)
            {
                _lock(false);
            }
        }
        return false;
    }
}
