#pragma once
#include "BaseClasses/BaseTypes.h"

// There are two versions of TransformChangeSystemMask, a struct and a typedef.
// The struct can be used to check that TransformChangeSystemMask is used everywhere
// it should be, while the typedef is for speed.
#define USE_TRANSFORM_CHANGE_SYSTEM_MASK_STRUCT 0

#if USE_TRANSFORM_CHANGE_SYSTEM_MASK_STRUCT

struct TransformChangeSystemMask
{
    typedef UInt64 ValueType;

    ValueType mask;

    TransformChangeSystemMask() {}
    explicit TransformChangeSystemMask(ValueType m) : mask(m) {}

    TransformChangeSystemMask operator&(TransformChangeSystemMask rhs) const { return TransformChangeSystemMask(mask & rhs.mask); }
    TransformChangeSystemMask operator|(TransformChangeSystemMask rhs) const { return TransformChangeSystemMask(mask | rhs.mask); }
    TransformChangeSystemMask operator~() const { return TransformChangeSystemMask(~mask); }

    TransformChangeSystemMask& operator&=(TransformChangeSystemMask rhs) { mask &= rhs.mask; return *this; }
    TransformChangeSystemMask& operator|=(TransformChangeSystemMask rhs) { mask |= rhs.mask; return *this; }

    bool operator==(TransformChangeSystemMask rhs) const { return (mask == rhs.mask); }
    bool operator!=(TransformChangeSystemMask rhs) const { return (mask != rhs.mask); }

    bool operator==(ValueType rhs) const { return (mask == rhs); }
    bool operator!=(ValueType rhs) const { return (mask != rhs); }
};

inline TransformChangeSystemMask operator*(bool lhs, TransformChangeSystemMask rhs) { return TransformChangeSystemMask(lhs * rhs.mask); }

#else

typedef UInt64 TransformChangeSystemMask;

#endif
