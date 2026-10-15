/*
 * BLOCK MAYHEM - deterministic random number generation.
 *
 * Gameplay uses several independent streams (pieces, special cells, hazards,
 * bosses, bots) derived from one session seed so that, for a given seed and
 * input sequence, the logical outcome is always identical. Presentation code
 * uses its own stream and can never change gameplay results.
 */
#ifndef BM_RNG_H
#define BM_RNG_H

#include <stdint.h>

namespace bm
{
    // SplitMix32-seeded xorshift128 variant (xoshiro128**). Small, fast, good quality.
    class rng
    {
    public:
        constexpr rng() = default;

        constexpr explicit rng(uint32_t seed)
        {
            reseed(seed);
        }

        constexpr void reseed(uint32_t seed)
        {
            uint32_t z = seed;
            for(uint32_t& word : _s)
            {
                z += 0x9E3779B9u;
                uint32_t x = z;
                x = (x ^ (x >> 16)) * 0x85EBCA6Bu;
                x = (x ^ (x >> 13)) * 0xC2B2AE35u;
                word = x ^ (x >> 16);
            }
            if(! (_s[0] | _s[1] | _s[2] | _s[3]))
            {
                _s[0] = 1;
            }
        }

        constexpr uint32_t next()
        {
            const uint32_t result = _rotl(_s[1] * 5, 7) * 9;
            const uint32_t t = _s[1] << 9;
            _s[2] ^= _s[0];
            _s[3] ^= _s[1];
            _s[1] ^= _s[2];
            _s[0] ^= _s[3];
            _s[2] ^= t;
            _s[3] = _rotl(_s[3], 11);
            return result;
        }

        // Uniform integer in [0, limit). limit must be > 0. Uses rejection to avoid bias.
        constexpr int below(int limit)
        {
            const uint32_t l = uint32_t(limit);
            const uint32_t threshold = (0u - l) % l;
            while(true)
            {
                const uint32_t r = next();
                if(r >= threshold)
                {
                    return int(r % l);
                }
            }
        }

        // Uniform integer in [low, high].
        constexpr int range(int low, int high)
        {
            return low + below(high - low + 1);
        }

        // Derives an independent stream seed (used to split one session seed).
        static constexpr uint32_t derive(uint32_t seed, uint32_t stream)
        {
            uint32_t x = seed ^ (stream * 0x9E3779B9u);
            x = (x ^ (x >> 15)) * 0x2C1B3C6Du;
            x = (x ^ (x >> 12)) * 0x297A2D39u;
            return x ^ (x >> 15);
        }

    private:
        uint32_t _s[4] = { 1, 2, 3, 4 };

        static constexpr uint32_t _rotl(uint32_t x, int k)
        {
            return (x << k) | (x >> (32 - k));
        }
    };

    enum rng_stream : uint32_t
    {
        stream_pieces = 1,
        stream_specials = 2,
        stream_hazards = 3,
        stream_boss = 4,
        stream_bot = 5,
        stream_run = 6,
        stream_garbage = 7
    };
}

#endif
