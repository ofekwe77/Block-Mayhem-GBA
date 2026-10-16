#include "bm_board.h"

namespace bm
{
    void board::init(int cols, int rows, int hidden)
    {
        ++_revision;
        _cols = cols;
        _rows = rows;
        _hidden = hidden;
        _full_mask = uint16_t((1u << cols) - 1);
        clear_all();
    }

    void board::set(int x, int y, const cell& c)
    {
        ++_revision;
        _cells[y][x] = c;
        if(c.occupied())
        {
            _bits[y] = uint16_t(_bits[y] | (1u << x));
        }
        else
        {
            _cells[y][x].reset();
            _bits[y] = uint16_t(_bits[y] & ~(1u << x));
        }
    }

    void board::clear(int x, int y)
    {
        ++_revision;
        _cells[y][x].reset();
        _bits[y] = uint16_t(_bits[y] & ~(1u << x));
    }

    void board::clear_all()
    {
        ++_revision;
        for(int y = 0; y < max_rows; ++y)
        {
            _bits[y] = 0;
            for(int x = 0; x < max_cols; ++x)
            {
                _cells[y][x].reset();
            }
        }
    }

    bool board::fits(const piece_state& p) const
    {
        for(int m = 0; m < 4; ++m)
        {
            const point c = p.cell(m);
            if(blocked(c.x, c.y))
            {
                return false;
            }
        }
        return true;
    }

    int board::drop_distance(const piece_state& p) const
    {
        piece_state probe = p;
        int distance = 0;
        while(true)
        {
            ++probe.y;
            if(! fits(probe))
            {
                return distance;
            }
            ++distance;
        }
    }

    int board::column_height(int x) const
    {
        for(int y = 0; y < _rows; ++y)
        {
            if((_bits[y] >> x) & 1)
            {
                return _rows - y;
            }
        }
        return 0;
    }

    int board::stack_height() const
    {
        for(int y = 0; y < _rows; ++y)
        {
            if(_bits[y])
            {
                return _rows - y;
            }
        }
        return 0;
    }

    int board::holes() const
    {
        int result = 0;
        for(int x = 0; x < _cols; ++x)
        {
            bool covered = false;
            for(int y = 0; y < _rows; ++y)
            {
                const bool filled = (_bits[y] >> x) & 1;
                if(filled)
                {
                    covered = true;
                }
                else if(covered)
                {
                    ++result;
                }
            }
        }
        return result;
    }

    int board::occupied_count() const
    {
        int result = 0;
        for(int y = 0; y < _rows; ++y)
        {
            uint16_t b = _bits[y];
            while(b)
            {
                b = uint16_t(b & (b - 1));
                ++result;
            }
        }
        return result;
    }

    int board::count_rows_in_hidden_area() const
    {
        int result = 0;
        for(int y = 0; y < _hidden; ++y)
        {
            if(_bits[y])
            {
                ++result;
            }
        }
        return result;
    }

    bool board::empty() const
    {
        for(int y = 0; y < _rows; ++y)
        {
            if(_bits[y])
            {
                return false;
            }
        }
        return true;
    }

    bool board::push_garbage(int count, const int8_t* gap_columns, coat_kind coat)
    {
        ++_revision;
        if(count <= 0)
        {
            return true;
        }
        if(count > _rows)
        {
            count = _rows;
        }
        bool overflow = false;
        for(int y = 0; y < count; ++y)
        {
            if(_bits[y])
            {
                overflow = true;
            }
        }
        for(int y = 0; y < _rows - count; ++y)
        {
            _bits[y] = _bits[y + count];
            for(int x = 0; x < _cols; ++x)
            {
                _cells[y][x] = _cells[y + count][x];
            }
        }
        for(int i = 0; i < count; ++i)
        {
            const int y = _rows - count + i;
            int gap = gap_columns[i];
            if(gap < 0 || gap >= _cols)
            {
                gap = 0;
            }
            _bits[y] = 0;
            for(int x = 0; x < _cols; ++x)
            {
                _cells[y][x].reset();
                if(x != gap)
                {
                    cell g;
                    g.color = color_garbage;
                    if(coat != coat_kind::none)
                    {
                        g.coat = coat;
                        g.coat_hp = uint8_t(coat == coat_kind::ice ? cfg::ice_hp : cfg::armor_hp);
                    }
                    set(x, y, g);
                }
            }
        }
        return ! overflow;
    }

    void board::remove_rows(uint32_t row_mask)
    {
        ++_revision;
        int write = _rows - 1;
        for(int read = _rows - 1; read >= 0; --read)
        {
            if((row_mask >> read) & 1)
            {
                continue;
            }
            if(write != read)
            {
                _bits[write] = _bits[read];
                for(int x = 0; x < _cols; ++x)
                {
                    _cells[write][x] = _cells[read][x];
                }
            }
            --write;
        }
        for(; write >= 0; --write)
        {
            _bits[write] = 0;
            for(int x = 0; x < _cols; ++x)
            {
                _cells[write][x].reset();
            }
        }
    }

    int board::validate() const
    {
        if(_cols <= 0 || _cols > max_cols || _rows <= 0 || _rows > max_rows || _hidden < 0 || _hidden >= _rows)
        {
            return 1;
        }
        for(int y = 0; y < max_rows; ++y)
        {
            for(int x = 0; x < max_cols; ++x)
            {
                const cell& c = _cells[y][x];
                const bool inside_board = x < _cols && y < _rows;
                const bool bit = inside_board && ((_bits[y] >> x) & 1);
                if(! inside_board)
                {
                    if(c.occupied())
                    {
                        return 2;   // occupied cell outside the logical board
                    }
                    continue;
                }
                if(bit != c.occupied())
                {
                    return 3;       // bitmask and cell disagree
                }
                if(c.empty() && (c.coat != coat_kind::none || c.coat_hp || c.special != special_kind::none || c.flags))
                {
                    return 4;       // empty cell retains state
                }
                if(c.color > color_stone)
                {
                    return 5;
                }
                if(c.coat == coat_kind::none && c.coat_hp)
                {
                    return 6;
                }
                if(c.coat != coat_kind::none && ! c.coat_hp)
                {
                    return 10;      // coating with no hit points must be removed
                }
                if(c.coat == coat_kind::ice && c.coat_hp > cfg::ice_hp)
                {
                    return 7;
                }
                if(c.coat == coat_kind::armor && c.coat_hp > cfg::armor_hp)
                {
                    return 8;
                }
            }
            if(y < _rows && (_bits[y] & ~_full_mask))
            {
                return 9;
            }
        }
        return 0;
    }
}
