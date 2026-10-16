/*
 * BLOCK MAYHEM - sprite helpers.
 */
#ifndef BM_SPRITE_UTIL_H
#define BM_SPRITE_UTIL_H

#include "bn_sprite_item.h"
#include "bn_sprite_ptr.h"

namespace bm
{
    // Swaps a sprite's tiles only when its animation frame changes: looking tiles up by item and index
    // searches the engine's tile cache, which is too slow to repeat for many sprites every frame.
    inline void set_frame(bn::sprite_ptr& sprite, const bn::sprite_item& item, int index, int& shown)
    {
        if(index != shown)
        {
            sprite.set_tiles(item.tiles_item(), index);
            shown = index;
        }
    }
}

#endif
