#pragma once

#include "UnityPrefix.h"
#include "Utilities.h"
#include "PlatformDependent/Win/WinUtils.h"

#include <io.h>
#include <fcntl.h>
#include <stdio.h>
#include <signal.h>

#ifdef NTDDI_VERSION
#   undef NTDDI_VERSION
#endif
#define NTDDI_VERSION NTDDI_VISTA
#include <ShlObj.h>

typedef HRESULT (WINAPI *PFN_SH_GET_KNOWN_FOLDER_PATH)(
    REFKNOWNFOLDERID rfid,
    DWORD /* KNOWN_FOLDER_FLAG */ dwFlags,
    HANDLE hToken,
    PWSTR *ppszPath); // free *ppszPath with CoTaskMemFree

typedef HRESULT (WINAPI *PFN_SH_GET_FOLDER_PATH_W)(
    HWND hwnd,
    int csidl,
    HANDLE hToken,
    DWORD dwFlags,
    LPWSTR pszPath);

std::wstring GetTempDirectory()
{
    wchar_t tempPath[MAX_PATH + 1];
    ::GetTempPathW(sizeof(tempPath) / sizeof(tempPath[0]), tempPath);
    return tempPath;
}

void XPGetKnownFolderPath(PFN_SH_GET_FOLDER_PATH_W pFunc, std::wstring& out)
{
    wchar_t widePath[MAX_PATH + 1] = { 0 };
    HRESULT hr = pFunc(nullptr, CSIDL_APPDATA, nullptr, SHGFP_TYPE_CURRENT, widePath);
    if (SUCCEEDED(hr) && hr != S_FALSE && widePath[0] != L'\0')
    {
        out = widePath;
    }
}

void VistaGetKnownFolderPath(PFN_SH_GET_KNOWN_FOLDER_PATH pFunc, std::wstring& out)
{
    wchar_t *widePath = nullptr;
    HRESULT hr = pFunc(FOLDERID_LocalAppDataLow, 0, nullptr, &widePath);

    if (SUCCEEDED(hr) && widePath[0] != L'\0')
    {
        out = widePath;
    }

    if (widePath != nullptr)
        CoTaskMemFree(widePath);
}

std::wstring GetAppDataLocalLowDirectory()
{
    static std::wstring s_LocalLow;
    if (s_LocalLow.empty())
    {
        // Set as temp directory for now
        s_LocalLow = GetTempDirectory();

        // Load Shell32.dll so we can try and get a better path, e.g. user's local app data directory
        HMODULE hShell32 = LoadLibraryA("Shell32.dll");
        if (hShell32 != nullptr)
        {
            // On Vista and above, we should use SHGetKnownFolderPath
            // else, use SHGetFolderPath.
            PFN_SH_GET_KNOWN_FOLDER_PATH pfGetKnownFolderPath = (PFN_SH_GET_KNOWN_FOLDER_PATH)
                GetProcAddress(hShell32, "SHGetKnownFolderPath");

            if (pfGetKnownFolderPath != nullptr)
            {
                VistaGetKnownFolderPath(pfGetKnownFolderPath, s_LocalLow);
            }
            else
            {
                PFN_SH_GET_FOLDER_PATH_W pfGetFolderPath = (PFN_SH_GET_FOLDER_PATH_W)
                    GetProcAddress(hShell32, "SHGetFolderPathW");
                if (pfGetFolderPath != nullptr)
                {
                    XPGetKnownFolderPath(pfGetFolderPath, s_LocalLow);
                }
            }

            FreeLibrary(hShell32);
        }
    }

    return s_LocalLow;
}

// Check for --wait-for-debugger
void WaitForDebugger()
{
    if (IsDebuggerAttached())
        return;

    printf_console("[CRASH HANDLER] Waiting for debugger...\n");

    while (!IsDebuggerAttached())
    {
        Sleep(500);
    }
}

// Check for --console
void MakeConsole()
{
    if (AllocConsole())
    {
        // The crash handler might have been started without a valid STDOUT.
        HANDLE hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
        if (hStdOut != nullptr && hStdOut != INVALID_HANDLE_VALUE)
        {
            // Hook up stdout to the new console
            FILE* fStdOut = _fdopen(
                _open_osfhandle(
                    (intptr_t)hStdOut,
                    _O_TEXT), "w");
            setvbuf(fStdOut, nullptr, _IONBF, 1);
            *stdout = *fStdOut;
        }
    }
}

// Searches for a command line switch and removes it and returns true if found
bool PopCommandLineSwitch(const wchar_t* clSwitch, int& argc, wchar_t** argv)
{
    int oldc = argc;
    argc = 1;

    for (int i = 1; i < oldc; ++i)
    {
        if (wcsicmp(clSwitch, argv[i]) == 0)
        {
            ++i;
        }

        if (i < oldc)
            argv[argc++] = argv[i];
    }

    return oldc != argc;
}

const wchar_t* GetFilePart(const wchar_t* lpszFile)
{
    const wchar_t *result = wcsrchr(lpszFile, L'\\');
    if (result)
        result++;
    else
        result = lpszFile;
    return result;
}

std::wstring GetDirectoryPart(const wchar_t* path)
{
    std::wstring tmp = path;

    const wchar_t* filePart = GetFilePart(path);
    if (filePart == path)
        return std::wstring();

    return tmp.substr(0, filePart - path);
}

std::wstring ChangeExtension(const std::wstring& path, const wchar_t* newExt)
{
    std::wstring::size_type lastDotPos = path.find_last_of(L'.');

    if (lastDotPos == std::wstring::npos)
        return path + newExt;
    else
        return path.substr(0, lastDotPos) + newExt;
}

bool FileExists(const wchar_t* path)
{
    DWORD attrib = GetFileAttributesW(path);
    return (attrib != INVALID_FILE_ATTRIBUTES && !(attrib & FILE_ATTRIBUTE_DIRECTORY));
}

static void MakeUniqueFilename(std::wstring& filename)
{
#define UNIQUE_FILENAME_PRECISION 3
#define UNIQUE_FILENAME_PRECISION_WIDE_(x) L##x
#define UNIQUE_FILENAME_PRECISION_STR_(x) UNIQUE_FILENAME_PRECISION_WIDE_(#x)
#define UNIQUE_FILENAME_PRECISION_STR(x) UNIQUE_FILENAME_PRECISION_STR_(x)

    size_t dotPos = filename.find_last_of(L'.');
    std::wstring filenamePart, extensionPart;
    if (dotPos != std::wstring::npos)
    {
        filenamePart = filename.substr(0, dotPos);
        extensionPart = filename.substr(dotPos);
    }
    else
    {
        filenamePart = filename;
    }

    std::vector<wchar_t> tmp(filename.length() + UNIQUE_FILENAME_PRECISION + 1);

    int retry = 0;
    do
    {
        swprintf_s(tmp.data(), tmp.size(), L"%s%0" UNIQUE_FILENAME_PRECISION_STR(UNIQUE_FILENAME_PRECISION) L"d%s", filenamePart.c_str(), ++retry, extensionPart.c_str());
    }
    while (FileExists(tmp.data()));

    filename = tmp.data();
}

std::wstring MakeUniqueFileNameInTargetPath(std::wstring sourcePath, const std::wstring& targetDir)
{
    std::replace(sourcePath.begin(), sourcePath.end(), L'/', L'\\');

    // Strip off the filename part
    const wchar_t* filename = GetFilePart(sourcePath.c_str());
    std::wstring targetFilename = targetDir + L"\\" + filename;

    // If the file exists as some other name, then increment the number
    if (FileExists(targetFilename.c_str()))
    {
        MakeUniqueFilename(targetFilename);
    }

    return targetFilename;
}

// Generate a timestamp string
std::wstring GetTimeString()
{
    SYSTEMTIME time;
    GetSystemTime(&time);

    wchar_t tmp[512] = { 0 };
    swprintf_s(
        tmp,
        _countof(tmp),
        L"%04d-%02d-%02d_%02d%02d%02d%03d",
        time.wYear,
        time.wMonth,
        time.wDay,
        time.wHour,
        time.wMinute,
        time.wSecond,
        time.wMilliseconds);
    return tmp;
}

bool CreateDumpDirectory(const std::wstring& preferredBasePath, std::wstring& dumpBasePath)
{
    std::wstring crashFolder(L"\\Crash_");
    crashFolder += GetTimeString();

    // We'll attempt to put it in the preferred path first... (if specified)
    int ok = -1;
    if (!preferredBasePath.empty())
    {
        dumpBasePath = preferredBasePath + crashFolder;
        ok = ::SHCreateDirectoryExW(NULL, dumpBasePath.c_str(), NULL);
    }

    if (ok != ERROR_SUCCESS && ok != ERROR_ALREADY_EXISTS && ok != ERROR_FILE_EXISTS)
    {
        // Fall back to the temporary directory
        dumpBasePath = GetTempDirectory() + crashFolder;
        ok = ::SHCreateDirectoryExW(NULL, dumpBasePath.c_str(), NULL);
        if (ok != ERROR_SUCCESS && ok != ERROR_ALREADY_EXISTS && ok != ERROR_FILE_EXISTS)
        {
            return false;
        }
    }

    return true;
}

bool ReadEntireFile(HANDLE h, std::vector<UInt8>& out)
{
    DWORD highSize = 0;
    DWORD size = GetFileSize(h, &highSize);
    if (highSize != 0)
        return false;

    size_t offset = out.size();
    out.resize(out.size() + size);

    DWORD totalRead = 0;
    if (!ReadFile(h, out.data() + offset, size, &totalRead, nullptr))
    {
        out.resize(offset + totalRead); // truncate the data
        return false;
    }

    return true;
}

bool ReadEntireFile(const wchar_t* filename, std::vector<UInt8>& data)
{
    // Open the file
    winutils::AutoHandle handle(CreateFileW(
        filename,
        GENERIC_READ, 0, nullptr, OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL, 0));
    if (!handle.IsValid())
    {
        return false;
    }

    // Read the entire file
    return ReadEntireFile(handle.Get(), data);
}
