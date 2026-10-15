/*
 * BLOCK MAYHEM - menu extras: options, records, achievements, credits, sound test
 * and the ability loadout setup for Chaos Arcade / Dual Board.
 */
#ifndef BM_EXTRA_SCENES_H
#define BM_EXTRA_SCENES_H

#include "bm_backdrop.h"
#include "bm_menu_scenes.h"

namespace bm
{
    class options_scene : public scene
    {
    public:
        options_scene();
        ~options_scene() override;
        [[nodiscard]] bn::optional<scene_id> update() override;

    private:
        static constexpr int rows = 8;

        backdrop _backdrop;
        menu_input _input;
        label _title;
        label _names[rows];
        label _values[rows];
        label _bars[rows];
        label _info;
        label _hint;
        int _index = 0;
        int _frame = 0;

        void _refresh();
        void _change(int delta);
    };

    class records_scene : public scene
    {
    public:
        records_scene();
        [[nodiscard]] bn::optional<scene_id> update() override;

    private:
        static constexpr int pages = 5;
        static constexpr int lines = 12;

        backdrop _backdrop;
        menu_input _input;
        label _title;
        label _page;
        label _left[lines];
        label _right[lines];
        label _hint;
        int _index = 0;

        void _refresh();
    };

    class achievements_scene : public scene
    {
    public:
        achievements_scene();
        [[nodiscard]] bn::optional<scene_id> update() override;

    private:
        static constexpr int visible = 8;

        backdrop _backdrop;
        menu_input _input;
        label _title;
        label _names[visible];
        label _progress[visible];
        label _desc;
        label _hint;
        bn::optional<bn::sprite_ptr> _icons[visible];
        bn::optional<bn::sprite_ptr> _cursor;
        int _index = 0;
        int _top = 0;

        void _refresh();
    };

    class credits_scene : public scene
    {
    public:
        credits_scene();
        [[nodiscard]] bn::optional<scene_id> update() override;

    private:
        static constexpr int slots = 16;

        backdrop _backdrop;
        menu_input _input;
        label _lines[slots];
        label _the_end;
        int _line_of_slot[slots] = {};
        int _scroll = 0;            // pixels scrolled (x2 for half-speed)
        int _frame = 0;

        void _place();
    };

    class sound_test_scene : public scene
    {
    public:
        sound_test_scene();
        ~sound_test_scene() override;
        [[nodiscard]] bn::optional<scene_id> update() override;

    private:
        backdrop _backdrop;
        menu_input _input;
        label _title;
        label _music;
        label _music_name;
        label _sfx;
        label _sfx_name;
        label _info;
        label _hint;
        int _row = 0;
        int _track = 1;
        int _effect = 0;

        void _refresh();
    };

    // Ability loadout before Chaos Arcade (three abilities) or Dual Board (one ability).
    class mode_setup_scene : public scene
    {
    public:
        mode_setup_scene();
        [[nodiscard]] bn::optional<scene_id> update() override;

    private:
        backdrop _backdrop;
        menu_input _input;
        label _title;
        label _names[8];
        label _info[2];
        label _hint;
        bn::optional<bn::sprite_ptr> _icons[8];
        bn::optional<bn::sprite_ptr> _cursor;
        label _slots[8];
        uint8_t _chosen[3] = {};
        int _chosen_count = 0;
        int _index = 0;
        int _frame = 0;

        [[nodiscard]] int _max_slots() const;
        void _refresh();
    };
}

#endif
