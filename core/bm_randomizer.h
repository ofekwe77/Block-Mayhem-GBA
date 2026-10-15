/*
 * BLOCK MAYHEM - shape randomizer and special-cell distribution.
 *
 * Shapes come from a seeded seven-bag (each bag holds exactly one of each
 * shape). Special cells are assigned by a separate, independently seeded
 * distributor so they can never break seven-bag fairness. Stages can add
 * guarantees ("a bomb at least every N pieces") so objectives never depend on
 * a lucky rare spawn.
 */
#ifndef BM_RANDOMIZER_H
#define BM_RANDOMIZER_H

#include "bm_piece.h"
#include "bm_rng.h"

namespace bm
{
    class bag_randomizer
    {
    public:
        void seed(uint32_t s);
        [[nodiscard]] piece_type next();

    private:
        rng _rng;
        piece_type _bag[7] = {};
        int _index = 7;

        void _refill();
    };

    struct special_guarantee
    {
        special_kind kind = special_kind::none;
        uint8_t every = 0;          // at least one 'kind' within every N generated pieces
        uint8_t max_count = 0;      // stop guaranteeing after this many (0 = forever)
    };

    struct special_config
    {
        // Relative weights for bomb, fire, spark, magnet, rainbow (index = special_kind - 1).
        uint8_t weights[5] = { 0, 0, 0, 0, 0 };
        uint8_t interval_min = 0;   // pieces between special-bearing pieces; 0 disables random specials
        uint8_t interval_max = 0;
        special_guarantee guarantees[3];
    };

    class special_distributor
    {
    public:
        void configure(const special_config& config, uint32_t seed);

        // Decides the special payload of the next generated piece.
        void decorate(queued_piece& piece);

        [[nodiscard]] int generated() const { return _generated; }

        // Replaces the configuration mid-session (e.g. boss phase), keeping the RNG state.
        void reconfigure(const special_config& config);

    private:
        special_config _config;
        rng _rng;
        int _countdown = 0;
        int _generated = 0;
        int _since[special_kind_count] = {};
        int _given[special_kind_count] = {};

        [[nodiscard]] special_kind _roll_kind();
        void _reset_countdown();
    };
}

#endif
