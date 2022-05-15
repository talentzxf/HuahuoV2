#pragma once

#include <string>
#include <stringapiset.h>
#include "../Baselib_InlineImplementation.h"

BASELIB_INLINE_IMPL std::wstring WinApi_StringConversions_UTF8ToUTF16(const char* str)
{
    auto length = strlen(str);
    auto utf16Length = MultiByteToWideChar(CP_UTF8, 0, str, static_cast<int>(length), nullptr, 0);
    if (utf16Length <= 0)
        return std::wstring();

    std::wstring utf16;
    utf16.resize(utf16Length);
    MultiByteToWideChar(CP_UTF8, 0, str, static_cast<int>(length), &utf16[0], utf16Length);
    return utf16;
}

BASELIB_INLINE_IMPL std::string WinApi_StringConversions_UTF16ToUTF8(const std::wstring& str)
{
    auto utf8Length = WideCharToMultiByte(CP_UTF8, 0, str.c_str(), static_cast<int>(str.size()), nullptr, 0, nullptr, nullptr);
    if (utf8Length <= 0)
        return std::string();

    std::string utf8;
    utf8.resize(utf8Length);
    WideCharToMultiByte(CP_UTF8, 0, str.c_str(), static_cast<int>(str.size()), &utf8[0], utf8Length, nullptr, nullptr);
    return utf8;
}
