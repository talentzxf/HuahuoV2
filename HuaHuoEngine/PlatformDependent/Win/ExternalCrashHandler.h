#pragma once

#if ENABLE_OUT_OF_PROCESS_CRASH_HANDLER

#include <windef.h>
#include <winbase.h>
// undo macro damage from winbase.h
#undef CreateFile
#undef DeleteFile
#undef SetCurrentDirectory
#undef GetCurrentDirectory
#undef CreateDirectory

#include "Runtime/Threads/Mutex.h"
#include "SharedCrashData.h"

namespace winutils
{
    class ExternalCrashHandler
    {
    public:
        ExternalCrashHandler();
        ~ExternalCrashHandler();

        bool Load();
        void Unload();

        bool IsProcessAlive() const;
        DWORD GetProcessID() const;

        bool HandleCrash(DWORD dwProcessID, DWORD dwThreadID, PEXCEPTION_POINTERS pExceptionPointers);
        bool HandleFatal(DWORD dwProcessID, DWORD dwThreadID, const char* description, PCONTEXT pContext);
        bool HandleNonFatal(DWORD dwProcessID, DWORD dwThreadID, const char* description, PCONTEXT pContext);

        void SendText(const char** messageParts, size_t messagePartCount);

    private:
        ExternalCrashHandler(const ExternalCrashHandler&) = delete;
        ExternalCrashHandler& operator=(const ExternalCrashHandler&) = delete;

        static void GenerateCrashHandlerCommandLine(wchar_t* outputPath, size_t outputPathLen, const wchar_t* exeName, const void* sharedAreaPtr);
        static HANDLE InitializeExternalProcess(wchar_t* commandLine, HANDLE hStdIn, DWORD* processID);

        void HandleCrash(DWORD processId, DWORD threadId, const char* description, PCONTEXT context, PEXCEPTION_RECORD exceptionRecord, bool isFatal = true);

        class BorderedVirtualAllocation
        {
        public:
            static const size_t kPageSize = 4096;

            BorderedVirtualAllocation(size_t size);
            ~BorderedVirtualAllocation();

            void* Ptr() const { return m_memory; }

            size_t Size() const { return m_size; }
            size_t TotalSize() const { return m_totalSize; }

        private:
            void* m_basePtr;
            void* m_memory;
            size_t m_totalSize;
            size_t m_size;

            static size_t RoundToPageSize(size_t size);
        };

        BorderedVirtualAllocation m_SharedMemory;

        winutils::AutoHandle m_HandlerProcessHandle;
        winutils::SharedCrashNotificationEvents m_NotificationEvents;
        winutils::AutoHandle m_WritePipe;
        winutils::AutoHandle m_ReadPipe;
        Mutex m_PipeMutex;
        DWORD m_ProcessID;

        // Defined as very large because this is the maximum length of a long
        // path in Windows. We could theoretically allocate this on the heap,
        // but for the sake of a few kilobytes, this is simpler.
        // (CreateProcess does not accept a string larger than this for
        // command and arguments.)
        wchar_t m_CrashHandlerPath[32768];
    };
}
#endif // ENABLE_OUT_OF_PROCESS_CRASH_HANDLER
