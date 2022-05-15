#pragma once

#include <errno.h>
#include <sys/prctl.h>

namespace LinuxApi
{
    static inline void Thread_SetNameForCurrentThread(const char* name)
    {
        // String is internally truncated to 15 chars (+ nullterminator).
        int result = prctl(PR_SET_NAME, name);
        BaselibAssert(result == 0, "prctl failed to set current thread name. Errno is set to: %i", errno);
    }
}
