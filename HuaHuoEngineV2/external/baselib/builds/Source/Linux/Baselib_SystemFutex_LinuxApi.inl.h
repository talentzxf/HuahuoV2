#pragma once

#include "Include/C/Baselib_Timer.h"
#include "Include/C/Baselib_WakeupFallbackStrategy.h"
#include "Source/Baselib_InlineImplementation.h"

#include <errno.h>
#include <time.h>
#include <unistd.h>
#include <linux/futex.h>
#include <sys/syscall.h>

namespace LinuxApi
{
namespace detail
{
    static inline int futex(int *uaddr, int futex_op, int val, const struct timespec *timeout, int *uaddr2, int val3)
    {
        BaselibAssert((reinterpret_cast<uintptr_t>(uaddr) & 0x3) == 0, "Address (%p) have to be naturally aligned, i.e. it needs 4 byte alignment.", uaddr);
        return syscall(SYS_futex, uaddr, futex_op, val, timeout, uaddr2, val3);
    }
}

    BASELIB_INLINE_IMPL void Baselib_SystemFutex_Wait(int32_t* address, const int32_t expected, const uint32_t timeoutInMilliseconds)
    {
        timespec timeToSleep;
        timeToSleep.tv_sec  = timeoutInMilliseconds / Baselib_MillisecondsPerSecond;
        timeToSleep.tv_nsec = timeoutInMilliseconds % Baselib_MillisecondsPerSecond * Baselib_NanosecondsPerMillisecond;

        const int res = detail::futex(address, FUTEX_WAIT | FUTEX_PRIVATE_FLAG, expected, &timeToSleep, nullptr, 0);
        BaselibAssert(res == 0 || errno == EAGAIN || errno == EINTR || errno == ETIMEDOUT);
    }

    BASELIB_INLINE_IMPL void Baselib_SystemFutex_Notify(int32_t* address, uint32_t count, const Baselib_WakeupFallbackStrategy wakeupFallbackStrategy)
    {
        const int signedCountValue = count > INT32_MAX ? INT32_MAX : static_cast<int>(count);
        const int res = detail::futex(address, FUTEX_WAKE | FUTEX_PRIVATE_FLAG, signedCountValue, nullptr, nullptr, 0);
        BaselibAssert(res >= 0);
    }
}
