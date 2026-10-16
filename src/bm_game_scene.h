/*
 * BLOCK MAYHEM - gameplay scene (all modes share it; layout depends on the mode).
 */
#ifndef BM_GAME_SCENE_H
#define BM_GAME_SCENE_H

#include "bm_app.h"
#include "bm_backdrop.h"
#include "bm_board_view.h"
#include "bm_boss_view.h"
#include "bm_fx.h"
#include "bm_scene.h"
#include "bm_text.h"

#include "bn_camera_ptr.h"
#include "bn_optional.h"
#include "bn_sprite_ptr.h"

namespace bm
{
    class game_scene : public scene
    {
    public:
        game_scene();
        ~game_scene() override;

        [[nodiscard]] bn::optional<scene_id> update() override;

    private:
        app& _app;
        session& _s;
        bn::camera_ptr _camera;
        backdrop _backdrop;
        board_view _view;
        fx _fx;

        label _hold_label;
        label _next_label;
        label _score_title;
        glyph_label _score_value;      // changes constantly: pooled glyphs, never regenerated
        label _level_title;
        label _level_value;
        label _lines_title;
        label _lines_value;
        label _heat_title;
        label _heat_name;
        label _status;
        label _objective[3];
        label _ability_cost[3];
        label _target_hint;
        label _center_text;
        label _center_sub;
        label _pause_items[6];
        label _warning;
        label _boss_name;
        label _tip;
        label _switch_label;
        label _settle;
        label _ach_title;
        label _ach_name;
        bn::optional<bn::sprite_ptr> _ach_icon;
        int _ach_timer = 0;
        int _ach_icon_x = 0;
        int _ach_current = -1;
        bn::optional<boss_view> _boss;
        int _tip_index = 0;
        int _tip_timer = 0;
        bool _board_b_shown = true;
        bn::optional<bn::sprite_ptr> _icons[3];
        bn::optional<bn::sprite_ptr> _goal_icons[3];
        bn::optional<bn::sprite_ptr> _icon_frame;
        bn::optional<bn::sprite_ptr> _cursor;
        bn::optional<bn::sprite_ptr> _marker;
        bn::optional<bn::sprite_ptr> _arrow;
        int _icon_frame_shown = -1;
        int _cursor_shown = -1;
        int _marker_shown = -1;
        int _arrow_shown = -1;

        int _frame = 0;
        int _events_handled = 0;
        // Purely cosmetic events (one per destroyed cell, per triggered special...) are drained at a fixed
        // rate so a huge chain spreads its particles and sounds over a few frames instead of one.
        static constexpr int cosmetic_capacity = 96;
        static constexpr int cosmetic_per_frame = 16;
        event _cosmetic[cosmetic_capacity];
        int _cosmetic_head = 0;
        int _cosmetic_count = 0;
        int _boss_fallback = 0;
        bool _paused = false;
        int _pause_index = 0;
        bool _confirm_quit = false;
        bool _show_controls = false;
        int _end_timer = 0;
        bool _end_recorded = false;
        uint32_t _shown_score = 0xFFFFFFFFu;
        int _shown_level = -1;
        int _shown_lines = -1;
        int _freeze_tint = 0;

        void _start_session();
        void _handle_events();
        void _handle_event(const event& e);
        void _handle_event_body(const event& e);
        void _update_hud();
        void _update_achievements();
        static void _show_middle(label& l, int x, int y, const bn::string_view& t, text_color c, bool tiny);
        void _update_targeting_sprites();
        [[nodiscard]] bn::optional<scene_id> _update_pause();
        void _show_pause(bool show);
        void _cell_center(int board, int x, int y, int& sx, int& sy) const;
    };
}

#endif
