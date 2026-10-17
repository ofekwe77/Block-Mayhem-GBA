/*
 * BLOCK MAYHEM - visual effects: pooled particles, explosions, laser beams,
 * text pop-ups, screen shake profiles, hitstop and accessible screen flashes.
 * Effects never touch game rules; dropping one only loses a visual.
 */
#ifndef BM_FX_H
#define BM_FX_H

#include "bm_text.h"

#include "bn_camera_ptr.h"
#include "bn_color.h"
#include "bn_fixed.h"
#include "bn_optional.h"
#include "bn_sprite_ptr.h"
#include "bn_sprite_tiles_ptr.h"

namespace bm
{
    enum class particle_kind : uint8_t
    {
        spark = 0, smoke = 4, ember = 8, snow = 12, crystal = 14, debris = 16, star = 20, electric = 22,
        shard_white = 24, shard_yellow = 25, shard_cyan = 26, shard_violet = 27, dust = 28, heart = 30, plus = 31
    };

    enum class shake_profile : uint8_t
    {
        small, medium, large, boss, final_blast
    };

    class fx
    {
    public:
        static constexpr int max_particles = 32;
        static constexpr int max_explosions = 6;
        static constexpr int max_popups = 3;
        static constexpr int max_beam = 6;
        // Per-frame spawn budgets: a 100-cell chain reaction must never cost more than one frame.
        static constexpr int particles_per_frame = 12;
        static constexpr int explosions_per_frame = 2;
        static constexpr int popup_cooldown_frames = 6;
        static constexpr int callout_cooldown_frames = 10;

        explicit fx(const bn::camera_ptr& camera);
        ~fx();

        void set_options(int shake_setting, int flash_setting);

        void particle(particle_kind kind, int x, int y, bn::fixed vx, bn::fixed vy, int life, bn::fixed gravity = 0);
        // Cosmetic particle style (PART 35.4): line-clear shards become stars, snow, embers or hearts.
        void set_particle_style(int style) { _particle_style = style; }
        void burst(particle_kind kind, int x, int y, int count, int speed = 2);
        void explosion(int x, int y);
        void beam(int x, int y_top, int y_bottom, int frames);
        void popup(int x, int y, const bn::string_view& text, text_color color, int life = 40);
        void callout(int x, int y, const bn::string_view& text, big_color color, int life = 60);
        void shake(shake_profile profile);
        void hitstop(int frames);
        void flash(bn::color color, int percent, int frames);
        // Persistent background tint (Time Freeze); flashes return to it instead of clearing it.
        void set_base_tint(bn::color color, bn::fixed intensity);

        [[nodiscard]] int hitstop_frames() const { return _hitstop; }
        // True while this frame may still spawn particles (callers skip computing parameters otherwise).
        [[nodiscard]] bool can_spawn() const { return _particle_budget > 0; }
        void update();
        void clear();

    private:
        int _particle_style = 0;
        struct particle_slot
        {
            bn::optional<bn::sprite_ptr> sprite;
            bn::fixed x, y, vx, vy, gravity;
            int16_t life = 0;
            int16_t max_life = 0;
            uint8_t first = 0;
            uint8_t frames = 1;
            int8_t shown = -1;      // frame currently on the sprite (tiles are only swapped when it changes)
        };

        struct explosion_slot
        {
            bn::optional<bn::sprite_ptr> sprite;
            int16_t timer = 0;
            int8_t shown = -1;
        };

        struct popup_slot
        {
            glyph_label text;
            int16_t life = 0;
            int16_t x = 0;
            int16_t y = 0;
            bool big = false;
        };

        struct beam_slot
        {
            bn::optional<bn::sprite_ptr> sprite;
            int16_t timer = 0;
        };

        bn::camera_ptr _camera;
        particle_slot _particles[max_particles];
        explosion_slot _explosions[max_explosions];
        popup_slot _popups[max_popups];
        popup_slot _callout;
        void _callout_hide();
        void _callout_visible(bool visible);
        beam_slot _beam[max_beam];
        struct deferred_text
        {
            bn::string<24> text;
            int16_t x = 0;
            int16_t y = 0;
            int16_t life = 0;
            uint8_t color = 0;
            bool pending = false;
        };

        // Tiles of every animation frame, created once: swapping a cached tiles pointer is cheap, while
        // asking for an item's frame by index searches the engine's tile cache every time.
        bn::optional<bn::sprite_tiles_ptr> _particle_tiles[32];
        bn::optional<bn::sprite_tiles_ptr> _explosion_tiles[6];

        [[nodiscard]] const bn::sprite_tiles_ptr& _particle_frame(int index);
        [[nodiscard]] const bn::sprite_tiles_ptr& _explosion_frame(int index);

        bn::color _base_color;
        bn::fixed _base_intensity;
        int _applied_fade = -1;     // last fade level written (sixteenths), -1 = unknown

        int _next_particle = 0;
        int _particle_budget = particles_per_frame;
        int _explosion_budget = explosions_per_frame;
        int _popup_cooldown = 0;
        int _callout_cooldown = 0;
        deferred_text _deferred_popup;
        deferred_text _deferred_callout;

        void _show_popup(int x, int y, const bn::string_view& text, text_color color, int life);
        void _show_callout(int x, int y, const bn::string_view& text, big_color color, int life);
        int _shake_timer = 0;
        int _shake_amp = 0;
        int _hitstop = 0;
        int _flash_timer = 0;
        int _flash_length = 0;
        int _flash_percent = 0;
        bn::color _flash_color;
        int _shake_setting = 2;
        int _flash_setting = 1;
        uint32_t _seed = 0x1234567u;

        int _rand(int n);
    };
}

#endif
