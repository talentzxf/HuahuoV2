#pragma once

#include "Memory/MemoryMacros.h"
#include "baselib/include/IntegerDefinitions.h"
#include "baselib/include/CoreMacros.h"
#include "baselib/include/PlatformEnvironment.h"

#include <algorithm>

FORCE_INLINE void memcpy_constrained_src(void* dst, const void* src, int size, const void* srcFrom, void* srcTo)
{
    UInt8* fromClamped = std::clamp((UInt8*)src, (UInt8*)srcFrom, (UInt8*)srcTo);
    UInt8* toClamped = std::clamp((UInt8*)src + size, (UInt8*)srcFrom, (UInt8*)srcTo);

    int offset = fromClamped - (UInt8*)src;
    size = toClamped - fromClamped;
    MEMCPY((UInt8*)dst + offset, (UInt8*)src + offset, size);
}

FORCE_INLINE void memcpy_constrained_dst(void* dst, const void* src, int size, const void* dstFrom, void* dstTo)
{
    UInt8* fromClamped = std::clamp((UInt8*)dst, (UInt8*)dstFrom, (UInt8*)dstTo);
    UInt8* toClamped = std::clamp((UInt8*)dst + size, (UInt8*)dstFrom, (UInt8*)dstTo);

    int offset = fromClamped - (UInt8*)dst;
    size = toClamped - fromClamped;
    MEMCPY((UInt8*)dst + offset, (UInt8*)src + offset, size);
}
