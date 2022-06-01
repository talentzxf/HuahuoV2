#pragma once

#include "Runtime/Utilities/Annotations.h"

// heavily used header file; declare minimal set of things instead of whole windows.h
#include <windef.h>
#include <handleapi.h>
#include <errhandlingapi.h>
#if !PLATFORM_WINRT && !defined(UNITY_WIN_API_SUBSET) && !PLATFORM_FAMILY_WINDOWSGAMES
typedef LRESULT (CALLBACK * WndProcType)(HWND, UINT, WPARAM, LPARAM);
#endif


namespace winutils
{
    EXTERN_C IMAGE_DOS_HEADER __ImageBase;

    inline HINSTANCE GetCurrentModuleHandle()
    {
        return reinterpret_cast<HINSTANCE>(&__ImageBase);
    }

    // Values returned by systeminfo::GetOperatingSystemNumeric
    enum WindowsVersion
    {
        kWindows2000 = 500,
        kWindowsXP = 510,
        kWindows2003 = 520,
        kWindowsVista = 600,
        kWindows7 = 610,
        kWindows8 = 620,
        kWindows81 = 630,
    };

    // Return current working directory reported by Windows
    // @param cwd: Working directory to be returned
    void GetWindowsCurrentDirectory(core::string& cwd);

    void SetDontDisplayDialogs(bool dontDisplayDialogs);
    bool GetDontDisplayDialogs();

    #if !UNITY_EXTERNAL_TOOL
    /// Display error messages added by AddErrorMessage
    void DisplayErrorMessages(const char* error);
    /// Display error messages added by AddErrorMessage and do exit(1)
    DOES_NOT_RETURN void DisplayErrorMessagesAndQuit(const char* error);
    #endif

    core::string ErrorCodeToMsg(DWORD code);
    void RegisterLogToDebugOutput();

    void EnableWindowsKey();
    void DisableWindowsKey();

    // In web player, we should use our DLL instance handle, not the containing
    // application's one. So make function to set it up.
    HINSTANCE GetInstanceHandle();
    void SetInstanceHandle(HINSTANCE instance);

    #if !PLATFORM_WINRT && !defined(UNITY_WIN_API_SUBSET) && !PLATFORM_FAMILY_WINDOWSGAMES
    ATOM RegisterWindowClass(const wchar_t* className, WndProcType windowProc, unsigned int style);
    void UnregisterWindowClass(const wchar_t* className);
    #endif

    HWND GetWindowTopmostParent(HWND wnd);

    #if DEBUGMODE
    core::string GetWindowsMessageInfo(UINT message, WPARAM wparam, LPARAM lparam);
    #endif
    void DebugMessageBox(LPCSTR format, ...);

    void CenterWindowOnParent(HWND window);

    class AutoHandle
    {
    public:
        AutoHandle() : m_Handle(INVALID_HANDLE_VALUE) {}
        explicit AutoHandle(HANDLE h) : m_Handle(h) {}
        ~AutoHandle() { Close(); }

        AutoHandle(AutoHandle&& other)
            : m_Handle(other.m_Handle)
        {
            other.m_Handle = INVALID_HANDLE_VALUE;
        }

        AutoHandle& operator=(AutoHandle&& other)
        {
            m_Handle = other.m_Handle;
            other.m_Handle = INVALID_HANDLE_VALUE;
            return *this;
        }

        bool IsValid() const
        {
            return m_Handle != INVALID_HANDLE_VALUE && m_Handle != nullptr;
        }

        void Close()
        {
            if (IsValid())
            {
                CloseHandle(m_Handle);
                m_Handle = INVALID_HANDLE_VALUE;
            }
        }

        AutoHandle& Reset(HANDLE h)
        {
            Close();
            m_Handle = h;
            return *this;
        }

        HANDLE Get() const
        {
            return m_Handle;
        }

        HANDLE* CloseAndGetAddressOf()
        {
            Close();
            return &m_Handle;
        }

    private:
        AutoHandle(const AutoHandle&) = delete;
        AutoHandle& operator=(const AutoHandle&) = delete;

        HANDLE  m_Handle;
    };

    enum DebugBreakType
    {
        kCommandLine    = 0x1,  // Default handling via commandline
        kEvent          = 0x2   // Use global events when command line isn't an option
    };

    // Handles invoking the Windows JIT Debugger pathway with a DebugBreak based on the types requested
    void HandleDebugBreak(int type);

    // There are two types of crash in Unity - one which comes from C++ code and the second one which comes from mono (see MonoManager.cpp)
    // The crashes from C++ are catched, but the crashes from mono are processed by int __cdecl HandleSignal( EXCEPTION_POINTERS* ep )
    // So we need to call this function from there
    int ProcessInternalCrash(PEXCEPTION_POINTERS pExInfo, bool passToDefaultCrashHandler);

    // Adds additional exception handler, so during the crash the callstack could be outputed to Editor.log
    // This won't override crash handler from the CrashHandlerLib.
    // So during the crash, the following sequence will be performed:
    // 1. ProcessInternalCrash in this file
    // 2. InternalProcessCrash in CrashHandler library
    void InstallInternalCrashHandler();
    void UninstallInternalCrashHandler();
    void SetCrashReportLocationForReporting(const char* location);

    // Utility for restoring the exception filter iff [sic] it was not changed by something else
    void UnsetUnhandledExceptionFilterIfStillInstalled(LPTOP_LEVEL_EXCEPTION_FILTER expected);

#if !PLATFORM_FAMILY_WINDOWSGAMES
    // Setup argv from wide-character command line
    void SetupWideArgs(LPWSTR szCmdLine = NULL);

    // Setup boot config from file and wchar command line
    void SetupBootConfig(const char* bootConfigFile);
#endif// !PLATFORM_FAMILY_WINDOWSGAMES

    // Turn commandline into an ARGV vector.  Actually modifies szCmdLine in place.
    std::vector<const char*> ChopUpCommandLine(LPSTR szCmdLine);

#if !UNITY_EXTERNAL_TOOL
    // Returns a path to %USERPROFILE%\AppData\LocalLow\<CompanyName>\<ProductName>.
    // This helps avoids conflicts if there more than one Unity player wants to read/write files.
    // We read this data from 'app.info', because we can't assume PlayerSettings (and other Unity managers) are initialized yet.
    core::string GetAppSpecificWindowsUserStorageLocation(const core::string& vendor, const core::string& appName);

    // Returns a path to %TEMP%\<CompanyName>\<ProductName>.
    core::string GetAppSpecificTemporaryStorageLocation(const core::string& vendor, const core::string& appName);

    // Returns a path to the crash report output folder (either the default or set by command line)
    core::string GetCrashReportOutputFolder();
#endif
}  // namespace winutils

#define WIN_ERROR_TEXT(errorCode)   (winutils::ErrorCodeToMsg(errorCode).c_str())
#define WIN_LAST_ERROR_TEXT WIN_ERROR_TEXT(GetLastError())
