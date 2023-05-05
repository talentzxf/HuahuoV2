#ifndef HUAHUOENGINE_ATOMICOPS_H
#define HUAHUOENGINE_ATOMICOPS_H
#include "Memory/MemoryMacros.h" // defines FORCE_INLINE (?!)
#include "Internal/ArchitectureDetection.h"
#include "BaseClasses/BaseTypes.h"
#include <cstdlib>

// AtomicAdd - Returns the new value, after the operation has been performed (as defined by OSAtomicAdd32Barrier)
FORCE_INLINE int AtomicAdd(int volatile *i, int value) {
    return *i += value;
}

// AtomicAdd - Returns the new value, after the operation has been performed (as defined by OSAtomicAdd64Barrier)
FORCE_INLINE SInt64 AtomicAdd64(SInt64 volatile* i, SInt64 value)
{
    return *i += value;
}

// AtomicAdd for size_t
FORCE_INLINE size_t AtomicAdd(size_t volatile* i, size_t value)
{
#if PLATFORM_ARCH_64
    return (size_t)AtomicAdd64((SInt64 volatile*)i, (SInt64)value);
#else
// CompileTimeAssert(sizeof(size_t) == sizeof(int), "size_t must be 4byte in !PLATFORM_ARCH_64");
    return (size_t)AtomicAdd((int volatile*)i, (int)value);
#endif
}

// AtomicIncrement - Returns the new value, after the operation has been performed (as defined by OSAtomicAdd32Barrier)
FORCE_INLINE
int AtomicIncrement(int volatile* i)
{
#if PLATFORM_WIN
    return _InterlockedIncrement((long volatile*)i);
#else
    return AtomicAdd(i, 1);
#endif
}

// AtomicSub - Returns the new value, after the operation has been performed (as defined by OSAtomicSub32Barrier)
FORCE_INLINE
int AtomicSub(int volatile* i, int value)
{
#if PLATFORM_LINUX || PLATFORM_ANDROID || (PLATFORM_WEBGL && SUPPORT_THREADS)
    return __sync_sub_and_fetch(i, value);
#else
    return AtomicAdd(i, -value);
#endif
}

// AtomicDecrement - Returns the new value, after the operation has been performed (as defined by OSAtomicAdd32Barrier)
FORCE_INLINE
int AtomicDecrement(int volatile* i)
{
#if PLATFORM_WIN
    return _InterlockedDecrement((long volatile*)i);
#else
    return AtomicSub(i, 1);
#endif
}

#endif