/*
 * BLOCK MAYHEM - ad-hoc profiling slots (emulator diagnostics): sub-sections of a frame, in per mille.
 */
#ifndef BM_PROFILE_H
#define BM_PROFILE_H

#include <stdint.h>

#include "bn_timer.h"
#include "bn_timers.h"

namespace bm::profile
{
    inline uint32_t slots[8] = {};
    inline uint32_t event_ticks[96] = {};      // per event type, this frame

    inline uint32_t per_mille(int ticks)
    {
        return uint32_t(ticks) * 1000u / uint32_t(bn::timers::ticks_per_frame());
    }
}

#endif
