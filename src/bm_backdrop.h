/*
 * BLOCK MAYHEM - animated world backgrounds (two parallax layers + palette cycling).
 */
#ifndef BM_BACKDROP_H
#define BM_BACKDROP_H

#include "bm_bg_cycles.h"

#include "bn_bg_palette_ptr.h"
#include "bn_camera_ptr.h"
#include "bn_color.h"
#include "bn_fixed.h"
#include "bn_optional.h"
#include "bn_regular_bg_ptr.h"
#include "bn_span.h"

namespace bm
{
    enum class backdrop_id : uint8_t
    {
        title,
        world1, world2, world3, world4, world5, world6,
        menu
    };

    class backdrop
    {
    public:
        backdrop(backdrop_id id, int priority);

        void update();

        // Environmental reaction: temporarily speeds up animation (big combos, Overheat, explosions).
        void excite(int frames, int level);
        void set_camera(const bn::camera_ptr& camera);
        void set_visible(bool visible);

    private:
        struct layer
        {
            bn::optional<bn::regular_bg_ptr> bg;
            bn::optional<bn::bg_palette_ptr> palette;
            bn::span<const bn::color> original;
            const bgfx::cycle* cycles = nullptr;
            int cycle_count = 0;
            bn::fixed scroll_x;
            bn::fixed speed;
            int counters[8] = {};
            int steps[8] = {};
        };

        layer _layers[2];
        backdrop_id _id;
        int _frame = 0;
        int _excite_frames = 0;
        int _excite_level = 0;
        uint32_t _noise = 12345;

        void _setup(layer& l, int index);
        void _animate(layer& l);
    };
}

#endif
