#ifndef HUAHUOENGINE_ATOMICOPS_H
#define HUAHUOENGINE_ATOMICOPS_H
#include "Internal/ArchitectureDetection.h"
#include "BaseClasses/BaseTypes.h"
#include <cstdlib>

// AtomicAdd - Returns the new value, after the operation has been performed (as defined by OSAtomicAdd32Barrier)
int AtomicAdd(int volatile *i, int value) {
    return *i += value;
}

// AtomicAdd - Returns the new value, after the operation has been performed (as defined by OSAtomicAdd64Barrier)
SInt64 AtomicAdd64(SInt64 volatile* i, SInt64 value)
{
    return *i += value;
}

// AtomicAdd for size_t
size_t AtomicAdd(size_t volatile* i, size_t value)
{
#if PLATFORM_ARCH_64
    return (size_t)AtomicAdd64((SInt64 volatile*)i, (SInt64)value);
#else
// CompileTimeAssert(sizeof(size_t) == sizeof(int), "size_t must be 4byte in !PLATFORM_ARCH_64");
    return (size_t)AtomicAdd((int volatile*)i, (int)value);
#endif
}

#endif