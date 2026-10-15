/*
 * BLOCK MAYHEM - boss presentation (PART 23.5, 51.2-51.3).
 *
 * A data-driven animation controller with the states INTRO, IDLE, TELEGRAPH,
 * ATTACK, RECOVER, HIT, PHASE_CHANGE, DEFEAT, FINISHED. Each boss is a rig of
 * sprite parts (roles) whose frames and procedural motion depend on the state.
 * The controller only reacts to gameplay events: combat state lives in the
 * session, so animation can never desynchronise the rules (a defeated boss
 * cannot attack because the session has already stopped).
 */
#ifndef BM_BOSS_VIEW_H
#define BM_BOSS_VIEW_H

#include "bm_events.h"
#include "bm_fx.h"
#include "bm_session.h"

#include "bn_camera_ptr.h"
#include "bn_fixed.h"
#include "bn_optional.h"
#include "bn_sprite_item.h"
#include "bn_sprite_palette_ptr.h"
#include "bn_sprite_ptr.h"

namespace bm
{
    enum class boss_anim : uint8_t
    {
        intro, idle, telegraph, attack, recover, hit, phase_change, defeat, finished
    };

    enum class part_role : uint8_t
    {
        body, eye, mouth, arm, head, crystal, breath, neck, fire, face, orb, bolt, ring, cube, wing, iris,
        tentacle, heart
    };

    struct part_def
    {
        const bn::sprite_item* item;
        part_role role;
        int8_t x;
        int8_t y;
        bool hflip;
        int8_t index;       // which instance of the role (left/right arm, head 0..2, orb 0..3...)
        int8_t z;           // smaller = in front
        bool behind_board;  // drawn behind the board background (huge parts must never cover the board)
    };

    class boss_view
    {
    public:
        static constexpr int max_parts = 14;

        boss_view(int boss_id, const bn::camera_ptr& camera, int center_x, int center_y);

        void on_event(const event& e, fx& effects);
        void update(const session& s, int frame, fx& effects);
        void move_to(int center_x, int center_y, int frames);

        [[nodiscard]] boss_anim state() const { return _state; }
        [[nodiscard]] bool defeat_done() const { return _state == boss_anim::finished; }
        [[nodiscard]] int center_x() const { return _cx.right_shift_integer(); }
        [[nodiscard]] int center_y() const { return _cy.right_shift_integer(); }

    private:
        struct part
        {
            bn::optional<bn::sprite_ptr> sprite;
            part_def def;
            int8_t frames = 1;
            int8_t shown = 0;
        };

        part _parts[max_parts];
        int _count = 0;
        int _id = 1;
        boss_anim _state = boss_anim::intro;
        int _timer = 0;
        int _phase = 0;
        int _attack_kind = 0;
        int _attacking_head = 0;
        bn::fixed _cx;
        bn::fixed _cy;
        bn::fixed _tx;
        bn::fixed _ty;
        int _move_frames = 0;
        int _flash = 0;
        int _intro = 90;
        int _blink = 0;
        uint32_t _seed = 0xBEEF;
        bn::optional<bn::sprite_palette_ptr> _palette;

        void _set_state(boss_anim state, int timer);
        void _apply(int frame);
        int _rand(int n);
    };

    [[nodiscard]] const char* attack_name(int attack_kind);
    [[nodiscard]] const char* hazard_name(int hazard_kind);
}

#endif
