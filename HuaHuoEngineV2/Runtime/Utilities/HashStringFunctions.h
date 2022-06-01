//
// Created by VincentZhang on 4/9/2022.
//

#ifndef HUAHUOENGINE_HASHSTRINGFUNCTIONS_H
#define HUAHUOENGINE_HASHSTRINGFUNCTIONS_H
#include <cstdio>
#include "BaseClasses/BaseTypes.h"
#include "HashFunctions.h"

// --------------------------------------------------------------------------
// String hashing. We have distinction on two use cases:
//
// - "short strings": single words/identifiers, i.e. typically less than 10-20 bytes.
//   For these, a very simple hash algorithm is usually the best.
//   Current implementation uses 32 bit FNV-1a.
// - "general strings": expected to be arbitrary size, possibly long.
//   For these, we just call ComputeHash* functions from HashFunctions.h; they are much better
//   performance than FNV as string size grows past 10-20 bytes.
//
// In this file:
// - ComputeShortStringHash32 and ShortStringHash32Function
// - ComputeStringHash32 and StringHash32Function
// - ComputeStringHash128
// - ComputeFNV1aHash and ComputeFNV1aModifiedHash
//
// For other hash functions (general data, integers, crypto), see HashFunctions.h


// 32 bit FNV-1a hash on C-style strings https://en.wikipedia.org/wiki/Fowler-Noll-Vo_hash_function
static inline UInt32 ComputeFNV1aHash(const char* key)
{
    const UInt32 fnvPrime = 16777619U;
    UInt32 hash = 2166136261U;
    const UInt8* ptr = (const UInt8*)key;
    while (*ptr)
    {
        hash = (hash ^ *ptr++) * fnvPrime;
    }
    return hash;
}

// 32 bit FNV-1a hash on data blob https://en.wikipedia.org/wiki/Fowler-Noll-Vo_hash_function
static inline UInt32 ComputeFNV1aHash(const void* data, size_t dataSize)
{
    const UInt32 fnvPrime = 16777619U;
    UInt32 hash = 2166136261U;
    const UInt8* ptr = (const UInt8*)data;
    const UInt8* end = ptr + dataSize;
    while (ptr < end)
    {
        hash = (hash ^ *ptr++) * fnvPrime;
    }
    return hash;
}

// 32 bit FNV-1a, modified for better avalanche behavior http://papa.bretmulvey.com/post/124027987928/hash-functions
static inline UInt32 ComputeFNV1aModifiedHash(const char* key)
{
    UInt32 hash = ComputeFNV1aHash(key);
    hash += hash << 13;
    hash ^= hash >> 7;
    hash += hash << 3;
    hash ^= hash >> 17;
    hash += hash << 5;
    return hash;
}

// --------------------------------------------------------------------------
// 32 bit hashes for "short" strings.
//
// Computes 32 bit hash value for a string. Best on short strings (expected size less than 10-20 bytes).
// For arbitrary strings that can be long, ComputeStringHash32 will give much better performance.
// If it's a C-style string, and you know the length, then pass that in too.
// Implementation: FNV-1a.

inline UInt32 ComputeShortStringHash32(const char* str, size_t strLen)
{
    return ComputeFNV1aHash(str, strLen);
}

inline UInt32 ComputeShortStringHash32(const char* str)
{
    return ComputeFNV1aHash(str);
}

//template<typename TString>
//inline UInt32 ComputeShortStringHash32(const TString& str)
//{
//    return ComputeShortStringHash32(StringTraits::AsConstTChars(str), StringTraits::GetLength(str));
//}

// STL-style 32 bit hash function for short strings.
template<typename T>
struct ShortStringHash32Function
{
    size_t operator()(const T& str) const
    {
        return ComputeShortStringHash32(str);
    }
};


//// --------------------------------------------------------------------------
//// 32 bit hashes
////
//// Computes 32 bit hash value for a string.
//// If it's a C-style string, and you know the length, then pass that in too.
//// Implementation: ComputeHash32 (xxHash32).
//
//inline UInt32 ComputeStringHash32(const char* str, size_t strLen)
//{
//    return ComputeHash32(str, strLen);
//}
//
//inline UInt32 ComputeStringHash32(const char* str)
//{
//    size_t strLen = strlen(str);
//    return ComputeStringHash32(str, strLen);
//}
//
//template<typename TString>
//inline UInt32 ComputeStringHash32(const TString& str)
//{
//    return ComputeStringHash32(StringTraits::AsConstTChars(str), StringTraits::GetLength(str));
//}
//
//// STL-style 32 bit hash function for strings.
//template<typename T>
//struct StringHash32Function
//{
//    size_t operator()(const T& str) const
//    {
//        return ComputeStringHash32(str);
//    }
//};
//
//
//// --------------------------------------------------------------------------
//// 128 bit hashes
////
//// Computes 128 bit hash value for a string.
//// If it's a C-style string, and you know the length, then pass that in too.
//// Implementation: ComputeHash128 (SpookyV2).
//
//inline Hash128 ComputeStringHash128(const char* str, size_t strLen)
//{
//    return ComputeHash128(str, strLen);
//}
//
//inline Hash128 ComputeStringHash128(const char* str)
//{
//    size_t strLen = strlen(str);
//    return ComputeStringHash128(str, strLen);
//}
//
//template<typename TString>
//inline Hash128 ComputeStringHash128(const TString& str)
//{
//    return ComputeStringHash128(StringTraits::AsConstTChars(str), StringTraits::GetLength(str));
//}
//
//// Variants for incremental hash building (Hash128 is in+out parameter)
//inline void ComputeStringHash128(const char* str, size_t strLen, Hash128& hashInOut)
//{
//    ComputeHash128(str, strLen, hashInOut);
//}
//
//inline void ComputeStringHash128(const char* str, Hash128& hashInOut)
//{
//    size_t strLen = strlen(str);
//    ComputeStringHash128(str, strLen, hashInOut);
//}
//
//template<typename TString>
//inline void ComputeStringHash128(const TString& str, Hash128& hashInOut)
//{
//    ComputeStringHash128(StringTraits::AsConstTChars(str), StringTraits::GetLength(str), hashInOut);
//}

#endif //HUAHUOENGINE_HASHSTRINGFUNCTIONS_H
