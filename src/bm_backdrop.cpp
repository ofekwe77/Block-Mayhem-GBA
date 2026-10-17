#include "bm_backdrop.h"

#include "bn_bg_palette_item.h"
#include "bn_math.h"
#include "bn_regular_bg_item.h"

#include "bn_regular_bg_items_bg_menu.h"
#include "bn_regular_bg_items_bg_title.h"
#include "bn_regular_bg_items_bg_w1_far.h"
#include "bn_regular_bg_items_bg_w1_near.h"
#include "bn_regular_bg_items_bg_w2_far.h"
#include "bn_regular_bg_items_bg_w2_near.h"
#include "bn_regular_bg_items_bg_w3_far.h"
#include "bn_regular_bg_items_bg_w3_near.h"
#include "bn_regular_bg_items_bg_w4_far.h"
#include "bn_regular_bg_items_bg_w4_near.h"
#include "bn_regular_bg_items_bg_w5_far.h"
#include "bn_regular_bg_items_bg_w5_near.h"
#include "bn_regular_bg_items_bg_w6_far.h"
#include "bn_regular_bg_items_bg_w6_near.h"
#include "bn_regular_bg_items_bg_worldmap.h"
#include "bn_sprite_items_spr_particles.h"

namespace bm
{
    namespace
    {
        struct layer_def
        {
            const bn::regular_bg_item* item;
            const bgfx::cycle* cycles;
            int cycle_count;
            int speed_x16;      // horizontal scroll speed in 1/16 px per frame
        };

        struct backdrop_def
        {
            layer_def far;
            layer_def near;
        };

#define BM_LAYER(name, speed) { &bn::regular_bg_items::name, bgfx::name##_cycles, bgfx::name##_cycle_count, speed }
#define BM_NO_LAYER { nullptr, nullptr, 0, 0 }

        const backdrop_def& def(backdrop_id id)
        {
            static const backdrop_def defs[] = {
                { BM_LAYER(bg_title, 0), BM_NO_LAYER },
                { BM_LAYER(bg_w1_far, 2), BM_LAYER(bg_w1_near, 6) },
                { BM_LAYER(bg_w2_far, 1), BM_LAYER(bg_w2_near, 4) },
                { BM_LAYER(bg_w3_far, 2), BM_LAYER(bg_w3_near, 5) },
                { BM_LAYER(bg_w4_far, 2), BM_LAYER(bg_w4_near, 7) },
                { BM_LAYER(bg_w5_far, 1), BM_LAYER(bg_w5_near, 3) },
                { BM_LAYER(bg_w6_far, 3), BM_LAYER(bg_w6_near, 8) },
                { BM_LAYER(bg_menu, 1), BM_NO_LAYER },
                { BM_LAYER(bg_worldmap, 0), BM_NO_LAYER },
            };
            const int index = int(id);
            return defs[index >= 0 && index < int(sizeof(defs) / sizeof(defs[0])) ? index : 1];
        }

#undef BM_LAYER
#undef BM_NO_LAYER

        bn::color scaled(bn::color c, int num, int den)
        {
            return bn::color(c.red() * num / den, c.green() * num / den, c.blue() * num / den);
        }

        bool is_world(backdrop_id id)
        {
            return id >= backdrop_id::world1 && id <= backdrop_id::world6;
        }
    }

    backdrop::backdrop(backdrop_id id, int priority) :
        _id(id)
    {
        const backdrop_def& d = def(id);
        const layer_def* defs[2] = { &d.far, &d.near };
        for(int i = 0; i < 2; ++i)
        {
            const layer_def& ld = *defs[i];
            if(! ld.item)
            {
                continue;
            }
            layer& l = _layers[i];
            l.bg = ld.item->create_bg(8, 48);
            l.bg->set_priority(priority);
            l.bg->set_z_order(i == 0 ? 1 : 0);
            l.palette = l.bg->palette();
            l.original = ld.item->palette_item().colors_ref();
            l.cycles = ld.cycles;
            l.cycle_count = ld.cycle_count;
            l.speed = bn::fixed(ld.speed_x16) / 16;
        }

        for(bn::fixed& delta : _deltas)
        {
            delta = 0;
        }
        if((id == backdrop_id::world3 || id == backdrop_id::world6) && _layers[0].bg)
        {
            _hbe = bn::regular_bg_position_hbe_ptr::create_horizontal(*_layers[0].bg, _deltas);
        }

        if(is_world(id))
        {
            for(ambient& a : _ambient)
            {
                _spawn_ambient(a, true);
            }
        }
    }

    void backdrop::set_camera(const bn::camera_ptr& camera)
    {
        _camera = camera;
        for(layer& l : _layers)
        {
            if(l.bg)
            {
                l.bg->set_camera(camera);
            }
        }
        for(ambient& a : _ambient)
        {
            if(a.sprite)
            {
                a.sprite->set_camera(camera);
            }
        }
    }

    void backdrop::set_visible(bool visible)
    {
        for(layer& l : _layers)
        {
            if(l.bg)
            {
                l.bg->set_visible(visible);
            }
        }
        for(ambient& a : _ambient)
        {
            if(a.sprite)
            {
                a.sprite->set_visible(visible);
            }
        }
    }

    void backdrop::set_dim(bn::fixed amount)
    {
        for(layer& l : _layers)
        {
            if(l.palette)
            {
                l.palette->set_fade(bn::color(1, 1, 4), amount);
            }
        }
        for(ambient& a : _ambient)
        {
            if(a.sprite)
            {
                a.sprite->set_visible(amount < bn::fixed(0.5));
            }
        }
    }

    void backdrop::set_color(int layer_index, int index, bn::color color)
    {
        layer& l = _layers[layer_index & 1];
        if(l.palette)
        {
            l.palette->set_color(index, color);
        }
    }

    void backdrop::excite(int frames, int level)
    {
        if(frames > _excite_frames)
        {
            _excite_frames = frames;
        }
        if(level > _excite_level || _excite_frames <= 0)
        {
            _excite_level = level;
        }
        if(_id == backdrop_id::world6 && level >= 2)
        {
            _glitch = 10;   // the final world destabilises during major chains
        }
    }

    void backdrop::_spawn_ambient(ambient& a, bool initial)
    {
        _noise = _noise * 1103515245u + 12345u;
        const int r1 = int((_noise >> 16) & 0xFF);
        _noise = _noise * 1103515245u + 12345u;
        const int r2 = int((_noise >> 16) & 0xFF);
        int frame = 0;
        a.x = r1 * 240 / 255;
        a.phase = int16_t(r2);
        switch(_id)
        {
        case backdrop_id::world1:      // sparks drifting up from the machines
            frame = 0;
            a.y = initial ? r2 * 160 / 255 : 168;
            a.vx = bn::fixed(r2 % 16 - 8) / 32;
            a.vy = -bn::fixed(8 + r1 % 8) / 16;
            break;
        case backdrop_id::world2:      // snow
            frame = 12;
            a.y = initial ? r2 * 160 / 255 : -8;
            a.vx = bn::fixed(r2 % 16 - 8) / 48;
            a.vy = bn::fixed(6 + r1 % 10) / 16;
            break;
        case backdrop_id::world3:      // rising embers
            frame = 8;
            a.y = initial ? r2 * 160 / 255 : 168;
            a.vx = bn::fixed(r2 % 16 - 8) / 24;
            a.vy = -bn::fixed(10 + r1 % 12) / 16;
            break;
        case backdrop_id::world4:      // electric rain
            frame = 22;
            a.y = initial ? r2 * 160 / 255 : -8;
            a.vx = -bn::fixed(4) / 16;
            a.vy = bn::fixed(32 + r1 % 16) / 16;
            break;
        case backdrop_id::world5:      // cosmic dust floating in every direction
            frame = 28;
            a.y = r2 * 160 / 255;
            a.vx = bn::fixed(r1 % 16 - 8) / 40;
            a.vy = bn::fixed(r2 % 16 - 8) / 40;
            break;
        case backdrop_id::world6:      // debris and embers
        default:
            frame = (r1 & 1) ? 16 : 8;
            a.y = initial ? r2 * 160 / 255 : 168;
            a.vx = bn::fixed(r2 % 16 - 8) / 20;
            a.vy = -bn::fixed(6 + r1 % 14) / 16;
            break;
        }
        if(! a.sprite)
        {
            a.sprite = bn::sprite_items::spr_particles.create_sprite(0, 0, frame);
            a.sprite->set_bg_priority(3);
            a.sprite->set_z_order(10);
            if(_camera)
            {
                a.sprite->set_camera(*_camera);
            }
        }
        else
        {
            a.sprite->set_tiles(bn::sprite_items::spr_particles.tiles_item(), frame);
        }
        a.sprite->set_position(a.x - 120, a.y - 80);
    }

    void backdrop::_update_ambient()
    {
        if(! is_world(_id))
        {
            return;
        }
        const int speed = _excite_frames > 0 ? 2 : 1;
        for(ambient& a : _ambient)
        {
            if(! a.sprite)
            {
                continue;
            }
            a.x += a.vx * speed;
            a.y += a.vy * speed;
            if(_id == backdrop_id::world2)
            {
                a.x += bn::degrees_lut_sin((_frame * 2 + a.phase) % 360) / 4;
            }
            if(a.y < -10 || a.y > 170 || a.x < -10 || a.x > 250)
            {
                _spawn_ambient(a, false);
                continue;
            }
            a.sprite->set_position(a.x - 120, a.y - 80);
        }
    }

    void backdrop::_update_hbe()
    {
        if(! _hbe)
        {
            return;
        }
        if(_id == backdrop_id::world3)
        {
            // Heat haze: a gentle wave that grows near the lava and while the world is excited.
            const int amp = _excite_frames > 0 ? 3 : 1;
            for(int y = 0; y < bn::display::height(); ++y)
            {
                const int angle = (y * 12 + _frame * 5) % 360;
                const bn::fixed strength = bn::fixed(y > 80 ? amp + 1 : amp) / 2;
                _deltas[y] = bn::degrees_lut_sin(angle) * strength;
            }
        }
        else
        {
            // Final Meltdown: brief reality glitches.
            if(_glitch == 0 && _frame % 240 == 120)
            {
                _glitch = 6;
            }
            for(int y = 0; y < bn::display::height(); ++y)
            {
                if(_glitch > 0)
                {
                    _noise = _noise * 1103515245u + 12345u;
                    _deltas[y] = ((y / 8) & 1) ? int((_noise >> 24) % 9) - 4 : 0;
                }
                else
                {
                    _deltas[y] = 0;
                }
            }
            if(_glitch > 0)
            {
                --_glitch;
            }
        }
        _hbe->reload_deltas_ref();
    }

    void backdrop::_animate(layer& l)
    {
        if(! l.palette)
        {
            return;
        }
        const int boost = _excite_frames > 0 ? 1 + _excite_level : 1;
        for(int c = 0; c < l.cycle_count && c < 8; ++c)
        {
            const bgfx::cycle& cy = l.cycles[c];
            const int base = cy.bank * 16 + cy.first;
            switch(cy.mode)
            {
            case bgfx::rotate:
            {
                l.counters[c] += boost;
                const int period = cy.period ? cy.period : 1;
                if(l.counters[c] >= period)
                {
                    l.counters[c] = 0;
                    l.steps[c] = (l.steps[c] + 1) % cy.count;
                    for(int k = 0; k < cy.count; ++k)
                    {
                        l.palette->set_color(base + k, l.original[base + (k + l.steps[c]) % cy.count]);
                    }
                }
                break;
            }

            case bgfx::blink:
            {
                l.counters[c] += boost;
                const int period = cy.period ? cy.period : 30;
                const bool on = (l.counters[c] / period) % 2 == 0;
                for(int k = 0; k < cy.count; ++k)
                {
                    const bn::color orig = l.original[base + k];
                    l.palette->set_color(base + k, on ? orig : scaled(orig, 2, 7));
                }
                break;
            }

            case bgfx::flicker:
            {
                _noise = _noise * 1103515245u + 12345u;
                const int r = int((_noise >> 24) & 31);
                const int level = r < 2 ? 2 : (r < 4 ? 5 : 7);
                for(int k = 0; k < cy.count; ++k)
                {
                    l.palette->set_color(base + k, scaled(l.original[base + k], level, 7));
                }
                break;
            }

            case bgfx::pulse:
            default:
            {
                l.counters[c] += boost;
                const int period = cy.period ? cy.period : 60;
                int t = l.counters[c] % (period * 2);
                t = t > period ? period * 2 - t : t;
                for(int k = 0; k < cy.count; ++k)
                {
                    l.palette->set_color(base + k, scaled(l.original[base + k], 3 + (4 * t) / period, 7));
                }
                break;
            }
            }
        }
    }

    void backdrop::update()
    {
        ++_frame;
        for(layer& l : _layers)
        {
            if(! l.bg)
            {
                continue;
            }
            l.scroll_x += l.speed * (_excite_frames > 0 ? 2 : 1);
            if(l.scroll_x >= 256)
            {
                l.scroll_x -= 256;
            }
            l.bg->set_x(8 - l.scroll_x);
            _animate(l);
        }
        _update_ambient();
        _update_hbe();
        if(_excite_frames > 0)
        {
            --_excite_frames;
        }
    }
}
