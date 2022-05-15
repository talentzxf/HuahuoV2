#pragma once

#include "Include/C/Baselib_Timer.h"
#include "Source/Baselib_InlineImplementation.h"

#include <algorithm>
#include <windef.h>
#include <WinBase.h>

namespace WinApi
{
    BASELIB_INLINE_IMPL Baselib_Timer_TickToNanosecondConversionRatio Baselib_Timer_GetTicksToNanosecondsConversionRatio()
    {
        LARGE_INTEGER frequency = {0};
        BOOL result = QueryPerformanceFrequency(&frequency);
        BaselibAssert(result == TRUE);
        return Baselib_Timer_TickToNanosecondConversionRatio { Baselib_NanosecondsPerSecond, static_cast<uint64_t>(frequency.QuadPart) };
    }

    BASELIB_INLINE_IMPL Baselib_Timer_Ticks Baselib_Timer_GetHighPrecisionTimerTicks()
    {
        LARGE_INTEGER start = {0};
        BOOL result = QueryPerformanceCounter(&start);
        BaselibAssert(result == TRUE);
        return start.QuadPart;
    }
}
