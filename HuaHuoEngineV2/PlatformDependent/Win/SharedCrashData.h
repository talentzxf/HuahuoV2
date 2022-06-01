#pragma once
#include "PlatformDependent\Win\WinUtils.h"

#if ENABLE_OUT_OF_PROCESS_CRASH_HANDLER
namespace winutils
{
    struct CrashDataHeader
    {
        static const uint64_t k_ID = 0x556e6974794348ull;
#ifdef _AMD64_
        static const uint64_t k_Arch = 1ull;
#else
        static const uint64_t k_Arch = 0ull;
#endif
        static const uint64_t k_Version = 3ull;

        static_assert((k_ID & 0xff00000000000000) == 0, "ID cannot be occupy more than 7 bytes");
        static_assert(k_Arch <= 1, "Arch must be a single bit");
        static_assert((k_Version & 0x80) == 0, "Version cannot be more than 7 bits");

        static const uint64_t k_Magic = (k_ID << 8) | (k_Arch << 7) | k_Version;

        uint64_t Magic;
        uint32_t ProcessID;
        uint32_t ThreadID;
        uint32_t HeaderSize;
        const void* StackTop;
        const void* MonoDomain;
        const void* MonoFAT64;
        EXCEPTION_RECORD ExceptionRecord;
        CONTEXT Context;
        struct
        {
            char IsFatal : 1;
        } Flags;
        char Description[1024]; // not used when ExceptionRecord is valid
    };

    struct SharedCrashNotificationEvents
    {
        AutoHandle HostDataReady;
        AutoHandle HostCanContinue;
        AutoHandle RequestTerminate;
    };

    bool CreateSharedCrashNotificationEvents(DWORD processID, SharedCrashNotificationEvents& events);

    enum CrashHandlerProcessExitCode
    {
        kCrashHandlerExitCodeNormal,
        kCrashHandlerExitCodeBugReporterStarted
    };
}
#endif // ENABLE_OUT_OF_PROCESS_CRASH_HANDLER
