#pragma once

#include "Include/C/Baselib_WakeupFallbackStrategy.h"
#include "Source/Baselib_InlineImplementation.h"

#include <windef.h>
#include <WinBase.h>

namespace WinApi
{
    BASELIB_INLINE_IMPL void Baselib_SystemFutex_Wait(int32_t* address, int32_t expected, uint32_t timeoutInMilliseconds)
    {
        // Guard against infinite sleep
        if (timeoutInMilliseconds == INFINITE)
            timeoutInMilliseconds -= 1;

        const BOOL result = WaitOnAddress(address, &expected, sizeof(expected), timeoutInMilliseconds);
        BaselibAssert(result || GetLastError() == ERROR_TIMEOUT);
    }

    BASELIB_INLINE_IMPL void Baselib_SystemFutex_Notify(int32_t* address, const uint32_t count, const Baselib_WakeupFallbackStrategy wakeupFallbackStrategy)
    {
        if (wakeupFallbackStrategy == Baselib_WakeupFallbackStrategy_All)
            return WakeByAddressAll(address);

        for (uint32_t i = 0; i < count; ++i)
            WakeByAddressSingle(address);
    }
}
