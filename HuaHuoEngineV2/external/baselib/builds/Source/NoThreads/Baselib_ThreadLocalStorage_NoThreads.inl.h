#pragma once

#include "Include/C/Baselib_ThreadLocalStorage.h"
#include "Include/C/Baselib_Memory.h"

namespace NoThreads
{
    BASELIB_INLINE_IMPL Baselib_TLS_Handle Baselib_TLS_Alloc(void)
    {
        uintptr_t* tlsAllocation = (uintptr_t*)Baselib_Memory_Allocate(sizeof(uintptr_t));
        *tlsAllocation = 0;
        return reinterpret_cast<Baselib_TLS_Handle>(tlsAllocation);
    }

    BASELIB_INLINE_IMPL void Baselib_TLS_Free(Baselib_TLS_Handle handle)
    {
        Baselib_Memory_Free(reinterpret_cast<void*>(handle));
    }

    BASELIB_INLINE_IMPL void Baselib_TLS_Set(Baselib_TLS_Handle handle, uintptr_t value)
    {
        *(uintptr_t*)handle = value;
    }

    BASELIB_INLINE_IMPL uintptr_t Baselib_TLS_Get(Baselib_TLS_Handle handle)
    {
        return *(uintptr_t*)handle;
    }
}
