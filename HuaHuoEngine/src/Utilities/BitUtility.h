#pragma once

#include <limits.h>
#include "baselib/include/IntegerDefinitions.h"
#include "Logging/LogAssert.h"
#if _MSC_VER

#include "Runtime/Misc/CPUInfo.h"

#include <intrin.h>
#pragma intrinsic(_BitScanReverse)
#if PLATFORM_ARCH_64
#pragma intrinsic(_BitScanReverse64)
#endif

#endif

// index of the most significant bit in the mask
// Returns -1 if no bits are set

const signed char gHighestBitLut[] = {-1, 0, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3};

inline int HighestBit(UInt32 mask)
{
    if (mask == 0)
        return -1;
#if _MSC_VER
    unsigned long res = 0;
    _BitScanReverse(&res, mask);
    return (int)res;
#elif __clang__ || __GNUC__ || __GCC__
    return 31 - __builtin_clz(mask);
#else
    int base = 0;

    if (mask & 0xffff0000)
    {
        base = 16;
        mask >>= 16;
    }
    if (mask & 0x0000ff00)
    {
        base += 8;
        mask >>= 8;
    }
    if (mask & 0x000000f0)
    {
        base += 4;
        mask >>= 4;
    }

    return base + gHighestBitLut[mask];
#endif
}

inline int HighestBit64(UInt64 mask)
{
    if (mask == 0)
        return -1;
#if _MSC_VER && PLATFORM_ARCH_64
    unsigned long res = 0;
    _BitScanReverse64(&res, mask);
    return (int)res;
#elif (__clang__ || __GNUC__ || __GCC__) && PLATFORM_ARCH_64
    return 63 - __builtin_clzll(mask);
#else
    if (mask & 0xffffffff00000000ULL)
    {
        return 32 + HighestBit((UInt32)(mask >> 32));
    }
    return HighestBit((UInt32)mask);
#endif
}

// index of the least significant bit in the mask
// Returns -1 if no bits are set
const signed char gLowestBitLut[] = {-1, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0};
inline int LowestBit8(UInt8 mask)
{
    if (mask == 0)
        return -1;
#if _MSC_VER
    unsigned long res = 0;
    _BitScanForward(&res, mask);
    return (int)res;
#elif (__clang__ || __GNUC__ || __GCC__)
    return __builtin_ctz(mask);
#else
    int base = 0;
    if ((mask & 0x000f) == 0)
    {
        base += 4;
        mask >>= 4;
    }
    return base + gLowestBitLut[mask & 15];
#endif
}

inline int LowestBit(UInt32 mask)
{
    if (mask == 0)
        return -1;
#if _MSC_VER
    unsigned long res = 0;
    _BitScanForward(&res, mask);
    return (int)res;
#elif (__clang__ || __GNUC__ || __GCC__)
    return __builtin_ctz(mask);
#else
    int base = 0;

    if ((mask & 0xffff) == 0)
    {
        base = 16;
        mask >>= 16;
    }
    if ((mask & 0x00ff) == 0)
    {
        base += 8;
        mask >>= 8;
    }
    return base + LowestBit8(mask);
#endif
}

inline int LowestBit64(UInt64 mask)
{
    if (mask == 0)
        return -1;
#if _MSC_VER && PLATFORM_ARCH_64
    unsigned long res = 0;
    _BitScanForward64(&res, mask);
    return (int)res;
#elif (__clang__ || __GNUC__ || __GCC__)
    return __builtin_ctzl(mask);
#else
    int base = 0;
    if ((mask & 0xffffffff) == 0)
    {
        base += 32;
        mask >>= 32;
    }
    return base + LowestBit((UInt32)mask);
#endif
}

// can be optimized later
inline int AnyBitFromMask(UInt32 mask)
{
    return HighestBit(mask);
}

// index of the first consecutiveBitCount enabled bits
// -1 if not available
inline int LowestBitConsecutive(UInt32 bitMask, int consecutiveBitCount)
{
    UInt32 tempBitMask = bitMask;
    int i;
    for (i = 1; i < consecutiveBitCount; i++)
        tempBitMask &= bitMask >> i;

    if (!tempBitMask)
        return -1;
    else
        return LowestBit(tempBitMask);
}

/*int LowestBitConsecutive ( u_int value, u_int consecutiveBitCount )
{
    u_int mask = (1 << consecutiveBitCount) - 1;
    u_int notValue = value ^ 0xffffffff;
    u_int workingMask = mask;
    u_int prevMask = 0;
    int match = notValue & workingMask;
    u_int shift = 1;
    while ( (match != 0) && (prevMask < workingMask) )
    {
        shift = 2*u_int(match & -match);
        prevMask = workingMask;
        workingMask = mask * shift;
        match = notValue & workingMask;
    }
    if ( prevMask < workingMask )
    {
        return LowestBit( shift );
    }
    else
    {
        return -1;
    }
}*/


// number of set bits (population count) in a 32 bit mask
inline int BitsInMask32(UInt32 v)
{
    // From http://www-graphics.stanford.edu/~seander/bithacks.html#CountBitsSetParallel
    // This variant about 30% faster on 360 than what was here before.
    v = v - ((v >> 1) & 0x55555555);
    v = (v & 0x33333333) + ((v >> 2) & 0x33333333);
    return (((v + (v >> 4)) & 0xF0F0F0F) * 0x1010101) >> 24;
}

// number of set bits (population count) in a 64 bit mask
inline int BitsInMask64(UInt64 v)
{
    // From http://www-graphics.stanford.edu/~seander/bithacks.html#CountBitsSetParallel
    v = v - ((v >> 1) & (UInt64) ~(UInt64)0 / 3);
    v = (v & (UInt64) ~(UInt64)0 / 15 * 3) + ((v >> 2) & (UInt64) ~(UInt64)0 / 15 * 3);
    v = (v + (v >> 4)) & (UInt64) ~(UInt64)0 / 255 * 15;
    return (UInt64)(v * ((UInt64) ~(UInt64)0 / 255)) >> (sizeof(UInt64) - 1) * CHAR_BIT;
}

// overloaded versions for 8, 16, 32 and 64 bits
inline int BitsInMask(UInt8 v) { return BitsInMask32(v); }  // todo: optimize
inline int BitsInMask(UInt16 v) { return BitsInMask32(v); }
inline int BitsInMask(UInt32 v) { return BitsInMask32(v); }
inline int BitsInMask(UInt64 v) { return BitsInMask64(v); }

// Number of set bits (population count) in an array of known size.
// Using Robert Harley and David Seal's algorithm from Hacker's Delight,
// variant that does 4 words in a loop iteration.
// http://www.hackersdelight.org/revisions.pdf
// http://www.hackersdelight.org/HDcode/newCode/pop_arrayHS.cc
template<typename WordT, int WordCount>
inline int BitsInArrayHarleySeal(const WordT* data)
{
#define HarleySealCSAStep(h, l, a, b, c) {\
WordT u = a ^ b; \
h = (a & b) | (u & c); l = u ^ c; \
}
    WordT ones, twos, twosA, twosB, fours;

    int i = 0;
    int tot = 0;
    twos = ones = 0;
    for (; i <= WordCount - 4; i = i + 4)
    {
        HarleySealCSAStep(twosA, ones, ones, data[i], data[i + 1])
        HarleySealCSAStep(twosB, ones, ones, data[i + 2], data[i + 3])
        HarleySealCSAStep(fours, twos, twos, twosA, twosB)
        tot = tot + BitsInMask(fours);
    }
    tot = 4 * tot + 2 * BitsInMask(twos) + BitsInMask(ones);

    for (; i < WordCount; i++) // Simply add in the last
        tot = tot + BitsInMask(data[i]); // 0 to 3 elements.

    return tot;
#undef HarleySealCSAStep
}

template<typename WordT, int WordCount>
inline int BitsInArray(const WordT* data)
{
    return BitsInArrayHarleySeal<WordT, WordCount>(data);
}

template<>
inline int BitsInArray<UInt64, 7>(const UInt64* data)
{
    int count = 0;
#if _MSC_VER && defined(_M_X64)
    if (CPUInfo::HasAdvancedBitManipulationSupport())
        for (int i = 0; i < 7; i++)
            count += __popcnt64(data[i]);
    else
        count = BitsInArrayHarleySeal<UInt64, 7>(data);
#elif (__clang__ || __GNUC__ || __GCC__) && PLATFORM_ARCH_64
    for (int i = 0; i < 7; i++)
        count += __builtin_popcountll(data[i]);
#else
    count = BitsInArrayHarleySeal<UInt64, 7>(data);
#endif
    return count;
}

template<>
inline int BitsInArray<UInt32, 14>(const UInt32* data)
{
    int count = 0;
#if _MSC_VER && (defined(_M_IX86) || defined(_M_X64))
    if (CPUInfo::HasAdvancedBitManipulationSupport())
        for (int i = 0; i < 14; i++)
            count += __popcnt(data[i]);
    else
        count = BitsInArrayHarleySeal<UInt32, 14>(data);
#elif (__clang__ || __GNUC__ || __GCC__)
    for (int i = 0; i < 14; i++)
        count += __builtin_popcount(data[i]);
#else
    count = BitsInArrayHarleySeal<UInt32, 14>(data);
#endif
    return count;
}

// reverse bit order
inline void ReverseBits(UInt32& mask)
{
    mask = ((mask >>  1) & 0x55555555) | ((mask <<  1) & 0xaaaaaaaa);
    mask = ((mask >>  2) & 0x33333333) | ((mask <<  2) & 0xcccccccc);
    mask = ((mask >>  4) & 0x0f0f0f0f) | ((mask <<  4) & 0xf0f0f0f0);
    mask = ((mask >>  8) & 0x00ff00ff) | ((mask <<  8) & 0xff00ff00);
    mask = ((mask >> 16) & 0x0000ffff) | ((mask << 16) & 0xffff0000);
}

inline UInt32 Rotate32(UInt32 x, int k)
{
    return (x << k) | (x >> (32 - k));
}

inline UInt64 Rotate64(UInt64 x, int k)
{
    return (x << k) | (x >> (64 - k));
}

// are two integers multiple together
template<typename T>
inline bool AreIntegersMultiple(T a, T b)
{
    if ((0 == a) || (0 == b))
        return false;                                   // if at least one integer is 0, consider false ( avoid div by 0 of the following modulo )
    return (0 == (std::max(a, b) % std::min(a, b)));
}

// is value a power-of-two
template<typename T>
inline bool IsPowerOfTwo(T mask)
{
    return (mask & (mask - 1)) == 0;
}

// return the next power-of-two of a 32bit number
inline UInt32 NextPowerOfTwo(UInt32 v)
{
    v -= 1;
    v |= v >> 16;
    v |= v >> 8;
    v |= v >> 4;
    v |= v >> 2;
    v |= v >> 1;
    return v + 1;
}

// return the next power-of-two of a 64bit number
inline UInt64 NextPowerOfTwo64(UInt64 v)
{
    v -= 1;
    v |= v >> 32;
    v |= v >> 16;
    v |= v >> 8;
    v |= v >> 4;
    v |= v >> 2;
    v |= v >> 1;
    return v + 1;
}

// return the closest power-of-two of a 32bit number
inline UInt32 ClosestPowerOfTwo(UInt32 v)
{
    UInt32 nextPower = NextPowerOfTwo(v);
    UInt32 prevPower = nextPower >> 1;
    if (v - prevPower < nextPower - v)
        return prevPower;
    else
        return nextPower;
}

inline UInt32 ToggleBit(UInt32 bitfield, int index)
{
    Assert(index >= 0 && index < 32);
    return bitfield ^ (1 << index);
}

inline UInt64 ToggleBit64(UInt64 bitfield, int index)
{
    Assert(index >= 0 && index < 64);
    return bitfield ^ (1ull << index);
}

// Template argument must be a power of 2
template<int n>
struct StaticLog2
{
    static const int value = StaticLog2<n / 2>::value + 1;
};

template<>
struct StaticLog2<1>
{
    static const int value = 0;
};

template<int n>
struct StaticPow2
{
    static const size_t value = 2 * StaticPow2<n - 1>::value;
};
template<>
struct StaticPow2<0>
{
    static const size_t value = 1;
};
