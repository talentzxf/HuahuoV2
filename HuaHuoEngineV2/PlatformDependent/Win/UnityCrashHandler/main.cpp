#include "UnityPrefix.h"
#include "CrashHandlerWindows.h"
#include "LocalReportCache.h"
#include "Utilities.h"
#include <winbase.h>
#include <shellapi.h>
#include <combaseapi.h>
#include <signal.h>
#include <sstream>

static const wchar_t* kConsoleCommandLineSwitch = L"--console";
static const wchar_t* kWaitForDebuggerCommandLineSwitch = L"--wait-for-debugger";

int RunCrashHandlerInUploadMode(int argc, wchar_t** argv)
{
    STARTUPINFOW si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    ZeroMemory(&pi, sizeof(pi));
    si.cb = sizeof(si);

    std::wstringstream cmdLineStr;
    for (int i = 0; i < argc; ++i)
        cmdLineStr << L"\"" << argv[i] << L"\" ";

    // Restore some command line parameters that we may have popped along the way
    LPCWSTR originalCmdLine = GetCommandLineW();
    if (wcsstr(originalCmdLine, kConsoleCommandLineSwitch) != nullptr)
        cmdLineStr << kConsoleCommandLineSwitch;
    if (wcsstr(originalCmdLine, kWaitForDebuggerCommandLineSwitch) != nullptr)
        cmdLineStr << kWaitForDebuggerCommandLineSwitch;

    std::wstring cmdLine(cmdLineStr.str());

    if (!CreateProcessW(
        nullptr,
        const_cast<wchar_t*>(cmdLine.c_str()),
        nullptr,
        nullptr,
        false,
        0,
        nullptr,
        nullptr,
        &si,
        &pi))
    {
        return 1;
    }

    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    return 0;
}

void __cdecl InvalidParameterHandler(const wchar_t*, const wchar_t*, const wchar_t*, unsigned int, uintptr_t)
{
    // Suppress, but log
    printf_console("Crash handler abnormally terminating due to invalid parameter handler\n");
    TerminateProcess(GetCurrentProcess(), E_INVALIDARG);
}

void __cdecl AbortHandler(int)
{
    // Suppress, but log
    printf_console("Crash handler abnormally terminating due to abort()\n");
    TerminateProcess(GetCurrentProcess(), E_FAIL);
}

int CALLBACK WinMain(
    _In_ HINSTANCE hInstance,
    _In_ HINSTANCE hPrevInstance,
    _In_ LPSTR     lpCmdLine,
    _In_ int       nCmdShow)
{
    RuntimeInitializeAndCleanupHandler runtimeStaticLifetimeHandler{};

    HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    if (FAILED(hr))
    {
        printf_console("CoInitializeEx failed with code 0x%0x\n", hr);
        return 1;
    }

    // Don't show WER on crash
    SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOGPFAULTERRORBOX);

    // Suppress other errors
    _set_invalid_parameter_handler(InvalidParameterHandler);
    _set_abort_behavior(_CALL_REPORTFAULT, _CALL_REPORTFAULT);
    signal(SIGABRT, &AbortHandler);

    // Configure based on command line arguments
    int argc = 0;
    wchar_t** argv = CommandLineToArgvW(GetCommandLineW(), &argc);
    if (PopCommandLineSwitch(kConsoleCommandLineSwitch, argc, argv))
    {
        MakeConsole();
    }
    if (PopCommandLineSwitch(kWaitForDebuggerCommandLineSwitch, argc, argv))
    {
        WaitForDebugger();
    }

    // Run either upload mode or attach mode
    if (PopCommandLineSwitch(L"--attach", argc, argv))
    {
        // Perform the crash handler duties for the specific executable
        int r = CrashHandlerMain(argc, argv);

        // On exit, upload pending crash dumps
        if (r >= 0)
        {
            RunCrashHandlerInUploadMode(argc, argv);
        }

        return r;
    }
    else
    {
        return ReportCacheMain(argc, argv);
    }
}

// Instead of pulling in Testing.cpp (which comes with a fair amount of
// baggage), define this here to prevent linker errors in debug.
#if ENABLE_NATIVE_TEST_FRAMEWORK
bool IsNativeTestExpectingAssertionFailure(const char* msg)
{
    return false;
}

#endif
