#pragma once

#include "Include/C/Baselib_Memory.h"
#include "Include/C/Baselib_Timer.h"
#include "Include/C/Baselib_Process.h"
#include "Include/C/Baselib_SystemSemaphore.h"
#include "Source/Baselib_InlineImplementation.h"
#include "Source/Baselib_Utilities.h"
#include "Include/Cpp/Atomic.h"

#include <dispatch/dispatch.h>
#include <algorithm>

namespace DarwinApi
{
// Grand Central Dispatch is the preferred API for synchronization primitives on Apple platforms.
// See https://developer.apple.com/library/mac/documentation/Performance/Reference/GCD_libdispatch_Ref/index.html#//apple_ref/c/func/dispatch_semaphore_create
// and possible implementation at http://www.opensource.apple.com/source/libdispatch/libdispatch-84.5.3/src/semaphore.c
//
// A problem with libdispatch is that there are no safetynets so we need to do this our selves.
// For instance, if a semaphore exceed MAX_INT in a 32bit application the internal counter will overflow and turn negative.
// If that happens libdispatch thinks there are threads waiting on the semaphore even tho there aren't any.
// This is a bit unfortunate. Because of this we need to do an extra heap allocation to make room for our own counter.
namespace detail
{
    struct Semaphore
    {
        dispatch_semaphore_t handle;
        baselib::atomic<int32_t> counter;
    };

    static bool Acquire(Baselib_SystemSemaphore_Handle _semaphore, dispatch_time_t timeout)
    {
        detail::Semaphore* semaphore = ::detail::AsNativeType<detail::Semaphore*>(_semaphore);
        bool success = dispatch_semaphore_wait(semaphore->handle, timeout) == 0;
        if (success)
            semaphore->counter.fetch_sub(1, baselib::memory_order_release);
        return success;
    }
}

    BASELIB_INLINE_IMPL Baselib_SystemSemaphore_Handle Baselib_SystemSemaphore_Create()
    {
        detail::Semaphore* semaphore = static_cast<detail::Semaphore*>(Baselib_Memory_Allocate(sizeof(detail::Semaphore)));
        semaphore->handle = dispatch_semaphore_create(0);
        semaphore->counter = 0;
        if (semaphore->handle == nullptr) // should never happen on apple systems (ulock based)
            Baselib_Process_Abort(Baselib_ErrorCode_OutOfSystemResources);
        return ::detail::AsBaselibHandle<Baselib_SystemSemaphore_Handle>(semaphore);
    }

    BASELIB_INLINE_IMPL void Baselib_SystemSemaphore_Acquire(Baselib_SystemSemaphore_Handle semaphore)
    {
        bool semaphoreAcquired = detail::Acquire(semaphore, DISPATCH_TIME_FOREVER);
        BaselibAssert(semaphoreAcquired, "unable to acquire semaphore");
    }

    BASELIB_INLINE_IMPL bool Baselib_SystemSemaphore_TryAcquire(Baselib_SystemSemaphore_Handle semaphore)
    {
        return detail::Acquire(semaphore, DISPATCH_TIME_NOW);
    }

    BASELIB_INLINE_IMPL bool Baselib_SystemSemaphore_TryTimedAcquire(Baselib_SystemSemaphore_Handle semaphore, uint32_t timeoutInMilliseconds)
    {
        const dispatch_time_t dispatchTimeout = dispatch_time(DISPATCH_TIME_NOW, timeoutInMilliseconds * Baselib_NanosecondsPerMillisecond);
        return detail::Acquire(semaphore, dispatchTimeout);
    }

    BASELIB_INLINE_IMPL void Baselib_SystemSemaphore_Release(Baselib_SystemSemaphore_Handle _semaphore, uint32_t count)
    {
        detail::Semaphore* semaphore = ::detail::AsNativeType<detail::Semaphore*>(_semaphore);

        int32_t newCount;
        int32_t previousCount = semaphore->counter.load(baselib::memory_order_relaxed);
        do
        {
            newCount = std::min<int64_t>(previousCount + count, Baselib_SystemSemaphore_MaxCount);
        }
        while (!semaphore->counter.compare_exchange_weak(previousCount, newCount, baselib::memory_order_acq_rel, baselib::memory_order_acquire));

        for (int32_t i = 0; i < newCount - previousCount; ++i)
            dispatch_semaphore_signal(semaphore->handle);
    }

    BASELIB_INLINE_IMPL void Baselib_SystemSemaphore_Free(Baselib_SystemSemaphore_Handle _semaphore)
    {
        detail::Semaphore* semaphore = ::detail::AsNativeType<detail::Semaphore*>(_semaphore);
        dispatch_release(semaphore->handle);
        Baselib_Memory_Free(semaphore);
    }
}
