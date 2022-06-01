#include "Include/Baselib.h"
#include "Include/C/Baselib_Timer.h"
#include "Source/Cpp11/Baselib_Timer_Cpp11.inl.h"
#include "Source/Common/Baselib_Time_Common.inl.h"

#include <emscripten.h>

BASELIB_C_INTERFACE
{
    Baselib_Timer_TickToNanosecondConversionRatio Baselib_Timer_GetTicksToNanosecondsConversionRatio()
    {
        return Baselib_Timer_TickToNanosecondConversionRatio { 1, 1 };
    }

    Baselib_Timer_Ticks Baselib_Timer_GetHighPrecisionTimerTicks()
    {
        return static_cast<Baselib_Timer_Ticks>(emscripten_get_now() * Baselib_NanosecondsPerMillisecond);
    }

    void Baselib_Timer_WaitForAtLeast(uint32_t timeInMilliseconds)
    {
        return Cpp11Api::Baselib_Timer_WaitForAtLeast(timeInMilliseconds);
    }

    double Baselib_Timer_GetTimeSinceStartupInSeconds()
    {
        return Common::Baselib_Timer_GetTimeSinceStartupInSeconds();
    }
}
