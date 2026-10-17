#include "bm_customize_scene.h"

#include "bm_audio.h"
#include "bm_cosmetics.h"
#include "bm_themes.h"
#include "bm_tile_ids.h"

#include "bn_bg_palette_ptr.h"
#include "bn_bg_tiles.h"
#include "bn_memory.h"
#include "bn_regular_bg_item.h"
#include "bn_regular_bg_map_item.h"
#include "bn_string.h"

#include "bn_bg_palette_items_board_palette.h"
#include "bn_regular_bg_tiles_items_board_tiles.h"
#include "bn_sprite_items_spr_particles.h"

namespace bm
{
    namespace
    {
        constexpr const char* row_names[4] = { "BLOCKS", "FRAME", "TITLE", "PARTICLES" };
        constexpr const char* row_info[4] = {
            "COLOURS OF THE SEVEN SHAPES", "THE METAL AROUND THE BOARD", "SKY AND SPARKS OF THE TITLE SCREEN",
            "WHAT FLIES WHEN ROWS CLEAR"
        };

        // Preview: a small framed well (7 x 9 cells) at map column 22, row 5. Digits are shape banks
        // (1 I .. 7 L), 'b' a bomb and 'e' a spark on a coloured cell.
        constexpr int well_x = 22;
        constexpr int well_y = 5;
        constexpr int well_w = 7;
        constexpr int well_h = 9;
        constexpr const char* sample[well_h] = {
            ".......",
            "..3....",
            ".333...",
            ".......",
            "......1",
            "55....1",
            "e56.441",
            "76622b1",
            "7762244",
        };

        constexpr int particle_first[5] = { 24, 20, 12, 8, 30 };     // shards, stars, snow, embers, hearts
        constexpr int particle_frames[5] = { 4, 2, 2, 4, 1 };
    }

    customize_scene::customize_scene() :
        _backdrop(backdrop_id::menu, 3),
        _cells(new preview_cells())
    {
        audio::play_music(audio::music_id::menu);
        const save_data& sv = the_app().save;
        for(int i = 0; i < rows; ++i)
        {
            _choice[i] = int8_t(selected_cosmetic(sv, cosmetic_kind(i)));
        }
        _title.show_big(120, 6, "CUSTOMIZE", big_color::steel, align::center);
        _hint.show(120, 151, "LEFT/RIGHT: CHANGE   B: SAVE", text_color::gray, align::center);
        _preview_label.show(204, 21, "PREVIEW", text_color::gray, align::center);

        bn::memory::clear(_cells->cells);
        bn::regular_bg_map_item map_item(_cells->cells[0], bn::size(32, 32));
        bn::bg_tiles::set_allow_offset(false);
        _preview = bn::regular_bg_item(bn::regular_bg_tiles_items::board_tiles, bn::bg_palette_items::board_palette,
                                       map_item).create_bg(8, 48);
        bn::bg_tiles::set_allow_offset(true);
        _preview->set_priority(2);
        _map = _preview->map();
        _draw_preview();
        _apply_colors();
        _refresh();
    }

    customize_scene::~customize_scene() = default;

    void customize_scene::_draw_preview()
    {
        auto put = [this](int x, int y, int tile, int bank)
        {
            _cells->cells[y * 32 + x] = bn::regular_bg_map_cell(tile | (bank << 12));
        };
        // Frame around the well (bank 0), the well, then the sample stack.
        put(well_x - 1, well_y - 1, tile::frame_tl, 0);
        put(well_x + well_w, well_y - 1, tile::frame_tr, 0);
        for(int x = 0; x < well_w; ++x)
        {
            put(well_x + x, well_y - 1, tile::frame_t, 0);
            put(well_x + x, well_y + well_h, tile::frame_b, 0);
        }
        for(int y = 0; y < well_h; ++y)
        {
            put(well_x - 1, well_y + y, tile::frame_l, 0);
            put(well_x + well_w, well_y + y, tile::frame_r, 0);
            for(int x = 0; x < well_w; ++x)
            {
                const char c = sample[y][x];
                if(c >= '1' && c <= '7')
                {
                    put(well_x + x, well_y + y, tile::block, c - '0');
                }
                else if(c == 'b')
                {
                    put(well_x + x, well_y + y, tile::bomb, 2);
                }
                else if(c == 'e')
                {
                    put(well_x + x, well_y + y, tile::spark, 5);
                }
                else
                {
                    put(well_x + x, well_y + y, tile::well, 0);
                }
            }
        }
        put(well_x - 1, well_y + well_h, tile::frame_bl, 0);
        put(well_x + well_w, well_y + well_h, tile::frame_br, 0);
        _map->reload_cells_ref();
    }

    void customize_scene::_apply_colors()
    {
        // The same colour data the board renderer uses (bm_themes), on a copy of the board palette.
        bn::bg_palette_ptr palette = _preview->palette();
        const bn::span<const bn::color> base = bn::bg_palette_items::board_palette.colors_ref();
        alignas(4) bn::color colors[192];
        for(int i = 0; i < 192; ++i)
        {
            colors[i] = i < base.size() ? base[i] : bn::color();
        }
        const int blocks = cosmetic_unlocked(the_app().save, cosmetic_kind::blocks, _choice[0]) ? _choice[0] : 0;
        if(blocks)
        {
            for(int shape = 0; shape < 7; ++shape)
            {
                bn::color c[5];
                themes::block_colors(blocks, shape, c);
                for(int i = 0; i < 5; ++i)
                {
                    colors[(shape + 1) * 16 + 1 + i] = c[i];
                }
            }
        }
        bn::color metal(14, 15, 18);
        bn::color accent(31, 25, 5);
        const int frame = cosmetic_unlocked(the_app().save, cosmetic_kind::frame, _choice[1]) ? _choice[1] : 0;
        themes::frame_colors(frame, metal, accent);
        auto mix = [](bn::color a, bn::color b, int k, int total)
        {
            return bn::color(a.red() + (b.red() - a.red()) * k / total, a.green() + (b.green() - a.green()) * k / total,
                             a.blue() + (b.blue() - a.blue()) * k / total);
        };
        const bn::color black(1, 1, 3);
        const bn::color white(31, 31, 31);
        colors[2] = mix(metal, black, 11, 20);
        colors[3] = mix(metal, black, 5, 20);
        colors[4] = metal;
        colors[5] = mix(metal, white, 9, 20);
        colors[6] = bn::color(2, 2, 4);
        colors[7] = mix(colors[6], white, 3, 20);
        colors[10] = accent;
        colors[11] = mix(accent, bn::color(31, 8, 0), 10, 20);
        palette.set_colors(bn::span<const bn::color>(colors, 192));
    }

    void customize_scene::_refresh()
    {
        const save_data& sv = the_app().save;
        int unlocked = 0;
        int total = 0;
        for(int i = 0; i < rows; ++i)
        {
            const cosmetic_kind kind = cosmetic_kind(i);
            const int c = _choice[i];
            const bool open = cosmetic_unlocked(sv, kind, c);
            const bool sel = i == _index;
            const int y = 34 + i * 16;
            bn::string<24> name = sel ? "> " : "  ";
            name.append(row_names[i]);
            _names[i].show(10, y, name, sel ? text_color::yellow : text_color::white, align::left);
            bn::string<20> v = sel ? "< " : "";
            v.append(open ? cosmetic(kind, c).name : "LOCKED");
            if(sel)
            {
                v.append(" >");
            }
            _values[i].show(160, y, v, ! open ? text_color::red : (sel ? text_color::yellow : text_color::cyan),
                            align::right);
            for(int k = 0; k < cosmetic_count(kind); ++k)
            {
                unlocked += cosmetic_unlocked(sv, kind, k) ? 1 : 0;
                ++total;
            }
        }
        const cosmetic_kind kind = cosmetic_kind(_index);
        const int c = _choice[_index];
        if(cosmetic_unlocked(sv, kind, c))
        {
            _info[0].show(120, 112, row_info[_index], text_color::white, align::center);
        }
        else
        {
            bn::string<40> how = cosmetic(kind, c).name;
            how.append(": ");
            how.append(cosmetic(kind, c).unlock);
            _info[0].show(120, 112, how, text_color::red, align::center);
        }
        bn::string<32> count = bn::to_string<4>(unlocked);
        count.append(" OF ");
        count.append(bn::to_string<4>(total));
        count.append(" STYLES UNLOCKED");
        _info[1].show(120, 126, count, text_color::gray, align::center);
    }

    void customize_scene::_change(int delta)
    {
        const cosmetic_kind kind = cosmetic_kind(_index);
        const int count = cosmetic_count(kind);
        _choice[_index] = int8_t((_choice[_index] + delta + count) % count);
        // Locked entries can be browsed (to see how they are earned) but never selected.
        save_data& sv = the_app().save;
        if(cosmetic_unlocked(sv, kind, _choice[_index]))
        {
            cosmetic_setting(sv.settings, kind) = uint8_t(_choice[_index]);
        }
        audio::sfx(audio::sfx_id::menu_move);
        _apply_colors();
        _refresh();
    }

    void customize_scene::_update_sparks()
    {
        const save_data& sv = the_app().save;
        const int style = cosmetic_unlocked(sv, cosmetic_kind::particles, _choice[3]) ? _choice[3] : 0;
        // A burst from the preview well every 40 frames in the selected particle style.
        if(_frame % 40 == 0)
        {
            static constexpr int8_t vxs[6] = { -14, -8, -3, 3, 8, 14 };
            for(int i = 0; i < 6; ++i)
            {
                spark& s = _sparks[i];
                s.x = (well_x + 3) * 8 + 4;
                s.y = (well_y + 5) * 8;
                s.vx = vxs[i];
                s.vy = -28 - (i & 1) * 8;
                s.life = 34;
                const int first = particle_first[style] + (style == 0 ? (i & 3) : 0);
                if(! s.sprite)
                {
                    s.sprite = bn::sprite_items::spr_particles.create_sprite(0, 0, first);
                    s.sprite->set_bg_priority(1);
                }
                s.sprite->set_tiles(bn::sprite_items::spr_particles.tiles_item(), first);
                s.sprite->set_visible(true);
            }
        }
        for(int i = 0; i < 6; ++i)
        {
            spark& s = _sparks[i];
            if(! s.sprite || s.life <= 0)
            {
                continue;
            }
            s.x += s.vx / 16;
            s.y += s.vy / 16;
            s.vy += 2;
            if(--s.life <= 0)
            {
                s.sprite->set_visible(false);
                continue;
            }
            const int frames = particle_frames[style];
            if(frames > 1 && style != 0)
            {
                s.sprite->set_tiles(bn::sprite_items::spr_particles.tiles_item(),
                                    particle_first[style] + (34 - s.life) / 6 % frames);
            }
            s.sprite->set_position(s.x - 120, s.y - 80);
        }
    }

    bn::optional<scene_id> customize_scene::update()
    {
        ++_frame;
        _input.update();
        _backdrop.update();
        _update_sparks();
        if(_input.up() || _input.down())
        {
            _index = (_index + (_input.up() ? rows - 1 : 1)) % rows;
            audio::sfx(audio::sfx_id::menu_move);
            _refresh();
        }
        else if(_input.left())
        {
            _change(-1);
        }
        else if(_input.right() || _input.ok())
        {
            _change(1);
        }
        else if(_input.back() || _input.start())
        {
            audio::sfx(audio::sfx_id::menu_back);
            the_app().write_save();
            return scene_id::options;
        }
        return bn::nullopt;
    }
}
