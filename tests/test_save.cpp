/*
 * Save system (PART 29, 39.7): integrity checks, two-slot recovery, defaults,
 * version mismatch and a full-completion round trip.
 */
#include "bm_test.h"

#include "bm_save.h"

#include <cstring>

using namespace bm;

BM_TEST(save, layout_and_crc_reference)
{
    CHECK_EQ(int(sizeof(save_data)), 512);
    const char text[] = "123456789";
    CHECK_EQ(crc32(text, 9), 0xCBF43926u);     // IEEE CRC-32 check value
}

BM_TEST(save, first_launch_blank_sram_is_empty_not_corrupt)
{
    save_data ff;
    std::memset(static_cast<void*>(&ff), 0xFF, sizeof(ff));
    save_data zero;
    std::memset(static_cast<void*>(&zero), 0x00, sizeof(zero));
    CHECK(validate(ff) == save_status::empty);
    CHECK(validate(zero) == save_status::empty);
    save_data out;
    int slot = 9;
    CHECK(pick(ff, zero, out, slot) == save_status::empty);
    CHECK_EQ(slot, -1);
    CHECK_EQ(int(out.worlds_unlocked), 1);
    CHECK_EQ(int(out.settings.difficulty), 1);
    CHECK_EQ(out.achievements, 0u);
}

BM_TEST(save, sealed_data_validates_and_any_flipped_byte_is_detected)
{
    save_data d;
    reset_defaults(d);
    d.worlds_unlocked = 3;
    d.classic_high[0] = 123456;
    seal(d, 7);
    CHECK(validate(d) == save_status::ok);
    uint8_t* bytes = reinterpret_cast<uint8_t*>(&d);
    // Flip one byte at a time across the payload (skipping magic/version/size which report other statuses).
    int detected = 0;
    int tried = 0;
    for(int i = 12; i < int(sizeof(d)) - 4; i += 7)
    {
        const uint8_t old = bytes[i];
        bytes[i] = uint8_t(old ^ 0x5A);
        ++tried;
        detected += validate(d) != save_status::ok ? 1 : 0;
        bytes[i] = old;
    }
    CHECK_EQ(detected, tried);
    CHECK(validate(d) == save_status::ok);
}

BM_TEST(save, version_mismatch_and_impossible_values_fall_back_to_defaults)
{
    save_data d;
    reset_defaults(d);
    seal(d, 1);
    d.version = uint16_t(save_version + 1);
    d.checksum = crc32(&d, int(sizeof(d)) - 4);
    CHECK(validate(d) == save_status::old_version);
    save_data blank;
    std::memset(static_cast<void*>(&blank), 0xFF, sizeof(blank));
    save_data out;
    int slot = 0;
    CHECK(pick(d, blank, out, slot) == save_status::old_version);
    CHECK_EQ(slot, -1);

    save_data bad;
    reset_defaults(bad);
    bad.worlds_unlocked = 9;        // impossible even with a matching checksum
    seal(bad, 2);
    CHECK(validate(bad) == save_status::corrupt);
}

BM_TEST(save, newest_valid_slot_wins_and_an_interrupted_write_keeps_the_previous)
{
    save_data a;
    save_data b;
    reset_defaults(a);
    reset_defaults(b);
    a.classic_high[0] = 100;
    b.classic_high[0] = 200;
    seal(a, 10);
    seal(b, 11);
    save_data out;
    int slot = -1;
    CHECK(pick(a, b, out, slot) == save_status::ok);
    CHECK_EQ(slot, 1);
    CHECK_EQ(out.classic_high[0], 200u);
    // The newer slot was being written when power failed: its checksum no longer matches.
    reinterpret_cast<uint8_t*>(&b)[100] ^= 0xFF;
    CHECK(pick(a, b, out, slot) == save_status::ok);
    CHECK_EQ(slot, 0);
    CHECK_EQ(out.classic_high[0], 100u);
}

BM_TEST(save, full_completion_round_trip)
{
    save_data d;
    reset_defaults(d);
    d.worlds_unlocked = 6;
    d.campaign_complete = 1;
    d.boss_rush_unlocked = 1;
    d.abilities_unlocked = 0xFF;
    for(int i = 0; i < stage_count; ++i)
    {
        d.stages[i].cleared = 1;
        d.stages[i].best_rank = 5;
        d.stages[i].best_score = uint32_t(100000 + i);
        d.stages[i].best_seconds = uint16_t(60 + i);
    }
    d.achievements = 0x3FFFFFFFu;
    d.run_best_score = 2500000;
    d.runs_completed = 12;
    d.run_best_act = 4;
    d.boss_rush_best_frames = 54321;
    d.stats.upgrades_seen = 0x7FFFFFFFu;
    d.settings.music_volume = 3;
    seal(d, 99);
    save_data copy;
    std::memcpy(&copy, &d, sizeof(d));      // what SRAM would hand back
    CHECK(validate(copy) == save_status::ok);
    save_data blank;
    std::memset(static_cast<void*>(&blank), 0xFF, sizeof(blank));
    save_data out;
    int slot = -1;
    CHECK(pick(blank, copy, out, slot) == save_status::ok);
    CHECK_EQ(slot, 1);
    CHECK_EQ(std::memcmp(&out, &d, sizeof(d)), 0);
}
