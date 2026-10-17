#include "bm_cosmetics.h"

#include "bm_content.h"

namespace bm
{
    namespace
    {
        constexpr cosmetic_def blocks[] = {
            { "CLASSIC", "" },
            { "CONTRAST", "" },
            { "NEON", "CLEAR WORLD 4: ELECTRIC CITY" },
            { "PASTEL", "CLEAR 15 ADVENTURE STAGES" },
            { "CHILI", "COMPLETE THE ADVENTURE" },
        };
        constexpr cosmetic_def frames[] = {
            { "WORLD", "" },
            { "CHROME", "CLEAR WORLD 5: GRAVITY" },
            { "CANDY", "UNLOCK 10 ACHIEVEMENTS" },
            { "CARBON", "COMPLETE A CHAOS RUN" },
            { "GOLD", "COMPLETE A FULL BOSS RUSH" },
        };
        constexpr cosmetic_def titles[] = {
            { "FACTORY", "" },
            { "MIDNIGHT", "CLEAR WORLD 1" },
            { "INFERNO", "SURVIVE 25 OVERHEATS" },
            { "GOLDEN", "COMPLETE THE ADVENTURE" },
        };
        constexpr cosmetic_def particles[] = {
            { "SHARDS", "" },
            { "STARS", "UNLOCK 5 ACHIEVEMENTS" },
            { "SNOW", "CLEAR WORLD 2: FROZEN FACTORY" },
            { "EMBERS", "CLEAR WORLD 3: VOLCANO" },
            { "HEARTS", "EARN AN S+ RANK" },
        };

        static_assert(sizeof(blocks) / sizeof(blocks[0]) <= cosmetic_max_per_kind);
        static_assert(sizeof(frames) / sizeof(frames[0]) <= cosmetic_max_per_kind);
        static_assert(sizeof(titles) / sizeof(titles[0]) <= cosmetic_max_per_kind);
        static_assert(sizeof(particles) / sizeof(particles[0]) <= cosmetic_max_per_kind);

        bool world_cleared(const save_data& sv, int world)
        {
            return sv.stages[stage_number(world, stages_per_world)].cleared != 0;
        }

        int bits(uint32_t v)
        {
            int n = 0;
            for(; v; v &= v - 1)
            {
                ++n;
            }
            return n;
        }

        // The unlock rule of one theme (index 0 and the accessibility theme are always available).
        bool earned(const save_data& sv, cosmetic_kind kind, int index)
        {
            if(index == 0)
            {
                return true;
            }
            int cleared = 0;
            bool s_plus = false;
            for(const stage_record& r : sv.stages)
            {
                cleared += r.cleared ? 1 : 0;
                s_plus = s_plus || r.best_rank >= 5;
            }
            const int achievements = bits(sv.achievements);
            switch(kind)
            {
            case cosmetic_kind::blocks:
                switch(index)
                {
                case 1: return true;
                case 2: return world_cleared(sv, 4);
                case 3: return cleared >= 15;
                case 4: return sv.campaign_complete != 0;
                default: return false;
                }
            case cosmetic_kind::frame:
                switch(index)
                {
                case 1: return world_cleared(sv, 5);
                case 2: return achievements >= 10;
                case 3: return sv.runs_completed > 0;
                case 4: return sv.boss_rush_best_frames > 0;
                default: return false;
                }
            case cosmetic_kind::title:
                switch(index)
                {
                case 1: return world_cleared(sv, 1);
                case 2: return sv.stats.overheat_success >= 25;
                case 3: return sv.campaign_complete != 0;
                default: return false;
                }
            case cosmetic_kind::particles:
                switch(index)
                {
                case 1: return achievements >= 5;
                case 2: return world_cleared(sv, 2);
                case 3: return world_cleared(sv, 3);
                case 4: return s_plus;
                default: return false;
                }
            default:
                return false;
            }
        }
    }

    int cosmetic_count(cosmetic_kind kind)
    {
        switch(kind)
        {
        case cosmetic_kind::blocks: return int(sizeof(blocks) / sizeof(blocks[0]));
        case cosmetic_kind::frame: return int(sizeof(frames) / sizeof(frames[0]));
        case cosmetic_kind::title: return int(sizeof(titles) / sizeof(titles[0]));
        case cosmetic_kind::particles: return int(sizeof(particles) / sizeof(particles[0]));
        default: return 1;
        }
    }

    const cosmetic_def& cosmetic(cosmetic_kind kind, int index)
    {
        if(index < 0 || index >= cosmetic_count(kind))
        {
            index = 0;
        }
        switch(kind)
        {
        case cosmetic_kind::frame: return frames[index];
        case cosmetic_kind::title: return titles[index];
        case cosmetic_kind::particles: return particles[index];
        case cosmetic_kind::blocks:
        default: return blocks[index];
        }
    }

    const char* cosmetic_kind_name(cosmetic_kind kind)
    {
        switch(kind)
        {
        case cosmetic_kind::frame: return "BOARD FRAME";
        case cosmetic_kind::title: return "TITLE SCREEN";
        case cosmetic_kind::particles: return "PARTICLES";
        case cosmetic_kind::blocks:
        default: return "BLOCKS";
        }
    }

    bool cosmetic_unlocked(const save_data& sv, cosmetic_kind kind, int index)
    {
        if(index < 0 || index >= cosmetic_count(kind))
        {
            return false;
        }
        if(index == 0 || (kind == cosmetic_kind::blocks && index == 1))
        {
            return true;
        }
        return (sv.cosmetics_unlocked & cosmetic_bit(kind, index)) != 0;
    }

    uint32_t update_cosmetics(save_data& sv)
    {
        uint32_t fresh = 0;
        for(int k = 0; k < cosmetic_kinds; ++k)
        {
            const cosmetic_kind kind = cosmetic_kind(k);
            for(int i = 0; i < cosmetic_count(kind); ++i)
            {
                const uint32_t bit = cosmetic_bit(kind, i);
                if(! (sv.cosmetics_unlocked & bit) && earned(sv, kind, i))
                {
                    sv.cosmetics_unlocked |= bit;
                    const bool always = i == 0 || (kind == cosmetic_kind::blocks && i == 1);
                    if(! always)
                    {
                        fresh |= bit;
                    }
                }
            }
        }
        return fresh;
    }

    uint8_t& cosmetic_setting(game_settings& g, cosmetic_kind kind)
    {
        switch(kind)
        {
        case cosmetic_kind::frame: return g.frame_theme;
        case cosmetic_kind::title: return g.title_variant;
        case cosmetic_kind::particles: return g.particle_style;
        case cosmetic_kind::blocks:
        default: return g.block_theme;
        }
    }

    int selected_cosmetic(const save_data& sv, cosmetic_kind kind)
    {
        int index;
        switch(kind)
        {
        case cosmetic_kind::frame: index = sv.settings.frame_theme; break;
        case cosmetic_kind::title: index = sv.settings.title_variant; break;
        case cosmetic_kind::particles: index = sv.settings.particle_style; break;
        case cosmetic_kind::blocks:
        default: index = sv.settings.block_theme; break;
        }
        return cosmetic_unlocked(sv, kind, index) ? index : 0;
    }
}
