//
// Created by VincentZhang on 6/4/2022.
//

#ifndef HUAHUOENGINEV2_STRINGCOMPARISON_H
#define HUAHUOENGINEV2_STRINGCOMPARISON_H

#include "Word.h"
#include <functional>


// String comparison (equality, ordering, case-insensitive ordering) helpers as STL-compatible functors.


template<typename TString>
struct compare_tstring : std::binary_function<const TString&, const TString&, bool>
{
    bool operator()(const TString& lhs, const TString& rhs) const { return lhs.compare(rhs) < 0; }
};

template<>
struct compare_tstring<const char*> : std::binary_function<const char*, const char*, bool>
{
    bool operator()(const char* lhs, const char* rhs) const { return StrCmp(lhs, rhs) < 0; }
};

template<typename TString>
struct compare_tstring_insensitive : std::binary_function<const TString&, const TString&, bool>
{
    bool operator()(const TString& lhs, const TString& rhs) const { return lhs.compare(rhs, kComparisonIgnoreCase) < 0; }
};

template<>
struct compare_tstring_insensitive<const char*> : std::binary_function<const char*, const char*, bool>
{
    bool operator()(const char* lhs, const char* rhs) const { return StrICmp(lhs, rhs) < 0; }
};

template<>
struct compare_tstring_insensitive<std::string> : std::binary_function<std::string, std::string, bool>
{
    bool operator()(std::string lhs, std::string rhs) const { return StrICmp(lhs.c_str(), rhs.c_str()) < 0; }
};

typedef compare_tstring<const char*> smaller_cstring;

typedef compare_tstring_insensitive<const char*> compare_cstring_insensitive;
typedef compare_tstring_insensitive<std::string> compare_string_insensitive;


template<typename TString>
struct smaller_tstring_pair : std::binary_function<const std::pair<TString, TString>&, const std::pair<TString, TString>&, bool>
{
    bool operator()(const std::pair<TString, TString>& lhs, const std::pair<TString, TString>& rhs) const
    {
        int first = StrCmp(lhs.first, rhs.first);
        if (first != 0)
            return first < 0;
        else
            return StrCmp(lhs.second, rhs.second) < 0;
    }
};

typedef smaller_tstring_pair<const char*> smaller_cstring_pair;

#endif //HUAHUOENGINEV2_STRINGCOMPARISON_H
