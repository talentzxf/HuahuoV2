#pragma once

#include "Include/C/Baselib_SystemFutex.h"

BASELIB_C_INTERFACE
{
    void Baselib_SystemFutex_Wait(int32_t* address, int32_t expected, uint32_t timeoutInMilliseconds)
    {
        return platform::Baselib_SystemFutex_Wait(address, expected, timeoutInMilliseconds);
    }

    void Baselib_SystemFutex_Notify(int32_t* address, uint32_t count, Baselib_WakeupFallbackStrategy wakeupFallbackStrategy)
    {
        return platform::Baselib_SystemFutex_Notify(address, count, wakeupFallbackStrategy);
    }
}
