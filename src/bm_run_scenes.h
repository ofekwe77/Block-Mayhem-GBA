/*
 * BLOCK MAYHEM - CHAOS RUN screens: run map, upgrade selection, run summary.
 */
#ifndef BM_RUN_SCENES_H
#define BM_RUN_SCENES_H

#include "bm_backdrop.h"
#include "bm_menu_scenes.h"

namespace bm
{
    // Shows the whole run (three acts of encounter nodes + bosses) and the next encounter in detail.
    class run_map_scene : public scene
    {
    public:
        run_map_scene();
        [[nodiscard]] bn::optional<scene_id> update() override;

    private:
        backdrop _backdrop;
        menu_input _input;
        label _title;
        label _score;
        label _acts[3];
        label _boss_names[3];
        label _name;
        label _idea;
        label _goals[3];
        label _hazard;
        label _info;
        label _hint;
        bn::optional<bn::sprite_ptr> _nodes[12];
        bn::optional<bn::sprite_ptr> _goal_icons[3];
        bn::optional<bn::sprite_ptr> _loadout[3];
        bn::optional<bn::sprite_ptr> _cursor;
        bool _confirm = false;
        int _frame = 0;

        void _refresh();
        void _show_hint();
    };

    // Three upgrade cards after every cleared encounter.
    class upgrade_scene : public scene
    {
    public:
        upgrade_scene();
        ~upgrade_scene() override;
        [[nodiscard]] bn::optional<scene_id> update() override;

    private:
        backdrop _backdrop;
        menu_input _input;
        label _title;
        label _names[3];
        label _levels[3];
        label _desc[3][2];
        label _extra[3];
        label _hint;
        bn::optional<bn::sprite_ptr> _icons[3];
        bn::optional<bn::sprite_ptr> _sparkles[4];
        bn::optional<bn::sprite_ptr> _cursor;
        int _index = 0;
        int _frame = 0;

        void _refresh();
    };

    // Victory / defeat summary with the final build.
    class run_summary_scene : public scene
    {
    public:
        run_summary_scene();
        [[nodiscard]] bn::optional<scene_id> update() override;

    private:
        backdrop _backdrop;
        menu_input _input;
        label _title;
        label _lines[6];
        label _synergies;
        label _hint;
        label _stack_labels[12];
        bn::optional<bn::sprite_ptr> _build_icons[24];
        int _frame = 0;
    };

    // Starts a fresh run with the given seed and counts it in the save.
    void begin_chaos_run(uint32_t seed);
}

#endif
