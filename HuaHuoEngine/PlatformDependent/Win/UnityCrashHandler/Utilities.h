#pragma once

#include "UnityPrefix.h"
#include "Runtime/Utilities/RegisterRuntimeInitializeAndCleanup.h"
#include <windef.h>

std::wstring GetTempDirectory();
std::wstring GetAppDataLocalLowDirectory();
std::wstring GetTimeString();

void WaitForDebugger();
void MakeConsole();

bool PopCommandLineSwitch(const wchar_t* clSwitch, int& argc, wchar_t** argv);

const wchar_t* GetFilePart(const wchar_t* lpszFile);
std::wstring GetDirectoryPart(const wchar_t* lpszFile);
std::wstring ChangeExtension(const std::wstring& filename, const wchar_t* newExt);
bool FileExists(const wchar_t* path);

std::wstring MakeUniqueFileNameInTargetPath(std::wstring sourcePath, const std::wstring& targetDir);
bool CreateDumpDirectory(const std::wstring& preferredBasePath, std::wstring& dumpBasePath);

// StringToInteger: Alternative to sscanf/atoi because this will error if the string is invalid
template<typename Integer, typename Char> bool StringToInteger(const Char* value, Integer* out)
{
    if (value == nullptr)
        return false;

    // Convert to integer. We don't use atoi because that doesn't offer
    // enough error detail.
    Integer i = 0;
    for (const Char* cur = value; *cur; ++cur)
    {
        if (*cur < (Char)L'0' || *cur > (Char)L'9')
            return false;

        // Check for overflow.
        if (DoesMultiplicationOverflow<Integer>(i, 10))
            return false;

        Integer newOrder = i * 10;
        Integer add = *cur - (Char)L'0';

        if (DoesAdditionOverflow(newOrder, add))
            return false;

        i = newOrder + add;
    }

    *out = i;
    return true;
}

template<typename Integer, typename Char> Integer StringToInteger(const Char* value, Integer default)
{
    Integer i;
    if (!StringToInteger<Integer, Char>(value, &i))
        return default;
    return i;
}

bool ReadEntireFile(HANDLE h, std::vector<UInt8>& out);
bool ReadEntireFile(const wchar_t* filename, std::vector<UInt8>& out);
