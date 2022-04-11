//
// Created by VincentZhang on 4/9/2022.
//

#ifndef PERSISTENTMANAGER_HASHFUNCTIONS_H
#define PERSISTENTMANAGER_HASHFUNCTIONS_H
#include <utility>
#include <stdint.h>
#include "BaseClasses/BaseTypes.h"
#include "baselib/include/IntegerDefinitions.h"
#include "HashFunctions/xxhash.h"

// --------------------------------------------------------------------------
// Hash functions in this file:
//
// General purpose data hashing:
//   ComputeHash* functions, for 32, 64 and 128 bit hashes.
//
// Integer / Pointer hashing:
//    ComputeIntHash, SInt32HashFunction, UInt32HashFunction
//    ComputePointerHash, PointerHashFunction
//
// For string hashing helpers, see HashStringFunctions.h


// --------------------------------------------------------------------------
// General "compute hash for a bunch of bytes" functions.
// - good hashing quality for use in hash tables, identifying unique blobs of data, checksums
//   (generally want a SMHasher score of 10)
// - fast performance, generally we try to use fastest good quality general hash function there is.
// - *NOT* cryptographically secure hashing! If you want that, use the hash functions provided by the TLS module.
//
// WARNING: Do *NOT* serialize hashed values and expect them to stay stable in the future! We might
// change underlying implementations of these hash functions at any point. If you want something future
// proof wrt exact hash values, call hash function of your choice (e.g. xxhash) directly.
//
// If you want to "stream" style hashing (hash data in parts, without having all of them in a block of memory),
// e.g. a combined hash of data A + B, you can do that by passing previous chunk hash as "seed" for input:
//   hashA = ComputeHash(A, sizeOfA);
//   hashAandB = ComputeHash(B, sizeofB, hashA);
// Note that this is *NOT* guaranteed to be the same as hashing A+B as a full block! But for use in hashtables
// of "combine some pieces of data" it's fine, as long as you hash it consistently in the same order.
//
// Currently the implementation uses:
// - xxHash32 for 32 bit hashes: http://cyan4973.github.io/xxHash/
// - CityHash for 64 bit hashes: https://github.com/google/cityhash
// - SpookyHashV2 for 128 bit hashes: http://burtleburtle.net/bob/hash/spooky.html
// These are all very good quality, and generaly seem to be best overall performance, while
// having consistent results between architectures, see details at
// http://aras-p.info/blog/2016/08/09/More-Hash-Function-Tests/
// If you want absolute best performance, especially on large data sets, you'd probably want to use
// different hash functions depending on CPU/architecture (e.g. CityHash64 if on 64 bit, even when
// computing a 32 bit hash value).


//// Compute a 64 bit hash.
//// Implementation: CityHash64.
//UInt64 ComputeHash64(const void* data, size_t dataSize)
//{
//    return CityHash64((const char*)data, dataSize);
//}
//
//UInt64 ComputeHash64(const void* data, size_t dataSize, UInt64 seed)
//{
//    return CityHash64WithSeed((const char*)data, dataSize, seed);
//}

//// Compute a 128 bit hash.
//// Implementation: SpookyHash V2.
////
//// Note that the hash is in-out parameter! Acts as a seed on input, and filled with hashed value on output.
//void ComputeHash128(const void* data, size_t dataSize, UInt64 inSeedOutHash[2])
//{
//    SpookyHash::Hash128(data, dataSize, &inSeedOutHash[0], &inSeedOutHash[1]);
//}
//
//// Note that the hash is in-out parameter! Acts as a seed on input, and filled with hashed value on output.
//void ComputeHash128(const void* data, size_t dataSize, Hash128& inSeedOutHash)
//{
//    ComputeHash128(data, dataSize, inSeedOutHash.hashData.u64);
//}
//
//Hash128 ComputeHash128(const void* data, size_t dataSize)
//{
//    Hash128 h;
//    ComputeHash128(data, dataSize, h);
//    return h;
//}

// --------------------------------------------------------------------------
// Simple data type (integer/pointer) hashing


// Hashing a 4 byte integer value.
// 6 shift function from http://burtleburtle.net/bob/hash/integer.html
// Why one needs to hash integers at all? Because hash tables don't scale well in case keys are
// sequential integers, or all multiple of some number, etc. The bucket distribution ends up
// being terrible, depending on how your integers are distributed.

static UInt32 ComputeIntHash(UInt32 a)
{
a = (a + 0x7ed55d16) + (a << 12);
a = (a ^ 0xc761c23c) ^ (a >> 19);
a = (a + 0x165667b1) + (a << 5);
a = (a + 0xd3a2646c) ^ (a << 9);
a = (a + 0xfd7046c5) + (a << 3);
a = (a ^ 0xb55a4f09) ^ (a >> 16);
return a;
}

static UInt32 ComputeIntHash(SInt32 a)
{
return ComputeIntHash((UInt32)a);
}

struct SInt32HashFunction
{
    size_t operator()(SInt32 a) const
    {
        return ComputeIntHash(a);
    }
};
struct UInt32HashFunction
{
    size_t operator()(UInt32 a) const
    {
        return ComputeIntHash(a);
    }
};


// Hashing a pointer (address) value.
// This multiplies the address value with a constant that seems to be good at scrabmling bits,
// see "An experimental exploration of Marsaglia’s xorshift generators, scrambled"
// http://vigna.di.unimi.it/ftp/papers/xorshift.pdf -- original idea is from Numerical Recipes book.

typedef UInt64 PointerHash;

template<typename T>
struct PointerHashFunction
{
    size_t operator()(const T& p) const { return ComputePointerHash(p); }
};

#endif //PERSISTENTMANAGER_HASHFUNCTIONS_H
