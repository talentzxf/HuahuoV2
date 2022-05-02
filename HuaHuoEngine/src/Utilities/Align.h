#pragma once

#include "BitUtility.h"
#include <cstdlib>


template<typename T, typename V>
inline T AlignToPowerOfTwo(T value, V alignment)
{
    Assert(IsPowerOfTwo(alignment));
    return (value + (alignment - 1)) & ~(alignment - 1);
}

template<typename T, typename V>
inline bool IsAlignedToPowerOfTwo(T value, V alignment)
{
    return ((value & (alignment - 1)) == 0);
}

inline bool IsPtrAligned(const void* value, size_t alignment)
{
    return (reinterpret_cast<uintptr_t>(value) & (alignment - 1)) == 0;
}

inline bool IsPtr16ByteAligned(const void* p)
{
    return IsPtrAligned(p, 16);
}

// Rounds up a number to the next multiple of 4.
// if /size/ is already a multiple of 4, then it will return /size/.
inline UInt32 Align4(UInt32 size)
{
    return AlignToPowerOfTwo<UInt32>(size, 4);
}

// A macro version of Align4. This is used by
#define ALIGN_4(size) ((size + 3) & ~3U)

// Returns the remainder of Aligning a number by 4.
inline UInt32 Align4LeftOver(UInt32 size)
{
    return Align4(size) - size;
}
