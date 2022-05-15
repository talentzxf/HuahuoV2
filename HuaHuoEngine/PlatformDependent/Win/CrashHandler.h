#pragma once

#include "Runtime/Core/Containers/hash_map.h"
#include "Runtime/Utilities/RuntimeStatic.h"
#include <windef.h>
#include <winbase.h>
#include <signal.h>
// undo macro damage from winbase.h
#undef CreateFile
#undef DeleteFile
#undef SetCurrentDirectory
#undef GetCurrentDirectory
#undef CreateDirectory

#if ENABLE_OUT_OF_PROCESS_CRASH_HANDLER
#   include "ExternalCrashHandler.h"
#endif

namespace unity
{
    class CrashReporter;
}

namespace winutils
{
    struct ErrorHandlers
    {
        LPTOP_LEVEL_EXCEPTION_FILTER exceptionFilter;
        UINT errorMode;
        _purecall_handler pureCallHandler;
        _invalid_parameter_handler invalidParamHandler;
        unsigned int abortHandlerBehavior;
        _crt_signal_t abortHandler;
        _crt_signal_t segfaultHandler;

        static ErrorHandlers Set(const ErrorHandlers& eh);
        static ErrorHandlers RevertIfUnchanged(const ErrorHandlers& prev, const ErrorHandlers& expected);
    };

    class CrashHandler
    {
    public:
        CrashHandler();
        ~CrashHandler();

        void Install();
        void Uninstall();

        void SetAppName(const char* appName);
        void SetVendor(const char* vendor);
        void SetAppVersion(const char* version);
        void SetIsEditor(bool isEditor);
        void SetShowDialog(bool show);
        void SetBugReporterAppPath(const char* path);
        void SetCrashReportOutputFolder(const char* appFolder);
        void AddFile(const char* path, const char* description);
        void RemoveFile(const char* path);
        void SetMetaData(const char* key, const char* value);
        void SetUserMetaData(const char* key, const char* value);
        void SetLogBufferSize(UInt32 logBufferSize);
        void RecordLogMessage(const char* key, const char* timestamp, const char* framecount, const char* type);
        void SetMonoDllPath(const char* path);

        typedef void(*TCrashCallback)(CrashHandler& handler, PEXCEPTION_POINTERS exceptionPointers);

        void SetCrashCallback(TCrashCallback callback);

        void HandleCrash(PEXCEPTION_POINTERS exceptionPointers);
        void HandleFatal(const char* description, PCONTEXT context);

#if ENABLE_OUT_OF_PROCESS_CRASH_HANDLER
        DWORD GetExternalProcessID() const { return m_ExternalCrashHandler.GetProcessID(); }
#endif

        static CrashHandler& GetInstalled() { Assert(s_InstalledInstance);  return *s_InstalledInstance; }
        static CrashHandler* GetInstalledPtr() { return s_InstalledInstance; }

        static LONG WINAPI DefaultCrashHandler(PEXCEPTION_POINTERS pExInfo);
        static void __cdecl DefaultPureVirtualCallHandler();
        static void __cdecl DefaultInvalidParameterHandler(const wchar_t*, const wchar_t*, const wchar_t*, unsigned int, uintptr_t);
        static void __cdecl DefaultSignalHandler(int signal);

    private:
        CrashHandler(const CrashHandler&) = delete;
        CrashHandler& operator=(const CrashHandler&) = delete;

        CrashHandler(CrashHandler&&) = delete;
        CrashHandler&& operator=(CrashHandler&&) = delete;

        // For restoring the call handlers
        ErrorHandlers m_PrevErrorHandlers;
        ErrorHandlers m_ErrorHandlers;

        TCrashCallback m_Callback;

#if ENABLE_OUT_OF_PROCESS_CRASH_HANDLER
        ExternalCrashHandler m_ExternalCrashHandler;
#endif

        static CrashHandler* s_InstalledInstance;
    };
}
