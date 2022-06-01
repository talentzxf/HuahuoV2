#pragma once

#include "Source/Baselib_InlineImplementation.h"
#include "Include/C/Baselib_ThreadLocalStorage.h"
#include "Source/AbortShim.h"

#include <processthreadsapi.h>
#include <errhandlingapi.h>
#ifndef ERROR_SUCCESS
#define ERROR_SUCCESS 0L
#endif

static_assert(sizeof(Baselib_TLS_Handle) >= sizeof(DWORD), "Baselib_TLS_Handle must be able to hold all values of DWORD");

namespace WinApi
{
namespace TLS
{
    BASELIB_INLINE_IMPL Baselib_TLS_Handle Baselib_TLS_Alloc()
    {
        DWORD r = TlsAlloc();
        if (r == TLS_OUT_OF_INDEXES)
            ShimmableAbort(Baselib_ErrorCode_OutOfSystemResources);
        return static_cast<Baselib_TLS_Handle>(r);
    }

    BASELIB_INLINE_IMPL void Baselib_TLS_Free(Baselib_TLS_Handle handle)
    {
        BOOL success = TlsFree(static_cast<DWORD>(handle));
        BaselibAssert(success != 0);
    }
}
}
