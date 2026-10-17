#include "bm_fireworks.h"

#include "bn_sprite_items_spr_particles.h"

namespace bm
{
    void fireworks::update(int frame, int x0, int y0, int width, int height, int interval)
    {
        if(interval > 0 && frame % interval == 1)
        {
            const uint32_t h = uint32_t(frame) * 2654435761u;
            const int cx = x0 + int((h >> 8) % uint32_t(width > 0 ? width : 1));
            const int cy = y0 + int((h >> 20) % uint32_t(height > 0 ? height : 1));
            static constexpr int8_t dirs[per_burst][2] = { { 20, 0 }, { 17, 10 }, { 10, 17 }, { 0, 20 }, { -10, 17 },
                                                           { -17, 10 }, { -20, 0 }, { -17, -10 }, { -10, -17 },
                                                           { 0, -20 }, { 10, -17 }, { 17, -10 } };
            static constexpr int frames[4] = { 20, 25, 26, 27 };    // star, yellow, cyan and violet shards
            const int base = (_bursts & 1) ? per_burst : 0;
            for(int i = 0; i < per_burst; ++i)
            {
                const int k = base + i;
                _x[k] = int16_t(cx * 16);
                _y[k] = int16_t(cy * 16);
                _vx[k] = dirs[i][0];
                _vy[k] = dirs[i][1];
                _life[k] = 34;
                const int f = frames[(_bursts + i) & 3];
                if(! _sparks[k])
                {
                    _sparks[k] = bn::sprite_items::spr_particles.create_sprite(0, 0, f);
                    _sparks[k]->set_z_order(10);        // behind the menu text
                }
                _sparks[k]->set_tiles(bn::sprite_items::spr_particles.tiles_item(), f);
                _sparks[k]->set_visible(true);
            }
            ++_bursts;
        }
        for(int k = 0; k < count; ++k)
        {
            if(! _sparks[k] || _life[k] <= 0)
            {
                continue;
            }
            _x[k] = int16_t(_x[k] + _vx[k]);
            _y[k] = int16_t(_y[k] + _vy[k]);
            _vy[k] = int16_t(_vy[k] + 1);
            if(--_life[k] <= 0)
            {
                _sparks[k]->set_visible(false);
                continue;
            }
            _sparks[k]->set_position(_x[k] / 16 - 120, _y[k] / 16 - 80);
        }
    }

    void fireworks::hide()
    {
        for(int k = 0; k < count; ++k)
        {
            _life[k] = 0;
            if(_sparks[k])
            {
                _sparks[k]->set_visible(false);
            }
        }
    }
}
