#include "Include/Baselib.h"
#include "Include/Cpp/Thread.h"

namespace baselib
{
    BASELIB_CPP_INTERFACE
    {
        Thread::~Thread()
        {
            BaselibAssert(thread == nullptr);
        }

        Thread::Thread(Thread&& other)
        {
            BaselibAssert(thread == nullptr);
            thread = other.thread;
            other.thread = nullptr;
        }

        Thread& Thread::operator=(Thread&& other)
        {
            // swap values
            auto t = thread;
            thread = other.thread;
            other.thread = t;
            return *this;
        }

        COMPILER_WARN_UNUSED_RESULT bool Thread::TryJoin(timeout_ms timeout)
        {
            if (thread == nullptr)
                return true;

            Baselib_ErrorState errorState = Baselib_ErrorState_Create();
            Baselib_Thread_Join(thread, timeout.count(), &errorState);
            if (Baselib_ErrorState_ErrorRaised(&errorState))
            {
                // ensure that we got timeout, and not some other error
                BaselibAssert(errorState.code == Baselib_ErrorCode_Timeout);
                return false;
            }
            else
            {
                thread = nullptr;
                return true;
            }
        }

        bool Thread::SupportsThreads()
        {
            return Baselib_Thread_SupportsThreads();
        }

        Baselib_Thread* Thread::CreateThread(Baselib_Thread_EntryPointFunction function, void* arg)
        {
            Baselib_Thread_Config cfg = {};
            cfg.entryPoint = function;
            cfg.entryPointArgument = arg;

            Baselib_ErrorState errorState = Baselib_ErrorState_Create();
            Baselib_Thread* thread = Baselib_Thread_Create(cfg, &errorState);
            thread = Baselib_ErrorState_ErrorRaised(&errorState) ? nullptr : thread;
            BaselibAssert(thread, "failed to create baselib thread");
            return thread;
        }
    }
}
