//
// Created by VincentZhang on 4/30/2022.
//

#ifndef HUAHUOENGINE_WORD_H
#define HUAHUOENGINE_WORD_H
#include "StringTraits.h"
#include "Memory/MemoryMacros.h"
#include <vector>
#include <cstring>

bool BeginsWith(const char* str, const char* prefix);

enum ComparisonType
{
    kComparisonCaseSensitive,
    kComparisonIgnoreCase
};

// $TODO#scobi(9-dec-14) see comment on ToLower/Upper. these use those functions and have similarly misleading names.
int StrNICmp(const char* str1, const char* str2, size_t count);
int StrICmp(const char* str1, const char* str2);
int StrNCmp(const char* str1, const char* str2, size_t count);
int StrCmp(const char* str1, const char* str2);

// $TODO#scobi(9-dec-14) add StrEquals and StrIEquals that overload where lengths available on both sides and early-out on mismatch

template<typename TString1, typename TString2>
inline bool StrIEquals(const TString1& str1, const TString2& str2)
{
    using namespace StringTraits;
    if ((HasConstantTimeLength<TString1>::value && HasConstantTimeLength<TString2>::value) &&
        (GetLength(str1) != GetLength(str2)))
        return false;
    return StrICmp(AsConstTChars(str1), AsConstTChars(str2)) == 0;
}


template<typename TString1, typename TString2>
inline int StrICmp(const TString1& str1, const TString2& str2)
{
    using namespace StringTraits;
    return StrICmp(AsConstTChars(str1), AsConstTChars(str2));
}

template<typename TString>
inline int StrICmp(std::string str1, const TString& str2)
{
    return str1.compare(str2, kComparisonIgnoreCase);
}


// $TODO#scobi(9-dec-14) fix name - this only works on A-Z and is not intended for even a latin char set, yet it has the same name as a std function and no guidance given on when it should be used.
inline char ToLower(char c) { return (c >= 'A' && c <= 'Z') ? (c + 'a' - 'A') : c; }
inline char ToUpper(char c) { return (c >= 'a' && c <= 'z') ? (c - ('a' - 'A')) : c; }
inline unsigned char ToLower(unsigned char c) { return (c >= 'A' && c <= 'Z') ? (c + 'a' - 'A') : c; }
inline unsigned char ToUpper(unsigned char c) { return (c >= 'a' && c <= 'z') ? (c - ('a' - 'A')) : c; }

// Converts string to integer representation. First, any whitespace characters are ignored.
// Then, optional '+' or '-' sign character determines the sign of the resulting number.
// Then, one or more decimal digits are parsed until either non-digit character or the
// end of the string is found. Any non-digit characters after the end of the number are ignored.
SInt32  StringToInt(std::string s);
SInt64  StringToSInt64(std::string s);

template<typename TString>
TString ToLower(const TString& input)
{
    TString s = input;
    for (typename TString::iterator i = s.begin(); i != s.end(); i++)
        *i = ToLower(*i);
    return s;
}

template<typename TString1, typename TString2>
inline bool StrEquals(const TString1& str1, const TString2& str2)
{
    using namespace StringTraits;
    if ((HasConstantTimeLength<TString1>::value && HasConstantTimeLength<TString2>::value) &&
        (GetLength(str1) != GetLength(str2)))
        return false;
    return StrCmp(AsConstTChars(str1), AsConstTChars(str2)) == 0;
}

template<typename TIterator, typename TString>
TIterator FindStringInRange(TIterator begin, TIterator end, TString str, bool ignoreCase)
{
    if (ignoreCase)
    {
        while (begin != end && !StrIEquals(*begin, str))
            ++begin;
    }
    else
    {
        while (begin != end && !StrEquals(*begin, str))
            ++begin;
    }

    return begin;
}

namespace core {
    // Split str using splitChar and store them into parts. f.ex:
    //    Split("t,es,t", ',', parts)    -> parts == "t","es","t"
    //    Split("t,es,t", ',', parts,2)  -> parts == "t","est"
    // REMARKS: Split("1,,,,,,,,,,2", '<>', parts)  -> parts == "1","2"
    void Split(std::string& str, char splitChar, std::vector<std::string> &parts, std::string::size_type maxSplits = std::string::npos);
}

inline bool IsDigit(char c) { return c >= '0' && c <= '9'; }
inline bool IsAlpha(char c) { return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'); }
inline bool IsSpace(char c) { return c == '\t' || c == '\n' || c == '\v' || c == '\f' || c == '\r' || c == ' '; }
inline bool IsTabSpace(char c) { return c == '\t' || c == ' '; }
inline bool IsAlphaNumeric(char c) { return IsDigit(c) || IsAlpha(c); }
inline bool IsHex(char c) { return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F'); }

std::string IntToString(SInt32 i);

// Converts string to integer representation. First, any whitespace characters are ignored.
// Then, optional '+' or '-' sign character determines the sign of the resulting number.
// Then, one or more decimal digits are parsed until either non-digit character or the
// end of the string is found. Any non-digit characters after the end of the number are ignored.
SInt32  StringToInt(std::string& s);

inline char* StrDup(MemLabelRef label, const char *str)
{
    size_t size = strlen(str) + 1;
    char* result = (char*)HUAHUO_MALLOC(label, size);
    if (OPTIMIZER_UNLIKELY(result == NULL))
        return NULL;
    return (char*)memcpy(result, str, size);
}
#endif //HUAHUOENGINE_WORD_H
