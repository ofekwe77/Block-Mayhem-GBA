/*
 * BLOCK MAYHEM - music and sound effects (Maxmod through Butano).
 *
 * Dynamic music: every world / boss track has a "hot" arrangement with the
 * same structure; Overheat switches to it at the current pattern position
 * (and back), so the music intensifies without restarting.
 */
#ifndef BM_AUDIO_H
#define BM_AUDIO_H

#include <stdint.h>

namespace bm::audio
{
    enum class music_id : uint8_t
    {
        none, title, menu, world1, world2, world3, world4, world5, world6, boss, final_boss, victory, game_over,
        ending, classic, count
    };

    enum class sfx_id : uint8_t
    {
        move, rotate, hold, hard_drop, lock, clear1, clear2, clear3, clear4, bomb, fire, spark, ice, armor,
        magnet, rainbow, laser, freeze, gravity, nuke, shield, vent, shift, chain, chain_big, overheat_start,
        overheat_success, overheat_fail, boss_hit, boss_attack, boss_charge, boss_defeat, stage_clear,
        menu_move, menu_ok, menu_back, unlock, game_over, denied, garbage, danger, level_up, pause,
        switch_board, count
    };

    enum class intensity : uint8_t
    {
        normal, overheat
    };

    void play_music(music_id id, bool loop = true);
    void stop_music();
    void pause_music();
    void resume_music();
    void set_intensity(intensity level);
    void sfx(sfx_id id);
    void apply_volumes();
    [[nodiscard]] music_id current();
    [[nodiscard]] bool track_has_hot_variant(music_id id);
    [[nodiscard]] const char* track_name(music_id id);
    [[nodiscard]] const char* sfx_name(sfx_id id);
}

#endif
