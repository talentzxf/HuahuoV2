#include "UnityPrefix.h"
#include "CrashHandler.h"
#include "ExternalCrashHandler.h"
#include "Runtime/Core/Format/Format.h"
#include "Runtime/Diagnostics/Stacktrace.h"
#include "WinUtils.h"

namespace winutils
{
    __forceinline CONTEXT GetCurrentThreadContext()
    {
        CONTEXT c;
        memset(&c, 0, sizeof(c));
        c.ContextFlags = CONTEXT_ALL;
        RtlCaptureContext(&c);
        return c;
    }

    CrashHandler* CrashHandler::s_InstalledInstance = nullptr;

    const int kPureVirtualCallSkipFrames = 5;

    static void PrintStackTrace(int skipFrames = 0)
    {
        core::string trace = GetStacktrace(skipFrames);
        printf_console("Stack trace:\n%s\n", trace.c_str());
    }

    LONG WINAPI CrashHandler::DefaultCrashHandler(PEXCEPTION_POINTERS pExInfo)
    {
        printf_console("windows exception 0x%x at address 0x%p\n", pExInfo->ExceptionRecord->ExceptionCode, pExInfo->ExceptionRecord->ExceptionAddress);
        PrintStackTrace();

        if (GetInstalledPtr())
            GetInstalled().HandleCrash(pExInfo);
        return EXCEPTION_CONTINUE_SEARCH; // continue to WER
    }

    void __cdecl CrashHandler::DefaultPureVirtualCallHandler()
    {
        const char* message = "attempted to call a pure virtual function";
        printf_console("%s\n", message);
        PrintStackTrace(kPureVirtualCallSkipFrames);

        // Crash on pure virtual call
        if (GetInstalledPtr())
        {
            CONTEXT context = GetCurrentThreadContext();
            GetInstalled().HandleFatal(message, &context);
        }

        // If there was no crash handler installed, there's nothing we can do but terminate
        TerminateProcess(GetCurrentProcess(), 1);
    }

    void __cdecl CrashHandler::DefaultInvalidParameterHandler(const wchar_t*, const wchar_t*, const wchar_t*, unsigned int, uintptr_t)
    {
        // (If you're wondering, all the parameters for this function are useless unless running with the debug CRT.)

        const char* message = "called a function with an invalid parameter";
        printf_console("%s\n", message);
        PrintStackTrace();

        if (GetInstalledPtr())
        {
            // Crash on invalid parameter
            CONTEXT context = GetCurrentThreadContext();
            GetInstalled().HandleFatal("called a function with an invalid parameter", &context);
        }

        // If there was no crash handler installed, there's nothing we can do but terminate
        TerminateProcess(GetCurrentProcess(), 1);
    }

    void __cdecl CrashHandler::DefaultSignalHandler(int signal)
    {
        core::string signame;

        switch (signal)
        {
            case SIGABRT:
                signame = "SIGABRT";
                break;
            case SIGSEGV:
                signame = "SIGSEGV";
                break;
            default:
                signame = core::Format("{0}", signal);
        }

        core::string message = core::Format("received signal from system: {0}", signame);

        // Don't print the signal code or stack trace for signal 0. This is for interop with MonoManager.
        if (signal)
        {
            printf_console("%s\n", message.c_str());
            PrintStackTrace();
        }

        if (GetInstalledPtr())
        {
            CONTEXT context = GetCurrentThreadContext();
            GetInstalled().HandleFatal(message.c_str(), &context);
        }

        // If there was no crash handler installed, there's nothing we can do but terminate
        TerminateProcess(GetCurrentProcess(), 1);
    }

    CrashHandler::CrashHandler()
        : m_Callback(nullptr)
    {
    }

    CrashHandler::~CrashHandler()
    {
        Uninstall();
    }

    void CrashHandler::Install()
    {
        if (s_InstalledInstance == this)
            return;
        if (s_InstalledInstance != nullptr)
            s_InstalledInstance->Uninstall();

        s_InstalledInstance = this;

#if ENABLE_OUT_OF_PROCESS_CRASH_HANDLER
        m_ExternalCrashHandler.Load();
#endif

        // Set the crash handlers
        m_ErrorHandlers = {};
        m_ErrorHandlers.exceptionFilter = DefaultCrashHandler;
        m_ErrorHandlers.errorMode = SEM_FAILCRITICALERRORS;
        m_ErrorHandlers.pureCallHandler = DefaultPureVirtualCallHandler;
        m_ErrorHandlers.invalidParamHandler = DefaultInvalidParameterHandler;
        m_ErrorHandlers.abortHandlerBehavior = _CALL_REPORTFAULT;
        m_ErrorHandlers.abortHandler = &DefaultSignalHandler;
        m_ErrorHandlers.segfaultHandler = &DefaultSignalHandler;
        m_PrevErrorHandlers = ErrorHandlers::Set(m_ErrorHandlers);
    }

    void CrashHandler::Uninstall()
    {
        if (s_InstalledInstance != this)
            return;

        // Remove the crash handler exception filter
        ErrorHandlers::RevertIfUnchanged(m_PrevErrorHandlers, m_ErrorHandlers);

#if ENABLE_OUT_OF_PROCESS_CRASH_HANDLER
        m_ExternalCrashHandler.Unload();
#endif
    }

    void CrashHandler::SetAppName(const char* appName)
    {
#if ENABLE_OUT_OF_PROCESS_CRASH_HANDLER
        const char* message[] = { "SetAppName", appName };
        m_ExternalCrashHandler.SendText(message, _countof(message));
#endif
    }

    void CrashHandler::SetVendor(const char* vendor)
    {
#if ENABLE_OUT_OF_PROCESS_CRASH_HANDLER
        const char* message[] = { "SetVendor", vendor };
        m_ExternalCrashHandler.SendText(message, _countof(message));
#endif
    }

    void CrashHandler::SetAppVersion(const char* appVersion)
    {
#if ENABLE_OUT_OF_PROCESS_CRASH_HANDLER
        const char* message[] = { "SetInfo", appVersion };
        m_ExternalCrashHandler.SendText(message, _countof(message));
#endif
    }

    void CrashHandler::SetIsEditor(bool isEditor)
    {
#if ENABLE_OUT_OF_PROCESS_CRASH_HANDLER
        const char* message[] = { "SetIsEditor", isEditor ? "true" : "false" };
        m_ExternalCrashHandler.SendText(message, _countof(message));
#endif
    }

    void CrashHandler::SetShowDialog(bool show)
    {
#if ENABLE_OUT_OF_PROCESS_CRASH_HANDLER
        const char* message[] = { "SetShowDialog", show ? "true" : "false" };
        m_ExternalCrashHandler.SendText(message, _countof(message));
#endif
    }

    void CrashHandler::SetCrashCallback(TCrashCallback callback)
    {
        m_Callback = callback;
    }

    void CrashHandler::AddFile(const char* path, const char* desc)
    {
#if ENABLE_OUT_OF_PROCESS_CRASH_HANDLER
        const char* message[] = { "AddFile", path, desc };
        m_ExternalCrashHandler.SendText(message, _countof(message));
#endif
    }

    void CrashHandler::RemoveFile(const char* path)
    {
#if ENABLE_OUT_OF_PROCESS_CRASH_HANDLER
        const char* message[] = { "RemoveFile", path };
        m_ExternalCrashHandler.SendText(message, _countof(message));
#endif
    }

    void CrashHandler::SetCrashReportOutputFolder(const char* path)
    {
#if ENABLE_OUT_OF_PROCESS_CRASH_HANDLER
        const char* message[] = { "SetCrashReportPath", path };
        m_ExternalCrashHandler.SendText(message, _countof(message));
#endif
    }

    void CrashHandler::SetBugReporterAppPath(const char* path)
    {
#if ENABLE_OUT_OF_PROCESS_CRASH_HANDLER
        const char* message[] = { "SetBugReporterAppPath", path };
        m_ExternalCrashHandler.SendText(message, _countof(message));
#endif
    }

    void CrashHandler::SetMonoDllPath(const char* path)
    {
#if ENABLE_OUT_OF_PROCESS_CRASH_HANDLER
        const char* message[] = { "SetMonoPath", path };
        m_ExternalCrashHandler.SendText(message, _countof(message));
#endif
    }

    void CrashHandler::SetMetaData(const char* key, const char* value)
    {
#if ENABLE_OUT_OF_PROCESS_CRASH_HANDLER
        const char* message[] = { "SetMetaData", key, value };
        m_ExternalCrashHandler.SendText(message, _countof(message));
#endif
    }

    void CrashHandler::SetUserMetaData(const char* key, const char* value)
    {
#if ENABLE_OUT_OF_PROCESS_CRASH_HANDLER
        const char* message[] = { "SetUserMetaData", key, value };
        m_ExternalCrashHandler.SendText(message, _countof(message));
#endif
    }

    void CrashHandler::SetLogBufferSize(UInt32 logBufferSize)
    {
#if ENABLE_OUT_OF_PROCESS_CRASH_HANDLER
        const char* message[] = { "SetLogBufferSize", Format("%d", logBufferSize).c_str() };
        m_ExternalCrashHandler.SendText(message, _countof(message));
#endif
    }

    void CrashHandler::RecordLogMessage(const char* message, const char* timestamp, const char* framecount, const char* type)
    {
#if ENABLE_OUT_OF_PROCESS_CRASH_HANDLER
        const char* messages[] = { "RecordLogMessage", message, timestamp, framecount, type };
        m_ExternalCrashHandler.SendText(messages, _countof(messages));
#endif
    }

    void CrashHandler::HandleCrash(PEXCEPTION_POINTERS exceptionPointers)
    {
        if (m_Callback)
            m_Callback(*this, exceptionPointers);

#if ENABLE_OUT_OF_PROCESS_CRASH_HANDLER
        m_ExternalCrashHandler.HandleCrash(GetCurrentProcessId(), GetCurrentThreadId(), exceptionPointers);
#endif
    }

    void CrashHandler::HandleFatal(const char* description, PCONTEXT context)
    {
        if (m_Callback)
            m_Callback(*this, nullptr);

#if ENABLE_OUT_OF_PROCESS_CRASH_HANDLER
        m_ExternalCrashHandler.HandleFatal(GetCurrentProcessId(), GetCurrentThreadId(), description, context);
#endif
    }

    // -----------------------------------------
    ErrorHandlers ErrorHandlers::Set(const ErrorHandlers& current)
    {
        ErrorHandlers prev = {};

        // Set this as the default crash handler (can be overridden later)
        prev.exceptionFilter = SetUnhandledExceptionFilter(current.exceptionFilter);

        // Add additional types of crash we want to catch
        prev.errorMode = SetErrorMode(current.errorMode);
        prev.pureCallHandler = _set_purecall_handler(current.pureCallHandler);
        prev.invalidParamHandler = _set_invalid_parameter_handler(current.invalidParamHandler);

        // Set the abort() CRT handler
        prev.abortHandlerBehavior = _set_abort_behavior(current.abortHandlerBehavior, _CALL_REPORTFAULT);
        prev.abortHandler = signal(SIGABRT, current.abortHandler);
        prev.segfaultHandler = signal(SIGSEGV, current.segfaultHandler);

        return prev;
    }

    // Restores the 'prev' state, but only if the current state hasn't been changed by something else.
    // Returns the new state.
    ErrorHandlers ErrorHandlers::RevertIfUnchanged(const ErrorHandlers& prev, const ErrorHandlers& expected)
    {
        ErrorHandlers current = {};

        current.exceptionFilter = SetUnhandledExceptionFilter(prev.exceptionFilter);
        if (current.exceptionFilter != expected.exceptionFilter)
            SetUnhandledExceptionFilter(current.exceptionFilter);

        current.errorMode = GetErrorMode();
        if (current.errorMode == expected.errorMode)
            SetErrorMode(prev.errorMode);

        current.pureCallHandler = _get_purecall_handler();
        if (current.pureCallHandler == expected.pureCallHandler)
            _set_purecall_handler(prev.pureCallHandler);

        current.invalidParamHandler = _get_invalid_parameter_handler();
        if (current.invalidParamHandler == expected.invalidParamHandler)
            _set_invalid_parameter_handler(prev.invalidParamHandler);

        current.abortHandlerBehavior = _set_abort_behavior(prev.abortHandlerBehavior, _CALL_REPORTFAULT);
        if (current.abortHandlerBehavior != expected.abortHandlerBehavior)
            _set_abort_behavior(current.abortHandlerBehavior, _CALL_REPORTFAULT);

        current.abortHandler = signal(SIGABRT, prev.abortHandler);
        if (current.abortHandler != expected.abortHandler)
            signal(SIGABRT, current.abortHandler);

        current.segfaultHandler = signal(SIGSEGV, prev.segfaultHandler);
        if (current.segfaultHandler != expected.segfaultHandler)
            signal(SIGSEGV, current.segfaultHandler);

        return current;
    }
}
