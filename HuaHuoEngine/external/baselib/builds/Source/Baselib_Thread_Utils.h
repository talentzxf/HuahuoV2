#pragma once

#include "Include/C/Baselib_Thread.h"
#include <cstring>

template<size_t length>
class TruncatedString
{
public:
    TruncatedString(const char* str) : TruncatedString(str, str ? strlen(str) : 0)
    {}

    TruncatedString(const char* str, size_t strLen)
    {
        const size_t charactersToCopy = strLen < length ? strLen : length;
        memcpy(strCopy, str, charactersToCopy);
        strCopy[charactersToCopy] = '\0';
    }

    const char* c_str() { return strCopy; }

private:
    char strCopy[length + 1];
};

struct Baselib_Thread_Common
{
    Baselib_Thread_Common(Baselib_Thread_EntryPointFunction threadEntryPoint, void* threadEntryPointArgument)
        : id(Baselib_Thread_InvalidId), entryPoint(threadEntryPoint), entryPointArgument(threadEntryPointArgument) {}
    Baselib_Thread_Id id;
    const Baselib_Thread_EntryPointFunction entryPoint;
    void* const entryPointArgument;
};
