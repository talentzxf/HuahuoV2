#pragma once

#include "Source/Baselib_InlineImplementation.h"
#include "Source/Baselib_Debug_Utils.h"
#include "Include/C/Baselib_Debug.h"
#include <sys/sysctl.h>
#include <string.h>
#include <unistd.h>

namespace DarwinApi
{
// taken from apple technical note QA1361
    BASELIB_INLINE_IMPL bool Baselib_Debug_IsDebuggerAttached_Native()
    {
        kinfo_proc info = {};

        int mib[4] = {CTL_KERN, KERN_PROC, KERN_PROC_PID, getpid()};

        size_t size = sizeof(info);
        sysctl(mib, sizeof(mib) / sizeof(*mib), &info, &size, nullptr, 0);

        return (info.kp_proc.p_flag & P_TRACED) != 0;
    }

    BASELIB_INLINE_IMPL bool Baselib_Debug_IsDebuggerAttached()
    {
        return Baselib_Debug_Cached_IsDebuggerAttached(Baselib_Debug_IsDebuggerAttached_Native);
    }
}
