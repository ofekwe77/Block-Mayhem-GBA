/*
 * BLOCK MAYHEM - animated world backgrounds (two parallax layers + palette cycling).
 */
#ifndef BM_BACKDROP_H
#define BM_BACKDROP_H

#include "bm_bg_cycles.h"

#include "bn_array.h"
#include "bn_bg_palette_ptr.h"
#include "bn_camera_ptr.h"
#include "bn_color.h"
#include "bn_display.h"
#include "bn_fixed.h"
#include "bn_optional.h"
#include "bn_regular_bg_position_hbe_ptr.h"
#include "bn_regular_bg_ptr.h"
#include "bn_span.h"
#include "bn_sprite_ptr.h"

namespace bm
{
    enum class backdrop_id : uint8_t
    {
        title,
        world1, world2, world3, world4, world5, world6,
        menu,
        worldmap
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
        // Darkens the scenery behind menus so text stays readable (0 = full brightness).
        void set_dim(bn::fixed amount);
        // Replaces one colour of a layer's palette (colours that are not palette-cycled stay replaced).
        void set_color(int layer, int index, bn::color color);

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

        struct ambient
        {
            bn::optional<bn::sprite_ptr> sprite;
            bn::fixed x, y, vx, vy;
            int16_t phase = 0;
        };

        static constexpr int ambient_count = 8;

        layer _layers[2];
        ambient _ambient[ambient_count];
        bn::array<bn::fixed, bn::display::height()> _deltas;
        bn::optional<bn::regular_bg_position_hbe_ptr> _hbe;
        bn::optional<bn::camera_ptr> _camera;
        backdrop_id _id;
        int _frame = 0;
        int _excite_frames = 0;
        int _excite_level = 0;
        int _glitch = 0;
        uint32_t _noise = 12345;

        void _animate(layer& l);
        void _spawn_ambient(ambient& a, bool initial);
        void _update_ambient();
        void _update_hbe();
    };
}

#endif
