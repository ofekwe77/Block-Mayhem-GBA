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
    X(mode)             /* bm::game_mode of the active session */ \
    X(stage)            /* world*10+stage of the active session (0 if none) */ \
    X(session_state)    /* bm::session_state */ \
    X(score)            \
    X(lines)            \
    X(level)            \
    X(pieces)           /* locked pieces */ \
    X(heat)             \
    X(overheat)         /* 1 while Overheat is active */ \
    X(overheat_success) \
    X(overheat_fail)    \
    X(max_chain)        \
    X(bombs)            /* bombs triggered */ \
    X(boss_hp)          \
    X(boss_max_hp)      \
    X(boss_phase)       \
    X(boss_attacks)     /* boss attacks executed */ \
    X(objective_done)   /* 1 when the stage objective is complete */ \
    X(stack_height)     /* tallest column of the active board */ \
    X(active_board)     \
    X(music_id)         /* current music track id (+1, 0 = none) */ \
    X(sfx_count)        /* sound effects started since boot */ \
    X(save_writes)      /* successful SRAM writes since boot */ \
    X(save_loaded)      /* 1 = valid save loaded at boot, 2 = fresh/corrupt reset */ \
    X(worlds_unlocked)  \
    X(stages_cleared)   \
    X(achievements)     /* unlocked achievement count */ \
    X(autoplay)         /* 1 while the diagnostic autoplay bot drives input */ \
    X(error_code)       /* non-zero if the ROM detected an internal error */     X(run_phase)        /* bm::run_phase of the Chaos Run */     X(run_index)        /* current Chaos Run encounter 0..11 */     X(run_cleared)      /* Chaos Run encounters cleared */     X(run_score)            X(run_upgrades)     /* distinct upgrades owned in the current run */ \
    X(cmd)              /* mailbox: command id written by the harness */ \
    X(arg0)             \
    X(arg1)             \
    X(arg2)             \
    X(arg3)             \
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
    BM_TI_CMD_WIPE_SAVE = 10
};

#endif
