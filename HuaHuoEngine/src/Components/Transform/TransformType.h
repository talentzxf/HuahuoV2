#pragma once

#include "Utilities/EnumFlags.h"
#include "baselib/include/IntegerDefinitions.h"

/// Uniform transform scales x, y, z in the same amount,
/// NonUniform transform scales x, y, z differently and might contain skew.
/// kOddNegativeScaleTransform means that FrontFace(CCW) should be used (An odd number of scale axes is negative)
enum TransformType : UInt8
{
    kNoScaleTransform = 0,
    kUniformScaleTransform = 1 << 0,
    kNonUniformScaleTransform = 1 << 1,
    kOddNegativeScaleTransform = 1 << 2
};
ENUM_FLAGS(TransformType);

inline bool IsNoScaleTransform(TransformType type) { return type == kNoScaleTransform; }
inline bool IsUniformScaleTransform(TransformType type) { return HasFlag(type, kUniformScaleTransform); }
inline bool IsNonUniformScaleTransform(TransformType type) { return HasFlag(type, kNonUniformScaleTransform); }
inline bool IsOddNegativeScaleTransform(TransformType type) { return HasFlag(type, kOddNegativeScaleTransform); }
