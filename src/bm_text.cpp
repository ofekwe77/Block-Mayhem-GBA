#include "bm_text.h"

#include "bm_font_big.h"
#include "bm_font_small.h"
#include "bm_font_tiny.h"

#include "bn_sprite_items_font_big.h"
#include "bn_sprite_items_font_small.h"
#include "bn_sprite_palette_ptr.h"
#include "bn_sprite_tiles_ptr.h"
#include "bn_timer.h"

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
            // Composite sprites (a few glyphs each) keep long titles within a label's sprite budget; the
            // frequent big texts of gameplay (chain call-outs) use pooled glyph labels instead.
            static bn::sprite_text_generator g(fonts::big);
            return g;
        }

        bn::sprite_text_generator& tiny_gen()
        {
            static bn::sprite_text_generator g(fonts::tiny);
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

        bn::sprite_text_generator& tiny(text_color color, align a)
        {
            bn::sprite_text_generator& g = tiny_gen();
            g.set_palette_item(small_palette(color));
            apply_align(g, a);
            return g;
        }

        int small_width(const bn::string_view& s)
        {
            return small_gen().width(s);
        }

        int tiny_width(const bn::string_view& s)
        {
            return tiny_gen().width(s);
        }

        int& generated_count_ref()
        {
            static int count = 0;
            return count;
        }

        int& generated_ticks_ref()
        {
            static int ticks = 0;
            return ticks;
        }

        int* generated_max_ref()
        {
            static int max[3] = {};
            return max;
        }

        int take_generated_count()
        {
            const int n = generated_count_ref();
            generated_count_ref() = 0;
            return n;
        }

        int big_width(const bn::string_view& s)
        {
            return big_gen().width(s);
        }
    }

    void label::_generate(bn::sprite_text_generator& g, int x, int y, const bn::string_view& s, int color, align a,
                          int font, int bg_priority, int y_offset)
    {
        _sprites.clear();
        _text = s.size() > 32 ? s.substr(0, 32) : s;
        _x = x;
        _y = y;
        _color = uint8_t(color);
        _align = uint8_t(a);
        _font = uint8_t(font);
        ++text::generated_count_ref();
        bn::timer timer;
        g.set_bg_priority(bg_priority);
        g.generate(x - 120, y - 80 + y_offset, _text, _sprites);
        const int spent = timer.elapsed_ticks();
        text::generated_ticks_ref() += spent;
        int* worst = text::generated_max_ref();
        if(spent > worst[0])
        {
            worst[0] = spent;
            worst[1] = font;
            worst[2] = _text.size();
        }
        for(bn::sprite_ptr& sp : _sprites)
        {
            sp.set_visible(_visible);
            if(_camera)
            {
                sp.set_camera(*_camera);
            }
        }
    }

    // ================================================================== glyph_label

    namespace
    {
        constexpr int glyph_count = 94;     // '!' .. '~'

        bn::optional<bn::sprite_tiles_ptr>* glyph_cache(bool big)
        {
            static bn::optional<bn::sprite_tiles_ptr> small_cache[glyph_count];
            static bn::optional<bn::sprite_tiles_ptr> big_cache[glyph_count];
            return big ? big_cache : small_cache;
        }

        const bn::sprite_tiles_ptr& glyph_tiles(bool big, int index)
        {
            bn::optional<bn::sprite_tiles_ptr>& slot = glyph_cache(big)[index];
            if(! slot)
            {
                slot = big ? bn::sprite_items::font_big.tiles_item().create_tiles(index)
                           : bn::sprite_items::font_small.tiles_item().create_tiles(index);
            }
            return *slot;
        }

        const bn::sprite_palette_ptr& glyph_palette(bool big, int color)
        {
            static bn::optional<bn::sprite_palette_ptr> small_cache[8];
            static bn::optional<bn::sprite_palette_ptr> big_cache[7];
            if(big)
            {
                const int c = color >= 0 && color < 7 ? color : 0;
                if(! big_cache[c])
                {
                    big_cache[c] = big_palette(big_color(c)).create_palette();
                }
                return *big_cache[c];
            }
            const int c = color >= 0 && color < 8 ? color : 0;
            if(! small_cache[c])
            {
                small_cache[c] = small_palette(text_color(c)).create_palette();
            }
            return *small_cache[c];
        }

        int glyph_width(bool big, char ch)
        {
            const int i = int(ch) - 32;
            if(i < 0 || i > glyph_count)
            {
                return big ? 8 : 4;
            }
            return big ? fonts::big_widths[i] : fonts::small_widths[i];
        }
    }

    void glyph_label::_layout(int x, int y, const bn::string_view& s, bool big, int color, align a)
    {
        int width = 0;
        for(char ch : s)
        {
            width += glyph_width(big, ch);
        }
        int left = a == align::center ? x - width / 2 : (a == align::right ? x - width : x);
        const int half = big ? 8 : 4;
        const bn::sprite_palette_ptr& palette = glyph_palette(big, color);
        int n = 0;
        for(char ch : s)
        {
            const int index = int(ch) - 33;
            if(index >= 0 && index < glyph_count && n < max_glyphs)
            {
                bn::optional<bn::sprite_ptr>& g = _glyphs[n];
                if(! g || _big != big)
                {
                    g = big ? bn::sprite_items::font_big.create_sprite(0, 0, index)
                            : bn::sprite_items::font_small.create_sprite(0, 0, index);
                    g->set_bg_priority(0);
                    if(_camera)
                    {
                        g->set_camera(*_camera);
                    }
                }
                g->set_tiles(glyph_tiles(big, index));
                g->set_palette(palette);
                g->set_position(left + half - 120, y + half - 80);
                g->set_visible(_visible);
                ++n;
            }
            left += glyph_width(big, ch);
        }
        for(int i = n; i < _count; ++i)
        {
            if(_glyphs[i])
            {
                _glyphs[i]->set_visible(false);
            }
        }
        // Sprites of the other size cannot be reused for this font: drop them.
        for(int i = n; i < max_glyphs && _big != big; ++i)
        {
            _glyphs[i].reset();
        }
        _count = n;
        _x = x;
        _y = y;
        _big = big;
    }

    void glyph_label::prepare(int glyphs, bool big)
    {
        glyphs = glyphs < max_glyphs ? glyphs : max_glyphs;
        for(int i = 0; i < glyphs; ++i)
        {
            if(! _glyphs[i] || _big != big)
            {
                _glyphs[i] = big ? bn::sprite_items::font_big.create_sprite(0, 0, 0)
                                 : bn::sprite_items::font_small.create_sprite(0, 0, 0);
                _glyphs[i]->set_bg_priority(0);
                if(_camera)
                {
                    _glyphs[i]->set_camera(*_camera);
                }
                _glyphs[i]->set_visible(false);
            }
        }
        _big = big;
    }

    void glyph_label::warm(const bn::string_view& chars, bool big)
    {
        for(char ch : chars)
        {
            const int index = int(ch) - 33;
            if(index >= 0 && index < glyph_count)
            {
                (void) glyph_tiles(big, index);
            }
        }
    }

    void glyph_label::release_cache()
    {
        for(int big = 0; big < 2; ++big)
        {
            bn::optional<bn::sprite_tiles_ptr>* cache = glyph_cache(big != 0);
            for(int i = 0; i < glyph_count; ++i)
            {
                cache[i].reset();
            }
        }
    }

    void glyph_label::show(int x, int y, const bn::string_view& s, text_color color, align a)
    {
        _visible = true;
        _layout(x, y, s, false, int(color), a);
    }

    void glyph_label::show_big(int x, int y, const bn::string_view& s, big_color color, align a)
    {
        _visible = true;
        _layout(x, y, s, true, int(color), a);
    }

    void glyph_label::hide()
    {
        for(int i = 0; i < _count; ++i)
        {
            _glyphs[i]->set_visible(false);
        }
        _count = 0;
    }

    void glyph_label::set_visible(bool visible)
    {
        _visible = visible;
        for(int i = 0; i < _count; ++i)
        {
            _glyphs[i]->set_visible(visible);
        }
    }

    void glyph_label::move(int x, int y)
    {
        const int dx = x - _x;
        const int dy = y - _y;
        if(! dx && ! dy)
        {
            return;
        }
        for(int i = 0; i < _count; ++i)
        {
            _glyphs[i]->set_position(_glyphs[i]->x() + dx, _glyphs[i]->y() + dy);
        }
        _x = x;
        _y = y;
    }

    // ================================================================== label

    void label::_recolor(const bn::sprite_palette_item& palette, int color)
    {
        const bn::sprite_palette_ptr ptr = palette.create_palette();
        for(bn::sprite_ptr& sp : _sprites)
        {
            sp.set_palette(ptr);
        }
        _color = uint8_t(color);
    }

    void label::show(int x, int y, const bn::string_view& s, text_color color, align a, int bg_priority)
    {
        if(_font == 0 && _x == x && _y == y && _align == uint8_t(a) && _text == s && ! _sprites.empty())
        {
            if(_color != uint8_t(color))
            {
                _recolor(small_palette(color), int(color));     // same text, new colour: no regeneration
            }
            return;
        }
        _generate(text::small(color, a), x, y, s, int(color), a, 0, bg_priority, 4);
    }

    void label::show_big(int x, int y, const bn::string_view& s, big_color color, align a, int bg_priority)
    {
        if(_font == 1 && _x == x && _y == y && _color == uint8_t(color) && _align == uint8_t(a) && _text == s &&
           ! _sprites.empty())
        {
            return;
        }
        _generate(text::big(color, a), x, y, s, int(color), a, 1, bg_priority, 8);
    }

    void label::show_tiny(int x, int y, const bn::string_view& s, text_color color, align a, int bg_priority)
    {
        if(_font == 2 && _x == x && _y == y && _align == uint8_t(a) && _text == s && ! _sprites.empty())
        {
            if(_color != uint8_t(color))
            {
                _recolor(small_palette(color), int(color));
            }
            return;
        }
        _generate(text::tiny(color, a), x, y, s, int(color), a, 2, bg_priority, 4);
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
