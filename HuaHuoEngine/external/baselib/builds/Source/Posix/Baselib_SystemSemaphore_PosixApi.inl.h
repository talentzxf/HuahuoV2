#pragma once

#include "Include/C/Baselib_Memory.h"
#include "Include/C/Baselib_SystemSemaphore.h"
#include "Include/C/Baselib_Process.h"
#include "Include/C/Baselib_Timer.h"
#include "Source/Baselib_InlineImplementation.h"
#include "Source/Baselib_Utilities.h"

#include <errno.h>
#include <limits.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>

namespace PosixApi
{
    static inline timespec Baselib_TimespecEpochFromMilliseconds(uint32_t timeoutInMilliseconds)
    {
        timespec tsTimeoutTimestamp = {0, 0};
        const int clock_gettime_result = clock_gettime(CLOCK_REALTIME, &tsTimeoutTimestamp);
        BaselibAssert(clock_gettime_result == 0, "Unexpected posix error %i", errno);

        tsTimeoutTimestamp.tv_sec += timeoutInMilliseconds / Baselib_MillisecondsPerSecond;
        tsTimeoutTimestamp.tv_nsec += timeoutInMilliseconds * Baselib_NanosecondsPerMillisecond % Baselib_NanosecondsPerSecond;
        if (tsTimeoutTimestamp.tv_nsec > static_cast<decltype(tsTimeoutTimestamp.tv_nsec)>(Baselib_NanosecondsPerSecond))
        {
            tsTimeoutTimestamp.tv_nsec -= Baselib_NanosecondsPerSecond;
            tsTimeoutTimestamp.tv_sec += 1;
        }
        return tsTimeoutTimestamp;
    }

    BASELIB_INLINE_IMPL Baselib_SystemSemaphore_Handle Baselib_SystemSemaphore_Create()
    {
        static_assert((static_cast<int32_t>(SEM_VALUE_MAX) <= Baselib_SystemSemaphore_MaxCount) && (Baselib_SystemSemaphore_MaxCount <= INT32_MAX), "Platform does not support a high enough semaphore value");

        sem_t* sem = static_cast<sem_t*>(Baselib_Memory_Allocate(sizeof(sem_t)));
        const int res = sem_init(sem, 0, 0);
        if (res != 0) // should never happen on modern (futex) based implementations
            Baselib_Process_Abort(Baselib_ErrorCode_OutOfSystemResources);
        return ::detail::AsBaselibHandle<Baselib_SystemSemaphore_Handle>(sem);
    }

    BASELIB_INLINE_IMPL void Baselib_SystemSemaphore_Release(Baselib_SystemSemaphore_Handle semaphore, uint32_t count)
    {
        for (uint32_t i = 0; i < count; ++i)
        {
            const int sem_post_result = sem_post(::detail::AsNativeType<sem_t*>(semaphore));
            if (sem_post_result != 0 && errno == EOVERFLOW)
                return;
            BaselibAssert(sem_post_result == 0, "Unexpected posix error %i", errno);
        }
    }

    BASELIB_INLINE_IMPL void Baselib_SystemSemaphore_Acquire(Baselib_SystemSemaphore_Handle semaphore)
    {
        while (sem_wait(::detail::AsNativeType<sem_t*>(semaphore)) != 0)
        {
            BaselibAssert(errno == EINTR, "Unexpected posix error %i", errno);
        }
    }

    BASELIB_INLINE_IMPL bool Baselib_SystemSemaphore_TryAcquire(Baselib_SystemSemaphore_Handle semaphore)
    {
        const int result = sem_trywait(::detail::AsNativeType<sem_t*>(semaphore));
        // spec says errno should be EAGAIN if semaphore is not flagged, but some implementations give ETIMEDOUT.
        BaselibAssert(result == 0 || errno == ETIMEDOUT || errno == EAGAIN, "Unexpected posix error %i", errno);
        return result == 0;
    }

    BASELIB_INLINE_IMPL bool Baselib_SystemSemaphore_TryTimedAcquire(Baselib_SystemSemaphore_Handle semaphore, uint32_t timeoutInMilliseconds)
    {
        const timespec tsTimeoutTimestamp = Baselib_TimespecEpochFromMilliseconds(timeoutInMilliseconds);
        while (sem_timedwait(::detail::AsNativeType<sem_t*>(semaphore), &tsTimeoutTimestamp) != 0)
        {
            if (errno == EINTR) // spurious wakeup
                continue;

            BaselibAssert(errno == ETIMEDOUT);
            return false;
        }
        return true;
    }

    BASELIB_INLINE_IMPL void Baselib_SystemSemaphore_Free(Baselib_SystemSemaphore_Handle semaphore)
    {
        const int res = sem_destroy(::detail::AsNativeType<sem_t*>(semaphore));
        Baselib_Memory_Free(::detail::AsNativeType<sem_t*>(semaphore));
        BaselibAssert(res == 0, "Unexpected posix error %i", errno);
    }
}
