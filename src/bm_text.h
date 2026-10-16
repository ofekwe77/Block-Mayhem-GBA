/*
 * BLOCK MAYHEM - sprite text helpers (original small and large pixel fonts).
 */
#ifndef BM_TEXT_H
#define BM_TEXT_H

#include "bn_camera_ptr.h"
#include "bn_optional.h"
#include "bn_sprite_palette_item.h"
#include "bn_sprite_ptr.h"
#include "bn_sprite_text_generator.h"
#include "bn_string.h"
#include "bn_vector.h"

namespace bm
{
    enum class text_color : uint8_t
    {
        white, yellow, red, cyan, green, orange, gray, violet
    };

    enum class big_color : uint8_t
    {
        fire, ice, gold, toxic, neon, steel, blood
    };

    enum class align : uint8_t
    {
        left, center, right
    };

    namespace text
    {
        bn::sprite_text_generator& small(text_color color = text_color::white, align a = align::left);
        bn::sprite_text_generator& big(big_color color = big_color::fire, align a = align::center);
        bn::sprite_text_generator& tiny(text_color color = text_color::white, align a = align::left);
        [[nodiscard]] int small_width(const bn::string_view& s);
        [[nodiscard]] int big_width(const bn::string_view& s);
        [[nodiscard]] int tiny_width(const bn::string_view& s);
        // Labels (re)generated since the counter was last read (profiling).
        [[nodiscard]] int take_generated_count();
        int& generated_count_ref();
        int& generated_ticks_ref();
        // The most expensive generation since the last reset: ticks, font, length.
        int* generated_max_ref();
    }

    // Text drawn as one sprite per glyph from glyph tiles and palettes that are created once and cached.
    // Updating it never allocates VRAM, so it is safe in the busiest gameplay frames (score pop-ups,
    // chain call-outs). Plain ASCII only; the sprites are pooled and reused.
    class glyph_label
    {
    public:
        static constexpr int max_glyphs = 16;

        void show(int x, int y, const bn::string_view& s, text_color color, align a = align::center);
        void show_big(int x, int y, const bn::string_view& s, big_color color, align a = align::center);
        void hide();
        void set_visible(bool visible);
        void move(int x, int y);
        void set_camera(const bn::camera_ptr& camera) { _camera = camera; }
        [[nodiscard]] bool shown() const { return _count > 0; }

        // Creates the pooled sprites ahead of time (hidden) for a label of up to 'glyphs' glyphs.
        void prepare(int glyphs, bool big);
        // Creates the cached tiles of every glyph in 'chars' ahead of time (scene loading, not mid-chain).
        static void warm(const bn::string_view& chars, bool big);
        // Drops the shared glyph tile cache (scenes release it when they end so menus get the VRAM back).
        static void release_cache();

    private:
        bn::optional<bn::sprite_ptr> _glyphs[max_glyphs];
        bn::optional<bn::camera_ptr> _camera;
        int _count = 0;
        int _x = 0;
        int _y = 0;
        bool _big = false;
        bool _visible = true;

        void _layout(int x, int y, const bn::string_view& s, bool big, int color, align a);
    };

    // A piece of on-screen text that is only regenerated when it changes.
    class label
    {
    public:
        static constexpr int max_sprites = 16;     // room for a 20-glyph big-font title

        void show(int x, int y, const bn::string_view& s, text_color color = text_color::white,
                  align a = align::left, int bg_priority = 0);
        void show_big(int x, int y, const bn::string_view& s, big_color color = big_color::fire,
                      align a = align::center, int bg_priority = 0);
        // Compact 3x5 HUD font (same colours as the small font).
        void show_tiny(int x, int y, const bn::string_view& s, text_color color = text_color::white,
                       align a = align::left, int bg_priority = 0);
        void hide();
        void set_visible(bool visible);
        void move(int x, int y);
        [[nodiscard]] bool shown() const { return ! _sprites.empty(); }
        void set_camera(const bn::optional<bn::camera_ptr>& camera) { _camera = camera; }

    private:
        bn::vector<bn::sprite_ptr, max_sprites> _sprites;
        bn::string<32> _text;
        bn::optional<bn::camera_ptr> _camera;
        int _x = -1000;
        int _y = -1000;
        uint8_t _color = 255;
        uint8_t _align = 0;
        uint8_t _font = 0;      // 0 small, 1 big, 2 tiny
        bool _visible = true;

        void _generate(bn::sprite_text_generator& g, int x, int y, const bn::string_view& s, int color, align a,
                       int font, int bg_priority, int y_offset);
        void _recolor(const bn::sprite_palette_item& palette, int color);
    };
}

#endif
