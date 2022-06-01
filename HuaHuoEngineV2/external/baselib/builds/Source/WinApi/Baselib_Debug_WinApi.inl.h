#pragma once

#include "Source/Baselib_InlineImplementation.h"
#include "Include/C/Baselib_Debug.h"
#include <windef.h>
#include <winbase.h>

namespace WinApi
{
    BASELIB_INLINE_IMPL bool Baselib_Debug_IsDebuggerAttached()
    {
        return IsDebuggerPresent() == TRUE;
    }
}
