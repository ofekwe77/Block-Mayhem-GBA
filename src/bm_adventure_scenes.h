/*
 * BLOCK MAYHEM - Adventure mode screens: world map and stage select / stage card.
 */
#ifndef BM_ADVENTURE_SCENES_H
#define BM_ADVENTURE_SCENES_H

#include "bm_backdrop.h"
#include "bm_menu_scenes.h"

namespace bm
{
    class world_map_scene : public scene
    {
    public:
        world_map_scene();
        [[nodiscard]] bn::optional<scene_id> update() override;

    private:
        backdrop _backdrop;
        menu_input _input;
        label _title;
        label _world;
        label _info;
        label _progress;
        label _hint;
        bn::optional<bn::sprite_ptr> _cursor;
        bn::optional<bn::sprite_ptr> _locks[6];
        bn::optional<bn::sprite_ptr> _stars[6];
        int _selected = 0;
        int _frame = 0;
        int _celebrate = 0;

        void _refresh();
    };

    // BOSS RUSH: the card before every fight, and the final summary once the rush is over.
    class boss_rush_scene : public scene
    {
    public:
        boss_rush_scene();
        [[nodiscard]] bn::optional<scene_id> update() override;

    private:
        backdrop _backdrop;
        menu_input _input;
        label _title;
        label _lines[6];
        label _hint;
        bn::optional<bn::sprite_ptr> _nodes[6];
        bn::optional<bn::sprite_ptr> _cursor;
        bool _confirm = false;
        int _frame = 0;

        void _show_hint();
    };

    class stage_select_scene : public scene
    {
    public:
        stage_select_scene();
        [[nodiscard]] bn::optional<scene_id> update() override;

    private:
        backdrop _backdrop;
        menu_input _input;
        label _title;
        label _rows[5];
        label _idea;
        label _goals[3];
        label _extra;
        label _best;
        label _hint;
        bn::optional<bn::sprite_ptr> _icons[3];
        bn::optional<bn::sprite_ptr> _goal_icons[4];
        int _selected = 0;
        int _frame = 0;

        void _refresh();
    };
}

#endif
