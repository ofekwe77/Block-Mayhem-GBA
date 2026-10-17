/*
 * BLOCK MAYHEM: THE SPICE PROTOCOL - Game Boy Advance entry point.
 */
#include "bm_adventure_scenes.h"
#include "bm_app.h"
#include "bm_audio.h"
#include "bm_customize_scene.h"
#include "bm_challenges.h"
#include "bm_content.h"
#include "bm_cosmetics.h"
#include "bm_flow.h"
#include "bm_game_scene.h"
#include "bm_menu_scenes.h"
#include "bm_extra_scenes.h"
#include "bm_run_scenes.h"
#include "bm_story_scenes.h"

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
        case bm::scene_id::intro:
            return bn::unique_ptr<bm::scene>(new bm::story_scene(bm::story_kind::intro));
        case bm::scene_id::ending:
            return bn::unique_ptr<bm::scene>(new bm::story_scene(bm::story_kind::ending));
        case bm::scene_id::world_intro:
            return bn::unique_ptr<bm::scene>(new bm::story_scene(bm::story_kind::world_intro));
        case bm::scene_id::options:
            return bn::unique_ptr<bm::scene>(new bm::options_scene());
        case bm::scene_id::records:
            return bn::unique_ptr<bm::scene>(new bm::records_scene());
        case bm::scene_id::achievements:
            return bn::unique_ptr<bm::scene>(new bm::achievements_scene());
        case bm::scene_id::credits:
            return bn::unique_ptr<bm::scene>(new bm::credits_scene());
        case bm::scene_id::sound_test:
            return bn::unique_ptr<bm::scene>(new bm::sound_test_scene());
        case bm::scene_id::mode_setup:
            return bn::unique_ptr<bm::scene>(new bm::mode_setup_scene());
        case bm::scene_id::customize:
            return bn::unique_ptr<bm::scene>(new bm::customize_scene());
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

        case BM_TI_CMD_STRESS:
            // 1: pack every board with specials, 2: detonate them (chain reactions + effects).
            for(int b = 0; b < a.game->board_count(); ++b)
            {
                if(a.pending_args[0] == 1)
                {
                    a.game->debug_fill_stress(b);
                }
                else if(a.pending_args[0] == 2)
                {
                    a.game->debug_detonate(b);
                }
            }
            return bn::nullopt;

        case BM_TI_CMD_PLAY_MUSIC:
            bm::audio::play_music(bm::audio::music_id(a.pending_args[0] < uint32_t(bm::audio::music_id::count) ?
                                                      a.pending_args[0] : 1), a.pending_args[2] != 0);
            bm::audio::set_intensity(a.pending_args[1] ? bm::audio::intensity::overheat : bm::audio::intensity::normal);
            return bn::nullopt;

        case BM_TI_CMD_PLAY_SFX:
            bm::audio::sfx(bm::audio::sfx_id(a.pending_args[0] < uint32_t(bm::audio::sfx_id::count) ?
                                             a.pending_args[0] : 0));
            return bn::nullopt;

        case BM_TI_CMD_WIPE_SAVE:
            bm::reset_defaults(a.save);
            a.write_save();
            return bn::nullopt;

        case BM_TI_CMD_SET_COSMETIC:
        {
            // Diagnostic: unlock and select one cosmetic in memory (the next scene applies it).
            const int kind = int(a.pending_args[0] & 3);
            const int index = int(a.pending_args[1]);
            if(index >= 0 && index < bm::cosmetic_count(bm::cosmetic_kind(kind)))
            {
                a.save.cosmetics_unlocked |= bm::cosmetic_bit(bm::cosmetic_kind(kind), index);
                bm::cosmetic_setting(a.save.settings, bm::cosmetic_kind(kind)) = uint8_t(index);
            }
            return bn::nullopt;
        }

        case BM_TI_CMD_SET_CHALLENGES:
        {
            // Diagnostic: the worlds that unlock these challenges count as cleared (in memory only).
            const uint8_t mask = uint8_t(a.pending_args[0] & ((1u << bm::challenge_total) - 1));
            for(int i = 0; i < bm::challenge_total; ++i)
            {
                if((mask >> i) & 1)
                {
                    a.save.stages[bm::stage_number(bm::challenge(i).unlock_world, bm::stages_per_world)].cleared = 1;
                }
            }
            a.save.settings.challenges = mask;
            return bn::nullopt;
        }

        case BM_TI_CMD_SET_LOADOUT:
            // Diagnostic: equip three abilities for Chaos Arcade (unlocked in memory).
            for(int k = 0; k < 3; ++k)
            {
                const uint8_t ab = uint8_t(a.pending_args[k] % bm::cfg::ability_count);
                a.flow.arcade_loadout[k] = ab;
                a.save.abilities_unlocked = uint8_t(a.save.abilities_unlocked | (1u << ab));
            }
            a.flow.arcade_count = 3;
            return bn::nullopt;

        case BM_TI_CMD_SET_OPTIONS:
            a.save.settings.shake = uint8_t(a.pending_args[0] > 2 ? 2 : a.pending_args[0]);
            a.save.settings.flashes = uint8_t(a.pending_args[1] ? 1 : 0);
            a.save.settings.music_volume = uint8_t(a.pending_args[2] > 10 ? 10 : a.pending_args[2]);
            a.save.settings.sfx_volume = uint8_t(a.pending_args[3] > 10 ? 10 : a.pending_args[3]);
            bm::audio::apply_volumes();
            return bn::nullopt;

        case BM_TI_CMD_SET_SECRET_RANK:
            // Diagnostic: show the results screen of the last session again with this stage rank.
            a.flow.rank = uint8_t(a.pending_args[0] > 5 ? 5 : a.pending_args[0]);
            return bm::scene_id::results;

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

    // First launch opens with the intro cutscene; afterwards the title comes straight up.
    const bm::scene_id first = a.save_status == int(bm::save_status::empty) ? bm::scene_id::intro : bm::scene_id::title;
    bn::unique_ptr<bm::scene> current = make_scene(first);
    a.test->scene = uint32_t(first);
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
