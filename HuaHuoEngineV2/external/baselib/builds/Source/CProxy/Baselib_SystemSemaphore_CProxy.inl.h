#pragma once

#include "Include/C/Baselib_SystemSemaphore.h"

BASELIB_C_INTERFACE
{
    Baselib_SystemSemaphore_Handle Baselib_SystemSemaphore_Create()
    {
        return platform::Baselib_SystemSemaphore_Create();
    }

    void Baselib_SystemSemaphore_Acquire(Baselib_SystemSemaphore_Handle semaphore)
    {
        return platform::Baselib_SystemSemaphore_Acquire(semaphore);
    }

    bool Baselib_SystemSemaphore_TryAcquire(Baselib_SystemSemaphore_Handle semaphore)
    {
        return platform::Baselib_SystemSemaphore_TryAcquire(semaphore);
    }

    bool Baselib_SystemSemaphore_TryTimedAcquire(Baselib_SystemSemaphore_Handle semaphore, uint32_t timeoutInMilliseconds)
    {
        return platform::Baselib_SystemSemaphore_TryTimedAcquire(semaphore, timeoutInMilliseconds);
    }

    void Baselib_SystemSemaphore_Release(Baselib_SystemSemaphore_Handle semaphore, uint32_t count)
    {
        return platform::Baselib_SystemSemaphore_Release(semaphore, count);
    }

    void Baselib_SystemSemaphore_Free(Baselib_SystemSemaphore_Handle semaphore)
    {
        return platform::Baselib_SystemSemaphore_Free(semaphore);
    }
}
