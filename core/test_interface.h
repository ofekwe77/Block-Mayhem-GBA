/*
 * BLOCK MAYHEM - emulator test interface.
 *
 * The ROM keeps one instance of this struct in RAM. The headless mGBA harness
 * (tools/harness) finds it by scanning for the magic string, reads the status
 * fields to verify what the game is doing, and can post diagnostic commands
 * through the mailbox (cmd/arg0..arg3 -> ack). Diagnostic commands are a test
 * path only: they are never reachable from the normal game UI.
 *
 * Plain C so the harness (C) and the ROM (C++) share one definition.
 */
#ifndef BM_TEST_INTERFACE_H
#define BM_TEST_INTERFACE_H

#include <stdint.h>

#define BM_TI_MAGIC "BMTESTIF"
#define BM_TI_VERSION 2u

/* X-list of every uint32 status field, in memory order. */
#define BM_TI_FIELDS(X) \
    X(frame)            /* frames since boot */ \
    X(scene)            /* bm::scene_id */ \
    X(cpu_last)         /* CPU usage of last frame, per mille */ \
    X(cpu_max)          /* max CPU usage since last reset, per mille */ \
    X(cpu_avg)          /* average CPU usage since last reset, per mille */ \
    X(cpu_frames)       /* frames accumulated into cpu_avg */ \
    X(cpu_overruns)     /* frames that needed more than one frame of CPU since the last reset */ \
    X(mode)             /* bm::game_mode of the active session */ \
    X(stage)            /* world*10+stage of the active session (0 if none) */ \
    X(session_state)    /* bm::session_state */ \
    X(score) \
    X(lines) \
    X(level) \
    X(pieces)           /* locked pieces */ \
    X(heat) \
    X(overheat)         /* 1 while Overheat is active */ \
    X(overheat_success) \
    X(overheat_fail) \
    X(max_chain) \
    X(bombs)            /* bombs triggered */ \
    X(boss_hp) \
    X(boss_max_hp) \
    X(boss_phase) \
    X(boss_attacks)     /* boss attacks executed */ \
    X(objective_done)   /* 1 when the stage objective is complete */ \
    X(stack_height)     /* tallest column of the active board */ \
    X(active_board) \
    X(music_id)         /* current music track id (+1, 0 = none) */ \
    X(sfx_count)        /* sound effects started since boot */ \
    X(save_writes)      /* successful SRAM writes since boot */ \
    X(save_loaded)      /* 1 = valid save loaded at boot, 2 = fresh/corrupt reset */ \
    X(worlds_unlocked) \
    X(stages_cleared) \
    X(achievements)     /* unlocked achievement count */ \
    X(autoplay)         /* 1 while the diagnostic autoplay bot drives input */ \
    X(error_code)       /* non-zero if the ROM detected an internal error */ \
    X(run_phase)        /* bm::run_phase of the Chaos Run */ \
    X(run_index)        /* current Chaos Run encounter 0..11 */ \
    X(run_cleared)      /* Chaos Run encounters cleared */ \
    X(run_score) \
    X(run_upgrades)     /* distinct upgrades owned in the current run */ \
    X(rush_index)       /* Boss Rush: bosses defeated so far */ \
    X(rush_state)       /* Boss Rush: 0 fighting, 1 complete, 2 over */ \
    X(rush_frames)      /* Boss Rush: total fight time */ \
    X(save_sequence)    /* sequence number of the loaded / last written save */ \
    X(setting_difficulty) \
    X(sprites_max)      /* most sprite items alive in one frame since the last stats reset */ \
    X(sprite_tiles_max) /* most sprite VRAM tiles in use since the last stats reset (of 1024) */ \
    X(sprite_tile_items_max)/* most sprite tile sets alive (engine limit 128, exceeding it is fatal) */ \
    X(prof_sim)         /* gameplay profiling: worst per-mille of a frame per section since the reset */ \
    X(prof_events) \
    X(prof_view) \
    X(prof_backdrop) \
    X(prof_fx) \
    X(prof_boss) \
    X(prof_hud) \
    X(prof_events_n)    /* events handled in that worst frame */ \
    X(prof_texts)       /* text labels generated in that worst frame */ \
    X(prof_text_cost)   /* per mille of that frame spent generating text */ \
    X(prof_view_boards) /* board view sub-sections of that frame: board cells */ \
    X(prof_view_marks)  /* telegraphs + boss HP bar */ \
    X(prof_view_panels) /* hold / next previews + heat meter */ \
    X(prof_view_palette)/* animated palette */ \
    X(prof_view_upload) /* map upload request */ \
    X(prof_ev_type1)    /* most expensive event type in that frame (bm::event_type) */ \
    X(prof_ev_cost1)    /* its total cost, per mille */ \
    X(prof_ev_type2) \
    X(prof_ev_cost2) \
    X(prof_text_max)    /* the most expensive single text generation of that frame, per mille */ \
    X(prof_text_font)   /* its font (0 small, 1 big, 2 tiny) */ \
    X(prof_text_len)    /* its length in characters */ \
    X(cross_combos)     /* dual board cross-over combos of the active session */ \
    X(trail_draws)      /* hard-drop trail frames drawn since boot (presentation check) */ \
    X(bridge_draws)     /* energy bridge pulse frames drawn since boot (presentation check) */ \
    X(secrets_seen)     /* secret events shown since boot (bit per bm::secret_bit) */ \
    X(secrets_found)    /* save_data::secrets_found */ \
    X(cosmetics)        /* save_data::cosmetics_unlocked */ \
    X(challenges)       /* challenge modifiers of the active session (bit per bm::challenge_id) */ \
    X(text_failures)    /* texts shown without their colour because every sprite palette was taken */ \
    X(session_frames)   /* frames the active session has simulated (stops while paused) */ \
    X(selected_slot)    /* ability slot selected in the loadout (SELECT cycles) */ \
    X(abilities_unlocked) /* save_data::abilities_unlocked */ \
    X(shake_max)        /* strongest screen shake applied since the stats reset (0 = shake off) */ \
    X(flash_max)        /* strongest screen flash applied since the stats reset, percent */ \
    X(hitstop_max)      /* longest hitstop (frames the simulation pauses on impact) since the stats reset */ \
    X(loss_cause)       /* bm::loss_cause of the active session (1 top out, 2 crushed by garbage, 3 objective) */ \
    X(cmd)              /* mailbox: command id written by the harness */ \
    X(arg0) \
    X(arg1) \
    X(arg2) \
    X(arg3) \
    X(ack)              /* mailbox: ROM copies cmd here when processed */

struct bm_test_interface
{
    char magic[8];
    uint32_t version;
#define BM_TI_DECLARE_FIELD(name) uint32_t name;
    BM_TI_FIELDS(BM_TI_DECLARE_FIELD)
#undef BM_TI_DECLARE_FIELD
};

/* Mailbox command ids (diagnostic test paths). */
enum bm_ti_command
{
    BM_TI_CMD_NONE = 0,
    BM_TI_CMD_START_STAGE = 1,     /* arg0 = world (1..6), arg1 = stage (1..5), arg2 = seed, arg3 = autoplay */
    BM_TI_CMD_START_MODE = 2,      /* arg0 = bm::game_mode, arg1 = seed, arg2 = autoplay */
    BM_TI_CMD_SET_AUTOPLAY = 3,    /* arg0 = 0/1 */
    BM_TI_CMD_GOTO_SCENE = 4,      /* arg0 = bm::scene_id */
    BM_TI_CMD_UNLOCK_ALL = 5,      /* diagnostic save state with everything unlocked */
    BM_TI_CMD_RESET_CPU_STATS = 6,
    BM_TI_CMD_ADD_HEAT = 7,        /* arg0 = amount */
    BM_TI_CMD_SET_BOSS_HP = 8,     /* arg0 = hp */
    BM_TI_CMD_STRESS = 9,          /* arg0 = stress scenario id */
    BM_TI_CMD_WIPE_SAVE = 10,
    BM_TI_CMD_PLAY_MUSIC = 11,     /* arg0 = music id (1..14), arg1 = 1 for the Overheat arrangement, arg2 = loop */
    BM_TI_CMD_PLAY_SFX = 12,       /* arg0 = sound effect id */
    BM_TI_CMD_SET_COSMETIC = 13,   /* arg0 = cosmetic kind (0 blocks 1 frame 2 title 3 particles), arg1 = index
                                      (unlocks it in RAM for the screenshot tests) */
    BM_TI_CMD_SET_CHALLENGES = 14, /* arg0 = challenge mask (unlocks those challenges in RAM) */
    BM_TI_CMD_SET_SECRET_RANK = 15, /* arg0 = rank shown by the next results screen (5 = S+) */
    BM_TI_CMD_SET_LOADOUT = 16,    /* arg0..arg2 = Chaos Arcade abilities (unlocked in RAM) */
    BM_TI_CMD_SET_OPTIONS = 17     /* arg0 = shake (0-2), arg1 = flashes (0-1), arg2 = music vol, arg3 = sfx vol (0-10) */
};

#endif
