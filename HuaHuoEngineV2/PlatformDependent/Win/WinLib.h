#pragma once

#include "WinLibLoader.h"
#include <shellscalingapi.h>
#include <Dbghelp.h>
#include <array>

namespace winlib
{
    // User32.dll
    struct User32Dll
    {
        static constexpr const char* DllName = "User32.dll";
        static constexpr std::array<const char*, 5> FnNames = {
            "GetDpiForWindow",
            "EnableNonClientDpiScaling",
            "AdjustWindowRectExForDpi",
            "SystemParametersInfoForDpi",
            "GetSystemMetricsForDpi"
        };

        enum class Fn
        {
            GetDpiForWindow = 0,
            EnableNonClientDpiScaling,
            AdjustWindowRectExForDpi,
            SystemParametersInfoForDpi,
            GetSystemMetricsForDpi
        };
    };

    using LibUser32 = winlibloader::Lib<
        User32Dll,
        UINT (WINAPI*)(HWND hWnd),
        UINT (WINAPI*)(HWND hWnd),
        BOOL (WINAPI*)(LPRECT lpRect, DWORD dwStyle, BOOL bMenu, DWORD dwExStyle, UINT dpi),
        BOOL (WINAPI*)(UINT uiAction, UINT uiParam, PVOID pvParam, UINT fWinIni, UINT dpi),
        INT  (WINAPI*)(int nIndex, UINT dpi)
    >;

    LibUser32& GetUser32();

    // shcore.dll
    struct ShcoreDll
    {
        static constexpr const char* DllName = "shcore.dll";
        static constexpr std::array<const char*, 1> FnNames = {
            "GetDpiForMonitor"
        };

        enum class Fn
        {
            GetDpiForMonitor
        };
    };

    using LibShcore = winlibloader::Lib<
        ShcoreDll,
        UINT(WINAPI*)(HMONITOR hmonitor, MONITOR_DPI_TYPE dpiType, UINT *dpiX, UINT *dpiY)
    >;

    LibShcore& GetShcore();

    // kernel32.dll
    struct Kernel32Dll
    {
        static constexpr const char* DllName = "kernel32.dll";
        static constexpr std::array<const char*, 3> FnNames = {
            "IsWow64Process",
            "GetProcessUserModeExceptionPolicy",
            "SetProcessUserModeExceptionPolicy",
        };

        enum class Fn
        {
            IsWow64Process = 0,
            GetProcessUserModeExceptionPolicy,
            SetProcessUserModeExceptionPolicy,
        };
    };

    using LibKernel32 = winlibloader::Lib<
        Kernel32Dll,
        BOOL(WINAPI*)(HANDLE hProcess, PBOOL Wow64Process),
        BOOL(WINAPI*)(LPUINT lpFlags),
        BOOL(WINAPI*)(UINT dwFlags)
    >;

    LibKernel32& GetKernel32();

    // dbghelp.dll
    struct DbghelpDll
    {
        static constexpr const char* DllName = "dbghelp.dll";
        static constexpr std::array<const char*, 1> FnNames = {
            "MiniDumpWriteDump"
        };

        enum class Fn
        {
            MiniDumpWriteDump = 0,
        };
    };

    using LibDbghelp = winlibloader::Lib<
        DbghelpDll,
        BOOL(WINAPI*)(
            HANDLE hProcess,
            DWORD ProcessId,
            HANDLE hFile,
            MINIDUMP_TYPE DumpType,
            PMINIDUMP_EXCEPTION_INFORMATION ExceptionParam,
            PMINIDUMP_USER_STREAM_INFORMATION UserStreamParam,
            PMINIDUMP_CALLBACK_INFORMATION CallbackParam)
    >;

    LibDbghelp& GetDbghelp();
}
