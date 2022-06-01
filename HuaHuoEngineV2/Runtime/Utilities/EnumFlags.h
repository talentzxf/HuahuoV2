#ifndef ENUMFLAGS_H
#define ENUMFLAGS_H
#include "Logging/LogAssert.h"

// Adds ability to use bit logic operators with enum type T.
// Enum must have appropriate values (e.g. kEnumValue1 = 1 << 1, kEnumValue2 = 1 << 2)!
#define ENUM_FLAGS(T) DETAIL__ENUM_FLAGS(T, )
#define ENUM_FLAGS_AS_MEMBER(T) DETAIL__ENUM_FLAGS(T, friend)

#define DETAIL__ENUM_FLAGS(T, PREFIX_) \
    PREFIX_ inline T operator |(const T left, const T right) { return static_cast<T>(static_cast<unsigned>(left) | static_cast<unsigned>(right)); } \
    PREFIX_ inline T operator &(const T left, const T right) { return static_cast<T>(static_cast<unsigned>(left) & static_cast<unsigned>(right)); } \
    PREFIX_ inline T operator ^(const T left, const T right) { return static_cast<T>(static_cast<unsigned>(left) ^ static_cast<unsigned>(right)); } \
    \
    PREFIX_ inline T operator ~(const T flags) { return static_cast<T>(~static_cast<unsigned>(flags)); } \
    \
    PREFIX_ inline T& operator |=(T& left, const T right) { return left = left | right; } \
    PREFIX_ inline T& operator &=(T& left, const T right) { return left = left & right; } \
    PREFIX_ inline T& operator ^=(T& left, const T right) { return left = left ^ right; } \
    \
    PREFIX_ inline bool HasFlag(const T flags, const T flagToTest) { DebugAssertMsg((static_cast<unsigned>(flagToTest) & (static_cast<unsigned>(flagToTest)-1)) == 0, "More than one flag specified in HasFlag()"); return (static_cast<unsigned>(flags) & static_cast<unsigned>(flagToTest)) != 0; } \
    PREFIX_ inline bool HasAnyFlags(const T flags, const T flagsToTest) { return (static_cast<unsigned>(flags) & static_cast<unsigned>(flagsToTest)) != 0; } \
    PREFIX_ inline bool HasAllFlags(const T flags, const T flagsToTest) { return (static_cast<unsigned>(flags) & static_cast<unsigned>(flagsToTest)) == static_cast<unsigned>(flagsToTest); } \
    PREFIX_ inline T SetFlags(const T flags, const T flagsToSet) { return (flags | flagsToSet); } \
    PREFIX_ inline T ClearFlags(const T flags, const T flagsToClear) { return (flags & ~flagsToClear); } \
    PREFIX_ inline T SetOrClearFlags(const T flags, const T flagsToSetOrClear, bool value) \
    { return value ? SetFlags(flags, flagsToSetOrClear) : ClearFlags(flags, flagsToSetOrClear); } \

// Adds ability to use increment operators with enum type T.
// Enum must have consecutive values!
#define ENUM_INCREMENT(T) \
    inline T& operator++(T& flags) { flags = static_cast<T>(static_cast<int>(flags) + 1); return flags; } \
    inline T operator++(T& flags, int) { T result = flags; ++flags; return result; }

#endif