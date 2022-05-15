#pragma once

//
// This code is only included on Mac to prevent coderot, it's not actually used.
// @marton and @povilas are discussing options for including this.
//
// ulocks are used by Apples "Grand Central Dispatch" library - libdispath
// https://github.com/apple/swift-corelibs-libdispatch/blob/631821c2bfcf296995a8424f148a9365470c2210/src/shims/lock.c#L330
//
#include "Include/C/Baselib_SystemFutex.h"
#include "Include/C/Baselib_Timer.h"
#include "Include/C/Baselib_WakeupFallbackStrategy.h"
#include "Source/Baselib_InlineImplementation.h"

#include <sys/errno.h>
#include <sys/syscall.h>
#include <unistd.h>

namespace DarwinApi
{
namespace detail
{
    //
    // ulock syscalls are declared using a 64 bit integers, but ulock specifically work on 32 bit integers
    // https://github.com/apple/darwin-xnu/blob/0a798f6738bc1db01281fc08ae024145e84df927/bsd/kern/sys_ulock.c#L407
    //

    static constexpr uint32_t UL_COMPARE_AND_WAIT = 1;
    static constexpr uint32_t ULF_WAKE_ALL = 0x00000100;

    BASELIB_INLINE_IMPL int ulock_wait(uint32_t operation, void *addr, uint64_t value, uint32_t timeout_us)
    {
        BaselibAssert((reinterpret_cast<uintptr_t>(addr) & 0x3) == 0, "Address (%p) have to be naturally aligned, i.e. it needs 4 byte alignment.", addr);
        return syscall(SYS_ulock_wait, operation, addr, value, timeout_us);
    }

    BASELIB_INLINE_IMPL int ulock_wake(uint32_t operation, void *addr, uint64_t wake_value)
    {
        BaselibAssert((reinterpret_cast<uintptr_t>(addr) & 0x3) == 0, "Address (%p) have to be naturally aligned, i.e. it needs 4 byte alignment.", addr);
        return syscall(SYS_ulock_wake, operation, addr, wake_value);
    }
}

    BASELIB_INLINE_IMPL void Baselib_SystemFutex_Wait(int32_t* address, const int32_t expected, const uint32_t timeoutInMilliseconds)
    {
        // apple likes the timeout in microseconds
        uint32_t timeoutInMicroseconds;

        // zero means wait forever. we don't want that :)
        if (timeoutInMilliseconds == 0)
            timeoutInMicroseconds = 1;
        // if we can't hold the timeout value in a uint32_t we wakeup spuriously after about an hour
        else if (timeoutInMilliseconds * Baselib_MicrosecondsPerMillisecond > UINT32_MAX)
            timeoutInMicroseconds = UINT32_MAX;
        // convert to micro seconds
        else
            timeoutInMicroseconds = timeoutInMilliseconds * Baselib_MicrosecondsPerMillisecond;

        const int res = detail::ulock_wait(detail::UL_COMPARE_AND_WAIT, address, expected, timeoutInMicroseconds);
        BaselibAssert(res >= 0 || errno == ETIMEDOUT);
    }

    BASELIB_INLINE_IMPL void Baselib_SystemFutex_Notify(int32_t* address, const uint32_t count, const Baselib_WakeupFallbackStrategy wakeupFallbackStrategy)
    {
        if (wakeupFallbackStrategy == Baselib_WakeupFallbackStrategy_All)
        {
            const int res = detail::ulock_wake(detail::UL_COMPARE_AND_WAIT | detail::ULF_WAKE_ALL, address, 0);
            BaselibAssert(res == 0 || errno == ENOENT);
            return;
        }
        for (uint32_t i = 0; i < count; ++i)
        {
            const int res = detail::ulock_wake(detail::UL_COMPARE_AND_WAIT, address, 0);
            BaselibAssert(res == 0 || errno == ENOENT);
            if (res != 0 && errno == ENOENT)
                return;
        }
    }
}
