#include "bm_audio.h"

#include "bm_app.h"

#include "bn_fixed.h"
#include "bn_music.h"
#include "bn_music_item.h"
#include "bn_music_items.h"
#include "bn_sound.h"
#include "bn_sound_item.h"
#include "bn_sound_items.h"

namespace bm::audio
{
    namespace
    {
        music_id current_track = music_id::none;
        intensity current_intensity = intensity::normal;
        bool current_loop = true;

        const bn::music_item* item(music_id id, bool hot)
        {
            switch(id)
            {
            case music_id::title: return &bn::music_items::mus_title;
            case music_id::menu: return &bn::music_items::mus_menu;
            case music_id::world1: return hot ? &bn::music_items::mus_w1_hot : &bn::music_items::mus_w1;
            case music_id::world2: return hot ? &bn::music_items::mus_w2_hot : &bn::music_items::mus_w2;
            case music_id::world3: return hot ? &bn::music_items::mus_w3_hot : &bn::music_items::mus_w3;
            case music_id::world4: return hot ? &bn::music_items::mus_w4_hot : &bn::music_items::mus_w4;
            case music_id::world5: return hot ? &bn::music_items::mus_w5_hot : &bn::music_items::mus_w5;
            case music_id::world6: return hot ? &bn::music_items::mus_w6_hot : &bn::music_items::mus_w6;
            case music_id::boss: return hot ? &bn::music_items::mus_boss_hot : &bn::music_items::mus_boss;
            case music_id::final_boss: return hot ? &bn::music_items::mus_final_hot : &bn::music_items::mus_final;
            case music_id::victory: return &bn::music_items::mus_victory;
            case music_id::game_over: return &bn::music_items::mus_gameover;
            case music_id::ending: return &bn::music_items::mus_ending;
            case music_id::classic: return &bn::music_items::mus_classic;
            default: return nullptr;
            }
        }

        const bn::sound_item& sound(sfx_id id)
        {
            switch(id)
            {
            case sfx_id::move: return bn::sound_items::sfx_move;
            case sfx_id::rotate: return bn::sound_items::sfx_rotate;
            case sfx_id::hold: return bn::sound_items::sfx_hold;
            case sfx_id::hard_drop: return bn::sound_items::sfx_hard_drop;
            case sfx_id::lock: return bn::sound_items::sfx_lock;
            case sfx_id::clear1: return bn::sound_items::sfx_clear1;
            case sfx_id::clear2: return bn::sound_items::sfx_clear2;
            case sfx_id::clear3: return bn::sound_items::sfx_clear3;
            case sfx_id::clear4: return bn::sound_items::sfx_clear4;
            case sfx_id::bomb: return bn::sound_items::sfx_bomb;
            case sfx_id::fire: return bn::sound_items::sfx_fire;
            case sfx_id::spark: return bn::sound_items::sfx_spark;
            case sfx_id::ice: return bn::sound_items::sfx_ice;
            case sfx_id::armor: return bn::sound_items::sfx_armor;
            case sfx_id::magnet: return bn::sound_items::sfx_magnet;
            case sfx_id::rainbow: return bn::sound_items::sfx_rainbow;
            case sfx_id::laser: return bn::sound_items::sfx_laser;
            case sfx_id::freeze: return bn::sound_items::sfx_freeze;
            case sfx_id::gravity: return bn::sound_items::sfx_gravity;
            case sfx_id::nuke: return bn::sound_items::sfx_nuke;
            case sfx_id::shield: return bn::sound_items::sfx_shield;
            case sfx_id::vent: return bn::sound_items::sfx_vent;
            case sfx_id::shift: return bn::sound_items::sfx_shift;
            case sfx_id::chain: return bn::sound_items::sfx_chain;
            case sfx_id::chain_big: return bn::sound_items::sfx_chain_big;
            case sfx_id::overheat_start: return bn::sound_items::sfx_overheat_start;
            case sfx_id::overheat_success: return bn::sound_items::sfx_overheat_success;
            case sfx_id::overheat_fail: return bn::sound_items::sfx_overheat_fail;
            case sfx_id::boss_hit: return bn::sound_items::sfx_boss_hit;
            case sfx_id::boss_attack: return bn::sound_items::sfx_boss_attack;
            case sfx_id::boss_charge: return bn::sound_items::sfx_boss_charge;
            case sfx_id::boss_defeat: return bn::sound_items::sfx_boss_defeat;
            case sfx_id::stage_clear: return bn::sound_items::sfx_stage_clear;
            case sfx_id::menu_move: return bn::sound_items::sfx_menu_move;
            case sfx_id::menu_ok: return bn::sound_items::sfx_menu_ok;
            case sfx_id::menu_back: return bn::sound_items::sfx_menu_back;
            case sfx_id::unlock: return bn::sound_items::sfx_unlock;
            case sfx_id::game_over: return bn::sound_items::sfx_game_over;
            case sfx_id::denied: return bn::sound_items::sfx_denied;
            case sfx_id::garbage: return bn::sound_items::sfx_garbage;
            case sfx_id::danger: return bn::sound_items::sfx_danger;
            case sfx_id::level_up: return bn::sound_items::sfx_level_up;
            case sfx_id::pause: return bn::sound_items::sfx_pause;
            case sfx_id::switch_board: return bn::sound_items::sfx_switch_board;
            default: return bn::sound_items::sfx_menu_move;
            }
        }

        // Higher priority sounds are never cut by busy chain reactions.
        int priority(sfx_id id)
        {
            switch(id)
            {
            case sfx_id::move:
            case sfx_id::rotate:
            case sfx_id::lock:
                return -2;
            case sfx_id::boss_defeat:
            case sfx_id::overheat_start:
            case sfx_id::overheat_success:
            case sfx_id::overheat_fail:
            case sfx_id::stage_clear:
            case sfx_id::game_over:
            case sfx_id::clear4:
            case sfx_id::nuke:
                return 4;
            case sfx_id::bomb:
            case sfx_id::laser:
            case sfx_id::chain_big:
            case sfx_id::boss_attack:
            case sfx_id::danger:
                return 2;
            default:
                return 0;
            }
        }

        bn::fixed sfx_gain(sfx_id id)
        {
            switch(id)
            {
            case sfx_id::move: return bn::fixed(0.35);
            case sfx_id::rotate: return bn::fixed(0.4);
            case sfx_id::lock: return bn::fixed(0.5);
            default: return 1;
            }
        }
    }

    void play_music(music_id id, bool loop)
    {
        app& a = the_app();
        if(id == current_track && bn::music::playing() && loop == current_loop)
        {
            return;
        }
        current_track = id;
        current_loop = loop;
        current_intensity = intensity::normal;
        const bn::music_item* it = item(id, false);
        if(! it)
        {
            if(bn::music::playing())
            {
                bn::music::stop();
            }
            a.test->music_id = 0;
            return;
        }
        it->play(a.music_volume(), loop);
        a.test->music_id = uint32_t(id);
    }

    void stop_music()
    {
        if(bn::music::playing())
        {
            bn::music::stop();
        }
        current_track = music_id::none;
        the_app().test->music_id = 0;
    }

    void pause_music()
    {
        if(bn::music::playing() && ! bn::music::paused())
        {
            bn::music::pause();
        }
    }

    void resume_music()
    {
        if(bn::music::playing() && bn::music::paused())
        {
            bn::music::resume();
        }
    }

    bool track_has_hot_variant(music_id id)
    {
        return item(id, true) != item(id, false);
    }

    void set_intensity(intensity level)
    {
        if(level == current_intensity || current_track == music_id::none || ! track_has_hot_variant(current_track))
        {
            current_intensity = level;
            return;
        }
        current_intensity = level;
        const int position = bn::music::playing() ? bn::music::position() : 0;
        const bn::music_item* it = item(current_track, level == intensity::overheat);
        it->play(the_app().music_volume(), true);
        bn::music::set_position(position);
    }

    void sfx(sfx_id id)
    {
        app& a = the_app();
        const bn::fixed volume = a.sfx_volume() * sfx_gain(id);
        if(volume <= 0)
        {
            return;
        }
        bn::sound::play_with_priority(priority(id), sound(id), volume);
        ++a.test->sfx_count;
    }

    void apply_volumes()
    {
        if(bn::music::playing())
        {
            bn::music::set_volume(the_app().music_volume());
        }
    }

    music_id current()
    {
        return current_track;
    }

    const char* sfx_name(sfx_id id)
    {
        static constexpr const char* names[int(sfx_id::count)] = {
            "MOVE", "ROTATE", "HOLD", "HARD DROP", "LOCK", "SINGLE", "DOUBLE", "TRIPLE", "QUAD BLAST", "BOMB",
            "FIRE", "SPARK", "ICE", "ARMOR", "MAGNET", "RAINBOW", "LASER BEAM", "TIME FREEZE", "GRAVITY PULSE",
            "NUCLEAR BURST", "SHIELD MATRIX", "EMERGENCY VENT", "SHAPE SHIFT", "CHAIN", "BIG CHAIN",
            "OVERHEAT", "OVERHEAT SUCCESS", "OVERHEAT FAIL", "BOSS HIT", "BOSS ATTACK", "BOSS CHARGE",
            "BOSS DEFEAT", "STAGE CLEAR", "MENU MOVE", "MENU OK", "MENU BACK", "UNLOCK", "GAME OVER", "DENIED",
            "GARBAGE", "DANGER", "LEVEL UP", "PAUSE", "BOARD SWITCH"
        };
        const int i = int(id);
        return i >= 0 && i < int(sfx_id::count) ? names[i] : "";
    }

    const char* track_name(music_id id)
    {
        switch(id)
        {
        case music_id::title: return "IGNITION THEME";
        case music_id::menu: return "CONTROL ROOM";
        case music_id::world1: return "ASSEMBLY LINE RIOT";
        case music_id::world2: return "CRYOSTASIS";
        case music_id::world3: return "MAGMA RUSH";
        case music_id::world4: return "NEON OVERCLOCK";
        case music_id::world5: return "EVENT HORIZON";
        case music_id::world6: return "TOTAL MELTDOWN";
        case music_id::boss: return "MECHANICAL MENACE";
        case music_id::final_boss: return "CORE OF MAYHEM";
        case music_id::victory: return "STAGE CLEAR";
        case music_id::game_over: return "SHUTDOWN";
        case music_id::ending: return "AFTERGLOW";
        case music_id::classic: return "QUIET ASSEMBLY";
        default: return "";
        }
    }
}
