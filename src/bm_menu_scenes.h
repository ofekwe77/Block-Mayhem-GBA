/*
 * BLOCK MAYHEM - title, main menu and results scenes.
 */
#ifndef BM_MENU_SCENES_H
#define BM_MENU_SCENES_H

#include "bm_app.h"
#include "bm_backdrop.h"
#include "bm_scene.h"
#include "bm_text.h"

#include "bn_optional.h"
#include "bn_sprite_ptr.h"

namespace bm
{
    // Shared menu input with hold-to-repeat and a short input lock after a scene change
    // (a button held through a transition never confirms the next menu).
    class menu_input
    {
    public:
        void update();
        [[nodiscard]] bool up() const { return _up; }
        [[nodiscard]] bool down() const { return _down; }
        [[nodiscard]] bool left() const { return _left; }
        [[nodiscard]] bool right() const { return _right; }
        [[nodiscard]] bool ok() const { return _ok; }
        [[nodiscard]] bool back() const { return _back; }
        [[nodiscard]] bool start() const { return _start; }

    private:
        int _lock = 12;
        int _repeat = 0;
        bool _up = false, _down = false, _left = false, _right = false, _ok = false, _back = false, _start = false;
    };

    class title_scene : public scene
    {
    public:
        title_scene();
        [[nodiscard]] bn::optional<scene_id> update() override;

    private:
        backdrop _backdrop;
        label _press;
        label _version;
        label _variant_label;
        label _spicy;
        menu_input _input;
        bn::optional<bn::sprite_ptr> _blocks[6];
        int _block_y[6] = {};
        int _frame = 0;
        int _variant = 0;               // title-screen cosmetic (0 FACTORY, 1 MIDNIGHT, 2 INFERNO, 3 GOLDEN)

        // Easter egg (PART 35.3): L R L R SELECT makes it rain chili peppers.
        static constexpr int chili_count = 10;
        bn::optional<bn::sprite_ptr> _chilies[chili_count];
        int _chili_x[chili_count] = {};
        int _chili_y[chili_count] = {};
        int _chili_timer = 0;
        int _code_step = 0;

        void _update_secret_code();
    };

    class main_menu_scene : public scene
    {
    public:
        main_menu_scene();
        [[nodiscard]] bn::optional<scene_id> update() override;

    private:
        struct entry
        {
            const char* name;
            const char* info;
            game_mode mode;
            scene_id target;
            bool locked;
            const char* lock_reason;
        };

        backdrop _backdrop;
        label _title;
        label _items[9];
        label _info;
        menu_input _input;
        entry _entries[9];
        int _count = 0;
        int _index = 0;
        int _frame = 0;

        void _refresh();
    };

    class results_scene : public scene
    {
    public:
        results_scene();
        [[nodiscard]] bn::optional<scene_id> update() override;

    private:
        backdrop _backdrop;
        label _title;
        label _lines[9];
        label _hint;
        label _unlock;                  // challenge / cosmetic rewards of this session (PART 27.3)
        menu_input _input;
        int _frame = 0;

        // Secret (PART 35.1): an S+ rank sets off fireworks behind the results and the rank shimmers.
        static constexpr int spark_count = 24;
        bn::optional<bn::sprite_ptr> _sparks[spark_count];
        int16_t _spark_x[spark_count] = {};
        int16_t _spark_y[spark_count] = {};
        int16_t _spark_vx[spark_count] = {};
        int16_t _spark_vy[spark_count] = {};
        int16_t _spark_life[spark_count] = {};
        bool _fireworks = false;
        int _rank_line = -1;
        bn::string<24> _rank_text;

        void _update_fireworks();
    };
}

#endif
