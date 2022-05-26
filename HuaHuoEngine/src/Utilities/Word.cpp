//
// Created by VincentZhang on 4/29/2022.
//

#include "Logging/LogAssert.h"
#include "Word.h"
#include <string>

bool BeginsWith(const char* str, const char* prefix)
{
    for (;;)
    {
        // end of prefix? we're done here (successful match).
        char p = *prefix++;
        if (p == 0)
            return true;

        // mismatch on current? fail.
        if (*str++ != p)
            return false;
    }
}

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

void SplitImpl(std::string& s, char splitChar, std::vector<std::string>& parts, size_t maxSplits)
{
    size_t n = 0, n1 = 0;
    while (true)
    {
        if (maxSplits == 1 || (n1 = s.find(splitChar, n)) == std::string::npos)
        {
            if (n < s.length())
                parts.push_back(s.substr(n));
            break;
        }
        if (n1 > n)
        {
            parts.push_back(s.substr(n, n1 - n));
            maxSplits--;
        }
        n = n1 + 1;
    }
}

namespace core{
    void Split(std::string& s, char splitChar, std::vector<std::string>& parts)
    {
        SplitImpl(s, splitChar, parts, std::string::npos);
    }
}
