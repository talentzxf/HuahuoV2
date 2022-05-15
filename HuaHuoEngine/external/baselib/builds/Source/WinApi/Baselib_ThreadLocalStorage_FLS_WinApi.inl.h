#pragma once

// On UWP, old Windows SDKs (10586 and older) did not have support for Tls* functions
// so instead we used forwarded them to Fls* equivalents. Using Tls* functions directly
// with these old SDKs causes linker errors.

#include "Source/Baselib_InlineImplementation.h"
#include "Include/C/Baselib_ThreadLocalStorage.h"
#include "Source/AbortShim.h"

#include <fibersapi.h>
#include <errhandlingapi.h>

static_assert(sizeof(Baselib_TLS_Handle) >= sizeof(DWORD), "Baselib_TLS_Handle must be able to hold all values of DWORD");

namespace WinApi
{
namespace FLS
{
    BASELIB_INLINE_IMPL Baselib_TLS_Handle Baselib_TLS_Alloc()
    {
        DWORD r = FlsAlloc(0);
        if (r == FLS_OUT_OF_INDEXES)
            ShimmableAbort(Baselib_ErrorCode_OutOfSystemResources);
        return static_cast<Baselib_TLS_Handle>(r);
    }

    BASELIB_INLINE_IMPL void Baselib_TLS_Free(Baselib_TLS_Handle handle)
    {
        BOOL success = FlsFree(static_cast<DWORD>(handle));
        BaselibAssert(success != 0);
    }
}
}
