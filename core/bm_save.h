/*
 * BLOCK MAYHEM - persistent save data (platform independent, host testable).
 *
 * Integrity: magic header, version, payload size and CRC-32. Two slots are
 * written alternately with a sequence number; loading picks the newest valid
 * slot, so an interrupted write can never destroy the previous progress.
 * Invalid or mismatching data falls back to defaults (never read blindly).
 */
#ifndef BM_SAVE_H
#define BM_SAVE_H

#include <stdint.h>

namespace bm
{
    constexpr uint32_t save_magic = 0x4D594842u;     // "BHYM"
    constexpr uint16_t save_version = 3;
    constexpr int stage_count = 30;
    constexpr int achievement_count = 32;

    struct game_settings
    {
        uint8_t difficulty = 1;         // 0 beginner, 1 standard, 2 expert
        uint8_t ghost = 1;
        uint8_t shake = 2;              // 0 off, 1 reduced, 2 full
        uint8_t flashes = 1;            // 0 reduced, 1 full
        uint8_t music_volume = 8;       // 0..10
        uint8_t sfx_volume = 8;         // 0..10
        uint8_t block_theme = 0;        // cosmetic (unlockable)
        uint8_t frame_theme = 0;
        uint8_t title_variant = 0;
        uint8_t particle_style = 0;
        uint8_t reserved[6] = {};
    };

    struct stage_record
    {
        uint32_t best_score = 0;
        uint16_t best_seconds = 0;
        uint8_t best_rank = 0;          // 0 none, 1 C, 2 B, 3 A, 4 S, 5 S+
        uint8_t cleared = 0;
    };

    struct lifetime_stats
    {
        uint32_t lines = 0;
        uint32_t pieces = 0;
        uint32_t bombs = 0;
        uint32_t fires = 0;
        uint32_t sparks = 0;
        uint32_t ice_broken = 0;
        uint32_t overheats = 0;
        uint32_t overheat_success = 0;
        uint32_t chains = 0;
        uint32_t max_chain = 0;
        uint32_t quads = 0;
        uint32_t bosses_defeated = 0;
        uint32_t play_seconds = 0;
        uint32_t cross_combos = 0;
        uint32_t abilities_used = 0;
        uint32_t upgrades_seen = 0;     // CHAOS RUN codex: bit per upgrade_id ever offered
        uint32_t reserved[4] = {};
    };

    struct save_data
    {
        uint32_t magic = save_magic;
        uint16_t version = save_version;
        uint16_t size = 0;
        uint32_t sequence = 0;
        game_settings settings;
        uint8_t worlds_unlocked = 1;    // 1..6
        uint8_t campaign_complete = 0;
        uint8_t boss_rush_unlocked = 0;
        uint8_t abilities_unlocked = 0x23;     // bitmask of cfg::ability_id (laser, freeze, shield at start)
        uint32_t cosmetics_unlocked = 0;
        uint32_t music_unlocked = 0x3;
        stage_record stages[stage_count];
        uint32_t classic_high[5] = {};
        uint32_t arcade_high[5] = {};
        uint32_t dual_high[5] = {};
        uint32_t boss_rush_best_score = 0;
        uint32_t boss_rush_best_frames = 0;
        uint32_t run_best_score = 0;
        uint16_t runs_started = 0;
        uint16_t runs_completed = 0;
        uint8_t run_best_act = 0;
        uint8_t reserved8[3] = {};
        uint32_t achievements = 0;      // unlocked bits
        uint16_t achievement_progress[achievement_count] = {};
        lifetime_stats stats;
        uint32_t checksum = 0;
    };

    // CRC-32 (IEEE) over arbitrary bytes.
    [[nodiscard]] uint32_t crc32(const void* data, int size);

    // Prepares a save for writing: sets magic/version/size/sequence and the checksum.
    void seal(save_data& data, uint32_t sequence);

    enum class save_status : uint8_t
    {
        ok,
        empty,          // no save found (first launch)
        corrupt,        // magic / checksum mismatch
        old_version     // other version: reset to defaults (documented)
    };

    // Validates a slot image. Returns ok only if magic, version, size and checksum all match.
    [[nodiscard]] save_status validate(const save_data& data);

    // Chooses which of two slot images to use. 'out' receives the newest valid one (or defaults).
    save_status pick(const save_data& a, const save_data& b, save_data& out, int& slot_used);

    // Resets to defaults (first launch / unreadable data).
    void reset_defaults(save_data& data);

    // Rank helpers.
    [[nodiscard]] constexpr const char* rank_name(int rank)
    {
        constexpr const char* names[] = { "-", "C", "B", "A", "S", "S+" };
        return names[rank < 0 ? 0 : (rank > 5 ? 5 : rank)];
    }
}

#endif
