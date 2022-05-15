#pragma once

#include "Include/C/Baselib_Timer.h"
#include "Source/Baselib_InlineImplementation.h"

#include <time.h>

namespace PosixApi
{
    BASELIB_INLINE_IMPL Baselib_Timer_TickToNanosecondConversionRatio Baselib_Timer_GetTicksToNanosecondsConversionRatio()
    {
        return Baselib_Timer_TickToNanosecondConversionRatio { 1, 1 };
    }

    BASELIB_INLINE_IMPL Baselib_Timer_Ticks Baselib_Timer_GetHighPrecisionTimerTicks(clockid_t id)
    {
        struct timespec ticks = { 0, 0 };
        const int result = clock_gettime(id, &ticks);
        BaselibAssert(result == 0);
        return (uint64_t)ticks.tv_sec * Baselib_NanosecondsPerSecond + ticks.tv_nsec;
    }
}
