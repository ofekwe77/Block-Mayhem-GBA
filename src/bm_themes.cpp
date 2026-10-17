#include "bm_themes.h"

#include <stdint.h>

namespace bm::themes
{
    namespace
    {
        struct rgb8
        {
            uint8_t r, g, b;
        };

        // Base colour of the seven shapes per theme (8-bit RGB). Every theme keeps the shapes apart in hue
        // and brightness; CONTRAST follows a colour-blind friendly set (distinct for the common deficiencies).
        constexpr rgb8 bases[block_theme_count][7] = {
            { { 40, 200, 232 }, { 248, 208, 40 }, { 176, 72, 232 }, { 72, 216, 80 }, { 240, 64, 72 }, { 56, 104, 240 },
              { 248, 144, 32 } },                                                                   // CLASSIC
            { { 86, 180, 233 }, { 240, 228, 66 }, { 204, 121, 167 }, { 0, 158, 115 }, { 213, 94, 0 }, { 0, 114, 178 },
              { 230, 159, 0 } },                                                                    // CONTRAST
            { { 0, 255, 240 }, { 255, 255, 64 }, { 255, 64, 255 }, { 64, 255, 64 }, { 255, 40, 96 }, { 64, 96, 255 },
              { 255, 150, 0 } },                                                                    // NEON
            { { 150, 225, 245 }, { 250, 235, 150 }, { 210, 170, 245 }, { 165, 235, 175 }, { 250, 165, 175 },
              { 165, 185, 250 }, { 250, 200, 145 } },                                               // PASTEL
            { { 255, 200, 90 }, { 230, 215, 60 }, { 130, 70, 170 }, { 90, 180, 60 }, { 225, 40, 30 }, { 150, 85, 50 },
              { 255, 120, 40 } },                                                                   // CHILI
        };

        rgb8 mix8(rgb8 a, int tr, int tg, int tb, int k100)
        {
            return { uint8_t(a.r + (tr - a.r) * k100 / 100), uint8_t(a.g + (tg - a.g) * k100 / 100),
                     uint8_t(a.b + (tb - a.b) * k100 / 100) };
        }

        bn::color to_color(rgb8 c)
        {
            return bn::color(c.r >> 3, c.g >> 3, c.b >> 3);
        }

        const bn::color frame_metal[frame_theme_count] = {
            bn::color(0, 0, 0), bn::color(24, 25, 27), bn::color(28, 14, 22), bn::color(6, 6, 7), bn::color(27, 21, 6)
        };
        const bn::color frame_accent[frame_theme_count] = {
            bn::color(0, 0, 0), bn::color(10, 22, 31), bn::color(12, 28, 20), bn::color(31, 10, 4), bn::color(31, 31, 20)
        };

        // Title sky gradients (darkest .. lightest) for MIDNIGHT, INFERNO and GOLDEN.
        constexpr rgb8 skies[3][8] = {
            { { 2, 4, 14 }, { 4, 10, 30 }, { 8, 18, 48 }, { 12, 28, 70 }, { 20, 40, 96 }, { 30, 56, 120 },
              { 60, 90, 160 }, { 120, 150, 210 } },
            { { 16, 2, 2 }, { 40, 6, 4 }, { 70, 10, 6 }, { 110, 20, 8 }, { 150, 34, 10 }, { 190, 56, 14 },
              { 230, 100, 30 }, { 255, 170, 80 } },
            { { 20, 12, 2 }, { 48, 30, 6 }, { 84, 54, 10 }, { 126, 84, 16 }, { 170, 120, 24 }, { 210, 160, 40 },
              { 240, 200, 80 }, { 255, 236, 160 } },
        };
    }

    void block_colors(int theme, int shape, bn::color out[5])
    {
        const rgb8 base = bases[theme >= 0 && theme < block_theme_count ? theme : 0][shape >= 0 && shape < 7 ? shape : 0];
        // Same shading as the generated palette (tools/gfx/tiles.py shade_bank).
        out[0] = to_color(mix8(base, 8, 8, 24, 80));
        out[1] = to_color(mix8(base, 16, 16, 40, 45));
        out[2] = to_color(base);
        out[3] = to_color(mix8(base, 255, 255, 255, 35));
        out[4] = to_color(mix8(base, 255, 255, 255, 70));
    }

    bool frame_colors(int theme, bn::color& metal, bn::color& accent)
    {
        if(theme <= 0 || theme >= frame_theme_count)
        {
            return false;
        }
        metal = frame_metal[theme];
        accent = frame_accent[theme];
        return true;
    }

    bool title_sky(int variant, bn::color out[8])
    {
        if(variant <= 0 || variant > 3)
        {
            return false;
        }
        for(int i = 0; i < 8; ++i)
        {
            out[i] = to_color(skies[variant - 1][i]);
        }
        return true;
    }
}
