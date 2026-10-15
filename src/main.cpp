/*
 * BLOCK MAYHEM: THE SPICE PROTOCOL - Game Boy Advance entry point.
 */
#include "bm_adventure_scenes.h"
#include "bm_app.h"
#include "bm_audio.h"
#include "bm_flow.h"
#include "bm_game_scene.h"
#include "bm_menu_scenes.h"
#include "bm_run_scenes.h"

#include "bn_bg_palettes.h"
#include "bn_color.h"
#include "bn_core.h"
#include "bn_sprite_palettes.h"
#include "bn_unique_ptr.h"

namespace
{
    bn::unique_ptr<bm::scene> make_scene(bm::scene_id id)
    {
        switch(id)
        {
        case bm::scene_id::gameplay:
            return bn::unique_ptr<bm::scene>(new bm::game_scene());
        case bm::scene_id::main_menu:
            return bn::unique_ptr<bm::scene>(new bm::main_menu_scene());
        case bm::scene_id::results:
            return bn::unique_ptr<bm::scene>(new bm::results_scene());
        case bm::scene_id::world_map:
            return bn::unique_ptr<bm::scene>(new bm::world_map_scene());
        case bm::scene_id::stage_card:
            return bn::unique_ptr<bm::scene>(new bm::stage_select_scene());
        case bm::scene_id::run_map:
            return bn::unique_ptr<bm::scene>(new bm::run_map_scene());
        case bm::scene_id::upgrade_select:
            return bn::unique_ptr<bm::scene>(new bm::upgrade_scene());
        case bm::scene_id::run_summary:
            return bn::unique_ptr<bm::scene>(new bm::run_summary_scene());
        case bm::scene_id::boss_rush:
            return bn::unique_ptr<bm::scene>(new bm::boss_rush_scene());
        case bm::scene_id::title:
        default:
            return bn::unique_ptr<bm::scene>(new bm::title_scene());
        }
    }

    void set_fade(int step, int steps)
    {
        const bn::fixed k = bn::fixed(step) / steps;
        bn::bg_palettes::set_fade(bn::color(0, 0, 0), k);
        bn::sprite_palettes::set_fade(bn::color(0, 0, 0), k);
    }

    // Diagnostic mailbox commands (emulator test path only; never reachable from the game UI).
    bn::optional<bm::scene_id> handle_diagnostics(bm::app& a)
    {
        const uint32_t cmd = a.pending_command;
        if(! cmd)
        {
            return bn::nullopt;
        }
        a.pending_command = 0;
        a.flow.diagnostic = true;
        switch(cmd)
        {
        case BM_TI_CMD_START_MODE:
            a.flow.mode = bm::game_mode(a.pending_args[0] < uint32_t(bm::game_mode::count) ? a.pending_args[0] : 0);
            a.flow.seed = a.pending_args[1] ? a.pending_args[1] : 1;
            a.flow.autoplay = a.pending_args[2] != 0;
            a.autoplayer.reset(bm::bot_strategy::resource_aware, a.flow.seed);
            if(a.flow.mode == bm::game_mode::chaos_run)
            {
                bm::begin_chaos_run(a.flow.seed);
                return bm::scene_id::run_map;
            }
            if(a.flow.mode == bm::game_mode::boss_rush)
            {
                return bm::begin_boss_rush(a) ? bm::scene_id::boss_rush : bm::scene_id::main_menu;
            }
            return bm::scene_id::gameplay;

        case BM_TI_CMD_START_STAGE:
            a.flow.mode = bm::game_mode::adventure;
            a.flow.world = uint8_t(a.pending_args[0] >= 1 && a.pending_args[0] <= 6 ? a.pending_args[0] : 1);
            a.flow.stage = uint8_t(a.pending_args[1] >= 1 && a.pending_args[1] <= 5 ? a.pending_args[1] : 1);
            a.flow.seed = a.pending_args[2] ? a.pending_args[2] : 1;
            a.flow.autoplay = a.pending_args[3] != 0;
            a.autoplayer.reset(bm::bot_strategy::resource_aware, a.flow.seed);
            return bm::scene_id::gameplay;

        case BM_TI_CMD_UNLOCK_ALL:
            // Diagnostic save image with every world reachable (in memory only, never written by this command).
            a.save.worlds_unlocked = 6;
            a.save.boss_rush_unlocked = 1;
            for(bm::stage_record& r : a.save.stages)
            {
                r.cleared = 1;
            }
            return bn::nullopt;

        case BM_TI_CMD_GOTO_SCENE:
            return bm::scene_id(a.pending_args[0]);

        case BM_TI_CMD_ADD_HEAT:
            a.game->debug_add_heat(int(a.pending_args[0]));
            return bn::nullopt;

        case BM_TI_CMD_SET_BOSS_HP:
            a.game->debug_set_boss_hp(int(a.pending_args[0]));
            return bn::nullopt;

        case BM_TI_CMD_WIPE_SAVE:
            bm::reset_defaults(a.save);
            a.write_save();
            return bn::nullopt;

        default:
            return bn::nullopt;
        }
    }
}

int main()
{
    bn::core::init();
    bn::bg_palettes::set_transparent_color(bn::color(1, 1, 4));

    bm::app& a = bm::the_app();
    a.load_save();
    bm::audio::apply_volumes();

    bn::unique_ptr<bm::scene> current = make_scene(bm::scene_id::title);
    a.test->scene = uint32_t(bm::scene_id::title);
    constexpr int fade_steps = 8;
    int fade_in = fade_steps;

    while(true)
    {
        a.tick();
        bn::optional<bm::scene_id> next = handle_diagnostics(a);
        bn::optional<bm::scene_id> wanted = current->update();
        if(! next)
        {
            next = wanted;
        }
        if(fade_in > 0)
        {
            --fade_in;
            set_fade(fade_in, fade_steps);
        }
        if(next)
        {
            for(int i = 1; i <= fade_steps; ++i)
            {
                set_fade(i, fade_steps);
                bn::core::update();
            }
            current.reset();
            current = make_scene(*next);
            a.test->scene = uint32_t(*next);
            fade_in = fade_steps;
        }
        bn::core::update();
    }
}
