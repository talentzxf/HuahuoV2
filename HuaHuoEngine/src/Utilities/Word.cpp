//
// Created by VincentZhang on 4/29/2022.
//

#include "Logging/LogAssert.h"
#include "Word.h"
#include <string>

std::string VFormat(const char* format, va_list ap)
{
    va_list zp;
    va_copy(zp, ap);
    char buffer[1024 * 10];
    vsnprintf(buffer, 1024 * 10, format, zp);
    va_end(zp);
    return std::string(buffer);
}

std::string Format(const char* format, ...)
{
    va_list va;
    va_start(va, format);
    std::string formatted = VFormat(format, va);
    va_end(va);
    return formatted;
}

int StrICmp(const char* str1, const char* str2)
{
    for (;;)
    {
        int c1 = (unsigned char)ToLower(*str1++);
        int c2 = (unsigned char)ToLower(*str2++);

        if (c1 == 0 || (c1 != c2))
            return c1 - c2;
    }
}

int StrCmp(const char* str1, const char* str2)
{
    for (;;)
    {
        int c1 = (unsigned char)*str1++;
        int c2 = (unsigned char)*str2++;

        if (c1 == 0 || (c1 != c2))
            return c1 - c2;
    }
}

