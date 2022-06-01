#pragma once

#include <stdint.h>

typedef int8_t   SInt8;
typedef int16_t  SInt16;
typedef int32_t  SInt32;
typedef int64_t  SInt64;
typedef uint8_t  UInt8;
typedef uint16_t UInt16;
typedef uint32_t UInt32;
typedef uint64_t UInt64;

#if PLATFORM_ARCH_64
typedef UInt64   UIntPtr;
typedef SInt64   SIntPtr;
#else
typedef UInt32   UIntPtr;
typedef SInt32   SIntPtr;
#endif // PLATFORM_ARCH_64
