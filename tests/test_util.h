/*
 * Test helpers: build boards from ASCII art, run resolutions to completion.
 *
 * Board legend ('.' empty):
 *   '#' garbage   'IOTSZJL' shape colours   'x' stone
 *   'b' bomb  'f' fire  'e' electric spark  'm' magnet  'r' rainbow  (on garbage)
 *   'i' ice-coated garbage   'A' armored garbage (2 hp)   'a' armored garbage (1 hp)
 *   'c' charged garbage (Chain Spark)   'B' ice-coated bomb   'd' disabled bomb
 */
#ifndef BM_TEST_UTIL_H
#define BM_TEST_UTIL_H

#include "bm_board.h"
#include "bm_player.h"
#include "bm_resolver.h"

#include <initializer_list>
#include <string>

namespace bmtest
{
    inline bm::cell cell_from_char(char ch)
    {
        bm::cell c;
        switch(ch)
        {
        case '.': return c;
        case '#': c.color = bm::color_garbage; break;
        case 'I': c.color = bm::color_i; break;
        case 'O': c.color = bm::color_o; break;
        case 'T': c.color = bm::color_t; break;
        case 'S': c.color = bm::color_s; break;
        case 'Z': c.color = bm::color_z; break;
        case 'J': c.color = bm::color_j; break;
        case 'L': c.color = bm::color_l; break;
        case 'x': c.color = bm::color_stone; break;
        case 'b': c.color = bm::color_garbage; c.special = bm::special_kind::bomb; break;
        case 'f': c.color = bm::color_garbage; c.special = bm::special_kind::fire; break;
        case 'e': c.color = bm::color_garbage; c.special = bm::special_kind::spark; break;
        case 'm': c.color = bm::color_garbage; c.special = bm::special_kind::magnet; break;
        case 'r': c.color = bm::color_garbage; c.special = bm::special_kind::rainbow; break;
        case 'i': c.color = bm::color_garbage; c.coat = bm::coat_kind::ice; c.coat_hp = 1; break;
        case 'A': c.color = bm::color_garbage; c.coat = bm::coat_kind::armor; c.coat_hp = 2; break;
        case 'a': c.color = bm::color_garbage; c.coat = bm::coat_kind::armor; c.coat_hp = 1; break;
        case 'c': c.color = bm::color_garbage; c.flags = bm::flag_charged | bm::flag_was_charged; break;
        case 'B': c.color = bm::color_garbage; c.special = bm::special_kind::bomb; c.coat = bm::coat_kind::ice;
                  c.coat_hp = 1; break;
        case 'd': c.color = bm::color_garbage; c.special = bm::special_kind::bomb; c.flags = bm::flag_disabled; break;
        default: c.color = bm::color_garbage; break;
        }
        return c;
    }

    // Loads rows aligned to the bottom of the board (last string = bottom row).
    inline void load(bm::board& b, std::initializer_list<const char*> rows)
    {
        b.clear_all();
        int y = b.rows() - int(rows.size());
        for(const char* r : rows)
        {
            for(int x = 0; x < b.cols() && r[x]; ++x)
            {
                b.set(x, y, cell_from_char(r[x]));
            }
            ++y;
        }
    }

    inline char char_of(const bm::cell& c)
    {
        if(c.empty()) return '.';
        if(c.coat == bm::coat_kind::ice) return c.special == bm::special_kind::bomb ? 'B' : 'i';
        if(c.coat == bm::coat_kind::armor) return c.coat_hp >= 2 ? 'A' : 'a';
        switch(c.special)
        {
        case bm::special_kind::bomb: return (c.flags & bm::flag_disabled) ? 'd' : 'b';
        case bm::special_kind::fire: return 'f';
        case bm::special_kind::spark: return 'e';
        case bm::special_kind::magnet: return 'm';
        case bm::special_kind::rainbow: return 'r';
        default: break;
        }
        if(c.flags & bm::flag_charged) return 'c';
        constexpr char colors[] = ".IOTSZJL#x";
        return colors[c.color];
    }

    // Returns the bottom 'count' rows as a string with '/' separators.
    inline std::string dump(const bm::board& b, int count)
    {
        std::string s;
        for(int y = b.rows() - count; y < b.rows(); ++y)
        {
            for(int x = 0; x < b.cols(); ++x)
            {
                s += char_of(b.at(x, y));
            }
            if(y + 1 < b.rows())
            {
                s += '/';
            }
        }
        return s;
    }

    inline bm::resolution_result resolve(bm::board& b, const bm::effect_mods& mods = bm::effect_mods())
    {
        static bm::resolver r;
        static bm::event_queue ev;
        ev.clear();
        r.begin(mods);
        for(int guard = 0; r.active() && guard < 2000; ++guard)
        {
            r.step(b, ev, 0);
        }
        return r.result();
    }
}

#endif
