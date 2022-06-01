//
// Created by VincentZhang on 5/13/2022.
//

#ifndef HUAHUOENGINE_UTILITY_H
#define HUAHUOENGINE_UTILITY_H
#include <cstdlib>
#include "Internal/ArchitectureDetection.h"
#include "Configuration/IntegerDefinitions.h"

#if PLATFORM_WEBGL || (defined(__arm__) && !(defined(__aarch64__) || defined(__arm64__) /*arm64 allows unaligned reads*/))
#define UNITY_NO_UNALIGNED_MEMORY_ACCESS 1
#else
#define UNITY_NO_UNALIGNED_MEMORY_ACCESS 0
#endif

// Check if it's safe to access a type using given alignment
#define IS_SAFE_ALIGNMENT_FOR_TYPE(Alignment, Type) ((Alignment) >= alignof(Type) || !UNITY_NO_UNALIGNED_MEMORY_ACCESS)

// Compare memory by casting to largest possible integer type. ElementSize and Alignment should be compile time constants.
#define COMPARE_FIXED_SIZE_AND_ALIGNMENT(CompareFunc, ElementSize, Alignment, lhs, rhs, elementCount) \
    if (PLATFORM_ARCH_64 && IS_SAFE_ALIGNMENT_FOR_TYPE(Alignment, UInt64) && ((ElementSize) % sizeof(UInt64)) == 0) \
        return CompareFunc(reinterpret_cast<const UInt64*>(lhs), reinterpret_cast<const UInt64*>(rhs), (ElementSize) * (elementCount) / sizeof(UInt64)); \
    else if (IS_SAFE_ALIGNMENT_FOR_TYPE(Alignment, UInt32) && ((ElementSize) % sizeof(UInt32)) == 0) \
        return CompareFunc(reinterpret_cast<const UInt32*>(lhs), reinterpret_cast<const UInt32*>(rhs), (ElementSize) * (elementCount) / sizeof(UInt32)); \
    else if (IS_SAFE_ALIGNMENT_FOR_TYPE(Alignment, UInt16) && ((ElementSize) % sizeof(UInt16)) == 0) \
        return CompareFunc(reinterpret_cast<const UInt16*>(lhs), reinterpret_cast<const UInt16*>(rhs), (ElementSize) * (elementCount) / sizeof(UInt16)); \
    else \
        return CompareFunc(reinterpret_cast<const UInt8*>(lhs), reinterpret_cast<const UInt8*>(rhs), (ElementSize) * (elementCount) / sizeof(UInt8));

template<class T>
inline T* Stride(T* p, size_t offset)
{
    return reinterpret_cast<T*>((char*)p + offset);
}


template<class T>
inline T clamp(const T&t, const T& t0, const T& t1)
{
    if (t < t0)
        return t0;
    else if (t > t1)
        return t1;
    else
        return t;
}

template<>
inline float clamp(const float&t, const float& t0, const float& t1)
{
    if (t < t0)
        return t0;
    else if (t > t1)
        return t1;
    else
        return t;
}

template<class T>
inline T clamp01(const T& t)
{
    if (t < 0)
        return 0;
    else if (t > 1)
        return 1;
    else
        return t;
}

template<>
inline float clamp01<float>(const float& t)
{
    if (t < 0.0F)
        return 0.0F;
    else if (t > 1.0F)
        return 1.0F;
    else
        return t;
}

template<class DataType>
inline bool ArrayEquals(const DataType* lhs, const DataType* rhs, size_t arraySize)
{
    for (size_t i = 0; i < arraySize; i++)
    {
        if (lhs[i] != rhs[i])
            return false;
    }
    return true;
}

template<class DataType>
inline bool MemoryEquals(const DataType& lhs, const DataType& rhs)
{
    COMPARE_FIXED_SIZE_AND_ALIGNMENT(ArrayEquals, sizeof(DataType), alignof(DataType), &lhs, &rhs, 1);
}

#endif //HUAHUOENGINE_UTILITY_H
