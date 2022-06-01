#pragma once

#include "Configuration/IntegerDefinitions.h"
#include <string>
#include <cstring>

class crc32
{
public:
    explicit    crc32() : rem(0xFFFFFFFF) {}

    void process_block(void const *bytes_begin, void const *bytes_end);
    void process_block_skip2(void const *  bytes_begin, void const *  bytes_end);

    inline void     process_bytes(void const *buffer, std::size_t byte_count);
    inline void     process_bytes_skip2(void const *   buffer, std::size_t  byte_count);

    inline UInt32  checksum() const { return rem ^ 0xFFFFFFFF; }

    static  void  InitializeTable();

private:
    static UInt32 s_Table[256];
    static bool s_Initialized;

    static  unsigned char  index(UInt32 rem, unsigned char x) { return static_cast<unsigned char>(x ^ rem); }
    static  UInt32  shift(UInt32 rem) { return rem >> 8; }

    UInt32  rem;
};


inline void crc32::process_bytes(void const *   buffer, std::size_t  byte_count)
{
    unsigned char const * const  b = static_cast<unsigned char const *>(buffer);
    process_block(b, b + byte_count);
}

inline void crc32::process_bytes_skip2(void const *   buffer, std::size_t  byte_count)
{
    unsigned char const * const  b = static_cast<unsigned char const *>(buffer);
    process_block_skip2(b, b + byte_count);
}

static inline int ComputeCRC32(std::string const& string)
{
    crc32 result;
    result.process_bytes(string.c_str(), string.size());
    return result.checksum();
}

//static inline int ComputeCRC32(core::string_ref string)
//{
//    crc32 result;
//    result.process_bytes(string.data(), string.size());
//    return result.checksum();
//}

static inline int ComputeCRC32(char const* string)
{
    crc32 result;
    result.process_bytes(string, strlen(string));
    return result.checksum();
}

static inline int ComputeCRC32UTF16Ascii(unsigned short const* string, std::size_t stringLength)
{
    crc32 result;
    result.process_bytes_skip2(string, stringLength * 2);
    return result.checksum();
}
