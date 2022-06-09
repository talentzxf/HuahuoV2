#pragma once
#include "Configuration/IntegerDefinitions.h"
#include "Utilities/UnionTuple.h"

inline void SwapEndianBytes(char&) {}
inline void SwapEndianBytes(unsigned char&) {}
inline void SwapEndianBytes(bool&) {}
inline void SwapEndianBytes(signed char&) {}

inline void SwapEndianBytes(UInt16& i) { i = static_cast<UInt16>((i << 8) | (i >> 8)); }

inline void SwapEndianBytes(SInt16& i) { SwapEndianBytes(reinterpret_cast<UInt16&>(i)); }

inline void SwapEndianBytes(UInt32& i)   { i = (i >> 24) | ((i >> 8) & 0x0000ff00) | ((i << 8) & 0x00ff0000) | (i << 24); }

inline void SwapEndianBytes(SInt32& i)   { SwapEndianBytes(reinterpret_cast<UInt32&>(i)); }
inline void SwapEndianBytes(float& i)
{
    UInt32 asUint = bit_cast<UInt32>(i);

    i = bit_cast<float>(asUint);
}

inline void SwapEndianBytes(UInt64& i)
{
    UInt32* p = reinterpret_cast<UInt32*>(&i);
    UInt32 u = (p[0] >> 24) | (p[0] << 24) | ((p[0] & 0x00ff0000) >> 8) | ((p[0] & 0x0000ff00) << 8);
    p[0] = (p[1] >> 24) | (p[1] << 24) | ((p[1] & 0x00ff0000) >> 8) | ((p[1] & 0x0000ff00) << 8);
    p[1] = u;
}

inline void SwapEndianBytes(SInt64& i) { SwapEndianBytes(reinterpret_cast<UInt64&>(i)); }
inline void SwapEndianBytes(double& i)
{
    UInt64 asUint = bit_cast<UInt64>(i);
    SwapEndianBytes(asUint);
    i = bit_cast<double>(asUint);
}

#if PLATFORM_ARCH_64 && PLATFORM_OSX
inline void SwapEndianBytes(size_t &i)   { SwapEndianBytes(reinterpret_cast<UInt64&>(i)); }
#endif

inline bool IsBigEndian()
{
    #if PLATFORM_ARCH_BIG_ENDIAN
    return true;
    #else
    return false;
    #endif
}

inline bool IsLittleEndian()
{
    return !IsBigEndian();
}

#if PLATFORM_ARCH_BIG_ENDIAN
#define SwapEndianBytesBigToNative(x)
#define SwapEndianBytesLittleToNative(x) SwapEndianBytes(x)
#define SwapEndianBytesNativeToLittle(x) SwapEndianBytes(x)
#elif PLATFORM_ARCH_LITTLE_ENDIAN
#define SwapEndianBytesBigToNative(x) SwapEndianBytes(x)
#define SwapEndianBytesLittleToNative(x)
#define SwapEndianBytesNativeToLittle(x)
#endif
