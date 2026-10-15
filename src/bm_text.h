/*
 * BLOCK MAYHEM - sprite text helpers (original small and large pixel fonts).
 */
#ifndef BM_TEXT_H
#define BM_TEXT_H

#include "bn_camera_ptr.h"
#include "bn_optional.h"
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
        [[nodiscard]] int small_width(const bn::string_view& s);
        [[nodiscard]] int big_width(const bn::string_view& s);
    }

    // A piece of on-screen text that is only regenerated when it changes.
    class label
    {
    public:
        static constexpr int max_sprites = 12;

        void show(int x, int y, const bn::string_view& s, text_color color = text_color::white,
                  align a = align::left, int bg_priority = 0);
        void show_big(int x, int y, const bn::string_view& s, big_color color = big_color::fire,
                      align a = align::center, int bg_priority = 0);
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
        bool _big = false;
        bool _visible = true;
    };
}

#endif
