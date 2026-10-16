#include "bm_session.h"

namespace bm
{
    namespace
    {
        int clampi(int v, int lo, int hi)
        {
            return v < lo ? lo : (v > hi ? hi : v);
        }

        int mini(int a, int b)
        {
            return a < b ? a : b;
        }

        int maxi(int a, int b)
        {
            return a > b ? a : b;
        }

        special_config arcade_specials()
        {
            special_config c;
            c.weights[0] = 40;      // bomb
            c.weights[1] = 24;      // fire
            c.weights[2] = 24;      // spark
            c.weights[3] = 8;       // magnet
            c.weights[4] = 4;       // rainbow (rare catalyst)
            c.interval_min = cfg::special_interval_normal_min;
            c.interval_max = cfg::special_interval_normal_max;
            return c;
        }

        bool attack_uses_columns(attack_kind k)
        {
            return k == attack_kind::crystal_shards || k == attack_kind::molten_rain ||
                   k == attack_kind::fire_breath || k == attack_kind::frozen_wall;
        }

        // Combination attacks: two previously learned attacks executed together.
        constexpr attack_kind core_combos[4][2] = {
            { attack_kind::garbage, attack_kind::ice_inject },
            { attack_kind::molten_rain, attack_kind::fire_breath },
            { attack_kind::gravity_flip, attack_kind::crystal_shards },
            { attack_kind::armor_inject, attack_kind::surge },
        };
    }

    // ================================================================== setup

    session_config make_stage_config(const stage_def& st, difficulty diff, uint32_t seed)
    {
        session_config c;
        c.mode = game_mode::adventure;
        c.diff = diff;
        c.seed = seed;
        c.stage = &st;
        c.start_level = st.base_level;
        c.dual = st.dual;
        c.heat = st.heat;
        c.specials = true;
        c.start_heat = st.start_heat;
        c.loadout_count = st.loadout_count;
        for(int i = 0; i < 3; ++i)
        {
            c.loadout[i] = st.loadout[i];
        }
        if(st.boss)
        {
            c.boss = &boss(st.boss);
            c.boss_id = st.boss;
            c.dual = c.boss->dual || st.dual;
        }
        return c;
    }

    void session::start(const session_config& config)
    {
        _config = config;
        if(_config.loadout_count > 3)
        {
            _config.loadout_count = 3;
        }
        if(_config.dual && _config.loadout_count > 1)
        {
            _config.loadout_count = 1;     // dual board: one equipped ability (buttons are shared with switching)
        }
        _events.clear();
        _stats = session_stats();
        _state = _config.skip_intro ? session_state::playing : session_state::intro;
        _cause = loss_cause::none;
        _score = _config.start_score;
        _level = clampi(_config.start_level, 1, cfg::max_level);
        _active = 0;
        _intro_timer = _config.skip_intro ? 0 : 100;
        _transition_timer = 0;
        switch(_config.diff)
        {
        case difficulty::beginner: _gravity_percent = cfg::beginner_gravity_percent; break;
        case difficulty::expert: _gravity_percent = cfg::expert_gravity_percent; break;
        case difficulty::standard:
        default: _gravity_percent = cfg::standard_gravity_percent; break;
        }

        _heat = _config.heat ? mini(_config.start_heat, cfg::heat_max - 1) : 0;
        _overheat_timer = 0;
        _overheat_rows = 0;
        _overheat_secured = false;
        _overheat_fail_pending = false;

        _selected = 0;
        for(int& c : _cooldowns)
        {
            c = 0;
        }
        _targeting = -1;
        _target_x = 0;
        _target_y = 0;
        _target_option = 0;
        _targeting_timer = 0;
        _shield_pieces = 0;
        _freeze_frames = 0;
        _chill_pieces = 0;
        _surge_pieces = 0;
        _pending_ability_damage = 0;
        _pending_ability = -1;
        _charged_cells = 0;

        _garbage_count[0] = 0;
        _garbage_count[1] = 0;
        _between_step = 0;
        _between_board = -1;
        _needs_counters = false;
        _last_resolution_from_lock = false;
        _last_cleared = false;
        _switch_pending = false;
        _iron_defense_left = _config.mods.iron_defense;

        _hazard = pending_action();
        _hazard_counter[0] = 0;
        _hazard_counter[1] = 0;
        _hazard_cycle = 0;
        _lava = 0;
        _settle_dir = _config.stage ? _config.stage->settle : gravity_dir::down;
        _reactive_armor_left = _config.mods.reactive_armor;

        _boss_defeated = false;
        _boss_phase = 0;
        _boss_pattern_index = 0;
        _boss_attacks_done = 0;
        _gravity_flip = 0;
        _boss_action = pending_action();
        if(_config.boss)
        {
            _boss_max_hp = maxi(1, int(_config.boss->hp) * clampi(_config.boss_hp_percent, 25, 200) / 100);
            _boss_hp = _boss_max_hp;
            _boss_countdown = _config.boss->phases[0].interval +
                              (_config.diff == difficulty::beginner ? 2 : (_config.diff == difficulty::expert ? -1 : 0));
            if(_boss_countdown < 3)
            {
                _boss_countdown = 3;
            }
        }
        else
        {
            _boss_max_hp = 0;
            _boss_hp = 0;
            _boss_countdown = 0;
        }

        _locks_on_board = 0;
        _cross_window[0] = 0;
        _cross_window[1] = 0;
        _total_locks = 0;
        _danger_state[0] = 0;
        _danger_state[1] = 0;
        _critical_seen = false;

        _hazard_rng.reseed(rng::derive(_config.seed, stream_hazards));
        _boss_rng.reseed(rng::derive(_config.seed, stream_boss));
        _garbage_rng.reseed(rng::derive(_config.seed, stream_garbage));

        special_config specials;
        if(_config.specials)
        {
            specials = _config.stage ? _config.stage->specials : arcade_specials();
        }
        _queue.init(rng::derive(_config.seed, stream_pieces), specials, rng::derive(_config.seed, stream_specials));

        const int cols = _config.dual ? cfg::dual_board_cols : cfg::board_cols;
        const int rows = _config.dual ? cfg::dual_board_rows : cfg::board_rows;
        for(int i = 0; i < board_count(); ++i)
        {
            _players[i].init(i, cols, rows, cfg::board_hidden_rows, &_queue, &_events);
        }

        _setup_stage();

        if(_state == session_state::playing)
        {
            if(! _players[0].spawn_now())
            {
                _lose(loss_cause::top_out);
            }
        }
    }

    void session::_setup_stage()
    {
        _objective_count = 0;
        _secondary = objective_state();
        const stage_def* s = _config.stage;
        if(s)
        {
            for(const objective_def& o : s->objectives)
            {
                if(o.kind != objective_kind::none && _objective_count < 3)
                {
                    objective_state& st = _objectives[_objective_count++];
                    st = objective_state();
                    st.def = o;
                }
            }
            _secondary.def = s->secondary;
            if(s->layout && s->layout_rows)
            {
                for(int b = 0; b < board_count(); ++b)
                {
                    board& grid = _players[b].grid();
                    const int top = grid.rows() - s->layout_rows;
                    for(int r = 0; r < s->layout_rows; ++r)
                    {
                        const char* row = s->layout[r];
                        for(int x = 0; x < grid.cols() && row[x]; ++x)
                        {
                            cell c;
                            switch(row[x])
                            {
                            case '#': c.color = color_garbage; break;
                            case 'x': c.color = color_stone; break;
                            case 'i': c.color = color_garbage; c.coat = coat_kind::ice; c.coat_hp = cfg::ice_hp; break;
                            case 'A': c.color = color_garbage; c.coat = coat_kind::armor; c.coat_hp = cfg::armor_hp; break;
                            case 'b': c.color = color_garbage; c.special = special_kind::bomb; break;
                            case 'f': c.color = color_garbage; c.special = special_kind::fire; break;
                            case 'e': c.color = color_garbage; c.special = special_kind::spark; break;
                            case 'm': c.color = color_garbage; c.special = special_kind::magnet; break;
                            case 'r': c.color = color_garbage; c.special = special_kind::rainbow; break;
                            default: break;
                            }
                            if(c.occupied() && top + r >= 0)
                            {
                                grid.set(x, top + r, c);
                            }
                        }
                    }
                }
            }
        }
        if(_config.boss)
        {
            objective_state& st = _objectives[0];
            st = objective_state();
            st.def.kind = objective_kind::boss_defeat;
            st.def.target = 1;
            _objective_count = 1;
        }
        else if(! s && _config.lines_goal)
        {
            objective_state& st = _objectives[0];
            st = objective_state();
            st.def.kind = objective_kind::lines;
            st.def.target = _config.lines_goal;
            _objective_count = 1;
        }
    }

    // ================================================================== frame update

    void session::update(const input_frame& input)
    {
        switch(_state)
        {
        case session_state::intro:
            if(--_intro_timer <= 0)
            {
                _state = session_state::playing;
                if(! _players[_active].spawn_now())
                {
                    _lose(loss_cause::top_out);
                }
            }
            return;

        case session_state::transition:
            if(--_transition_timer <= 0)
            {
                _state = session_state::playing;
            }
            return;

        case session_state::playing:
            _update_playing(input);
            return;

        case session_state::won:
        case session_state::lost:
        default:
            return;
        }
    }

    int session::gravity_frames() const
    {
        const int index = clampi(_level - 1, 0, 11);
        int frames = cfg::gravity_table[index] * _gravity_percent / 100;
        if(_overheat_timer > 0)
        {
            frames = frames * cfg::overheat_gravity_percent / 100;
        }
        return maxi(frames, cfg::min_gravity_frames);
    }

    effect_mods session::_effect_mods() const
    {
        effect_mods m = _config.mods.effects;
        m.settle_dir = _settle_dir;
        m.disable_new_specials = _surge_pieces > 0;
        return m;
    }

    bool session::_any_resolving() const
    {
        for(int i = 0; i < board_count(); ++i)
        {
            if(_players[i].state() == player_state::resolving)
            {
                return true;
            }
        }
        return false;
    }

    void session::_update_playing(const input_frame& input)
    {
        ++_stats.frames;

        if(_targeting >= 0)
        {
            _update_targeting(input);
        }
        else
        {
            _handle_ability_input(input);
        }
        if(_state != session_state::playing)
        {
            return;
        }

        if(_freeze_frames > 0)
        {
            --_freeze_frames;
        }

        const effect_mods mods = _effect_mods();
        for(int i = 0; i < board_count(); ++i)
        {
            player& p = _players[i];
            const bool is_active = i == _active;
            frame_params fp;
            fp.gravity_frames = _freeze_frames > 0 ? 0 : gravity_frames();
            fp.accept_input = is_active && _targeting < 0;
            fp.frozen = _targeting >= 0 || ! is_active;
            const input_frame in = is_active ? input : input_frame();
            if(p.update(in, fp, mods))
            {
                p.acknowledge_resolution();
                _on_resolution(p);
                if(_state != session_state::playing)
                {
                    return;
                }
            }

            if(p.state() == player_state::topped_out)
            {
                if(_stats.second_chances_used < _config.mods.second_chances)
                {
                    // SECOND CHANCE: the four highest occupied rows are vented and play continues.
                    ++_stats.second_chances_used;
                    board& b = p.grid();
                    uint32_t mask = 0;
                    int removed_rows = 0;
                    for(int y = 0; y < b.rows() && removed_rows < 4; ++y)
                    {
                        if(! b.row_empty(y))
                        {
                            mask |= 1u << y;
                            ++removed_rows;
                        }
                    }
                    b.remove_rows(mask);
                    p.revive();
                    _events.push(event_type::ability_fired, i, cfg::ability_emergency_vent, 0, 0, 99);
                }
                else
                {
                    _lose(loss_cause::top_out);
                    return;
                }
            }

            if(is_active && p.state() == player_state::waiting_spawn && ! p.resolution_finished())
            {
                _between_pieces(p);
                if(_state != session_state::playing)
                {
                    return;
                }
            }
        }

        _update_overheat();
        _update_danger();
        _check_objectives();
    }

    // ================================================================== abilities

    int session::ability_cost(int ability) const
    {
        const int cost = cfg::ability_cost[ability] + _config.mods.ability_cost_delta[ability];
        return maxi(cost, cfg::ability_min_cost);
    }

    int session::ability_cooldown_max(int ability) const
    {
        const int cd = cfg::ability_cooldown[ability] + _config.mods.ability_cooldown_delta[ability];
        return maxi(cd, cfg::ability_min_cooldown);
    }

    bool session::ability_ready(int ability) const
    {
        return _config.loadout_count && _cooldowns[ability] == 0 && _heat >= ability_cost(ability) &&
               _players[_active].state() == player_state::falling;
    }

    piece_type session::shape_option(int option) const
    {
        const piece_type current = _players[_active].active().type;
        int n = 0;
        for(int t = 1; t <= 7; ++t)
        {
            if(piece_type(t) == current)
            {
                continue;
            }
            if(n == option)
            {
                return piece_type(t);
            }
            ++n;
        }
        return piece_type::none;
    }

    bool session::shape_option_legal(int option) const
    {
        player& p = const_cast<player&>(_players[_active]);
        return p.transform_active(shape_option(option), false);
    }

    bool session::shape_option_preview(int option, piece_state& out) const
    {
        player& p = const_cast<player&>(_players[_active]);
        return p.transform_active(shape_option(option), false, &out);
    }

    void session::_handle_ability_input(const input_frame& input)
    {
        if(input.hit(key_select))
        {
            if(_config.dual)
            {
                _switch_board(false);
            }
            else if(_config.loadout_count > 1)
            {
                _selected = (_selected + 1) % _config.loadout_count;
                _events.push(event_type::ability_selected, _active, _config.loadout[_selected]);
            }
        }
        if(input.hit(key_r) && _config.loadout_count)
        {
            _activate(_config.loadout[_selected]);
        }
    }

    bool session::_activate(int ability)
    {
        player& p = _players[_active];
        if(p.state() != player_state::falling)
        {
            _events.push(event_type::ability_denied, _active, ability, 3);
            return false;
        }
        if(_cooldowns[ability] > 0)
        {
            _events.push(event_type::ability_denied, _active, ability, 2);
            return false;
        }
        const int cost = ability_cost(ability);
        if(_heat < cost)
        {
            _events.push(event_type::ability_denied, _active, ability, 1);
            return false;
        }

        const board& b = p.grid();
        switch(ability)
        {
        case cfg::ability_time_freeze:
            _spend_heat(cost);
            _cooldowns[ability] = ability_cooldown_max(ability);
            _freeze_frames = mini(cfg::time_freeze_frames + _config.mods.freeze_bonus_frames, cfg::time_freeze_max_frames);
            ++_stats.abilities_used;
            ++_stats.ability_uses[ability];
            _events.push(event_type::ability_fired, _active, ability);
            return true;

        case cfg::ability_shield_matrix:
            _spend_heat(cost);
            _cooldowns[ability] = ability_cooldown_max(ability);
            _shield_pieces = cfg::shield_duration_pieces;
            ++_stats.abilities_used;
            ++_stats.ability_uses[ability];
            _events.push(event_type::shield_up, _active);
            _events.push(event_type::ability_fired, _active, ability);
            return true;

        case cfg::ability_emergency_vent:
            if(b.empty())
            {
                _events.push(event_type::ability_denied, _active, ability, 3);
                return false;
            }
            _spend_heat(cost);
            _cooldowns[ability] = ability_cooldown_max(ability);
            ++_stats.abilities_used;
            ++_stats.ability_uses[ability];
            _fire_vent();
            return true;

        default:
            break;
        }

        // Targeted abilities: enter the targeting state (piece frozen, no cost until confirmed).
        const piece_state& piece = p.active();
        const int center = piece.x + (box_size(piece.type) == 4 ? 2 : 1);
        _target_x = clampi(center, 0, b.cols() - 1);
        _target_y = b.rows() - 1;
        for(int y = b.hidden(); y < b.rows(); ++y)
        {
            if(b.at(_target_x, y).occupied())
            {
                _target_y = y;
                break;
            }
        }
        _target_option = 0;
        if(ability == cfg::ability_chain_spark)
        {
            bool any = false;
            for(int y = b.hidden(); y < b.rows() && ! any; ++y)
            {
                for(int x = 0; x < b.cols(); ++x)
                {
                    const cell& c = b.at(x, y);
                    if(c.occupied() && c.color != color_stone && ! (c.flags & flag_was_charged))
                    {
                        any = true;
                        break;
                    }
                }
            }
            if(! any)
            {
                _events.push(event_type::ability_denied, _active, ability, 3);
                return false;
            }
        }
        if(ability == cfg::ability_shape_shift)
        {
            bool any = false;
            for(int o = 0; o < 6; ++o)
            {
                if(shape_option_legal(o))
                {
                    _target_option = o;
                    any = true;
                    break;
                }
            }
            if(! any)
            {
                _events.push(event_type::ability_denied, _active, ability, 3);
                return false;
            }
        }
        _targeting = ability;
        _targeting_timer = cfg::targeting_timeout_frames;
        _events.push(event_type::ability_targeting, _active, ability);
        return true;
    }

    void session::_update_targeting(const input_frame& input)
    {
        if(--_targeting_timer <= 0)
        {
            _cancel_targeting();
            return;
        }
        if(input.hit(key_b))
        {
            _cancel_targeting();
            return;
        }
        if(input.hit(key_a) || input.hit(key_r))
        {
            _confirm_target();
            return;
        }
        const board& b = _players[_active].grid();
        const int dx = (input.hit(key_right) ? 1 : 0) - (input.hit(key_left) ? 1 : 0);
        const int dy = (input.hit(key_down) ? 1 : 0) - (input.hit(key_up) ? 1 : 0);
        switch(_targeting)
        {
        case cfg::ability_laser:
            _target_x = clampi(_target_x + dx, 0, b.cols() - 1);
            break;

        case cfg::ability_nuclear_burst:
        case cfg::ability_chain_spark:
            _target_x = clampi(_target_x + dx, 0, b.cols() - 1);
            _target_y = clampi(_target_y + dy, b.hidden(), b.rows() - 1);
            break;

        case cfg::ability_gravity_pulse:
            if(input.hit(key_down)) { _target_option = 0; }
            else if(input.hit(key_left)) { _target_option = 1; }
            else if(input.hit(key_right)) { _target_option = 2; }
            else if(input.hit(key_up)) { _target_option = 3; }
            break;

        case cfg::ability_shape_shift:
            if(dx)
            {
                _target_option = (_target_option + dx + 6) % 6;
            }
            break;

        default:
            break;
        }
    }

    void session::_cancel_targeting()
    {
        _events.push(event_type::ability_cancelled, _active, _targeting);
        _targeting = -1;
    }

    void session::_confirm_target()
    {
        const int ability = _targeting;
        const int cost = ability_cost(ability);
        if(_heat < cost || _cooldowns[ability] > 0 || _players[_active].state() != player_state::falling)
        {
            _events.push(event_type::ability_denied, _active, ability, 1);
            _targeting = -1;
            return;
        }
        board& b = _players[_active].grid();
        switch(ability)
        {
        case cfg::ability_chain_spark:
        {
            const cell& c = b.at(_target_x, _target_y);
            int charged = 0;
            for(int y = 0; y < b.rows(); ++y)
            {
                for(int x = 0; x < b.cols(); ++x)
                {
                    if(b.at(x, y).flags & flag_charged)
                    {
                        ++charged;
                    }
                }
            }
            if(c.empty() || c.color == color_stone || (c.flags & flag_was_charged) ||
               charged >= cfg::chain_spark_max_charged)
            {
                _events.push(event_type::ability_denied, _active, ability, 3);
                return;     // stay in targeting: pick another cell
            }
            break;
        }

        case cfg::ability_shape_shift:
            if(! shape_option_legal(_target_option))
            {
                _events.push(event_type::ability_denied, _active, ability, 3);
                return;
            }
            break;

        default:
            break;
        }

        _spend_heat(cost);
        _cooldowns[ability] = ability_cooldown_max(ability);
        ++_stats.abilities_used;
        ++_stats.ability_uses[ability];
        _targeting = -1;

        switch(ability)
        {
        case cfg::ability_laser:
            _fire_laser(_target_x);
            break;

        case cfg::ability_nuclear_burst:
            _fire_nuke(_target_x, _target_y);
            break;

        case cfg::ability_chain_spark:
            _fire_chain_spark(_target_x, _target_y);
            break;

        case cfg::ability_gravity_pulse:
        {
            static constexpr gravity_dir dirs[4] = { gravity_dir::down, gravity_dir::left, gravity_dir::right,
                                                     gravity_dir::up };
            _fire_gravity(dirs[_target_option & 3]);
            break;
        }

        case cfg::ability_shape_shift:
            _fire_shape_shift(_target_option);
            break;

        default:
            break;
        }
    }

    void session::_fire_laser(int column)
    {
        player& p = _players[_active];
        board& b = p.grid();
        resolver& r = p.resolution();
        const effect_mods m = _effect_mods();
        r.begin_external(m);
        int hits = 0;
        for(int y = 0; y < b.rows() && hits < cfg::laser_max_hits; ++y)
        {
            if(b.at(column, y).occupied())
            {
                r.hit(b, column, y, 1, cause_laser, _events, _active);
                ++hits;
            }
        }
        p.start_external_resolution(m);
        _pending_ability = cfg::ability_laser;
        _pending_ability_damage = cfg::boss_ability_damage_laser;
        _events.push(event_type::ability_fired, _active, cfg::ability_laser, column, 0, hits);
    }

    void session::_fire_nuke(int cx, int cy)
    {
        player& p = _players[_active];
        board& b = p.grid();
        resolver& r = p.resolution();
        const effect_mods m = _effect_mods();
        r.begin_external(m);
        const int rad = cfg::nuke_radius;
        for(int dy = -rad; dy <= rad; ++dy)
        {
            for(int dx = -rad; dx <= rad; ++dx)
            {
                r.hit(b, cx + dx, cy + dy, 2, cause_nuke, _events, _active);
            }
        }
        p.start_external_resolution(m);
        _pending_ability = cfg::ability_nuclear_burst;
        _pending_ability_damage = cfg::boss_ability_damage_nuke;
        _events.push(event_type::ability_fired, _active, cfg::ability_nuclear_burst, cx, cy);
    }

    void session::debug_fill_stress(int board_index)
    {
        player& p = _players[board_index >= 0 && board_index < board_count() ? board_index : 0];
        board& b = p.grid();
        const int cols = b.cols();
        for(int y = b.hidden() + 4; y < b.rows(); ++y)
        {
            const int gap = (y * 3) % cols;     // one gap per row: nothing clears by itself
            for(int x = 0; x < cols; ++x)
            {
                cell c;
                if(x != gap)
                {
                    c.color = color_garbage;
                    const int k = (x + y * 2) % 6;
                    c.special = k == 0 ? special_kind::bomb : (k == 2 ? special_kind::fire :
                                                               (k == 4 ? special_kind::spark : special_kind::none));
                }
                b.set(x, y, c);
            }
        }
    }

    void session::debug_detonate(int board_index)
    {
        player& p = _players[board_index >= 0 && board_index < board_count() ? board_index : 0];
        if(p.state() != player_state::falling)
        {
            return;
        }
        board& b = p.grid();
        resolver& r = p.resolution();
        const effect_mods m = _effect_mods();
        r.begin_external(m);
        const int y = b.rows() - 1;
        for(int x = 0; x < b.cols(); ++x)
        {
            if(b.at(x, y).special != special_kind::none)
            {
                r.hit(b, x, y, 2, cause_nuke, _events, p.index());
                break;
            }
        }
        p.start_external_resolution(m);
    }

    void session::_fire_vent()
    {
        player& p = _players[_active];
        board& b = p.grid();
        resolver& r = p.resolution();
        const effect_mods m = _effect_mods();
        r.begin_external(m);
        const int budget = mini(cfg::vent_cells + _config.mods.vent_bonus_cells, cfg::vent_cells_max);
        int removed = 0;
        // Target rule: the highest occupied cells, scanning rows top to bottom, left to right.
        for(int y = 0; y < b.rows() && removed < budget; ++y)
        {
            for(int x = 0; x < b.cols() && removed < budget; ++x)
            {
                if(b.at(x, y).occupied())
                {
                    r.remove(b, x, y, cause_vent, _events, _active);
                    ++removed;
                }
            }
        }
        p.start_external_resolution(m);
        _pending_ability = cfg::ability_emergency_vent;
        _pending_ability_damage = 0;
        _events.push(event_type::ability_fired, _active, cfg::ability_emergency_vent, 0, 0, removed);
    }

    void session::_fire_chain_spark(int x, int y)
    {
        board& b = _players[_active].grid();
        cell& c = b.attributes(x, y);
        c.flags = uint8_t(c.flags | flag_charged | flag_was_charged);
        _events.push(event_type::ability_fired, _active, cfg::ability_chain_spark, x, y);
    }

    void session::_fire_gravity(gravity_dir dir)
    {
        player& p = _players[_active];
        p.start_compaction(dir, _effect_mods());
        _pending_ability = cfg::ability_gravity_pulse;
        _pending_ability_damage = 0;
        ++_stats.gravity_uses;
        _events.push(event_type::ability_fired, _active, cfg::ability_gravity_pulse, int(dir));
    }

    bool session::_fire_shape_shift(int option)
    {
        const bool ok = _players[_active].transform_active(shape_option(option));
        _events.push(event_type::ability_fired, _active, cfg::ability_shape_shift, int(shape_option(option)));
        return ok;
    }

    // ================================================================== resolution scoring

    void session::_on_resolution(player& p)
    {
        const resolution_result& r = p.resolution().result();
        const bool from_lock = p.lock_started_resolution();
        const lock_info& li = p.last_lock();
        const int lv = _level;
        const int rows = r.first_rows;
        uint32_t points = 0;
        int heat_gain = 0;
        int line_base = 0;
        bool difficult = false;
        twist_kind twist = twist_kind::none;

        if(from_lock)
        {
            points += uint32_t(li.soft_drop_rows * cfg::soft_drop_points + li.hard_drop_rows * cfg::hard_drop_points);
            twist = li.twist;
            if(twist == twist_kind::full)
            {
                line_base = cfg::twist_points[mini(rows, 3)];
                if(rows > 0)
                {
                    difficult = true;
                    ++_stats.twists;
                    _bump_objective(objective_kind::twists, 1);
                }
            }
            else if(twist == twist_kind::mini)
            {
                line_base = (rows ? cfg::line_points[mini(rows, 4)] : 0) + cfg::twist_mini_points;
            }
            else
            {
                line_base = cfg::line_points[mini(rows, 4)];
            }
            if(rows >= 4)
            {
                difficult = true;
            }
            if(rows > 0)
            {
                if(difficult)
                {
                    if(p.b2b > 0)
                    {
                        line_base = line_base * cfg::b2b_percent / 100;
                        heat_gain += cfg::b2b_heat;
                        _events.push(event_type::b2b, p.index(), p.b2b + 1);
                    }
                    ++p.b2b;
                }
                else
                {
                    p.b2b = 0;
                }
                ++p.combo;
                if(p.combo >= 2)
                {
                    const int index = mini(p.combo - 1, cfg::combo_index_cap);
                    points += uint32_t(cfg::combo_points * index * lv);
                    heat_gain += _config.mods.combo_battery_heat;
                    _events.push(event_type::combo, p.index(), index);
                }
                if(p.combo - 1 > _stats.max_combo)
                {
                    _stats.max_combo = uint16_t(p.combo - 1);
                }
                if(rows >= 3 || twist == twist_kind::full)
                {
                    line_base = line_base * _config.mods.precision_percent / 100;
                }
                switch(rows)
                {
                case 1: ++_stats.singles; break;
                case 2: ++_stats.doubles; break;
                case 3: ++_stats.triples; break;
                default: ++_stats.quads; _bump_objective(objective_kind::quads, 1); break;
                }
            }
            else
            {
                p.combo = 0;
                if(twist != twist_kind::none)
                {
                    _events.push(event_type::twist_no_lines, p.index(), twist == twist_kind::mini ? 1 : 0);
                }
            }
        }
        else
        {
            line_base = cfg::line_points[mini(rows, 4)];
        }

        // Cascade line waves score as their own clears.
        for(int w = 1; w < mini(r.line_waves, 8); ++w)
        {
            line_base += cfg::line_points[mini(int(r.line_wave_rows[w]), 4)];
        }

        uint32_t line_total = uint32_t(line_base * lv);
        if(_overheat_timer > 0)
        {
            line_total = line_total * cfg::overheat_score_percent / 100;
        }
        points += line_total;
        if(r.waves >= 2)
        {
            points += uint32_t(cfg::chain_points[mini(r.waves, 8)] * lv * _config.mods.chain_score_percent / 100);
            ++_stats.chains2;
            if(r.waves >= 5)
            {
                ++_stats.chains5;
            }
            if(r.waves > _stats.max_chain)
            {
                _stats.max_chain = uint16_t(r.waves);
            }
            for(int i = 0; i < _objective_count; ++i)
            {
                objective_state& o = _objectives[i];
                if(o.def.kind == objective_kind::chains && r.waves >= maxi(int(o.def.param), 2))
                {
                    _bump_objective(objective_kind::chains, 1, o.def.param);
                    break;
                }
            }
        }
        points += uint32_t(r.special_points);
        if(r.perfect_clear)
        {
            points += uint32_t(cfg::perfect_clear_points * lv);
            ++_stats.perfect_clears;
        }
        if(_config.mods.gambit_percent > 100)
        {
            const board& b = p.grid();
            const int visible = b.rows() - b.hidden();
            if(b.stack_height() * 10 >= visible * 7)
            {
                points = points * uint32_t(_config.mods.gambit_percent) / 100;
            }
        }
        if(points)
        {
            _add_score(points, li.lowest_row, rows);
        }

        // Heat.
        if(rows > 0)
        {
            heat_gain += cfg::line_heat[mini(rows, 4)];
        }
        if(twist == twist_kind::full && rows > 0)
        {
            heat_gain += cfg::twist_heat_bonus;
        }
        if(r.waves > 1)
        {
            heat_gain += mini((r.waves - 1) * cfg::chain_wave_heat, cfg::chain_heat_cap);
        }
        heat_gain += r.magnet_fizzles * cfg::magnet_fizzle_heat;
        if(heat_gain > 0)
        {
            _gain_heat(heat_gain);
        }

        // Statistics and objectives.
        const int total_rows = r.rows_total;
        _last_cleared = total_rows > 0;
        _stats.lines = uint16_t(_stats.lines + total_rows);
        _stats.bombs = uint16_t(_stats.bombs + r.triggers[int(trigger_kind::bomb)]);
        _stats.fires = uint16_t(_stats.fires + r.triggers[int(trigger_kind::fire)]);
        _stats.sparks = uint16_t(_stats.sparks + r.triggers[int(trigger_kind::spark)]);
        _stats.magnets = uint16_t(_stats.magnets + r.triggers[int(trigger_kind::magnet)]);
        _stats.rainbows = uint16_t(_stats.rainbows + r.triggers[int(trigger_kind::rainbow)]);
        _stats.magnet_moves = uint16_t(_stats.magnet_moves + r.magnet_moves);
        _stats.ice_broken = uint16_t(_stats.ice_broken + r.ice_broken);
        _stats.armor_broken = uint16_t(_stats.armor_broken + r.armor_broken);
        _stats.specials_destroyed = uint16_t(_stats.specials_destroyed + r.destroyed);
        _bump_objective(objective_kind::lines, total_rows);
        _bump_objective(objective_kind::bombs, r.triggers[int(trigger_kind::bomb)]);
        _bump_objective(objective_kind::sparks, r.triggers[int(trigger_kind::spark)]);
        _bump_objective(objective_kind::fires, r.triggers[int(trigger_kind::fire)]);
        _bump_objective(objective_kind::magnets, r.magnet_moves);
        _bump_objective(objective_kind::ice_broken, r.ice_broken);

        if(_config.mode != game_mode::classic && _lava > 0 && total_rows > 0)
        {
            _lava = maxi(0, _lava - total_rows * 2);
        }

        // Overheat success: two rows during the Overheat window.
        if(_overheat_timer > 0 && total_rows > 0)
        {
            _overheat_rows += total_rows;
            if(! _overheat_secured && _overheat_rows >= cfg::overheat_success_rows)
            {
                _overheat_secured = true;
                ++_stats.overheat_success;
                const uint32_t bonus = uint32_t(cfg::overheat_success_points * lv) *
                                       uint32_t(_config.mods.overheat_bonus_percent) / 100;
                _add_score(bonus);
                _events.push(event_type::overheat_success, p.index());
                _bump_objective(objective_kind::overheat_success, 1);
            }
        }

        // Level progression.
        if(! _config.stage || _config.stage->level_progression)
        {
            const int cap = _config.stage ? _config.stage->level_cap : cfg::max_level;
            const int base = _config.stage ? _config.stage->base_level : _config.start_level;
            const int target = clampi(base + _stats.lines / cfg::lines_per_level, 1, cap);
            if(target > _level)
            {
                _level = target;
                _events.push(event_type::level_up, p.index(), _level);
            }
        }

        // Boss damage (only from real clears / offensive abilities, capped per resolution).
        if(_config.boss && ! _boss_defeated)
        {
            int damage = 0;
            for(int w = 0; w < mini(r.line_waves, 8); ++w)
            {
                damage += cfg::boss_line_damage[mini(int(r.line_wave_rows[w]), 4)];
            }
            if(twist == twist_kind::full && rows > 0)
            {
                damage += cfg::boss_twist_damage_bonus;
            }
            if(r.waves > 1)
            {
                damage += mini((r.waves - 1) * cfg::boss_chain_damage_per_wave, cfg::boss_chain_damage_cap);
            }
            if(! from_lock && _pending_ability_damage)
            {
                damage += _pending_ability_damage;
            }
            if(_overheat_timer > 0)
            {
                damage = damage * cfg::overheat_boss_damage_percent / 100;
            }
            damage = mini(damage, _boss_max_hp * cfg::boss_resolution_damage_cap_percent / 100);
            if(damage > 0)
            {
                _apply_boss_damage(damage);
            }
        }
        if(! from_lock)
        {
            if(_pending_ability == cfg::ability_laser && total_rows > 0)
            {
                ++_stats.laser_clears;
            }
            _pending_ability = -1;
            _pending_ability_damage = 0;
        }

        // Dual board interactions.
        if(_config.dual && total_rows > 0)
        {
            const int me = p.index();
            const int other = 1 - me;
            player& op = _players[other];
            if(_cross_window[other] > 0)
            {
                ++_stats.cross_combos;
                _cross_window[0] = 0;
                _cross_window[1] = 0;
                _add_score(uint32_t(cfg::cross_combo_points * lv));
                _gain_heat(cfg::cross_combo_heat);
                _events.push(event_type::cross_combo, me, _stats.cross_combos);
                _bump_objective(objective_kind::cross_combos, 1);
            }
            else
            {
                _cross_window[me] = cfg::cross_combo_window_locks;
            }
            if(rows >= 2)
            {
                // Two rows on this board: heat plus a defensive buffer for the other board
                // (its next pending garbage entry loses one row).
                _gain_heat(cfg::cross_double_heat);
                if(_garbage_count[other] > 0 && _garbage[other][0].rows > 0)
                {
                    --_garbage[other][0].rows;
                    _events.push(event_type::garbage_cancelled, other, 1);
                }
            }
            if(rows >= 4)
            {
                int cancel = cfg::cross_quad_garbage_cancel;
                for(int i = 0; i < _garbage_count[other] && cancel > 0; ++i)
                {
                    const int c = mini(cancel, _garbage[other][i].rows);
                    _garbage[other][i].rows = int8_t(_garbage[other][i].rows - c);
                    cancel -= c;
                }
                _events.push(event_type::garbage_cancelled, other, cfg::cross_quad_garbage_cancel - cancel);
            }
            if(r.triggers[int(trigger_kind::spark)] > 0)
            {
                // Electrical chain: charge the lowest uncharged spark cell on the other board.
                board& ob = op.grid();
                for(int y = ob.rows() - 1; y >= 0; --y)
                {
                    bool done = false;
                    for(int x = 0; x < ob.cols(); ++x)
                    {
                        cell& c = ob.attributes(x, y);
                        if(c.occupied() && c.special == special_kind::spark && ! (c.flags & flag_was_charged))
                        {
                            c.flags = uint8_t(c.flags | flag_charged | flag_was_charged);
                            done = true;
                            break;
                        }
                    }
                    if(done)
                    {
                        break;
                    }
                }
            }
            // Remove emptied garbage entries.
            for(int b = 0; b < 2; ++b)
            {
                int w = 0;
                for(int i = 0; i < _garbage_count[b]; ++i)
                {
                    if(_garbage[b][i].rows > 0)
                    {
                        _garbage[b][w++] = _garbage[b][i];
                    }
                }
                _garbage_count[b] = w;
            }
        }

        _last_resolution_from_lock = from_lock;
        if(from_lock)
        {
            _needs_counters = true;
            _between_step = 0;
            _between_board = p.index();
        }
        _check_objectives();
    }

    // ================================================================== between pieces

    void session::_between_pieces(player& first)
    {
        player* p = &first;
        if(_between_board != p->index())
        {
            _between_board = p->index();
            _between_step = 1;      // a board that just became active: no lock to count
        }
        for(int guard = 0; guard < 16; ++guard)
        {
            switch(_between_step)
            {
            case 0:
                if(_needs_counters)
                {
                    _needs_counters = false;
                    _advance_counters(*p);
                    if(_state != session_state::playing)
                    {
                        return;
                    }
                }
                ++_between_step;
                break;

            case 1:
                ++_between_step;
                if(_insert_due_garbage(*p))
                {
                    return;     // topped out by garbage
                }
                break;

            case 2:
            {
                ++_between_step;
                if(_run_boss_attack(*p))
                {
                    return;     // a gravity attack started a resolution: continue afterwards
                }
                if(_state != session_state::playing)
                {
                    return;
                }
                break;
            }

            case 3:
            {
                ++_between_step;
                if(_run_hazard(*p))
                {
                    return;
                }
                if(_state != session_state::playing)
                {
                    return;
                }
                break;
            }

            case 4:
                ++_between_step;
                if(_config.dual && (_switch_pending || _locks_on_board >= cfg::dual_forced_switch_locks))
                {
                    _switch_pending = false;
                    const int before = _active;
                    _switch_board(true);
                    if(_active != before)
                    {
                        _between_board = _active;
                        _between_step = 1;
                        player& np = _players[_active];
                        if(np.state() != player_state::waiting_spawn)
                        {
                            _between_step = 0;
                            _between_board = -1;
                            return;     // resumes its paused falling piece
                        }
                        p = &np;
                        continue;
                    }
                }
                break;

            case 5:
            default:
                _between_step = 0;
                _between_board = -1;
                _players[_active].allow_spawn(_last_cleared ? 1 : cfg::spawn_delay);
                return;
            }
        }
    }

    void session::_advance_counters(player& p)
    {
        ++_total_locks;
        ++_stats.pieces;
        _bump_objective(objective_kind::survive_pieces, 1);
        for(int& c : _cooldowns)
        {
            if(c > 0)
            {
                --c;
            }
        }
        if(_shield_pieces > 0 && --_shield_pieces == 0)
        {
            _events.push(event_type::shield_down, _active);
        }
        if(_chill_pieces > 0)
        {
            --_chill_pieces;
        }
        if(_surge_pieces > 0 && --_surge_pieces == 0)
        {
            for(int b = 0; b < board_count(); ++b)
            {
                board& grid = _players[b].grid();
                for(int y = 0; y < grid.rows(); ++y)
                {
                    for(int x = 0; x < grid.cols(); ++x)
                    {
                        cell& c = grid.attributes(x, y);
                        c.flags = uint8_t(c.flags & ~flag_disabled);
                    }
                }
            }
        }
        if(_config.dual)
        {
            ++_locks_on_board;
            for(int& w : _cross_window)
            {
                if(w > 0)
                {
                    --w;
                }
            }
            const int left = cfg::dual_forced_switch_locks - _locks_on_board;
            if(left > 0 && left <= 2 && ! board_sealed(1))
            {
                _events.push(event_type::forced_switch_warn, _active, left);
            }
        }
        for(int i = 0; i < _garbage_count[p.index()]; ++i)
        {
            if(_garbage[p.index()][i].countdown > 0)
            {
                --_garbage[p.index()][i].countdown;
            }
        }

        // Lava pressure rises on locks that clear nothing.
        if(_config.stage)
        {
            for(const hazard_def& h : _config.stage->hazards)
            {
                if(h.kind == hazard_kind::lava_pressure && ! _last_cleared)
                {
                    ++_lava;
                }
            }
        }

        // Boss countdown (pieces locked during Time Freeze do not advance it).
        if(_config.boss && ! _boss_defeated && _freeze_frames <= 0)
        {
            if(_boss_countdown > 0)
            {
                --_boss_countdown;
            }
            const boss_phase_def& phase = _config.boss->phases[_boss_phase];
            if(phase.pattern_length && ! _boss_action.source)
            {
                const attack_def& atk = phase.pattern[_boss_pattern_index % phase.pattern_length];
                int telegraph = maxi(int(atk.telegraph), cfg::boss_min_telegraph_pieces) +
                                (_config.diff == difficulty::beginner ? 1 : 0);
                if(_boss_countdown <= telegraph)
                {
                    _schedule_boss_attack();
                }
            }
            else if(_boss_action.source)
            {
                _boss_action.countdown = int8_t(_boss_countdown);
            }
        }

        // Stage hazards.
        if(_config.stage)
        {
            for(int i = 0; i < 2; ++i)
            {
                const hazard_def& h = _config.stage->hazards[i];
                if(h.kind == hazard_kind::none || h.kind == hazard_kind::lava_pressure || ! h.interval)
                {
                    continue;
                }
                ++_hazard_counter[i];
            }
            if(_hazard.source)
            {
                if(_hazard.countdown > 0)
                {
                    --_hazard.countdown;
                }
            }
            else
            {
                _schedule_hazard();
            }
        }

        // Height statistics / Last Stand.
        for(int b = 0; b < board_count(); ++b)
        {
            const int h = _stack_rows(b);
            if(h > _stats.max_height)
            {
                _stats.max_height = uint8_t(h);
            }
            const int visible = _players[b].grid().rows() - _players[b].grid().hidden();
            if(h >= visible - 2)
            {
                _critical_seen = true;
            }
            else if(_critical_seen && h <= visible / 2)
            {
                _critical_seen = false;
                ++_stats.last_stand;
            }
        }
    }

    int session::_stack_rows(int b) const
    {
        return _players[b].grid().stack_height();
    }

    bool session::danger(int b) const
    {
        const board& g = _players[b].grid();
        return _stack_rows(b) >= (g.rows() - g.hidden()) - 4;
    }

    void session::_update_danger()
    {
        for(int b = 0; b < board_count(); ++b)
        {
            const int d = danger(b) ? 1 : 0;
            if(d != _danger_state[b])
            {
                _danger_state[b] = d;
                _events.push(d ? event_type::danger_on : event_type::danger_off, b);
            }
        }
    }

    // ================================================================== garbage

    void session::_queue_garbage(int b, int rows, int gap, int countdown, coat_kind coat, bool hostile, bool overheat)
    {
        if(rows <= 0 || _garbage_count[b] >= 8)
        {
            return;
        }
        garbage_entry& g = _garbage[b][_garbage_count[b]++];
        g.rows = int8_t(rows);
        g.gap = int8_t(gap);
        g.countdown = int8_t(countdown);
        g.coat = coat;
        g.hostile = hostile;
        g.overheat = overheat;
        _events.push(event_type::garbage_warning, b, rows, b);
    }

    int session::incoming_rows(int b) const
    {
        int total = 0;
        for(int i = 0; i < _garbage_count[b]; ++i)
        {
            total += _garbage[b][i].rows;
        }
        return total;
    }

    int session::overheat_fail_rows() const
    {
        switch(_config.diff)
        {
        case difficulty::beginner: return cfg::overheat_fail_rows_beginner;
        case difficulty::expert: return cfg::overheat_fail_rows_expert;
        case difficulty::standard:
        default: return cfg::overheat_fail_rows_standard;
        }
    }

    bool session::_insert_due_garbage(player& p)
    {
        const int b = p.index();
        int w = 0;
        for(int i = 0; i < _garbage_count[b]; ++i)
        {
            garbage_entry g = _garbage[b][i];
            if(g.countdown > 0 || g.rows <= 0)
            {
                if(g.rows > 0)
                {
                    _garbage[b][w++] = g;
                }
                continue;
            }
            if(g.hostile && _shield_pieces > 0)
            {
                _shield_pieces = 0;
                ++_stats.shield_blocks;
                _events.push(event_type::shield_block, b, int(attack_kind::garbage));
                continue;
            }
            int rows = g.rows;
            if(g.hostile && _iron_defense_left > 0 && rows > 1)
            {
                --_iron_defense_left;
                --rows;
            }
            int8_t gaps[20];
            for(int k = 0; k < rows && k < 20; ++k)
            {
                gaps[k] = g.gap;
            }
            if(g.overheat && _config.diff == difficulty::expert && rows >= 2)
            {
                gaps[1] = int8_t((g.gap + p.grid().cols() / 2) % p.grid().cols());     // more restrictive pattern
            }
            if(g.hostile)
            {
                ++_stats.hostile_attacks_taken;
            }
            _stats.garbage_received = uint16_t(_stats.garbage_received + rows);
            const bool ok = p.grid().push_garbage(rows, gaps, g.coat);
            _events.push(event_type::garbage_inserted, b, rows);
            if(! ok)
            {
                _garbage_count[b] = 0;
                _lose(loss_cause::crushed_by_garbage);
                return true;
            }
        }
        _garbage_count[b] = w;
        return false;
    }

    // ================================================================== bosses and hazards

    void session::_schedule_boss_attack()
    {
        const boss_phase_def& phase = _config.boss->phases[_boss_phase];
        const attack_def& atk = phase.pattern[_boss_pattern_index % phase.pattern_length];
        pending_action& a = _boss_action;
        a = pending_action();
        a.source = 2;
        a.kind = uint8_t(atk.kind);
        a.strength = atk.strength;
        a.param = atk.param;
        a.board = int8_t(_active);
        a.countdown = int8_t(_boss_countdown);
        if(_config.diff == difficulty::beginner && a.strength > 1 &&
           (atk.kind == attack_kind::garbage || atk.kind == attack_kind::punch))
        {
            --a.strength;   // slightly more forgiving hazards
        }
        if(_config.dual && atk.kind == attack_kind::cross_garbage && ! board_sealed(1 - _active))
        {
            a.board = int8_t(1 - _active);
        }
        _prepare_action(a, _players[a.board]);
        _events.push(event_type::boss_telegraph, a.board, a.kind, a.countdown);
    }

    void session::_schedule_hazard()
    {
        if(! _config.stage)
        {
            return;
        }
        for(int i = 0; i < 2; ++i)
        {
            const hazard_def& h = _config.stage->hazards[i];
            if(h.kind == hazard_kind::none)
            {
                continue;
            }
            int due = -1;
            if(h.kind == hazard_kind::lava_pressure)
            {
                const int limit = h.param ? h.param : 8;
                if(_lava >= limit)
                {
                    due = h.telegraph;
                }
            }
            else if(h.interval)
            {
                int interval = h.interval;
                if(_config.mode == game_mode::arcade)
                {
                    // Chaos Arcade: events come one lock sooner per level, down to a floor.
                    interval = maxi(cfg::arcade_event_min_interval, interval - (_level - 1));
                }
                const int telegraph = h.telegraph + (_config.diff == difficulty::beginner ? 1 : 0);
                if(_hazard_counter[i] >= interval - telegraph)
                {
                    due = maxi(interval - _hazard_counter[i], 0);
                }
            }
            if(due < 0)
            {
                continue;
            }
            pending_action& a = _hazard;
            a = pending_action();
            a.source = 1;
            a.kind = uint8_t(h.kind);
            a.strength = h.strength;
            a.param = h.param;
            a.board = int8_t(_active);
            a.countdown = int8_t(maxi(due, 1));
            if(h.kind == hazard_kind::mixed)
            {
                static constexpr hazard_kind cycle[4] = { hazard_kind::freeze_cells, hazard_kind::molten_rain,
                                                          hazard_kind::gravity_shift, hazard_kind::surge };
                // param 1 (Chaos Arcade): six event kinds, including garbage and armor plates.
                static constexpr hazard_kind arcade_cycle[6] = {
                    hazard_kind::garbage_drip, hazard_kind::freeze_cells, hazard_kind::surge, hazard_kind::molten_rain,
                    hazard_kind::armor_plates, hazard_kind::gravity_shift
                };
                a.kind = uint8_t(h.param == 1 ? arcade_cycle[_hazard_cycle % 6] : cycle[_hazard_cycle % 4]);
                if(h.param == 1)
                {
                    a.param = 0;        // arcade gravity events alternate left / right
                }
            }
            _hazard_counter[i] = -a.countdown;     // the period is measured from activation to activation
            _prepare_action(a, _players[a.board]);
            _events.push(event_type::hazard_warning, a.board, a.kind, a.countdown);
            return;
        }
    }

    void session::_prepare_action(pending_action& a, player& p)
    {
        board& b = p.grid();
        const int cols = b.cols();
        int kind_attack = a.source == 2 ? a.kind : 0;
        const hazard_kind hk = a.source == 1 ? hazard_kind(a.kind) : hazard_kind::none;
        rng& r = a.source == 2 ? _boss_rng : _hazard_rng;

        // Garbage gap.
        a.gap = int8_t(r.below(cols));

        // Column targets.
        const bool columns = (a.source == 2 && attack_uses_columns(attack_kind(kind_attack))) ||
                             hk == hazard_kind::molten_rain;
        if(columns)
        {
            int n = a.strength;
            if(attack_kind(kind_attack) == attack_kind::fire_breath || attack_kind(kind_attack) == attack_kind::frozen_wall)
            {
                n = 1;
            }
            n = clampi(n, 1, 4);
            if(attack_kind(kind_attack) == attack_kind::frozen_wall)
            {
                // The Cryo Titan freezes your lowest column (deterministic, learnable).
                int best = 0;
                int best_h = 99;
                for(int x = 0; x < cols; ++x)
                {
                    const int h = b.column_height(x);
                    if(h < best_h)
                    {
                        best_h = h;
                        best = x;
                    }
                }
                a.columns[0] = int8_t(best);
            }
            else
            {
                for(int k = 0; k < n; ++k)
                {
                    int c = r.below(cols);
                    for(int tries = 0; tries < cols; ++tries)
                    {
                        bool used = false;
                        for(int j = 0; j < k; ++j)
                        {
                            if(a.columns[j] == c)
                            {
                                used = true;
                            }
                        }
                        if(! used)
                        {
                            break;
                        }
                        c = (c + 1) % cols;
                    }
                    a.columns[k] = int8_t(c);
                }
            }
        }

        // Cell marks (armor / ice injections and freezing events): surface cells of the tallest columns.
        const bool armor = attack_kind(kind_attack) == attack_kind::armor_inject || hk == hazard_kind::armor_plates;
        const bool ice = attack_kind(kind_attack) == attack_kind::ice_inject || hk == hazard_kind::freeze_cells;
        if(armor || ice)
        {
            int marked = 0;
            const int want = clampi(a.strength, 1, 6);
            bool taken[board::max_cols] = {};
            for(int k = 0; k < want; ++k)
            {
                int best = -1;
                int best_h = 0;
                for(int x = 0; x < cols; ++x)
                {
                    if(taken[x])
                    {
                        continue;
                    }
                    const int h = b.column_height(x);
                    if(h > best_h)
                    {
                        best_h = h;
                        best = x;
                    }
                }
                if(best < 0)
                {
                    break;
                }
                taken[best] = true;
                const int y = b.rows() - best_h;
                cell& c = b.attributes(best, y);
                if(c.occupied() && ! c.coated() && c.color != color_stone)
                {
                    c.flags = uint8_t(c.flags | (armor ? flag_armor_mark : flag_frost_mark));
                    ++marked;
                }
            }
            a.marked = marked > 0;
        }
    }

    bool session::_run_boss_attack(player& p)
    {
        if(! _config.boss || _boss_defeated || ! _boss_action.source || _boss_countdown > 0)
        {
            return false;
        }
        pending_action a = _boss_action;
        _boss_action = pending_action();
        bool started = false;
        player& target = _players[a.board];
        const bool shielded = _shield_pieces > 0 && a.kind != uint8_t(attack_kind::cross_garbage);
        if(shielded)
        {
            _shield_pieces = 0;
            ++_stats.shield_blocks;
            _events.push(event_type::shield_block, a.board, a.kind);
            // Remove telegraph marks.
            board& b = target.grid();
            for(int y = 0; y < b.rows(); ++y)
            {
                for(int x = 0; x < b.cols(); ++x)
                {
                    cell& c = b.attributes(x, y);
                    c.flags = uint8_t(c.flags & ~(flag_armor_mark | flag_frost_mark));
                }
            }
        }
        else
        {
            ++_stats.hostile_attacks_taken;
            _execute_action(a, target, started);
        }
        ++_boss_attacks_done;
        const boss_phase_def& phase = _config.boss->phases[_boss_phase];
        _boss_pattern_index = phase.pattern_length ? (_boss_pattern_index + 1) % phase.pattern_length : 0;
        int interval = phase.interval + (_config.diff == difficulty::beginner ? 2 : 0) -
                       (_config.diff == difficulty::expert ? 1 : 0);
        _boss_countdown = maxi(interval, 3);
        return started && a.board == p.index();
    }

    bool session::_run_hazard(player& p)
    {
        if(! _hazard.source || _hazard.countdown > 0)
        {
            return false;
        }
        pending_action a = _hazard;
        _hazard = pending_action();
        ++_hazard_cycle;
        bool started = false;
        player& target = _players[a.board];
        if(_shield_pieces > 0 || _reactive_armor_left > 0)
        {
            if(_shield_pieces > 0)
            {
                _shield_pieces = 0;
                ++_stats.shield_blocks;
            }
            else
            {
                --_reactive_armor_left;
            }
            _events.push(event_type::shield_block, a.board, 100 + a.kind);
            board& b = target.grid();
            for(int y = 0; y < b.rows(); ++y)
            {
                for(int x = 0; x < b.cols(); ++x)
                {
                    cell& c = b.attributes(x, y);
                    c.flags = uint8_t(c.flags & ~(flag_armor_mark | flag_frost_mark));
                }
            }
            if(hazard_kind(a.kind) == hazard_kind::lava_pressure)
            {
                _lava = 0;
            }
            return false;
        }
        _execute_action(a, target, started);
        return started && a.board == p.index();
    }

    void session::_execute_action(pending_action& a, player& p, bool& started)
    {
        board& b = p.grid();
        const int cols = b.cols();
        attack_kind k = attack_kind::none;
        if(a.source == 2)
        {
            k = attack_kind(a.kind);
        }
        else
        {
            switch(hazard_kind(a.kind))
            {
            case hazard_kind::freeze_cells: k = attack_kind::ice_inject; break;
            case hazard_kind::armor_plates: k = attack_kind::armor_inject; break;
            case hazard_kind::molten_rain: k = attack_kind::molten_rain; break;
            case hazard_kind::surge: k = attack_kind::surge; break;
            case hazard_kind::garbage_drip: k = attack_kind::garbage; break;
            case hazard_kind::lava_pressure: k = attack_kind::garbage; _lava = 0; break;
            case hazard_kind::gravity_shift:
            {
                static constexpr attack_kind cycles[3][4] = {
                    { attack_kind::gravity_left, attack_kind::gravity_right, attack_kind::gravity_left, attack_kind::gravity_right },
                    { attack_kind::gravity_up, attack_kind::gravity_up, attack_kind::gravity_up, attack_kind::gravity_up },
                    { attack_kind::gravity_left, attack_kind::gravity_up, attack_kind::gravity_right, attack_kind::gravity_up },
                };
                k = cycles[a.param % 3][_hazard_cycle % 4];
                break;
            }
            case hazard_kind::settle_cycle:
            {
                static constexpr gravity_dir cycles[3][4] = {
                    { gravity_dir::left, gravity_dir::right, gravity_dir::left, gravity_dir::right },
                    { gravity_dir::up, gravity_dir::down, gravity_dir::up, gravity_dir::down },
                    { gravity_dir::left, gravity_dir::up, gravity_dir::right, gravity_dir::down },
                };
                _settle_dir = cycles[a.param % 3][_hazard_cycle % 4];
                _events.push(event_type::gravity_shift, p.index(), int(_settle_dir), 1);
                _events.push(event_type::hazard_fired, p.index(), a.kind, 0);
                return;
            }
            default: break;
            }
        }

        int affected = 0;
        auto apply = [&](attack_kind kind, int strength)
        {
            switch(kind)
            {
            case attack_kind::garbage:
            case attack_kind::punch:
            case attack_kind::cross_garbage:
            {
                _queue_garbage(p.index(), strength, a.gap, 0, coat_kind::none, true, false);
                if(kind != attack_kind::cross_garbage)
                {
                    _insert_due_garbage(p);
                }
                affected += strength;
                break;
            }

            case attack_kind::armor_inject:
            case attack_kind::ice_inject:
            {
                const uint8_t mark = kind == attack_kind::armor_inject ? flag_armor_mark : flag_frost_mark;
                for(int y = 0; y < b.rows(); ++y)
                {
                    for(int x = 0; x < cols; ++x)
                    {
                        cell& c = b.attributes(x, y);
                        if(! (c.flags & mark))
                        {
                            continue;
                        }
                        c.flags = uint8_t(c.flags & ~mark);
                        if(c.occupied() && ! c.coated() && c.color != color_stone)
                        {
                            c.coat = kind == attack_kind::armor_inject ? coat_kind::armor : coat_kind::ice;
                            c.coat_hp = uint8_t(kind == attack_kind::armor_inject ? cfg::armor_hp : cfg::ice_hp);
                            ++affected;
                        }
                    }
                }
                break;
            }

            case attack_kind::frozen_wall:
            case attack_kind::fire_breath:
            case attack_kind::crystal_shards:
            case attack_kind::molten_rain:
            {
                const int per_column = (kind == attack_kind::frozen_wall || kind == attack_kind::fire_breath) ? strength : 1;
                for(int k2 = 0; k2 < 4; ++k2)
                {
                    const int x = a.columns[k2];
                    if(x < 0 || x >= cols)
                    {
                        continue;
                    }
                    for(int n = 0; n < per_column; ++n)
                    {
                        const int top = b.rows() - b.column_height(x) - 1;
                        if(top < b.hidden())
                        {
                            break;      // never builds into the spawn area
                        }
                        cell c;
                        c.color = color_garbage;
                        if(kind == attack_kind::frozen_wall || kind == attack_kind::crystal_shards)
                        {
                            c.coat = coat_kind::ice;
                            c.coat_hp = cfg::ice_hp;
                        }
                        b.set(x, top, c);
                        ++affected;
                    }
                }
                break;
            }

            case attack_kind::chill:
                _chill_pieces = maxi(_chill_pieces, strength * 3);
                affected = strength;
                break;

            case attack_kind::heat_drain:
            {
                const int drained = mini(_heat, strength * 5);
                _heat -= drained;
                _events.push(event_type::heat_changed, p.index(), _heat, -drained);
                affected = drained;
                break;
            }

            case attack_kind::surge:
            {
                _surge_pieces = maxi(_surge_pieces, strength);
                for(int bi = 0; bi < board_count(); ++bi)
                {
                    board& g = _players[bi].grid();
                    for(int y = 0; y < g.rows(); ++y)
                    {
                        for(int x = 0; x < g.cols(); ++x)
                        {
                            cell& c = g.attributes(x, y);
                            if(c.occupied() && c.is_special())
                            {
                                c.flags = uint8_t(c.flags | flag_disabled);
                                ++affected;
                            }
                        }
                    }
                }
                break;
            }

            case attack_kind::forced_switch:
                if(_config.dual)
                {
                    _switch_pending = true;
                }
                break;

            case attack_kind::gravity_left:
            case attack_kind::gravity_right:
            case attack_kind::gravity_up:
            case attack_kind::gravity_flip:
            {
                gravity_dir dir = gravity_dir::left;
                if(kind == attack_kind::gravity_right) { dir = gravity_dir::right; }
                else if(kind == attack_kind::gravity_up) { dir = gravity_dir::up; }
                else if(kind == attack_kind::gravity_flip)
                {
                    dir = (_gravity_flip++ & 1) ? gravity_dir::right : gravity_dir::left;
                }
                if(! started)
                {
                    p.start_compaction(dir, _effect_mods());
                    started = true;
                }
                break;
            }

            default:
                break;
            }
        };

        if(k == attack_kind::core_combo)
        {
            const int combo = a.param % 4;
            apply(core_combos[combo][0], maxi(1, a.strength));
            apply(core_combos[combo][1], maxi(1, a.strength));
        }
        else
        {
            apply(k, a.strength);
        }

        if(a.source == 2)
        {
            _events.push(event_type::boss_attack, p.index(), a.kind, affected);
        }
        else
        {
            _events.push(event_type::hazard_fired, p.index(), a.kind, affected);
        }
    }

    void session::_apply_boss_damage(int damage)
    {
        if(_boss_defeated || damage <= 0)
        {
            return;
        }
        _boss_hp = maxi(0, _boss_hp - damage);
        _stats.boss_damage = uint16_t(_stats.boss_damage + damage);
        _events.push(event_type::boss_damage, 0, damage, _boss_hp);
        if(_boss_hp == 0)
        {
            _boss_defeated = true;
            _boss_action = pending_action();
            _events.push(event_type::boss_defeated);
            _bump_objective(objective_kind::boss_defeat, 1);
            return;
        }
        _update_boss_phase();
    }

    void session::_update_boss_phase()
    {
        const boss_def& boss = *_config.boss;
        int phase = _boss_phase;
        while(phase + 1 < boss.phase_count &&
              _boss_hp * 100 <= _boss_max_hp * boss.phases[phase + 1].hp_percent)
        {
            ++phase;
        }
        if(phase != _boss_phase)
        {
            _boss_phase = phase;
            _boss_pattern_index = 0;
            // Telegraphs of the old phase are cancelled visibly; the new phase starts its own schedule.
            for(int bi = 0; bi < board_count(); ++bi)
            {
                board& b = _players[bi].grid();
                for(int y = 0; y < b.rows(); ++y)
                {
                    for(int x = 0; x < b.cols(); ++x)
                    {
                        cell& c = b.attributes(x, y);
                        c.flags = uint8_t(c.flags & ~(flag_armor_mark | flag_frost_mark));
                    }
                }
            }
            _boss_action = pending_action();
            _boss_countdown = maxi(int(boss.phases[phase].interval), 3);
            _locks_on_board = 0;    // a newly unsealed board never forces an immediate switch
            _state = session_state::transition;
            _transition_timer = cfg::boss_phase_transition_frames;
            _events.push(event_type::boss_phase, 0, phase);
        }
    }

    void session::debug_set_boss_hp(int hp)
    {
        if(! _config.boss || _state == session_state::won || _state == session_state::lost)
        {
            return;     // a finished session is never brought back
        }
        _boss_hp = clampi(hp, 1, _boss_max_hp);
        _update_boss_phase();
    }

    // ================================================================== heat / Overheat

    void session::_gain_heat(int amount)
    {
        if(amount <= 0)
        {
            return;
        }
        const int percent = clampi(_config.mods.heat_gain_percent, 50, 150);
        amount = amount * percent / 100;
        if(_chill_pieces > 0)
        {
            amount = amount * cfg::chill_heat_percent / 100;
        }
        _gain_heat_raw(amount);
    }

    void session::_gain_heat_raw(int amount)
    {
        if(! _config.heat || amount <= 0)
        {
            return;
        }
        const int cap = _overheat_timer > 0 ? cfg::overheat_heat_keep_cap : cfg::heat_max;
        const int before = _heat;
        _heat = mini(_heat + amount, cap);
        if(_heat != before)
        {
            _events.push(event_type::heat_changed, _active, _heat, _heat - before);
        }
        if(_overheat_timer <= 0 && _heat >= cfg::heat_max)
        {
            _heat = 0;      // the full meter is consumed
            _overheat_timer = cfg::overheat_frames;
            _overheat_rows = 0;
            _overheat_secured = false;
            ++_stats.overheats;
            _events.push(event_type::overheat_start, _active);
        }
    }

    void session::_spend_heat(int amount)
    {
        _heat = maxi(0, _heat - amount);
        _events.push(event_type::heat_changed, _active, _heat, -amount);
    }

    void session::_update_overheat()
    {
        if(_overheat_timer <= 0 || _any_resolving())
        {
            return;
        }
        --_overheat_timer;
        if(_overheat_timer == cfg::overheat_garbage_warning_frames && ! _overheat_secured)
        {
            _events.push(event_type::overheat_warning, _active, overheat_fail_rows());
        }
        if(_overheat_timer == 0)
        {
            _end_overheat();
        }
    }

    void session::_end_overheat()
    {
        if(! _overheat_secured)
        {
            ++_stats.overheat_fail;
            const int rows = overheat_fail_rows();
            _queue_garbage(_active, rows, _garbage_rng.below(_players[_active].grid().cols()), 0,
                           coat_kind::none, false, true);
            _events.push(event_type::overheat_fail, _active, rows);
        }
        _events.push(event_type::overheat_end, _active);
    }

    // ================================================================== objectives, win / loss

    void session::_bump_objective(objective_kind kind, int amount, int param)
    {
        if(amount <= 0)
        {
            return;
        }
        for(int i = 0; i < _objective_count; ++i)
        {
            objective_state& o = _objectives[i];
            if(o.def.kind != kind || o.done)
            {
                continue;
            }
            if(kind == objective_kind::chains && param != o.def.param)
            {
                continue;
            }
            o.value = uint16_t(mini(int(o.value) + amount, 9999));
            _events.push(event_type::objective_progress, _active, i, o.value, o.def.target);
            if(o.value >= o.def.target)
            {
                o.done = true;
                _events.push(event_type::objective_complete, _active, i);
            }
        }
    }

    bool session::objectives_complete() const
    {
        if(! _objective_count)
        {
            return false;
        }
        for(int i = 0; i < _objective_count; ++i)
        {
            if(! _objectives[i].done)
            {
                return false;
            }
        }
        return true;
    }

    void session::_check_objectives()
    {
        if(_state != session_state::playing && _state != session_state::transition)
        {
            return;
        }
        if(objectives_complete() && ! _any_resolving())
        {
            _win();
        }
    }

    void session::_win()
    {
        if(_state == session_state::won || _state == session_state::lost)
        {
            return;
        }
        // Secondary objective evaluation.
        objective_state& s = _secondary;
        switch(s.def.kind)
        {
        case objective_kind::max_height:
            s.value = _stats.max_height;
            s.done = _stats.max_height <= s.def.target;
            break;

        case objective_kind::no_garbage:
            s.value = _stats.garbage_received;
            s.done = _stats.garbage_received == 0;
            break;

        case objective_kind::time_limit:
            s.value = uint16_t(_stats.frames / 60);
            s.done = _stats.frames / 60 <= s.def.target;
            break;

        case objective_kind::none:
            break;

        default:
            break;
        }
        _state = session_state::won;
        _targeting = -1;
        _events.push(event_type::stage_clear, _active);
    }

    void session::_lose(loss_cause cause)
    {
        if(_state == session_state::won || _state == session_state::lost)
        {
            return;
        }
        _cause = cause;
        _state = session_state::lost;
        _targeting = -1;
        _events.push(event_type::game_over, _active, int(cause));
    }

    void session::_add_score(uint32_t points, int x, int y)
    {
        const uint32_t before = _score;
        _score += points;
        if(_score < before)
        {
            _score = 0xFFFFFFFFu;   // saturate, never wrap
        }
        _events.push(event_type::score, _active, int(points & 0xFFFF), int(points >> 16), x, y);
    }

    // ================================================================== dual board

    bool session::board_sealed(int board) const
    {
        if(! _config.dual || board == 0 || ! _config.boss || ! _config.boss->dual_phase)
        {
            return false;
        }
        return _boss_phase + 1 < _config.boss->dual_phase && ! _boss_defeated;
    }

    void session::_switch_board(bool forced)
    {
        if(! _config.dual || board_sealed(1))
        {
            return;
        }
        if(_any_resolving())
        {
            if(forced)
            {
                _switch_pending = true;
            }
            return;
        }
        const int other = 1 - _active;
        if(_players[other].state() == player_state::topped_out)
        {
            return;
        }
        if(! forced && _players[_active].state() != player_state::falling &&
           _players[_active].state() != player_state::waiting_spawn)
        {
            return;
        }
        _active = other;
        _locks_on_board = 0;
        _events.push(event_type::board_switched, other, other, forced ? 1 : 0);
    }

    // ================================================================== invariants

    int session::validate() const
    {
        for(int b = 0; b < board_count(); ++b)
        {
            const int v = _players[b].grid().validate();
            if(v)
            {
                return 100 + v;
            }
            if(_players[b].state() == player_state::falling && ! _players[b].grid().fits(_players[b].active()))
            {
                return 200;     // active piece overlaps settled cells
            }
            if(_garbage_count[b] < 0 || _garbage_count[b] > 8)
            {
                return 201;
            }
        }
        if(_heat < 0 || _heat > cfg::heat_max)
        {
            return 300;
        }
        if(_overheat_timer > 0 && _heat > cfg::overheat_heat_keep_cap)
        {
            return 301;
        }
        for(int c : _cooldowns)
        {
            if(c < 0)
            {
                return 302;
            }
        }
        if(_boss_hp < 0 || _boss_hp > _boss_max_hp)
        {
            return 400;
        }
        return 0;
    }
}
