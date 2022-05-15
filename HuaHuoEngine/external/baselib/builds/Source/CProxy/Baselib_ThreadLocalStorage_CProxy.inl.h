#pragma once

#include "Include/C/Baselib_ThreadLocalStorage.h"

BASELIB_C_INTERFACE
{
    Baselib_TLS_Handle Baselib_TLS_Alloc()
    {
        return platform::Baselib_TLS_Alloc();
    }

    void Baselib_TLS_Free(Baselib_TLS_Handle handle)
    {
        return platform::Baselib_TLS_Free(handle);
    }
}
