/*
 * BLOCK MAYHEM - unlockable cosmetics (PART 35.4, PART 27.3).
 *
 * Four kinds: block themes, board frames, title-screen variations and particle
 * styles. Index 0 of every kind is the default and always available; block theme
 * 1 (CONTRAST, colour-blind friendly) is an accessibility option and is never
 * locked. Every other theme is unlocked by real progress recorded in the save.
 * Cosmetics only change colours and decoration, never rules: shape colours stay
 * distinct in every block theme and special-block symbols keep their own colours.
 */
#ifndef BM_COSMETICS_H
#define BM_COSMETICS_H

#include <stdint.h>

#include "bm_save.h"

namespace bm
{
    enum class cosmetic_kind : uint8_t
    {
        blocks, frame, title, particles
    };

    constexpr int cosmetic_kinds = 4;
    constexpr int cosmetic_max_per_kind = 5;

    struct cosmetic_def
    {
        const char* name;
        const char* unlock;     // how it is earned (shown while locked)
    };

    [[nodiscard]] int cosmetic_count(cosmetic_kind kind);
    [[nodiscard]] const cosmetic_def& cosmetic(cosmetic_kind kind, int index);
    [[nodiscard]] const char* cosmetic_kind_name(cosmetic_kind kind);

    // Bit (kind * 8 + index) of save_data::cosmetics_unlocked.
    [[nodiscard]] constexpr uint32_t cosmetic_bit(cosmetic_kind kind, int index)
    {
        return 1u << (int(kind) * 8 + index);
    }

    [[nodiscard]] bool cosmetic_unlocked(const save_data& sv, cosmetic_kind kind, int index);

    // Evaluates every unlock rule against the save and stores the bits. Returns the newly unlocked bits
    // (defaults and accessibility themes are never reported as new).
    uint32_t update_cosmetics(save_data& sv);

    // The selected theme of a kind in the settings (falls back to the default when not unlocked).
    [[nodiscard]] int selected_cosmetic(const save_data& sv, cosmetic_kind kind);
    [[nodiscard]] uint8_t& cosmetic_setting(game_settings& g, cosmetic_kind kind);
}

#endif
