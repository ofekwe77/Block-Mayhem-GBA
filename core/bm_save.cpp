#include "bm_save.h"

#include <stddef.h>
#include <string.h>

namespace bm
{
    namespace
    {
        uint32_t crc_table_entry(uint32_t n)
        {
            uint32_t c = n;
            for(int k = 0; k < 8; ++k)
            {
                c = (c & 1) ? 0xEDB88320u ^ (c >> 1) : c >> 1;
            }
            return c;
        }

        int payload_size()
        {
            return int(offsetof(save_data, checksum));
        }
    }

    uint32_t crc32(const void* data, int size)
    {
        const uint8_t* p = static_cast<const uint8_t*>(data);
        uint32_t c = 0xFFFFFFFFu;
        for(int i = 0; i < size; ++i)
        {
            c = crc_table_entry((c ^ p[i]) & 0xFF) ^ (c >> 8);
        }
        return c ^ 0xFFFFFFFFu;
    }

    void seal(save_data& data, uint32_t sequence)
    {
        data.magic = save_magic;
        data.version = save_version;
        data.size = uint16_t(sizeof(save_data));
        data.sequence = sequence;
        data.checksum = crc32(&data, payload_size());
    }

    save_status validate(const save_data& data)
    {
        if(data.magic != save_magic)
        {
            // An erased / never written SRAM reads as 0xFF or 0x00 everywhere.
            const uint8_t* p = reinterpret_cast<const uint8_t*>(&data);
            bool blank = true;
            for(int i = 0; i < 16; ++i)
            {
                if(p[i] != 0xFF && p[i] != 0x00)
                {
                    blank = false;
                }
            }
            return blank ? save_status::empty : save_status::corrupt;
        }
        if(data.version != save_version || data.size != sizeof(save_data))
        {
            return save_status::old_version;
        }
        if(data.checksum != crc32(&data, payload_size()))
        {
            return save_status::corrupt;
        }
        if(data.worlds_unlocked < 1 || data.worlds_unlocked > 6 || data.settings.difficulty > 2)
        {
            return save_status::corrupt;    // structurally impossible values
        }
        return save_status::ok;
    }

    void reset_defaults(save_data& data)
    {
        data = save_data();
        data.size = uint16_t(sizeof(save_data));
    }

    save_status pick(const save_data& a, const save_data& b, save_data& out, int& slot_used)
    {
        const save_status sa = validate(a);
        const save_status sb = validate(b);
        if(sa == save_status::ok && (sb != save_status::ok || a.sequence >= b.sequence))
        {
            out = a;
            slot_used = 0;
            return save_status::ok;
        }
        if(sb == save_status::ok)
        {
            out = b;
            slot_used = 1;
            return save_status::ok;
        }
        reset_defaults(out);
        slot_used = -1;
        if(sa == save_status::empty && sb == save_status::empty)
        {
            return save_status::empty;
        }
        if(sa == save_status::old_version || sb == save_status::old_version)
        {
            return save_status::old_version;
        }
        return save_status::corrupt;
    }
}
