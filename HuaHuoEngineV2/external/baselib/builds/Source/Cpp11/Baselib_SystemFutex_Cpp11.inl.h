#pragma once

#include "Include/C/Baselib_WakeupFallbackStrategy.h"
#include "Source/Baselib_InlineImplementation.h"

#include <mutex>
#include <condition_variable>

#if PLATFORM_FUTEX_NATIVE_SUPPORT
    #error "This implementation should only be used when the platform doesn't have native futex support."
#endif

namespace Cpp11Api
{
namespace detail
{
    static struct Futex
    {
        std::condition_variable condition;
        std::mutex              mutex;
    } s_Futex;
}

    BASELIB_INLINE_IMPL void Baselib_SystemFutex_Wait(int32_t* address, int32_t expected, uint32_t timeoutInMilliseconds)
    {
        detail::Futex& futex = detail::s_Futex;
        std::unique_lock<std::mutex> lock(futex.mutex);

        // a volatile compare should be good enough
        // but a relaxed load here when we get access to baselib atomics would be better.
        BaselibAssert((reinterpret_cast<uintptr_t>(address) & 0x3) == 0, "Address (%p) have to be naturally aligned, i.e. it needs 4 byte alignment.", address);
        if (*static_cast<volatile int32_t*>(address) != expected)
            return;

        futex.condition.wait_for(lock, std::chrono::milliseconds(timeoutInMilliseconds));
    }

    BASELIB_INLINE_IMPL void Baselib_SystemFutex_Notify(int32_t*, const uint32_t count, const Baselib_WakeupFallbackStrategy wakeupFallbackStrategy)
    {
        detail::Futex& futex = detail::s_Futex;
        {
            std::unique_lock<std::mutex>(futex.mutex);
        }
        if (wakeupFallbackStrategy == Baselib_WakeupFallbackStrategy_All)
            return futex.condition.notify_all();

        for (uint32_t i = 0; i < count; ++i)
            futex.condition.notify_one();
    }
}
