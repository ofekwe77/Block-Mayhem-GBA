/*
 * BLOCK MAYHEM - story presentation (PART 36): the intro cutscene, world
 * introduction cards and the final ending. Every sequence is short and can be
 * skipped with START (A advances a beat).
 */
#ifndef BM_STORY_SCENES_H
#define BM_STORY_SCENES_H

#include "bm_backdrop.h"
#include "bm_boss_view.h"
#include "bm_fx.h"
#include "bm_menu_scenes.h"

#include "bn_camera_ptr.h"

namespace bm
{
    enum class story_kind : uint8_t
    {
        intro, world_intro, ending
    };

    class story_scene : public scene
    {
    public:
        explicit story_scene(story_kind kind);
        ~story_scene() override;
        [[nodiscard]] bn::optional<scene_id> update() override;

    private:
        static constexpr int rain_count = 10;

        story_kind _kind;
        bn::camera_ptr _camera;
        bn::optional<backdrop> _backdrop;
        backdrop_id _backdrop_id = backdrop_id::world1;
        fx _fx;
        bn::optional<boss_view> _boss;
        menu_input _input;
        label _big;
        label _sub;
        label _lines[4];
        label _hint;
        bn::optional<bn::sprite_ptr> _rain[rain_count];
        int _rain_y[rain_count] = {};
        int _beat = -1;
        int _beat_timer = 0;
        int _frame = 0;
        int _typed = 0;             // typewriter progress of the current beat's lines
        bool _raining = false;
        bool _alarm = false;

        void _set_backdrop(backdrop_id id, bn::fixed dim);
        void _start_beat(int beat);
        void _type_lines();
        [[nodiscard]] int _beat_count() const;
        [[nodiscard]] int _beat_length(int beat) const;
        [[nodiscard]] scene_id _next_scene() const;
    };
}

#endif
