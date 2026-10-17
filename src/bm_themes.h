/*
 * BLOCK MAYHEM - colour data of the unlockable cosmetics (PART 35.4), shared by the board
 * renderer and the CUSTOMIZE preview so both always show the same colours.
 */
#ifndef BM_THEMES_H
#define BM_THEMES_H

#include "bn_color.h"

namespace bm::themes
{
    constexpr int block_theme_count = 5;
    constexpr int frame_theme_count = 5;

    // Block palette bank entries 1..5 (outline, shadow, base, light, highlight) of shape 0..6 (I O T S Z J L)
    // in a block theme. Theme 0 returns the generated default colours.
    void block_colors(int theme, int shape, bn::color out[5]);

    // Frame metal and accent of a frame theme. Theme 0 (WORLD) returns false: the world's own colours apply.
    bool frame_colors(int theme, bn::color& metal, bn::color& accent);

    // Sky colours 1..8 of the title backdrop's first bank for a title variant (0 = keep the original).
    bool title_sky(int variant, bn::color out[8]);
}

#endif
