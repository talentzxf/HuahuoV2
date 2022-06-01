#pragma once

#include "Include/C/Baselib_Memory.h"
#include "Include/C/Baselib_Process.h"
#include "Include/C/Baselib_Thread.h"
#include "Include/Cpp/EventSemaphore.h"
#include "Source/Baselib_Thread_Utils.h"

#include "Source/Posix/ErrorStateBuilder_PosixApi.inl.h"

#include <pthread.h>
#include <new>
#include <string>

BASELIB_C_INTERFACE
{
    struct Baselib_Thread : Baselib_Thread_Common
    {
        Baselib_Thread(Baselib_Thread_EntryPointFunction threadEntryPoint, void* threadEntryPointArgument, const std::string& threadName)
            : Baselib_Thread_Common(threadEntryPoint, threadEntryPointArgument), eventSemaphore(), name(threadName), handle() {}
        baselib::EventSemaphore eventSemaphore;
        std::string name;
        pthread_t handle;
    };
}

namespace PosixApi
{
namespace detail
{
    template<void(*SetNameForCurrentThread)(const char*)>
    static void* ThreadEntryPoint(void* baselibThread)
    {
        Baselib_Thread* thread = static_cast<Baselib_Thread*>(baselibThread);
        // Note that thread is not guaranteed to be completely constructed at this point, specifically thread->id might be unset.
        if (!thread->name.empty())
            SetNameForCurrentThread(thread->name.c_str());
        thread->entryPoint(thread->entryPointArgument);
        thread->eventSemaphore.Set();
        return nullptr;
    }
}

    static inline void FreeThread(Baselib_Thread* thread)
    {
        thread->~Baselib_Thread();
        Baselib_Memory_Free(thread);
    }

#ifndef PTHREAD_STACK_MIN
#define PTHREAD_STACK_MIN 16384
#endif


    struct ScopedAttributes
    {
        ScopedAttributes()
        {
            pthread_attr_init(&attr);
        }

        ~ScopedAttributes()
        {
            pthread_attr_destroy(&attr);
        }

        pthread_attr_t attr;
    };


    template<void(*SetNameForCurrentThread)(const char*)>
    BASELIB_INLINE_IMPL Baselib_Thread* Baselib_Thread_Create(Baselib_Thread_Config config, Baselib_ErrorState* errorState)
    {
        ScopedAttributes scopedAttributes;
        pthread_attr_t& attr = scopedAttributes.attr;

        size_t stackSize = (size_t)config.stackSize; // safety of this case already ensured by cproxy

        if (stackSize != 0)
        {
            if (stackSize < PTHREAD_STACK_MIN)
                stackSize = PTHREAD_STACK_MIN;
            Baselib_Memory_PageSizeInfo info;
            ::Baselib_Memory_GetPageSizeInfo(&info);

            if (stackSize % info.defaultPageSize) // This requirement is only for Darwin, not for Linux.
                                                  // It's put here anyway as it doesn't hurt that much.
                stackSize += info.defaultPageSize - stackSize % info.defaultPageSize;
            int errorCode = pthread_attr_setstacksize(&attr, stackSize);
            if (errorCode)
            {
                BaselibAssert(false, "Could not set stack size");
                errorState |= RaiseError(Baselib_ErrorCode_UnexpectedError) | WithErrno(errorCode);
                return nullptr;
            }
        }

        Baselib_Thread* thread = new(Baselib_Memory_Allocate(sizeof(Baselib_Thread)))
            Baselib_Thread(config.entryPoint, config.entryPointArgument, std::string(config.name ? config.name : ""));

        int errorCode = pthread_create(&thread->handle, &attr, detail::ThreadEntryPoint<SetNameForCurrentThread>, thread);
        if (errorCode)
        {
            FreeThread(thread);
            switch (errorCode)
            {
                case EAGAIN:
                    errorState |= RaiseError(Baselib_ErrorCode_OutOfSystemResources) | WithErrno(errorCode);
                    return nullptr;
                case EPERM:
                // Error code for this?
                case EINVAL:
                default:
                    BaselibAssert(false, "Could not create thread");
                    errorState |= RaiseError(Baselib_ErrorCode_UnexpectedError) | WithErrno(errorCode);
                    return nullptr;
            }
        }

        static_assert(sizeof(thread->id) >= sizeof(thread->handle), "Thread handle does not fit into Baselib_Thread_Id");
        thread->id = 0;
        std::memcpy(&thread->id, &thread->handle, sizeof(thread->handle));

        return thread;
    }

    BASELIB_INLINE_IMPL void Baselib_Thread_Join(Baselib_Thread* thread, uint32_t timeoutInMilliseconds, Baselib_ErrorState* errorState)
    {
        if (!thread->eventSemaphore.TryTimedAcquire(baselib::timeout_ms(timeoutInMilliseconds)))
        {
            errorState |= RaiseError(Baselib_ErrorCode_Timeout);
            return;
        }
        int errorCode = pthread_join(thread->handle, NULL);
        if (errorCode)
        {
            BaselibAssert(false, "pthread_join failed with %i", errorCode);
            Baselib_Process_Abort(Baselib_ErrorCode_UnexpectedError);
        }
        FreeThread(thread);
    }

    BASELIB_INLINE_IMPL void Baselib_Thread_YieldExecution()
    {
        sched_yield();
    }

    BASELIB_INLINE_IMPL Baselib_Thread_Id Baselib_Thread_GetCurrentThreadId()
    {
        auto threadHandle = pthread_self();
        static_assert(sizeof(Baselib_Thread_Id) >= sizeof(threadHandle), "Thread handle does not fit into Baselib_Thread_Id");
        Baselib_Thread_Id threadId = 0;
        std::memcpy(&threadId, &threadHandle, sizeof(threadHandle));
        return threadId;
    }
}
