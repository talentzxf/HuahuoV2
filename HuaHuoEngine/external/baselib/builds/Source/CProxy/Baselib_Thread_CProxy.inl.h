#pragma once

#include "Include/C/Baselib_Thread.h"

static inline bool IsValid(const Baselib_Thread_Config& config)
{
    if (PLATFORM_ARCH_32 && config.stackSize > std::numeric_limits<size_t>::max())
        return false;
    return config.entryPoint != nullptr;
}

BASELIB_C_INTERFACE
{
    Baselib_Thread* Baselib_Thread_Create(Baselib_Thread_Config config, Baselib_ErrorState* errorState)
    {
        errorState |= Validate(config);
        if (Baselib_ErrorState_ErrorRaised(errorState))
            return nullptr;
        return platform::Baselib_Thread_Create(config, errorState);
    }

    void Baselib_Thread_Join(Baselib_Thread* thread, uint32_t timeoutInMilliseconds, Baselib_ErrorState* errorState)
    {
        errorState |= Validate(AsPointer(thread));
        if (Baselib_ErrorState_ErrorRaised(errorState))
            return;
        if (Baselib_Thread_GetId(thread) == Baselib_Thread_GetCurrentThreadId())
            errorState |= RaiseError(Baselib_ErrorCode_ThreadCannotJoinSelf);
        if (Baselib_ErrorState_ErrorRaised(errorState))
            return;
        return platform::Baselib_Thread_Join(thread, timeoutInMilliseconds, errorState);
    }

    void Baselib_Thread_YieldExecution()
    {
        return platform::Baselib_Thread_YieldExecution();
    }

    Baselib_Thread_Id Baselib_Thread_GetCurrentThreadId()
    {
        return platform::Baselib_Thread_GetCurrentThreadId();
    }

    Baselib_Thread_Id Baselib_Thread_GetId(Baselib_Thread* thread)
    {
        if (!thread)
            return Baselib_Thread_InvalidId;

        // Assuming Baselib_Thread inherits Baselib_Thread_Common
        return thread->id;
    }

    bool Baselib_Thread_SupportsThreads()
    {
        return platform::Baselib_Thread_SupportsThreads();
    }
}
