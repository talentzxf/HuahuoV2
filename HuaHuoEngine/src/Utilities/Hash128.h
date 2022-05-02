#pragma once

#include <cstdio>
#include <cstdlib>
#include "baselib/include/IntegerDefinitions.h"
#include "Serialize/SerializeUtility.h"
#include <string>
#include <cstring>

#if UNITY_EDITOR
class YAMLRead;
class YAMLWrite;
#endif

enum { kHashStringLength = 32 };

// Holds a 128 bit hash value.
// Note that Hash128 itself, and String<->Hash functions do not do any hashing by themselves!
// They are merely hashed value bytes, and encoding/decoding that into hex-based strings.
// For hashing functions, see HashFunctions.h
struct Hash128
{
    // Data
    union
    {
        UInt8   bytes[16];
        // Note: accessing u64 and u32 will result in different things on big vs little endian!
        UInt64  u64[2];
        UInt32  u32[4];
    } hashData;


    // Initializes to "invalid" (all bits zero) hash
    Hash128() { memset(this, 0, sizeof(*this)); }

    // Initializes with already computed hash value.
    // Only 16 first bytes are used. If less than 16 is provided, the
    // rest are set to zero.
    Hash128(const UInt8* dataBytes, size_t dataBytesLength);

    const UInt8* Data() const { return hashData.bytes; }
    UInt8* Data() { return hashData.bytes; }

    // Same as constructor; sets hash data bytes directly.
    void SetData(const UInt8* dataBytes, size_t dataBytesLength);

    // Resets to all-zero / invalid value.
    void Reset();

    // Is hash value valid? Values with all bits zero are considered invalid.
    bool IsValid() const { return hashData.u64[0] != 0 || hashData.u64[1] != 0; }

    // Returns a simple 32 bit value from this hash (really just all 32 bit words xor'ed together).
    // This is fine for visualizing random colors, or other simpler things -- but of course
    // 32 bits is much less "quality" than full 128 bit hash value.
    UInt32 PackToUInt32() const;

    // Reduce the hash to 64 bits
    UInt64 PackToUInt64() const;

    // Needed by SortedHashArray template
    static Hash128 GetHash(const Hash128& hash) { return hash; }


    // Operators
    friend inline bool operator==(const Hash128& lhs, const Hash128& rhs) { return lhs.hashData.u64[0] == rhs.hashData.u64[0] && lhs.hashData.u64[1] == rhs.hashData.u64[1]; }
    friend inline bool operator!=(const Hash128& lhs, const Hash128& rhs) { return lhs.hashData.u64[0] != rhs.hashData.u64[0] || lhs.hashData.u64[1] != rhs.hashData.u64[1]; }
    friend inline bool operator<(const Hash128& lhs, const Hash128& rhs)
    {
        if (lhs.hashData.u64[0] != rhs.hashData.u64[0])
            return lhs.hashData.u64[0] < rhs.hashData.u64[0];
        else
            return lhs.hashData.u64[1] < rhs.hashData.u64[1];
    }

    friend inline bool operator<=(const Hash128& lhs, const Hash128& rhs)
    {
        return (lhs == rhs) || (lhs < rhs);
    }

    Hash128& operator=(const Hash128& rhs)
    {
        this->hashData.u64[0] = rhs.hashData.u64[0];
        this->hashData.u64[1] = rhs.hashData.u64[1];
        return *this;
    }

    static Hash128 CreateMinimumLexicographicalValue() { return Hash128(); }

    // Serialization
    DECLARE_SERIALIZE_NO_PPTR(Hash128)
};

// BIND_MANAGED_TYPE_NAME(Hash128, UnityEngine_Hash128)


// Serialization
template<class T>
void Hash128::Transfer(T& transfer)
{
    UInt8* bytes = hashData.bytes;
    TRANSFER(bytes[0]);
    TRANSFER(bytes[1]);
    TRANSFER(bytes[2]);
    TRANSFER(bytes[3]);
    TRANSFER(bytes[4]);
    TRANSFER(bytes[5]);
    TRANSFER(bytes[6]);
    TRANSFER(bytes[7]);
    TRANSFER(bytes[8]);
    TRANSFER(bytes[9]);
    TRANSFER(bytes[10]);
    TRANSFER(bytes[11]);
    TRANSFER(bytes[12]);
    TRANSFER(bytes[13]);
    TRANSFER(bytes[14]);
    TRANSFER(bytes[15]);
}

namespace core
{
    template<class T>
    struct hash;

    template<>
    struct hash<Hash128>
    {
        UInt32 operator()(const Hash128& h) const
        {
            return h.hashData.u32[0];
        }
    };
}

namespace std
{
    template<>
    struct hash<Hash128>
    {
        size_t operator()(Hash128 const& h) const noexcept
        {
            static_assert(sizeof(size_t) == sizeof(uint32_t) || sizeof(size_t) == sizeof(uint64_t), "Unexpected sizeof(size_t)");
            //this is resolved at compile time
            if (sizeof(size_t) == sizeof(uint32_t))
                return h.PackToUInt32();
            if (sizeof(size_t) == sizeof(uint64_t))
                return h.PackToUInt64();
        }
    };
}

#if UNITY_EDITOR
template<>
void Hash128::Transfer(YAMLRead& transfer);
template<>
void Hash128::Transfer(YAMLWrite& transfer);
#endif // #if UNITY_EDITOR


// String<->Hash128 conversion functions.
// These encode hash value to a hex (two characteers per byte) representation,
// and decode respectively.
std::string Hash128ToString(const Hash128& digest);
void        Hash128ToString(const Hash128& digest, char* str);
std::string Hash128ToString(UInt32 d0, UInt32 d1, UInt32 d2, UInt32 d3);

Hash128 StringToHash128(std::string hexEncodedHash);
Hash128 StringToHash128(const char* hexEncodedHash, size_t length);

#if ENABLE_NATIVE_TEST_FRAMEWORK
#include "External/UnitTest++/src/MemoryOutStream.h"
inline UnitTest::MemoryOutStream& operator<<(UnitTest::MemoryOutStream& stream, const Hash128& hash)
{
    stream << Hash128ToString(hash);
    return stream;
}

#endif
