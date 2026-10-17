/*
 * BLOCK MAYHEM - CUSTOMIZE screen (PART 35.4): block theme, board frame, title-screen variation
 * and particle style, with a live preview built from the real board tiles and theme colours.
 * Locked entries show how they are earned. Reached from OPTIONS.
 */
#ifndef BM_CUSTOMIZE_SCENE_H
#define BM_CUSTOMIZE_SCENE_H

#include "bm_backdrop.h"
#include "bm_menu_scenes.h"

#include "bn_regular_bg_map_cell.h"
#include "bn_regular_bg_map_ptr.h"
#include "bn_regular_bg_ptr.h"
#include "bn_unique_ptr.h"

namespace bm
{
    class customize_scene : public scene
    {
    public:
        customize_scene();
        ~customize_scene() override;
        [[nodiscard]] bn::optional<scene_id> update() override;

    private:
        static constexpr int rows = 4;

        struct preview_cells
        {
            alignas(4) bn::regular_bg_map_cell cells[32 * 32];
        };

        struct spark
        {
            bn::optional<bn::sprite_ptr> sprite;
            int x = 0;
            int y = 0;
            int vx = 0;         // 1/16 pixel units
            int vy = 0;
            int life = 0;
        };

        backdrop _backdrop;
        menu_input _input;
        label _title;
        label _names[rows];
        label _values[rows];
        label _info[2];
        label _hint;
        label _preview_label;
        bn::unique_ptr<preview_cells> _cells;
        bn::optional<bn::regular_bg_ptr> _preview;
        bn::optional<bn::regular_bg_map_ptr> _map;
        spark _sparks[6];
        int8_t _choice[rows] = {};      // highlighted entry per row (may be locked)
        int _index = 0;
        int _frame = 0;

        void _refresh();
        void _change(int delta);
        void _draw_preview();
        void _apply_colors();
        void _update_sparks();
    };
}

#endif
