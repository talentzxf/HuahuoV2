#pragma once

#include "Include/C/Baselib_SystemSemaphore.h"
#include "Include/C/Baselib_Process.h"
#include "Source/Baselib_ErrorState_Utils.h"
#include "Source/Baselib_InlineImplementation.h"
#include "Source/Baselib_Utilities.h"
#include "Baselib_Utilities_WinApi.h"

#include <windef.h>
#include <WinBase.h>

#include <algorithm>
#include <climits>

namespace WinApi
{
    BASELIB_INLINE_IMPL Baselib_SystemSemaphore_Handle Baselib_SystemSemaphore_Create()
    {
        HANDLE semaphore = CreateSemaphoreExW(NULL, 0, Baselib_SystemSemaphore_MaxCount, NULL, 0, (STANDARD_RIGHTS_REQUIRED | SEMAPHORE_MODIFY_STATE | SYNCHRONIZE));
        if (semaphore == NULL)
            Baselib_Process_Abort(Baselib_ErrorCode_OutOfSystemResources);
        return ::detail::AsBaselibHandle<Baselib_SystemSemaphore_Handle>(semaphore);
    }

    BASELIB_INLINE_IMPL void Baselib_SystemSemaphore_Acquire(Baselib_SystemSemaphore_Handle _semaphore)
    {
        const HANDLE semaphore = detail::AsHANDLE(_semaphore);
        const DWORD result = detail::ResolveErrorCode(WaitForSingleObjectEx(semaphore, INFINITE, FALSE));
        BaselibAssert(result == WAIT_OBJECT_0, "Unexpected windows error %d", result);
    }

    BASELIB_INLINE_IMPL bool Baselib_SystemSemaphore_TryAcquire(Baselib_SystemSemaphore_Handle _semaphore)
    {
        const HANDLE semaphore = detail::AsHANDLE(_semaphore);
        const DWORD result = detail::ResolveErrorCode(WaitForSingleObjectEx(semaphore, 0, FALSE));
        BaselibAssert(result == WAIT_OBJECT_0 || result == WAIT_TIMEOUT, "Unexpected windows error %d", result);
        return result == WAIT_OBJECT_0;
    }

    BASELIB_INLINE_IMPL bool Baselib_SystemSemaphore_TryTimedAcquire(Baselib_SystemSemaphore_Handle _semaphore, uint32_t timeoutInMilliseconds)
    {
        // Guard against infinite sleep
        if (timeoutInMilliseconds == INFINITE)
            timeoutInMilliseconds -= 1;

        const HANDLE semaphore = detail::AsHANDLE(_semaphore);
        const DWORD result = detail::ResolveErrorCode(WaitForSingleObjectEx(semaphore, timeoutInMilliseconds, FALSE));
        BaselibAssert(result == WAIT_OBJECT_0 || result == WAIT_TIMEOUT, "Unexpected windows error %d", result);
        return result == WAIT_OBJECT_0;
    }

    BASELIB_INLINE_IMPL void Baselib_SystemSemaphore_Release(Baselib_SystemSemaphore_Handle semaphore, uint32_t count)
    {
        if (count == 0)
            return;

        while (!ReleaseSemaphore(detail::AsHANDLE(semaphore), count, nullptr))
        {
            DWORD error = GetLastError();
            BaselibAssert(error == ERROR_TOO_MANY_POSTS, "Unexpected windows error %d", error);

            // exceeded max, increase one and get previous value
            LONG previousCount = 0;
            if (!ReleaseSemaphore(detail::AsHANDLE(semaphore), 1, &previousCount))
            {
                error = GetLastError();
                BaselibAssert(error == ERROR_TOO_MANY_POSTS, "Unexpected windows error %d", error);

                // at max capacity
                return;
            }
            const uint32_t deltaToMax = Baselib_SystemSemaphore_MaxCount - previousCount;
            count = std::min(deltaToMax, count) - 1;
        }
    }

    BASELIB_INLINE_IMPL void Baselib_SystemSemaphore_Free(Baselib_SystemSemaphore_Handle semaphore)
    {
        const BOOL closeHandleResult = CloseHandle(detail::AsHANDLE(semaphore));
        BaselibAssert(closeHandleResult, "Unexpected windows error %d", GetLastError());
    }
}
