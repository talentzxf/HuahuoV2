//
// Created by VincentZhang on 6/20/2022.
//
#include "HashFunctions.h"

void ComputeHash128(const void* data, size_t dataSize, UInt64 inSeedOutHash[2])
{
    SpookyHash::Hash128(data, dataSize, &inSeedOutHash[0], &inSeedOutHash[1]);
}

// Note that the hash is in-out parameter! Acts as a seed on input, and filled with hashed value on output.
void ComputeHash128(const void* data, size_t dataSize, Hash128& inSeedOutHash)
{
    ComputeHash128(data, dataSize, inSeedOutHash.hashData.u64);
}

Hash128 ComputeHash128(const void* data, size_t dataSize)
{
    Hash128 h;
    ComputeHash128(data, dataSize, h);
    return h;
}