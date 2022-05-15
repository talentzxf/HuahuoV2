#pragma once

#include "Include/C/Baselib_Timer.h"

BASELIB_C_INTERFACE
{
    Baselib_Timer_TickToNanosecondConversionRatio Baselib_Timer_GetTicksToNanosecondsConversionRatio()
    {
        return platform::Baselib_Timer_GetTicksToNanosecondsConversionRatio();
    }

    Baselib_Timer_Ticks Baselib_Timer_GetHighPrecisionTimerTicks()
    {
        return platform::Baselib_Timer_GetHighPrecisionTimerTicks();
    }

    void Baselib_Timer_WaitForAtLeast(uint32_t timeInMilliseconds)
    {
        return platform::Baselib_Timer_WaitForAtLeast(timeInMilliseconds);
    }

    double Baselib_Timer_GetTimeSinceStartupInSeconds()
    {
        return platform::Baselib_Timer_GetTimeSinceStartupInSeconds();
    }
}
