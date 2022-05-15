#pragma once

#include "Source/Baselib_InlineImplementation.h"
#include "Source/Baselib_Debug_Utils.h"
#include "Include/C/Baselib_Debug.h"
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <ctype.h>

namespace LinuxApi
{
    BASELIB_INLINE_IMPL bool Baselib_Debug_IsDebuggerAttached_Native()
    {
        // try reading /proc/self/status into memory
        int fd = open("/proc/self/status", O_RDONLY);
        if (fd < 0)
            return false;

        char buf[4096];
        ssize_t size = read(fd, buf, sizeof(buf) - 1);
        int rc = close(fd);
        BaselibAssert(rc == 0);
        if (size <= 0)
            return false;
        buf[size] = '\0';

        // try finding TracerPid key, and skip after it
        const char keyword[11] = "TracerPid:";
        const char * substring = strstr(buf, keyword);
        if (!substring)
            return false;
        substring += sizeof(keyword) - 1;

        // non zero TracerPid means that there is a tracer present
        // so we check that first non-space character is in [1, 9]
        for (; substring <= buf + size; ++substring)
        {
            char c = *substring;
            if (isspace(c))
            {
                continue;
            }
            else
            {
                return (c >= '1' && c <= '9');
            }
        }

        return false;
    }

    BASELIB_INLINE_IMPL bool Baselib_Debug_IsDebuggerAttached()
    {
        return Baselib_Debug_Cached_IsDebuggerAttached(Baselib_Debug_IsDebuggerAttached_Native);
    }
}
