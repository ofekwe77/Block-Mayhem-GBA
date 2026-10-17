/*
 * BLOCK MAYHEM - fireworks for celebrations outside gameplay (PART 61.4 dynamic celebration): the S+ results
 * secret and the world-clear celebration on the world map. A fixed pool of particle sprites, bursts of twelve
 * sparks at a time, drawn behind the menu text.
 */
#ifndef BM_FIREWORKS_H
#define BM_FIREWORKS_H

#include "bn_optional.h"
#include "bn_sprite_ptr.h"

namespace bm
{
    class fireworks
    {
    public:
        // Bursts appear every 'interval' frames at pseudo-random points inside the given screen rectangle.
        void update(int frame, int x0, int y0, int width, int height, int interval = 22);
        void hide();

    private:
        static constexpr int count = 24;
        static constexpr int per_burst = 12;

        bn::optional<bn::sprite_ptr> _sparks[count];
        int16_t _x[count] = {};
        int16_t _y[count] = {};
        int16_t _vx[count] = {};
        int16_t _vy[count] = {};
        int16_t _life[count] = {};
        int _bursts = 0;
    };
}

#endif
