#include "bm_backdrop.h"

#include "bn_bg_palette_item.h"
#include "bn_regular_bg_item.h"

#include "bn_regular_bg_items_bg_menu.h"
#include "bn_regular_bg_items_bg_title.h"
#include "bn_regular_bg_items_bg_w1_far.h"
#include "bn_regular_bg_items_bg_w1_near.h"

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

        const backdrop_def& def(backdrop_id id)
        {
            static const backdrop_def title_def = {
                { &bn::regular_bg_items::bg_title, bgfx::bg_title_cycles, bgfx::bg_title_cycle_count, 0 },
                { nullptr, nullptr, 0, 0 }
            };
            static const backdrop_def menu_def = {
                { &bn::regular_bg_items::bg_menu, bgfx::bg_menu_cycles, bgfx::bg_menu_cycle_count, 1 },
                { nullptr, nullptr, 0, 0 }
            };
            static const backdrop_def w1 = {
                { &bn::regular_bg_items::bg_w1_far, bgfx::bg_w1_far_cycles, bgfx::bg_w1_far_cycle_count, 2 },
                { &bn::regular_bg_items::bg_w1_near, bgfx::bg_w1_near_cycles, bgfx::bg_w1_near_cycle_count, 6 }
            };
            switch(id)
            {
            case backdrop_id::title:
                return title_def;
            case backdrop_id::menu:
                return menu_def;
            default:
                return w1;
            }
        }

        bn::color scaled(bn::color c, int num, int den)
        {
            return bn::color(c.red() * num / den, c.green() * num / den, c.blue() * num / den);
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
    }

    void backdrop::set_camera(const bn::camera_ptr& camera)
    {
        for(layer& l : _layers)
        {
            if(l.bg)
            {
                l.bg->set_camera(camera);
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
        if(_excite_frames > 0)
        {
            --_excite_frames;
        }
    }
}
