#include "Runtime/Utilities/LaunchUtilities.h"
#include "Runtime/Utilities/PathNameUtility.h"
#include "Runtime/Utilities/File.h"
#include "PathUnicodeConversion.h"

#include "Runtime/Testing/Faking.h"

#include <winbase.h>
#include <winuser.h>
#include <ShellAPI.h>

#if UNITY_EDITOR
bool OpenWithBuiltinEditor(const core::string &path);
#endif

bool LaunchApplication(const core::string& path, const dynamic_array<core::string>& arguments, bool quotableArgs, bool hideAndWait)
{
    __FAKEABLE_FUNCTION__(LaunchApplication, (path, arguments, quotableArgs, hideAndWait));

    wchar_t widePath[kDefaultPathBufferSize];
    ConvertUnityPathName(path.c_str(), widePath, kDefaultPathBufferSize);

    if (arguments.empty())
    {
        SHELLEXECUTEINFOW shellExecuteInfo = {0};
        shellExecuteInfo.cbSize = sizeof(SHELLEXECUTEINFOW);
        shellExecuteInfo.fMask = hideAndWait ? SEE_MASK_NOCLOSEPROCESS : SEE_MASK_NOASYNC;
        shellExecuteInfo.hwnd = NULL;
        shellExecuteInfo.lpVerb = L"open";
        shellExecuteInfo.lpFile = widePath;
        shellExecuteInfo.lpParameters = NULL;
        shellExecuteInfo.lpDirectory = NULL;
        shellExecuteInfo.nShow = hideAndWait ? SW_HIDE : SW_SHOWNORMAL;
        shellExecuteInfo.hInstApp = NULL;

        bool res = true;
        if (ShellExecuteExW(&shellExecuteInfo) == FALSE)
            res = false;

        if (hideAndWait && res && shellExecuteInfo.hProcess != NULL)
            WaitForSingleObject(shellExecuteInfo.hProcess, INFINITE);

        return res;
    }

    // generate an argument list
    core::wstring argumentString;
    core::wstring argWide;
    for (int i = 0; i < arguments.size();)
    {
        core::string arg = arguments[i];
        // Check if arg is a path or not. In windows anything beginning with a '/' is not a valid path.
        if (!arg.empty() && arg[0] != '/')
        {
            __FAKEABLE_SEQUENCE_POINT__(LaunchApplication, ConvertPathSeparators);
            ConvertSeparatorsToWindows(arg);
        }
        if (quotableArgs)
            arg = QuoteString(arg);
        ConvertUTF8ToWideString(arg, argWide);
        argumentString += argWide;
        if (++i < arguments.size())
            argumentString += L' ';
    }

    wchar_t* argumentBuffer = new wchar_t[argumentString.size() + 1];
    memcpy(argumentBuffer, argumentString.c_str(), (argumentString.size() + 1) * sizeof(wchar_t));

    SHELLEXECUTEINFOW shellExecuteInfo = {0};
    shellExecuteInfo.cbSize = sizeof(SHELLEXECUTEINFOW);
    shellExecuteInfo.fMask = hideAndWait ? SEE_MASK_NOCLOSEPROCESS : SEE_MASK_NOASYNC;
    shellExecuteInfo.hwnd = NULL;
    shellExecuteInfo.lpVerb = L"open";
    shellExecuteInfo.lpFile = widePath;
    shellExecuteInfo.lpParameters = argumentBuffer;
    shellExecuteInfo.lpDirectory = NULL;
    shellExecuteInfo.nShow = hideAndWait ? SW_HIDE : SW_SHOWNORMAL;
    shellExecuteInfo.hInstApp = NULL;

    bool res = true;
    if (SYSTEM_CALL(ShellExecuteExW, (&shellExecuteInfo)) == FALSE)
        res = false;

    if (hideAndWait && res && shellExecuteInfo.hProcess != NULL)
        WaitForSingleObject(shellExecuteInfo.hProcess, INFINITE);

    delete[] argumentBuffer;
    return res;
}

bool OpenWithDefaultApp(core::string_ref path)
{
    core::wstring widePath;
    ConvertUnityPathName(path, widePath);

    size_t res = (size_t)::ShellExecuteW(NULL, L"open", widePath.c_str(), NULL, NULL, SW_SHOWNORMAL);
    if (res <= 32)
        res = (size_t)::ShellExecuteW(NULL, L"edit", widePath.c_str(), NULL, NULL, SW_SHOWNORMAL);

    return (res > 32);
}

bool OpenFileWithApplication(core::string_ref path, const core::string& app)
{
    core::string sapp = PathToAbsolutePath(app);
    core::string spath = QuoteString(PathToAbsolutePath(path));

    wchar_t widePath[kDefaultPathBufferSize];
    wchar_t wideApp[kDefaultPathBufferSize];

    ConvertUnityPathName(sapp.c_str(), wideApp, kDefaultPathBufferSize);
    ConvertUnityPathName(spath.c_str(), widePath, kDefaultPathBufferSize);

    size_t res = (size_t)::ShellExecuteW(NULL, L"open", wideApp, widePath, NULL, SW_SHOWNORMAL);
    if (res <= 32)
    {
        res = (size_t)::ShellExecuteW(NULL, L"edit", wideApp, widePath, NULL, SW_SHOWNORMAL);
    }

    return res > 32;
}

core::string GetDefaultApplicationForFile(const core::string& path)
{
    core::string appPath;

    wchar_t wfile[kDefaultPathBufferSize];
    wchar_t wapp[kDefaultPathBufferSize];

    ConvertUnityPathName(path.c_str(), wfile + 1, kDefaultPathBufferSize - 1);

    wfile[0] = L'"';
    wfile[wcslen(wfile) + 1] = 0;
    wfile[wcslen(wfile)] = L'"';

    if ((size_t)FindExecutableW(wfile, NULL, wapp) <= 32)
        return core::string();

    ConvertWindowsPathName(wapp, appPath);

    return appPath;
}
