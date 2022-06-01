#pragma once

#include "Include/C/Baselib_SystemFutex.h"
#include "Include/C/Baselib_Timer.h"
#include "Include/Cpp/Algorithm.h"

namespace NoThreads
{
    BASELIB_INLINE_IMPL void Baselib_SystemFutex_Wait(int32_t* address, int32_t expected, uint32_t timeoutInMilliseconds)
    {
        if (*address == expected)
            Baselib_Timer_WaitForAtLeast(timeoutInMilliseconds);
    }

    BASELIB_INLINE_IMPL void Baselib_SystemFutex_Notify(int32_t* address, uint32_t count, Baselib_WakeupFallbackStrategy wakeupFallbackStrategy)
    {
    }
}
