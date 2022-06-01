#pragma once

#include "Include/C/Baselib_Debug.h"

BASELIB_C_INTERFACE
{
    bool Baselib_Debug_IsDebuggerAttached()
    {
        return platform::Baselib_Debug_IsDebuggerAttached();
    }
}
