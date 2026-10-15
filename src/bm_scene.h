/*
 * BLOCK MAYHEM - scene interface and identifiers.
 */
#ifndef BM_SCENE_H
#define BM_SCENE_H

#include "bn_optional.h"

namespace bm
{
    enum class scene_id : uint8_t
    {
        none,
        boot,
        intro,
        title,
        main_menu,
        world_map,
        stage_card,
        gameplay,
        results,
        options,
        records,
        achievements,
        upgrade_select,
        run_summary,
        credits,
        ending,
        sound_test,
        mode_setup,
        diagnostic
    };

    class scene
    {
    public:
        virtual ~scene() = default;

        // Advances one frame. Returns the next scene to switch to, if any.
        [[nodiscard]] virtual bn::optional<scene_id> update() = 0;
    };
}

#endif
