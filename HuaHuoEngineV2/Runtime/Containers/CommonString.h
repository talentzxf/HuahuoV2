#pragma once

#include <cstdlib>

namespace HuaHuo
{
namespace CommonString
{
    // buffer begin/end address
    extern const char* const BufferBegin;
    extern const char* const BufferEnd;

    // enums for string Id
    enum
    {
        IdBase = -1,
#define COMMON_STRING_ENTRY(a, b) Id_##a,
#include "CommonStrings.h"
#undef COMMON_STRING_ENTRY
        IdEmpty,

        Count = IdEmpty
    };

    // literal pointers
#define COMMON_STRING_ENTRY(a, b) extern const char* const gLiteral_##a;
#include "CommonStrings.h"
#undef COMMON_STRING_ENTRY
    extern const char* const gLiteralEmpty;
} // namespace CommonString
} // namespace HuaHuo

#define CommonString(a) ::HuaHuo::CommonString::gLiteral_##a
#define CommonStringEmpty ::HuaHuo::CommonString::gLiteralEmpty

inline bool IsCommonString(const char* str)
{
    return str == NULL || (str >= HuaHuo::CommonString::BufferBegin && str < HuaHuo::CommonString::BufferEnd);
}
