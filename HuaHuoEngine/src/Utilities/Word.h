//
// Created by VincentZhang on 4/30/2022.
//

#ifndef HUAHUOENGINE_WORD_H
#define HUAHUOENGINE_WORD_H
#include "StringTraits.h"
#include <vector>

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
    void Split(std::string& str, char splitChar, std::vector<std::string> &parts);
}
#endif //HUAHUOENGINE_WORD_H
