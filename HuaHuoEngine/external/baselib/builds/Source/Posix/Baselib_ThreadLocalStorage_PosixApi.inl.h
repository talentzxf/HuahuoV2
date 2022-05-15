#pragma once

#include "Source/Baselib_InlineImplementation.h"
#include "Include/C/Baselib_ThreadLocalStorage.h"
#include "Source/AbortShim.h"

#include <pthread.h>
#include <errno.h>

static_assert(sizeof(Baselib_TLS_Handle) >= sizeof(pthread_key_t), "Baselib_TLS_Handle must be able to hold all values of pthread_key_t");

namespace PosixApi
{
    BASELIB_INLINE_IMPL Baselib_TLS_Handle Baselib_TLS_Alloc()
    {
        pthread_key_t r;
        int rc = pthread_key_create(&r, NULL);
        if (rc != 0)
            ShimmableAbort(Baselib_ErrorCode_OutOfSystemResources);
        return static_cast<Baselib_TLS_Handle>(r);
    }

    BASELIB_INLINE_IMPL void Baselib_TLS_Free(Baselib_TLS_Handle handle)
    {
        int rc = pthread_key_delete(static_cast<pthread_key_t>(handle));
        BaselibAssert(rc == 0);
    }
}
