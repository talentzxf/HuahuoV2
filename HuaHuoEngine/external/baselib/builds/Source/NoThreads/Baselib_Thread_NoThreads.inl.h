#pragma once

#include "Include/C/Baselib_ErrorState.h"
#include "Include/C/Baselib_Thread.h"
#include "Source/Baselib_Thread_Utils.h"

BASELIB_C_INTERFACE
{
    struct Baselib_Thread : Baselib_Thread_Common
    {
        Baselib_Thread(Baselib_Thread_EntryPointFunction threadEntryPoint, void* threadEntryPointArgument, const std::string& threadName)
            : Baselib_Thread_Common(threadEntryPoint, threadEntryPointArgument), name(threadName) {}
        std::string name;
    };
}

namespace NoThreads
{
    BASELIB_INLINE_IMPL Baselib_Thread* Baselib_Thread_Create(Baselib_Thread_Config config, Baselib_ErrorState* errorState)
    {
        errorState |= RaiseError(Baselib_ErrorCode_NotSupported);
        return nullptr;
    }

    BASELIB_INLINE_IMPL void Baselib_Thread_Join(Baselib_Thread* thread, uint32_t timeoutInMilliseconds, Baselib_ErrorState* errorState)
    {
        errorState |= RaiseError(Baselib_ErrorCode_NotSupported);
    }

    BASELIB_INLINE_IMPL void Baselib_Thread_YieldExecution()
    {
    }

    BASELIB_INLINE_IMPL Baselib_Thread_Id Baselib_Thread_GetId(Baselib_Thread* thread)
    {
        return Baselib_Thread_InvalidId;
    }

    BASELIB_INLINE_IMPL Baselib_Thread_Id Baselib_Thread_GetCurrentThreadId()
    {
        return 0x1337;
    }

    BASELIB_INLINE_IMPL bool Baselib_Thread_SupportsThreads()
    {
        return false;
    }
}
