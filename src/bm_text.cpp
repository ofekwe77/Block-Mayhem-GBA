#include "bm_text.h"

#include "bm_font_big.h"
#include "bm_font_small.h"

#include "bn_sprite_palette_items_pal_big_blood.h"
#include "bn_sprite_palette_items_pal_big_fire.h"
#include "bn_sprite_palette_items_pal_big_gold.h"
#include "bn_sprite_palette_items_pal_big_ice.h"
#include "bn_sprite_palette_items_pal_big_neon.h"
#include "bn_sprite_palette_items_pal_big_steel.h"
#include "bn_sprite_palette_items_pal_big_toxic.h"
#include "bn_sprite_palette_items_pal_text_cyan.h"
#include "bn_sprite_palette_items_pal_text_gray.h"
#include "bn_sprite_palette_items_pal_text_green.h"
#include "bn_sprite_palette_items_pal_text_orange.h"
#include "bn_sprite_palette_items_pal_text_red.h"
#include "bn_sprite_palette_items_pal_text_violet.h"
#include "bn_sprite_palette_items_pal_text_white.h"
#include "bn_sprite_palette_items_pal_text_yellow.h"

namespace bm
{
    namespace
    {
        const bn::sprite_palette_item& small_palette(text_color c)
        {
            switch(c)
            {
            case text_color::yellow: return bn::sprite_palette_items::pal_text_yellow;
            case text_color::red: return bn::sprite_palette_items::pal_text_red;
            case text_color::cyan: return bn::sprite_palette_items::pal_text_cyan;
            case text_color::green: return bn::sprite_palette_items::pal_text_green;
            case text_color::orange: return bn::sprite_palette_items::pal_text_orange;
            case text_color::gray: return bn::sprite_palette_items::pal_text_gray;
            case text_color::violet: return bn::sprite_palette_items::pal_text_violet;
            case text_color::white:
            default: return bn::sprite_palette_items::pal_text_white;
            }
        }

        const bn::sprite_palette_item& big_palette(big_color c)
        {
            switch(c)
            {
            case big_color::ice: return bn::sprite_palette_items::pal_big_ice;
            case big_color::gold: return bn::sprite_palette_items::pal_big_gold;
            case big_color::toxic: return bn::sprite_palette_items::pal_big_toxic;
            case big_color::neon: return bn::sprite_palette_items::pal_big_neon;
            case big_color::steel: return bn::sprite_palette_items::pal_big_steel;
            case big_color::blood: return bn::sprite_palette_items::pal_big_blood;
            case big_color::fire:
            default: return bn::sprite_palette_items::pal_big_fire;
            }
        }

        void apply_align(bn::sprite_text_generator& g, align a)
        {
            switch(a)
            {
            case align::center: g.set_center_alignment(); break;
            case align::right: g.set_right_alignment(); break;
            case align::left:
            default: g.set_left_alignment(); break;
            }
        }

        bn::sprite_text_generator& small_gen()
        {
            static bn::sprite_text_generator g(fonts::small);
            return g;
        }

        bn::sprite_text_generator& big_gen()
        {
            static bn::sprite_text_generator g(fonts::big);
            return g;
        }
    }

    namespace text
    {
        bn::sprite_text_generator& small(text_color color, align a)
        {
            bn::sprite_text_generator& g = small_gen();
            g.set_palette_item(small_palette(color));
            apply_align(g, a);
            return g;
        }

        bn::sprite_text_generator& big(big_color color, align a)
        {
            bn::sprite_text_generator& g = big_gen();
            g.set_palette_item(big_palette(color));
            apply_align(g, a);
            return g;
        }

        int small_width(const bn::string_view& s)
        {
            return small_gen().width(s);
        }

        int big_width(const bn::string_view& s)
        {
            return big_gen().width(s);
        }
    }

    void label::show(int x, int y, const bn::string_view& s, text_color color, align a, int bg_priority)
    {
        if(! _big && _x == x && _y == y && _color == uint8_t(color) && _align == uint8_t(a) && _text == s &&
           ! _sprites.empty())
        {
            return;
        }
        _sprites.clear();
        _text = s.size() > 32 ? s.substr(0, 32) : s;
        _x = x;
        _y = y;
        _color = uint8_t(color);
        _align = uint8_t(a);
        _big = false;
        bn::sprite_text_generator& g = text::small(color, a);
        g.set_bg_priority(bg_priority);
        g.generate(x - 120, y - 80 + 4, _text, _sprites);
        for(bn::sprite_ptr& sp : _sprites)
        {
            sp.set_visible(_visible);
            if(_camera)
            {
                sp.set_camera(*_camera);
            }
        }
    }

    void label::show_big(int x, int y, const bn::string_view& s, big_color color, align a, int bg_priority)
    {
        if(_big && _x == x && _y == y && _color == uint8_t(color) && _align == uint8_t(a) && _text == s &&
           ! _sprites.empty())
        {
            return;
        }
        _sprites.clear();
        _text = s.size() > 32 ? s.substr(0, 32) : s;
        _x = x;
        _y = y;
        _color = uint8_t(color);
        _align = uint8_t(a);
        _big = true;
        bn::sprite_text_generator& g = text::big(color, a);
        g.set_bg_priority(bg_priority);
        g.generate(x - 120, y - 80 + 8, _text, _sprites);
        for(bn::sprite_ptr& sp : _sprites)
        {
            sp.set_visible(_visible);
            if(_camera)
            {
                sp.set_camera(*_camera);
            }
        }
    }

    void label::hide()
    {
        _sprites.clear();
        _text.clear();
        _x = -1000;
    }

    void label::set_visible(bool visible)
    {
        _visible = visible;
        for(bn::sprite_ptr& sp : _sprites)
        {
            sp.set_visible(visible);
        }
    }

    void label::move(int x, int y)
    {
        const int dx = x - _x;
        const int dy = y - _y;
        if(! dx && ! dy)
        {
            return;
        }
        for(bn::sprite_ptr& sp : _sprites)
        {
            sp.set_position(sp.x() + dx, sp.y() + dy);
        }
        _x = x;
        _y = y;
    }
}
