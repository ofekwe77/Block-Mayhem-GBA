#include "bm_randomizer.h"

namespace bm
{
    void bag_randomizer::seed(uint32_t s)
    {
        _rng.reseed(s);
        _index = 7;
    }

    piece_type bag_randomizer::next()
    {
        if(_index >= 7)
        {
            _refill();
        }
        return _bag[_index++];
    }

    void bag_randomizer::_refill()
    {
        for(int i = 0; i < 7; ++i)
        {
            _bag[i] = piece_type(i + 1);
        }
        // Fisher-Yates shuffle.
        for(int i = 6; i > 0; --i)
        {
            const int j = _rng.below(i + 1);
            const piece_type t = _bag[i];
            _bag[i] = _bag[j];
            _bag[j] = t;
        }
        _index = 0;
    }

    void special_distributor::configure(const special_config& config, uint32_t seed)
    {
        _config = config;
        _rng.reseed(seed);
        _generated = 0;
        for(int i = 0; i < special_kind_count; ++i)
        {
            _since[i] = 0;
            _given[i] = 0;
        }
        _reset_countdown();
    }

    void special_distributor::reconfigure(const special_config& config)
    {
        _config = config;
        _reset_countdown();
    }

    void special_distributor::_reset_countdown()
    {
        if(_config.interval_min <= 0)
        {
            _countdown = -1;
            return;
        }
        int high = _config.interval_max < _config.interval_min ? _config.interval_min : _config.interval_max;
        _countdown = _rng.range(_config.interval_min, high);
    }

    special_kind special_distributor::_roll_kind()
    {
        int total = 0;
        for(uint8_t w : _config.weights)
        {
            total += w;
        }
        if(total <= 0)
        {
            return special_kind::bomb;
        }
        int r = _rng.below(total);
        for(int i = 0; i < 5; ++i)
        {
            if(r < _config.weights[i])
            {
                return special_kind(i + 1);
            }
            r -= _config.weights[i];
        }
        return special_kind::bomb;
    }

    void special_distributor::decorate(queued_piece& piece)
    {
        ++_generated;
        for(int i = 1; i < special_kind_count; ++i)
        {
            ++_since[i];
        }

        special_kind chosen = special_kind::none;

        // Guarantees first: a guaranteed kind that is due is forced onto this piece.
        for(const special_guarantee& g : _config.guarantees)
        {
            if(g.kind == special_kind::none || g.every == 0)
            {
                continue;
            }
            if(g.max_count && _given[int(g.kind)] >= g.max_count)
            {
                continue;
            }
            if(_since[int(g.kind)] >= g.every)
            {
                chosen = g.kind;
                break;
            }
        }

        if(chosen == special_kind::none && _countdown >= 0)
        {
            if(--_countdown <= 0)
            {
                chosen = _roll_kind();
                _reset_countdown();
            }
        }

        if(chosen == special_kind::none)
        {
            piece.special = special_kind::none;
            piece.special_mino = -1;
            return;
        }

        piece.special = chosen;
        piece.special_mino = int8_t(_rng.below(4));
        _since[int(chosen)] = 0;
        ++_given[int(chosen)];
    }
}
