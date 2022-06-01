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


template<typename T>
static inline std::string _ToString(const char* formatString, T value)
{
    char buf[255];
    sprintf(buf, formatString, value);
    return std::string(buf);
}

std::string IntToString(SInt32 i)
{
    return _ToString("%i", i);
}

namespace core{
    void Split(std::string& s, char splitChar, std::vector<std::string>& parts, std::string::size_type maxSplits)
    {
        SplitImpl(s, splitChar, parts, maxSplits);
    }
}

inline UInt64 StringToIntBase(std::string& s, bool& isNegative)
{
    isNegative = false;
    std::string::iterator it = s.begin();
    while (it != s.end() && IsSpace(*it))
        it++;

    if (it == s.end())
        return 0U;

    // + or - are only allowed at the beginning of the string
    switch (*it)
    {
        case '-': isNegative = true;
        case '+': it++;
    }

    UInt64 value = 0;

    for (; it != s.end(); ++it)
    {
        UInt32 a = *it - '0';

        if (a > 9)
            break;

        value = value * 10 + a;
    }

    return value;
}


template<class T>
T StringToSignedType(const std::string& s)
{
    bool negative = false;
    UInt64 value = StringToIntBase(const_cast<std::string&>(s), negative);

    return negative ? -(T)value : (T)value;
}

SInt32 StringToInt(std::string& s)
{
    return StringToSignedType<SInt32>(s);
}
