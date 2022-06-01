#include "CRC32.h"

#include "Logging/LogAssert.h"
#include "Utilities/RegisterRuntimeInitializeAndCleanup.h"


inline UInt8 ReflectBits8(UInt8 x)
{
    return ((x * 0x0802LU & 0x22110LU) | (x * 0x8020LU & 0x88440LU)) * 0x10101LU >> 16;
}

inline UInt32 ReflectBits32(UInt32 x)
{
    x = ((x >> 1) & 0x55555555) | ((x << 1) & 0xaaaaaaaa);
    x = ((x >> 2) & 0x33333333) | ((x << 2) & 0xcccccccc);
    x = ((x >> 4) & 0x0f0f0f0f) | ((x << 4) & 0xf0f0f0f0);
    x = ((x >> 8) & 0x00ff00ff) | ((x << 8) & 0xff00ff00);
    x = ((x >> 16) & 0x0000ffff) | ((x << 16) & 0xffff0000);
    return x;
}

UInt32 crc32::s_Table[256] = { 0 };
bool crc32::s_Initialized = false;


void crc32::InitializeTable()
{
    const UInt32 poly = 0x04C11DB7;

    // factor-out constants to avoid recalculation
    UInt32 const fast_hi_bit = 1ul << 31;
    UInt8 const  byte_hi_bit = 1u << 7;

    // loop over every possible dividend value
    unsigned char  dividend = 0;
    do
    {
        UInt32  remainder = 0;

        // go through all the dividend's bits
        for (unsigned char mask = byte_hi_bit; mask; mask >>= 1)
        {
            // check if divisor fits
            if (dividend & mask)
            {
                remainder ^= fast_hi_bit;
            }

            // do polynomial division
            if (remainder & fast_hi_bit)
            {
                remainder <<= 1;
                remainder ^= poly;
            }
            else
            {
                remainder <<= 1;
            }
        }

        s_Table[ReflectBits8(dividend)] = ReflectBits32(remainder);
    }
    while (++dividend);

    s_Initialized = true;
}

void crc32::process_block(void const *  bytes_begin, void const *  bytes_end)
{
    Assert(s_Initialized);

    for (unsigned char const * p = static_cast<unsigned char const *>(bytes_begin); p < bytes_end; ++p)
    {
        unsigned char const  byte_index = index(rem, *p);
        rem = shift(rem);
        rem ^= s_Table[byte_index];
    }
}

void crc32::process_block_skip2(void const *  bytes_begin, void const *  bytes_end)
{
    Assert(s_Initialized);
    unsigned char const * p;

#if PLATFORM_ARCH_BIG_ENDIAN
    p = static_cast<unsigned char const *>(bytes_begin) + 1;
#else
    p = static_cast<unsigned char const *>(bytes_begin);
#endif

    for (; p < bytes_end; p += 2)
    {
        unsigned char const  byte_index = index(rem, *p);
        rem = shift(rem);
        rem ^= s_Table[byte_index];
    }
}

void InitializeCRC32(void*)
{
    crc32::InitializeTable();
}

// crc32 table is needed by InitializeGenericAnimationBindingCache which is initialized with order = 0
static RegisterRuntimeInitializeAndCleanup s_RegisterBindingCache(InitializeCRC32, NULL, -1);
