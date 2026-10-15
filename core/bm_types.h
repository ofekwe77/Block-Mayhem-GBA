/*
 * BLOCK MAYHEM - basic gameplay types shared by every core module.
 */
#ifndef BM_TYPES_H
#define BM_TYPES_H

#include <stdint.h>

namespace bm
{
    // The seven four-cell shapes. Values double as cell colors 1..7.
    enum class piece_type : uint8_t
    {
        none = 0, i, o, t, s, z, j, l
    };

    constexpr int piece_type_count = 7;

    // Cell base color. 1..7 match piece_type, 8 = garbage, 9 = stone (indestructible by effects).
    enum cell_color : uint8_t
    {
        color_empty = 0,
        color_i = 1, color_o, color_t, color_s, color_z, color_j, color_l,
        color_garbage = 8,
        color_stone = 9
    };

    enum class special_kind : uint8_t
    {
        none = 0,
        bomb,
        fire,
        spark,
        magnet,
        rainbow
    };

    constexpr int special_kind_count = 6;   // including none

    enum class coat_kind : uint8_t
    {
        none = 0,
        ice,
        armor
    };

    enum cell_flags : uint8_t
    {
        flag_charged = 1,       // Chain Spark charge: pulses when destroyed or cleared
        flag_disabled = 2,      // special temporarily inert (Electric Emperor surge)
        flag_frost_mark = 4,    // telegraphed: will freeze at the next freezing event
        flag_armor_mark = 8,    // telegraphed: will be armored by a boss injection
        flag_was_charged = 16   // cell was charged once; cannot be charged again
    };

    struct cell
    {
        uint8_t color = color_empty;
        special_kind special = special_kind::none;
        coat_kind coat = coat_kind::none;
        uint8_t coat_hp = 0;
        uint8_t flags = 0;
        uint8_t reserved = 0;

        [[nodiscard]] constexpr bool empty() const
        {
            return color == color_empty;
        }

        [[nodiscard]] constexpr bool occupied() const
        {
            return color != color_empty;
        }

        [[nodiscard]] constexpr bool coated() const
        {
            return coat != coat_kind::none && coat_hp > 0;
        }

        [[nodiscard]] constexpr bool is_special() const
        {
            return special != special_kind::none;
        }

        [[nodiscard]] constexpr bool ordinary() const
        {
            return occupied() && ! is_special() && ! coated() && color != color_stone;
        }

        constexpr void reset()
        {
            *this = cell();
        }
    };

    static_assert(sizeof(cell) == 6, "cell must stay compact");

    enum class difficulty : uint8_t
    {
        beginner,
        standard,
        expert
    };

    enum class gravity_dir : uint8_t
    {
        down,
        left,
        right,
        up
    };

    // Joypad bits used by the simulation (match the GBA KEYINPUT layout).
    enum key_bits : uint16_t
    {
        key_a = 1 << 0,
        key_b = 1 << 1,
        key_select = 1 << 2,
        key_start = 1 << 3,
        key_right = 1 << 4,
        key_left = 1 << 5,
        key_up = 1 << 6,
        key_down = 1 << 7,
        key_r = 1 << 8,
        key_l = 1 << 9
    };

    struct input_frame
    {
        uint16_t held = 0;
        uint16_t pressed = 0;   // keys that went down this frame

        [[nodiscard]] constexpr bool down(uint16_t k) const
        {
            return held & k;
        }

        [[nodiscard]] constexpr bool hit(uint16_t k) const
        {
            return pressed & k;
        }
    };

    struct point
    {
        int8_t x = 0;
        int8_t y = 0;
    };
}

#endif
