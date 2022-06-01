#include "UnityPrefix.h"

#if ENABLE_OUT_OF_PROCESS_CRASH_HANDLER
#include "ExternalCrashHandler.h"
#include "Editor/Platform/Interface/BugReportingTools.h"
#include "Runtime/Mono/MonoIncludes.h"
#include "Runtime/Mono/MonoManager.h"
#include "PlatformDependent/Win/ProcessThreadSnapshot.h"
#include "WinUtils.h"

#include <winuser.h>
#include <signal.h>

#if UNITY_64
#   define UNITY_CRASH_HANDLER_EXE L"UnityCrashHandler64.exe"
#else
#   define UNITY_CRASH_HANDLER_EXE L"UnityCrashHandler32.exe"
#endif

#if UNITY_EDITOR
#   define UNITY_CRASH_HANDLER_PATH L"Data\\Tools\\" UNITY_CRASH_HANDLER_EXE
#else
#   define UNITY_CRASH_HANDLER_PATH UNITY_CRASH_HANDLER_EXE // same directory as exe
#endif

namespace winutils
{
    ExternalCrashHandler::ExternalCrashHandler()
        : m_SharedMemory(sizeof(CrashDataHeader))
        , m_ProcessID(0)
    {
        m_CrashHandlerPath[0] = L'\0';
    }

    ExternalCrashHandler::~ExternalCrashHandler()
    {
        Unload();
    }

    void ExternalCrashHandler::GenerateCrashHandlerCommandLine(
        wchar_t* outputPath,
        size_t outputPathLen,
        const wchar_t* crashHandlerExe,
        const void* sharedAreaPtr)
    {
        //
        // NOTE: We aren't using GetApplicationContentsPath here deliberately.
        // This function relies on LocalFileSystem, VirtualFileSystem, etc,
        // which are all Unity subsystems that haven't been initialized yet.
        //

        memset(outputPath, 0, outputPathLen * sizeof(*outputPath));

        outputPath[0] = L'\"';

        int pathLen = (int)GetModuleFileNameW(
            GetModuleHandleW(nullptr),
            outputPath + 1,
            outputPathLen - 1) + 1;
        if (GetLastError() == ERROR_INSUFFICIENT_BUFFER)
        {
            printf("WARNING: may have issues launching crash handler because launch path is too long.\n");
        }

        // Wind back and find the last \ or //
        for (;
             outputPath[pathLen] != L'\\' &&
             outputPath[pathLen] != '/' &&
             pathLen >= 0;
             --pathLen)
            ;
        outputPath[pathLen + 1] = L'\0';

        // Now generate argument string for the PID and VA
        // (%p is outputs inconsistently across various platforms)
        wchar_t args[256] = { 0 };
        swprintf_s(
            args,
            _countof(args),
            L"%s\" --attach %u %llu",
            crashHandlerExe,
            GetCurrentProcessId(),
            reinterpret_cast<uint64_t>(sharedAreaPtr));

        // Append this to the exe name
        wcscat_s(
            outputPath,
            outputPathLen,
            args);

        // These are only useful if you are debugging the crash handler process itself
        if (wcsstr(GetCommandLineW(), L"-show-crash-handler") != nullptr)
        {
            wcscat_s(outputPath, outputPathLen, L" --console --wait-for-user");
        }
        if (wcsstr(GetCommandLineW(), L"-debug-crash-handler") != nullptr)
        {
            wcscat_s(outputPath, outputPathLen, L" --wait-for-debugger");
        }
        if (wcsstr(GetCommandLineW(), L"-no-crash-quit") != nullptr)
        {
            wcscat_s(outputPath, outputPathLen, L" --no-terminate");
        }
    }

    HANDLE ExternalCrashHandler::InitializeExternalProcess(wchar_t* commandLine, HANDLE hStdIn, DWORD* processID)
    {
        STARTUPINFOW startupInfo = { 0 };
        startupInfo.cb = sizeof(STARTUPINFOW);
        startupInfo.hStdError = nullptr;
        startupInfo.hStdOutput = nullptr;
        startupInfo.hStdInput = hStdIn;
        startupInfo.dwFlags = STARTF_USESTDHANDLES;

        PROCESS_INFORMATION dumperProcessInfo;
        if (CreateProcessW(nullptr, commandLine, nullptr, nullptr, true, NORMAL_PRIORITY_CLASS, nullptr, nullptr, &startupInfo, &dumperProcessInfo) == 0)
            return false;

        // Don't need the thread handle so close it to avoid leaking
        CloseHandle(dumperProcessInfo.hThread);

        *processID = dumperProcessInfo.dwProcessId;
        return dumperProcessInfo.hProcess;
    }

    bool ExternalCrashHandler::Load()
    {
        if (m_HandlerProcessHandle.IsValid())
            return false;

        // Check the shared memory is set up
        if (!m_SharedMemory.Ptr())
            return false;

        // Set up the shared event that we'll use to notify the crash reporter of a crash
        if (!CreateSharedCrashNotificationEvents(GetCurrentProcessId(), m_NotificationEvents))
            return false;

        // Open the anonymous pipe that we'll use to send crash report data to the crash handler
        SECURITY_ATTRIBUTES sa;
        memset(&sa, 0, sizeof(sa));
        sa.nLength = sizeof(sa);
        sa.bInheritHandle = true;
        if (!CreatePipe(m_ReadPipe.CloseAndGetAddressOf(), m_WritePipe.CloseAndGetAddressOf(), &sa, 0))
            return false;
        if (!SetHandleInformation(m_WritePipe.Get(), HANDLE_FLAG_INHERIT, 0))
            return false;

        // Get the appropriate crash handler exe name and command line
        GenerateCrashHandlerCommandLine(m_CrashHandlerPath, _countof(m_CrashHandlerPath), UNITY_CRASH_HANDLER_PATH, m_SharedMemory.Ptr());

        // Start the process
        m_HandlerProcessHandle.Reset(InitializeExternalProcess(m_CrashHandlerPath, m_ReadPipe.Get(), &m_ProcessID));
        if (!m_HandlerProcessHandle.IsValid())
            return false;

        return true;
    }

    void ExternalCrashHandler::Unload()
    {
        // Notify the crash handler process that it should shut down gracefully
        SetEvent(m_NotificationEvents.RequestTerminate.Get());

        m_NotificationEvents = {};
        m_HandlerProcessHandle = {};
        m_WritePipe = {};
        m_ReadPipe = {};
        m_ProcessID = 0;
    }

    bool ExternalCrashHandler::IsProcessAlive() const
    {
        return WaitForSingleObject(m_HandlerProcessHandle.Get(), 0) == WAIT_TIMEOUT;
    }

    DWORD ExternalCrashHandler::GetProcessID() const
    {
        return m_ProcessID;
    }

    void ExternalCrashHandler::HandleCrash(DWORD processId, DWORD threadId, const char* description, PCONTEXT context, PEXCEPTION_RECORD exceptionRecord, bool isFatal /* = true */)
    {
        if (!IsProcessAlive() || !m_NotificationEvents.HostDataReady.IsValid())
            return;

        // Flush the communication pipe to make sure the handler's data is up-to-date
        FlushFileBuffers(m_WritePipe.Get());

        // Allow the crash handler application to steal focus
        if (!AllowSetForegroundWindow(GetProcessId(m_HandlerProcessHandle.Get())))
            OutputDebugStringA("AllowSetForegroundWindow failed.\n");

        // Suspend all threads except this one
        ProcessThreadsSnapshot threadSnapshot(GetCurrentProcessId(), THREAD_GET_CONTEXT | THREAD_QUERY_INFORMATION, true, true);

        // Fill the crash data header
        CrashDataHeader* crashData = reinterpret_cast<CrashDataHeader*>(m_SharedMemory.Ptr());
        memset(crashData, 0, sizeof(crashData));

        crashData->Magic = CrashDataHeader::k_Magic;
        crashData->ProcessID = processId;
        crashData->ThreadID = threadId;
        crashData->HeaderSize = sizeof(CrashDataHeader);
        crashData->Flags.IsFatal = isFatal;

        // Copy the description
        if (description != nullptr)
            strcpy_s(crashData->Description, _countof(crashData->Description), description);
        else
            strcpy_s(crashData->Description, _countof(crashData->Description), "caused an access violation");

        // Copy the exception and context records. Alternatively, we could get the crash handler
        // to read these from the pointers, but this is only ~1k in total and probably fine.
        if (context != nullptr)
            memcpy(&crashData->Context, context, sizeof(CONTEXT));
        if (exceptionRecord != nullptr)
            memcpy(&crashData->ExceptionRecord, exceptionRecord, sizeof(EXCEPTION_RECORD));

        // Get the top of the stack
#if defined(_M_IX86)
        void* pStackTop;
        __asm
        {
            // Load the top (highest address) of the stack from the
            // thread information block. It will be found there in
            // Win9x and Windows NT.
            mov eax, fs:[4]
            mov pStackTop, eax
        }
        crashData->StackTop = pStackTop;
#elif defined(_M_X64)
        crashData->StackTop = reinterpret_cast<void*>(__readgsqword(8));
#else
#   error Unknown platform. Need stack top pointer for stack dumps.
#endif

#if ENABLE_MONO

        // Get the mono domain for this thread
        if (GetMonoManagerPtr() != nullptr)
        {
            crashData->MonoDomain = mono_domain_get();
#   if UNITY_64
            // We try and take a lock here to preserve the integrity of the FATs for the external process.
            // However, this has two problems that conflict with eachother:
            //   1) If another thread is modifying the FATs, we ideally want to wait for that thread to finish.
            //   2) But if the *current thread* is using it, we can't acquire the lock because SRW locks aren't re-entrant.
            // As such, the only option is to spin-wait until it's available, and give up after a while and hope for the best.
            // In the general case, this won't spin at all.
            // In rare cases, another thread will have the lock, but the wait will succeed.
            // In rarer cases, another thread will have the lock, but it might take too long to finish and
            // acquiring the lock times out. In this case we risk corruption of the FATs, but there's nothing
            // we can reasonably do about that.
            // In even rarer cases, this thread has the lock already. The spin will time out. The data will
            // probably be corrupt anyway because the process crashed while manipulating them.
            if (mono_unity_lock_dynamic_function_access_tables64 != nullptr)
            {
                const unsigned int monoFATSpinWait = 10000;
                crashData->MonoFAT64 = mono_unity_lock_dynamic_function_access_tables64(monoFATSpinWait);
            }
#   endif
        }
#endif

        // Ensure the data has landed in memory before we set the event
        MemoryBarrier();

        // Send the crash handler the signal that we have crashed and it should begin
        SetEvent(m_NotificationEvents.HostDataReady.Get());

        // Wait for the crash handler to exit, or to signal us to continue. If we're crashing, it may terminate us
        // while we wait, which is fine. If it signals us or terminates, we'll continue with our own handling - for
        // a crash this means allowing WER to run, and for other invocations it means continuing to execute as normal.
        HANDLE waitOn[] = { m_HandlerProcessHandle.Get(), m_NotificationEvents.HostCanContinue.Get() };
        DWORD waitResult = WaitForMultipleObjects(2, waitOn, FALSE, INFINITE);

        if (waitResult == WAIT_OBJECT_0)
        {
            // The crash handler exited, but did not terminate us
            DWORD chExitCode = E_FAIL;
            if (GetExitCodeProcess(m_HandlerProcessHandle.Get(), &chExitCode))
            {
                switch (chExitCode)
                {
                    case winutils::kCrashHandlerExitCodeNormal:
                        // Normal termination means that no bug reporter app was started, so we should
                        // go ahead and proceed to WER as usual.
                        break;
                    case winutils::kCrashHandlerExitCodeBugReporterStarted:
                        // We suppress WER here to avoid confusing UX where the WER box and the bug
                        // reporter are started at the same time.
                        SetErrorMode(SEM_NOGPFAULTERRORBOX);
                        break;
                    default:
                        // we could log this, but I don't want to risk it
                        break;
                }
            }
        }

#if ENABLE_MONO && UNITY_64
        // Unlock the mono FATs
        if (crashData->MonoFAT64 && mono_unity_unlock_dynamic_function_access_tables64 != nullptr)
        {
            mono_unity_unlock_dynamic_function_access_tables64();
        }
#endif
    }

    static bool WriteCompleteData(HANDLE hFile, const void* data, size_t size)
    {
        const char* bytes = reinterpret_cast<const char*>(data);
        size_t written = 0;

        do
        {
            // How large is this chunk?
#if UNITY_64
            DWORD chunkSize = static_cast<DWORD>(std::min(size - written, 0xffffffffull));
#else
            DWORD chunkSize = static_cast<DWORD>(size - written);
#endif

            DWORD thisWriteSize = 0;
            bool writeStatus = WriteFile(
                hFile,
                bytes,
                chunkSize,
                &thisWriteSize,
                nullptr);
            if (!writeStatus)
            {
                return false;
            }

            written += thisWriteSize;
            bytes += thisWriteSize;
        }
        while (written < size);

        return true;
    }

    bool ExternalCrashHandler::HandleCrash(DWORD dwProcessID, DWORD dwThreadID, PEXCEPTION_POINTERS pExceptionPointers)
    {
        HandleCrash(dwProcessID, dwThreadID, nullptr, pExceptionPointers->ContextRecord, pExceptionPointers->ExceptionRecord);
        return true;
    }

    bool ExternalCrashHandler::HandleFatal(DWORD dwProcessID, DWORD dwThreadID, const char* description, PCONTEXT pContext)
    {
        HandleCrash(dwProcessID, dwThreadID, description, pContext, nullptr);
        return true;
    }

    bool ExternalCrashHandler::HandleNonFatal(DWORD dwProcessID, DWORD dwThreadID, const char* description, PCONTEXT pContext)
    {
        HandleCrash(dwProcessID, dwThreadID, description, pContext, nullptr, false);
        return true;
    }

    static bool SendMessageOrError(HANDLE hPipe, Mutex& pipeMutex, size_t totalSize, const char** messageData, const size_t* messageDataSizes, size_t messageDataCount)
    {
        Mutex::AutoLock lock(pipeMutex);

        if (!WriteCompleteData(hPipe, &totalSize, sizeof(totalSize)))
            return false;
        for (size_t i = 0; i < messageDataCount; ++i)
        {
            const char* msgData = messageData[i] ? messageData[i] : "";
            if (!WriteCompleteData(hPipe, msgData, messageDataSizes[i]))
                return false;
        }

        // @petele note: Normally I would consider FlushFileBuffers(hPipe) here,
        // but there's a caveat that makes me uneasy: flushing blocks until the
        // other process extracts the data from the pipe. If - for some reason -
        // the crash handler crashes or gets stuck in such a way that WriteFile
        // succeeds but the data isn't being Read, I don't want to risk hanging
        // Unity.

        return true;
    }

    void ExternalCrashHandler::SendText(const char** message, size_t messageCount)
    {
        if (messageCount == 0)
            return;
        if (!IsProcessAlive())
            return;

        // Get the size of the message
        size_t totalSize = 0;
        size_t* messageSizes = reinterpret_cast<size_t*>(_alloca(sizeof(size_t) * messageCount));
        for (size_t i = 0; i < messageCount; ++i)
        {
            size_t thisSize = (message[i] ? strlen(message[i]) : 0) + 1;
            messageSizes[i] = thisSize;
            totalSize += thisSize;
        }

        if (!SendMessageOrError(m_WritePipe.Get(), m_PipeMutex, totalSize, message, messageSizes, messageCount))
        {
            printf_console("WARNING: Failed to send a message to the crash handler:\n");
            for (size_t i = 0; i < messageCount; ++i)
            {
                printf_console(" [%d] %s\n", i, message[i]);
            }
        }
    }

    // -----------------------------------------
    size_t ExternalCrashHandler::BorderedVirtualAllocation::RoundToPageSize(size_t size)
    {
        static_assert(kPageSize != 0, "kPageSize cannot be 0.");
        static_assert((kPageSize & (kPageSize - 1)) == 0, "kPageSize must be a power-of-2");
        return (size + kPageSize - 1) & ~(kPageSize - 1);
    }

    ExternalCrashHandler::BorderedVirtualAllocation::BorderedVirtualAllocation(size_t size)
        : m_basePtr(nullptr)
        , m_memory(nullptr)
        , m_size(0)
        , m_totalSize(0)
    {
        if (!size)
            return;

        // Round the size up to the page size
        size = RoundToPageSize(size);

        // Add pages either side of the memory
        size_t totalSize = size + 2 * kPageSize;

        // Allocate it
        char* basePtr = reinterpret_cast<char*>(VirtualAlloc(nullptr, totalSize, MEM_COMMIT, PAGE_NOACCESS));
        if (!basePtr)
            return;

        // Unprotect the writeable region
        DWORD oldProtect;
        if (!VirtualProtect(basePtr + kPageSize, size, PAGE_READWRITE, &oldProtect))
            return;

        // Cache the info
        m_basePtr = basePtr;
        m_memory = basePtr + kPageSize;
        m_size = size;
        m_totalSize = totalSize;
    }

    ExternalCrashHandler::BorderedVirtualAllocation::~BorderedVirtualAllocation()
    {
        if (m_basePtr)
            VirtualFree(m_basePtr, m_totalSize, MEM_FREE);
    }
}

#endif // ENABLE_OUT_OF_PROCESS_CRASH_HANDLER
