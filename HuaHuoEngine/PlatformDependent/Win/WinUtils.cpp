#include "UnityPrefix.h"
#include "WinUtils.h"
#include <string>
#include <stdio.h>
#include "../WinPlayer/resource.h"
#include <stdarg.h>
#include <wtypes.h>
#include <shellapi.h>
#include <winuser.h>
#include "ErrorMessages.h"
#include "WinUnicode.h"
#include "Handle.h"
#include "Runtime/Bootstrap/BootConfig.h"
#if !UNITY_EXTERNAL_TOOL
#include "Runtime/Misc/Player.h"
#endif
#include "Runtime/Utilities/Argv.h"
#include <io.h>
#include <fcntl.h>
#if UNITY_EDITOR
#include "Editor/Platform/Windows/Utility/SplashScreen.h"
#include "Tlhelp32.h"
#endif
#include "Runtime/Allocator/MemoryManager.h"
#include "Runtime/Logging/LogSystem.h"
#include "Runtime/Utilities/LogUtility.h"
#include "Runtime/Misc/SystemInfo.h"
#include "PlatformDependent/Win/VersionHelpers.h"


#include "CrashHandler.h"

// define to 1 to print lots of debug info
#define DEBUG_WIN_UTILS 0

const wchar_t* kWindowClassName = L"UnityWndClass";
const wchar_t* kChannelWindowName = L"Unity Secondary Display";

const char*    kFatalErrorStr = "Fatal error";

static HINSTANCE gInstanceHandle = NULL;
static bool g_DontDisplayDialogs = false;
static core::string s_UTF8CommandLine;
static std::vector<const char*> s_UTF8Args;

// We need to cache some data for the crash handler that may change during run-time.
// A RuntimeStatic might not have been initialized or already destroyed, so we can't
// rely on a static core::string here.
static char s_CachedCrashHandlerOutputPath[2048] = { 0 };
static bool g_BreakToAllowDebuggerAttach = false;


void winutils::GetWindowsCurrentDirectory(core::string& cwd)
{
    DWORD cwdSize = GetCurrentDirectoryW(0, NULL);
    if (cwdSize == 0)
    {
        cwd.clear();
        return;
    }

    TempWString cwdW;
    cwdW.resize(cwdSize - 1);
    GetCurrentDirectoryW(cwdSize, &cwdW[0]);

    ConvertWideToUTF8String(cwdW, cwd);
}

void winutils::SetDontDisplayDialogs(bool dontDisplayDialogs)
{
    g_DontDisplayDialogs = dontDisplayDialogs;
}

bool winutils::GetDontDisplayDialogs()
{
    return g_DontDisplayDialogs;
}

HINSTANCE winutils::GetInstanceHandle()
{
    Assert(gInstanceHandle);
    return gInstanceHandle;
}

void winutils::SetInstanceHandle(HINSTANCE instance)
{
    Assert(!gInstanceHandle);
    gInstanceHandle = instance;
}

#if !PLATFORM_FAMILY_WINDOWSGAMES
static const char* gErrorsTitle;

static INT_PTR errorsDialogProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);

    switch (msg)
    {
        case WM_INITDIALOG:
        {
            // initialize dialog

            // center on the screen
            RECT rc, rcOwner, rcDlg;
            HWND hwndOwner = GetParent(hDlg);
            if (hwndOwner == NULL)
                hwndOwner = GetDesktopWindow();
            GetWindowRect(hwndOwner, &rcOwner);
            GetWindowRect(hDlg, &rcDlg);
            CopyRect(&rc, &rcOwner);

            OffsetRect(&rcDlg, -rcDlg.left, -rcDlg.top);
            OffsetRect(&rc, -rc.left, -rc.top);
            OffsetRect(&rc, -rcDlg.right, -rcDlg.bottom);

            SetWindowPos(hDlg, HWND_TOP, rcOwner.left + (rc.right / 2), rcOwner.top + (rc.bottom / 2), 0, 0, SWP_NOSIZE);

            // set logo image
            SendDlgItemMessageW(hDlg, IDC_ST_ERRICON, STM_SETICON, (WPARAM)LoadIcon(NULL, IDI_ERROR), 0L);
            // set error message
            SetDlgItemTextA(hDlg, IDC_ST_ERROR, gErrorsTitle);
            // set error details

            core::wstring wideText;
            ConvertUTF8ToWideString(winutils::GetErrorMessages(), wideText);
            SetDlgItemTextW(hDlg, IDC_EDIT_ERRORS, wideText.c_str());
        }

            return TRUE;

        case WM_COMMAND:
            switch (LOWORD(wParam))
            {
                case IDOK:
                    EndDialog(hDlg, IDOK);
                    return TRUE;
                case IDCANCEL:
                    EndDialog(hDlg, IDCANCEL);
                    return TRUE;
            }
            break;
    }

    return FALSE;
}

#if !UNITY_EXTERNAL_TOOL

void winutils::DisplayErrorMessages(const char* error)
{
#if UNITY_EDITOR
    HideSplashScreen();
    if (IsBatchmode())
    {
        printf_console("Fatal errors, exiting: %s\n", error);
        printf_console("%s", winutils::GetErrorMessages().c_str());
        return;
    }
#endif

    if (winutils::GetDontDisplayDialogs())
    {
        // error messages already shown to the console
        // in AddErrorMessage()
        return;
    }
    gErrorsTitle = error;
    if (!GetErrorMessages().empty())
        DialogBoxW(winutils::GetInstanceHandle(), MAKEINTRESOURCEW(IDD_ERRORS), NULL, (DLGPROC)errorsDialogProc);
    else
        MessageBoxA(NULL, error, kFatalErrorStr, MB_ICONERROR | MB_OK);
}

void winutils::DisplayErrorMessagesAndQuit(const char* error)
{
    DisplayErrorMessages(error);
    exit(1);
}

#endif
#endif// !PLATFORM_FAMILY_WINDOWSGAMES

#if defined(_MSC_VER) && _MSC_VER < 1900
#define snprintf _snprintf
#endif

core::string winutils::ErrorCodeToMsg(DWORD code)
{
    LPWSTR msgBuf = NULL;
    if (!FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL, code, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPWSTR)&msgBuf, 0, NULL))
    {
        char buf[100];
        snprintf(buf, 100, "Unknown error [%i]", code);
        return buf;
    }
    else
    {
        core::string msg = WideToUtf8(msgBuf);
        LocalFree(msgBuf);
        return msg;
    }
}

static void LoggerToWindowsDebugOutput(const DebugStringToFileData& data)
{
    char bufferUtf8[1024];

    snprintf(bufferUtf8, 1024, "UNITY: %s at %s:%i\n",
        data.message,
        data.file,
        data.line);

    core::wstring bufferUtf16 = Utf8ToWide(bufferUtf8);
    OutputDebugStringW(bufferUtf16.c_str());
}

void winutils::RegisterLogToDebugOutput()
{
    RegisterLogToConsole(LoggerToWindowsDebugOutput);
}

// ---------------------------------------------------------------------------

#if !PLATFORM_FAMILY_WINDOWSGAMES
static HHOOK gKeyboardHook;

static LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    if (nCode < 0 || nCode != HC_ACTION)  // do not process message
        return CallNextHookEx(gKeyboardHook, nCode, wParam, lParam);

    bool eatKey = false;
    const KBDLLHOOKSTRUCT* p = (const KBDLLHOOKSTRUCT*)lParam;
    switch (wParam)
    {
        case WM_KEYDOWN:
        case WM_KEYUP:
            eatKey = (p->vkCode == VK_LWIN || p->vkCode == VK_RWIN);
            break;
    }

    if (eatKey)
    {
        printf_console("FOOBAR: just ate windows key!\n");
        return 1;
    }
    else
    {
        return CallNextHookEx(gKeyboardHook, nCode, wParam, lParam);
    }
}

void winutils::EnableWindowsKey()
{
    if (gKeyboardHook)
    {
        printf_console("ENABLE windows key\n");
        UnhookWindowsHookEx(gKeyboardHook);
        gKeyboardHook = NULL;
    }
}

void winutils::DisableWindowsKey()
{
    if (gKeyboardHook == NULL)
    {
        //printf_console("DISABLE windows key\n");
        //gKeyboardHook = SetWindowsHookEx( WH_KEYBOARD_LL, LowLevelKeyboardProc, GetInstanceHandle(), 0 );
    }
}

// ---------------------------------------------------------------------------
#if !defined(UNITY_WIN_API_SUBSET)
ATOM winutils::RegisterWindowClass(const wchar_t* className, WNDPROC windowProc, unsigned int style)
{
    #if DEBUG_WIN_UTILS
    printf_console("Debug winutils: register class %s\n", className);
    #endif

    WNDCLASSEXW wcex;
    memset(&wcex, 0, sizeof(wcex));
    wcex.cbSize = sizeof(wcex);
    wcex.style          = style;
    wcex.lpfnWndProc    = windowProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = winutils::GetInstanceHandle();

    // We always load icon from the exe, because that's where it's embedded.
    wcex.hIcon = LoadIcon(reinterpret_cast<HINSTANCE>(GetModuleHandleW(NULL)), (LPCTSTR)IDI_APP_ICON);

    wcex.hCursor        = NULL;
    wcex.hbrBackground  = NULL;
    wcex.lpszMenuName   = NULL;
    wcex.lpszClassName  = className;
    ATOM classAtom = RegisterClassExW(&wcex);
    if (!classAtom)
        printf_console("Failed to register window class %s: %s\n", className, WIN_LAST_ERROR_TEXT);
    return classAtom;
}

void winutils::UnregisterWindowClass(const wchar_t* className)
{
    #if DEBUG_WIN_UTILS
    printf_console("Debug winutils: unregister class %s\n", className);
    #endif
    UnregisterClassW(className, winutils::GetInstanceHandle());
}

#endif
HWND winutils::GetWindowTopmostParent(HWND wnd)
{
    return GetAncestor(wnd, GA_ROOT);
}

void winutils::CenterWindowOnParent(HWND window)
{
    // center on parent or screen
    RECT rc, rcOwner, rcWindow;
    HWND hwndOwner = GetParent(window);
    if (hwndOwner == NULL)
        hwndOwner = GetDesktopWindow();
    GetWindowRect(hwndOwner, &rcOwner);
    GetWindowRect(window, &rcWindow);
    CopyRect(&rc, &rcOwner);

    OffsetRect(&rcWindow, -rcWindow.left, -rcWindow.top);
    OffsetRect(&rc, -rc.left, -rc.top);
    OffsetRect(&rc, -rcWindow.right, -rcWindow.bottom);

    SetWindowPos(window, HWND_TOP,
        rcOwner.left + (rc.right / 2),
        rcOwner.top + (rc.bottom / 2),
        0, 0, // no size arguments
        SWP_NOSIZE | SWP_NOACTIVATE);
}

#endif// !PLATFORM_FAMILY_WINDOWSGAMES

#if DEBUGMODE
core::string winutils::GetWindowsMessageInfo(UINT message, WPARAM wparam, LPARAM lparam)
{
    switch (message)
    {
        case WM_SHOWWINDOW: return "WM_SHOWWINDOW";
        case WM_ACTIVATE: return Format("WM_ACTIVATE active=%i  state=%i", LOWORD(wparam), HIWORD(wparam));
        case WM_ACTIVATEAPP: return Format("WM_ACTIVATEAPP active=%i", wparam);
        case WM_SETCURSOR: return "WM_SETCURSOR";
        case WM_DISPLAYCHANGE: return Format("WM_DISPLAYCHANGE %i x %i, %i bpp", LOWORD(lparam), HIWORD(lparam), wparam);
        case WM_SETFOCUS: return "WM_SETFOCUS";
        case WM_KILLFOCUS: return "WM_KILLFOCUS";
        case WM_MOUSEACTIVATE: return "WM_MOUSEACTIVATE";
#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
        case WM_WINDOWPOSCHANGING: return Format("WM_WINDOWPOSCHANGING flags=0x%X", ((WINDOWPOS*)lparam)->flags);
#endif// WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
        case WM_WINDOWPOSCHANGED: return "WM_WINDOWPOSCHANGED";
        case WM_NCHITTEST: return "WM_NCHITTEST";
        case WM_NCACTIVATE: return Format("WM_NCACTIVATE active=%i", wparam);
        case WM_NCLBUTTONDOWN: return "WM_NCLBUTTONDOWN";
        case WM_NCRBUTTONDOWN: return "WM_NCRBUTTONDOWN";
        case WM_KEYDOWN: return Format("WM_KEYDOWN keycode=0x%X scan=0x%X", wparam, (lparam >> 16) & 0xFF);
        case WM_KEYUP: return "WM_KEYUP";
        case WM_DEADCHAR: return "WM_DEADCHAR";
        case WM_SYSKEYDOWN: return Format("WM_SYSKEYDOWN keycode=0x%X", wparam);
        case WM_SYSKEYUP: return "WM_SYSKEYUP";
        case WM_SYSCHAR: return "WM_SYSCHAR";
        case WM_SYSDEADCHAR: return "WM_SYSDEADCHAR";
        case WM_SYSCOMMAND: return Format("WM_SYSCOMMAND type=0x%X", wparam);
        case WM_LBUTTONDOWN: return "WM_LBUTTONDOWN";
        case WM_RBUTTONDOWN: return "WM_RBUTTONDOWN";
        case WM_ENTERMENULOOP: return "WM_ENTERMENULOOP";
        case WM_EXITMENULOOP: return "WM_EXITMENULOOP";
        case WM_SIZE: return "WM_SIZE";
        case WM_CHAR: return Format("WM_CHAR char=0x%X ('%c') scan=0x%X", wparam, wparam, (lparam >> 16) & 0xFF);
        case WM_TIMER: return "WM_TIMER";
        case WM_SETTINGCHANGE: return Format("WM_SETTINGCHANGE w=0x%X l=%s", wparam, lparam);
        case WM_POWERBROADCAST: return Format("WM_POWERBROADCAST w=0x%X", wparam);
        case WM_ERASEBKGND: return "WM_ERASEBKGND";
        case WM_PAINT: return "WM_PAINT";
        case WM_MOUSEMOVE: return "WM_MOUSEMOVE";
        case WM_CTLCOLORSTATIC: return "WM_CTLCOLORSTATIC";
        case WM_SETTEXT: return Format("WM_SETTEXT %s", WideToUtf8((const wchar_t*)lparam).c_str());
        case WM_NCMOUSEMOVE: return "WM_NCMOUSEMOVE";
        case WM_NCMOUSELEAVE: return "WM_NCMOUSELEAVE";
        case WM_CAPTURECHANGED: return "WM_CAPTURECHANGED";
        case WM_SETFONT: return "WM_SETFONT";
        case WM_NCCREATE: return "WM_NCCREATE";
        case WM_NCDESTROY: return "WM_NCDESTROY";
        case WM_NOTIFYFORMAT: return "WM_NOTIFYFORMAT";
        case WM_CHANGEUISTATE: return "WM_CHANGEUISTATE";
        case WM_UPDATEUISTATE: return "WM_UPDATEUISTATE";
        case WM_QUERYUISTATE: return "WM_QUERYUISTATE";
        case WM_INITDIALOG: return "WM_INITDIALOG";
        case WM_MOVE: return "WM_MOVE";
        case 0xAE: return "WM_NCUAHDRAWCAPTION";
        case 0xAF: return "WM_NCUAHDRAWFRAME";
        case WM_DPICHANGED:
        {
            RECT *rect = (LPRECT)lparam;
            return Format("WM_DPICHANGED dpi=%d Rect( %d, %d, %d, %d)", LOWORD(wparam), rect->left, rect->top, rect->right - rect->left, rect->bottom - rect->top);
        }
        default: return Format("WM %05X", message);
    }
}

#endif

#if !PLATFORM_GAMECORE
namespace
{
    DWORD WINAPI DebugMessageBoxRun(LPVOID parameter)
    {
        LPCSTR message = static_cast<LPCSTR>(parameter);
        MessageBoxA(NULL, message, "Debug", (MB_OK | MB_ICONINFORMATION));
        return 0;
    }
}

void winutils::DebugMessageBox(LPCSTR format, ...)
{
    // format message

    CHAR message[256];

    va_list args;
    va_start(args, format);

    vsprintf_s(message, format, args);

    va_end(args);

    // we want message box in a separate thread so it wouldn't pump messages in the main thread which might mess things up

    win::ThreadHandle thread(CreateThread(NULL, 0, DebugMessageBoxRun, message, 0, NULL));
    Assert(thread.IsOpen());

    WaitForSingleObject(thread, INFINITE);
}

#endif// !PLATFORM_GAMECORE

void winutils::HandleDebugBreak(int type)
{
    if (type & kCommandLine)
    {
        // Engage DebugBreak for the editor if opted to do so.
        // Will then allow attaching of debugger at startup
        // Requires aeDebug registry set for Auto request of debugger
        // See External/Utilities/Windows/AeDebug/aedebug.reg
        if (HasARGV("dbgbreak"))
        {
            ::DebugBreak();
        }
    }
}

#if !UNITY_EXTERNAL_TOOL
#if !PLATFORM_FAMILY_WINDOWSGAMES
#include "Runtime/Utilities/FileUtilities.h"
#include "PlatformDependent/Win/StackWalker.h"

class InternalStackWalker : public StackWalker
{
public:
    InternalStackWalker()
    {
        AssertMsg(m_szSymPath == NULL, "InternalStackWalker: Symbol path should not be set by parent class");
#if UNITY_EDITOR
        // If we are in the middle of startup or shutdown, we may not have a file system, so carry on
        // without symbols rather than crashing in the crash handler.
        if (FileSystemIsMounted())
        {
            core::string monoPdbPath = GetApplicationFolder() + "/Data/Mono";
            m_szSymPath = _strdup(monoPdbPath.c_str());
        }
#endif
        // Standalone players don't have mono.pdb files in their data folder, so there's nothing to setup
    }

    void OnDbgHelpErr(LPCSTR szFuncName, DWORD gle, DWORD64 addr)
    {
        core::string lastErrorMsg = winutils::ErrorCodeToMsg(gle);
        int l = -1;
        if ((l = lastErrorMsg.find_last_of('.')) != core::string::npos)
        {
            lastErrorMsg.resize(l + 1);
        }
        printf_console("  ERROR: %s, GetLastError: '%s' (Address: %p)\n", szFuncName, lastErrorMsg.c_str(), (LPVOID)addr);
    }

    void OnOutput(LPCSTR buffer)
    {
        printf_console(buffer);
    }
};

static void PrintStackTraceToConsole(PEXCEPTION_POINTERS pExInfo)
{
    InternalStackWalker stackWalk;

    printf_console("Crash!!!\n");

    stackWalk.LoadModules();
    printf_console("\n========== OUTPUTTING STACK TRACE ==================\n\n");
    stackWalk.ShowCallstack(NULL, pExInfo->ContextRecord);
    printf_console("\n========== END OF STACKTRACE ===========\n\n");
}

//
// *** IMPORTANT NOTE ***
//
// It is highly likely that Unity is in a very bad state when this is called.
//
// To avoid compromising the crash handling process, do not call any methods that:
//  * allocate memory
//  * rely on the file system
//  * read from or write to RuntimeStatics
//  * rely on any other Unity subsystem
//
// If you want to do anything that might require the above, you must cache the
// required data at runtime, and you cannot rely on it being valid.
//
// DO NOT ADD MORE __try/__except BLOCKS HERE unless absolutely necessary.
// If you feel the need to do this, it means you're doing something unsafe and
// you should probably consider a different approach. The only exception is
// logging, because more diagnostic information is always good when we can get it.
//
// Do not log in __except blocks. It's likely the logging failed and that is why
// the block was executed in the first place. Ideally, do nothing in these blocks.
//
// This function is recursive. If the crash handler crashes, this function will
// be called again with the new exception information.
//
int winutils::ProcessInternalCrash(PEXCEPTION_POINTERS pExInfo, bool passToDefaultCrashHandler)
{
    static char entryCount = 0;

    // Only dump the stack on the first recursion of the crash handler
    if (entryCount++ == 0)
    {
        // In some circumstances - such as when the Asset GC is running - memory allocations are disabled.
        // We actually need to do some allocations in some circumstances during this crash handler - such
        // as when converting strings between ANSI and widechar - so if the exception is thrown while the
        // Asset GC is running, and we then try to allocate, we crash the crash handler, and end up with
        // no callstack information. To prevent this, we re-enable allocations here.

        if (GetMemoryManagerPtr())
            GetMemoryManager().ReallowAllocationsOnThisThread();

        // ** DO NOT ALLOCATE MEMORY BEFORE THIS LINE **

        __try
        {
            PrintStackTraceToConsole(pExInfo);
        }
        __except (EXCEPTION_EXECUTE_HANDLER)
        {
        }
    }

    // If a debugger is attached, let the debugger handle it now that we've dumped the stack trace
    if (IsDebuggerAttached())
    {
        return EXCEPTION_CONTINUE_SEARCH;
    }
    // For debugging, allow someone to attach a debugger
    else if (g_BreakToAllowDebuggerAttach && !IsBatchmode())
    {
        ::MessageBoxA(nullptr, "A crash has been intercepted. Attach a debugger now and press OK to debug, or just press OK to resume normal crash handling.", "Crash", MB_OK | MB_ICONEXCLAMATION);
    }

    if (passToDefaultCrashHandler)
    {
        // print_console might further compromise the crash handling process but this information is useful, so we'll try it
        __try
        {
            if (s_CachedCrashHandlerOutputPath[0])
                printf_console("A crash has been intercepted by the crash handler. For call stack and other details, see the latest crash report generated in:\n * %s\n", s_CachedCrashHandlerOutputPath);
            else
                printf_console("A crash has been intercepted by the crash handler, but either no crash report folder has been set or the crash occurred before Unity was fully initialized or while Unity was shutting down.\n");
        }
        __except (EXCEPTION_EXECUTE_HANDLER)
        {
        }

        // Hand over to the crash handler.
        winutils::CrashHandler::GetInstalled().HandleCrash(pExInfo);

        // If HandleCrash works as expected, we will never reach here because the external crash handler will
        // have terminated the process. As such, if we *do* reach here, the crash handler aborted abnormally.
        // In that case, we'll fall back to WER.
    }

    return EXCEPTION_CONTINUE_SEARCH;
}

LONG WINAPI _ProcessInternalCrash(PEXCEPTION_POINTERS pExInfo)
{
    return winutils::ProcessInternalCrash(pExInfo, true);
}

void winutils::InstallInternalCrashHandler()
{
    g_BreakToAllowDebuggerAttach = HasARGV("break-on-crash");
    SetUnhandledExceptionFilter(_ProcessInternalCrash);
}

void winutils::UninstallInternalCrashHandler()
{
    UnsetUnhandledExceptionFilterIfStillInstalled(_ProcessInternalCrash);
}

void winutils::SetCrashReportLocationForReporting(const char* location)
{
    strncpy_s(s_CachedCrashHandlerOutputPath, sizeof(s_CachedCrashHandlerOutputPath), location ? location : "", _TRUNCATE);
}

void winutils::UnsetUnhandledExceptionFilterIfStillInstalled(LPTOP_LEVEL_EXCEPTION_FILTER expected)
{
    LPTOP_LEVEL_EXCEPTION_FILTER oldHandler = SetUnhandledExceptionFilter(nullptr);
    if (oldHandler != expected && oldHandler != nullptr)
    {
        SetUnhandledExceptionFilter(oldHandler);
    }
}

#endif // !PLATFORM_FAMILY_WINDOWSGAMES

std::vector<const char*> winutils::ChopUpCommandLine(LPSTR szCmdLine)
{
    std::vector<const char*> result;

    CHAR* pos = szCmdLine;
    CHAR* lastPos;

    while (*pos)
    {
        // Skip leading whitespace.
        while (*pos && isspace(*pos))
            ++pos;

        // Check if this segment is quoted.
        bool isQuoted = (*pos == '"');
        if (isQuoted)
            ++pos;

        // First param is always the executable path and we know that windows
        // don't allow quotes in paths. This information will become relevant below.
        bool isExePath = (result.size() == 0);

        // Remember start of segment.
        lastPos = pos;

        // Skip to next whitespace or end of string.
        while (*pos)
        {
            if (IsSpace(*pos))
            {
                if (isQuoted && pos > lastPos && *(pos - 1) == '"')
                    break;
                else if (!isQuoted)
                    break;
            }
            else if (isQuoted && pos[0] == '\\' && pos[1] == '"')
            {
                if (!isExePath)
                    ++pos;
            }
            else if (pos[0] == '"' && isExePath && isQuoted)
            {
                isQuoted = false;
                // Remove the quote from the executable path.
                memmove(pos, pos + 1, strlen(pos));
                --pos;
            }

            ++pos;
        }

        // If we have a non-empty segment, push it
        // onto the result stack.
        if (pos != lastPos)
        {
            // Append trailing null.
            if (isQuoted)
            {
                *(pos - 1) = '\0';
            }
            else if (*pos != '\0')
            {
                *pos = '\0';
                ++pos;
            }

            result.push_back(lastPos);
        }
    }

    return result;
}

#if !PLATFORM_FAMILY_WINDOWSGAMES
void winutils::SetupWideArgs(LPWSTR szCmdLine)
{
    // If parameters were supplied via WinMain then we'll use these values
    // However we still need to parse executable path from GetCommandLineW
    if (szCmdLine != nullptr)
    {
        LPWSTR *argList;
        int numArgs;

        // Adapted from CommandLineToArgvW API sample code
        argList = CommandLineToArgvW(GetCommandLineW(), &numArgs);
        if (argList != nullptr && numArgs > 0)
        {
            // Case 1215557
            // Surround the executable file path (1st parameter) with quotes.
            // This is necessary in order for ChopUpCommandLine() to correctly
            // parse the path when it contains spaces, e.g. C:\project\name with spaces.exe.
            core::wstring filePath(kMemTempAlloc);
            filePath.reserve(wcslen(argList[0] + 2));
            filePath.append(L"\"");
            filePath.append(argList[0]);
            filePath.append(L"\"");

            ConvertWideToUTF8String(filePath, s_UTF8CommandLine);
            LocalFree(argList);
        }

        // Append remaining arguments if any
        // NOTE: Even if szCmdLine is empty, we still want to use this code path (not the 'else')
        // If WinMain is overridden by the user they may want to remove parameters by passing
        // an empty string into UnityMain; need to honor this by not calling GetCommandLineW.
        if (wcslen(szCmdLine) > 0)
        {
            core::string commandLineArgs(kMemTempAlloc);
            ConvertWideToUTF8String(szCmdLine, commandLineArgs);
            s_UTF8CommandLine.append(" ");
            s_UTF8CommandLine.append(commandLineArgs);
        }
    }
    else
    {
        // If CmdList is explicitly null (as with the Editor) then use GetCommandLineW
        ConvertWideToUTF8String(::GetCommandLineW(), s_UTF8CommandLine);
    }

    s_UTF8Args = ChopUpCommandLine((LPSTR)(s_UTF8CommandLine.c_str()));
    SetupArgv(s_UTF8Args.size(), s_UTF8Args.data());
}

void winutils::SetupBootConfig(const char* bootConfigFile)
{
    int     argc;
    LPWSTR* argvW = CommandLineToArgvW(GetCommandLineW(), &argc);
    argvW = &argvW[1]; argc = argc - 1; // skip program name

    char** argv = static_cast<char**>(alloca(sizeof(*argv) * argc));
    for (int i = 0; i < argc; ++i)
    {
        size_t strlen = ::WideCharToMultiByte(CP_UTF8, 0, argvW[i], -1, NULL, 0, NULL, NULL);
        argv[i] = static_cast<char*>(alloca(strlen));
        ::WideCharToMultiByte(CP_UTF8, 0, argvW[i], -1, argv[i], strlen, NULL, NULL);
    }

    if (bootConfigFile)
    {
        if (!BootConfig::InitFromFile(const_cast<const char**>(argv), argc, bootConfigFile))
        {
            WarningString("no boot config - using default values");
            BootConfig::Init(const_cast<const char**>(argv), argc);
        }
    }
    else
        BootConfig::Init(const_cast<const char**>(argv), argc);

    LocalFree(argvW);
}

#if !UNITY_EXTERNAL_TOOL
// Returns a path to %USERPROFILE%\AppData\LocalLow\<CompanyName>\<ProductName> (Vista+)
// Or C:\Documents and Settings\username\Local Settings\Application Data\<CompanyName>\<ProductName> (XP).
core::string winutils::GetAppSpecificWindowsUserStorageLocation(const core::string& vendor, const core::string& appName)
{
    return AppendPathName(AppendPathName(systeminfo::GetPersistentDataPath(), vendor), appName);
}

// Returns a path to %TEMP%\<CompanyName>\<ProductName>
core::string winutils::GetAppSpecificTemporaryStorageLocation(const core::string& vendor, const core::string& appName)
{
    return AppendPathName(AppendPathName(systeminfo::GetTemporaryCachePath(), vendor), appName);
}

core::string winutils::GetCrashReportOutputFolder()
{
    // Set the crash report folder
    core::string crashReportFolder = GetFirstValueForARGV("crash-report-folder");
    if (crashReportFolder.empty())
    {
        core::string vendor, appName;
        GetApplicationVendorAndName(vendor, appName);

        // Get the application storage directory in Temp
        core::string appStoragePath = winutils::GetAppSpecificTemporaryStorageLocation(vendor, appName);

        crashReportFolder = AppendPathName(appStoragePath, "Crashes");
    }
    return crashReportFolder;
}

#endif

#endif// !PLATFORM_FAMILY_WINDOWSGAMES

#endif
