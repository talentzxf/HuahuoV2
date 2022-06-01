#include "Hash128.h"
#include "HashStringFunctions.h"

#if UNITY_EDITOR
#include "Runtime/Serialize/TransferFunctions/YAMLWrite.h"
#include "Runtime/Serialize/TransferFunctions/YAMLRead.h"

template<>
void Hash128::Transfer(YAMLWrite& transfer)
{
    transfer.SetVersion(2);

    if (transfer.IsVersionSmallerOrEqual(1))
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
    else
    {
        core::string hashString = Hash128ToString(*this);
        transfer.Transfer(hashString, "Hash");
    }
}

template<>
void Hash128::Transfer(YAMLRead& transfer)
{
    transfer.SetVersion(2);

    if (transfer.IsVersionSmallerOrEqual(1))
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
    else
    {
        core::string hashString;
        transfer.Transfer(hashString, "Hash");

        *this = StringToHash128(hashString);
    }
}

#endif

Hash128::Hash128(const UInt8* dataBytes, size_t dataBytesLength)
{
    SetData(dataBytes, dataBytesLength);
}

UInt32 Hash128::PackToUInt32() const
{
    UInt32 const* pu = hashData.u32;
    return pu[0] ^ pu[1] ^ pu[2] ^ pu[3];
}

UInt64 Hash128::PackToUInt64() const
{
    return hashData.u64[0] ^ hashData.u64[1];
}

void Hash128::SetData(const UInt8* dataBytes, size_t dataBytesLength)
{
    // copy up to 16 passed bytes into data
    dataBytesLength = std::min(dataBytesLength, (size_t)16);
    memcpy(hashData.bytes, dataBytes, dataBytesLength);
    // fill the rest with zeroes
    memset(hashData.bytes + dataBytesLength, 0, 16 - dataBytesLength);
}

void Hash128::Reset()
{
    memset(hashData.bytes, 0, sizeof(hashData.bytes));
}

std::string Hash128ToString(const Hash128& hash)
{
    char name[kHashStringLength + 1];
    Hash128ToString(hash, name);
    name[kHashStringLength] = '\0';
    return std::string(name);
}

void Hash128ToString(const Hash128& hash, char* str)
{
    for (int i = 0; i < 16; i++)
        snprintf(&str[i * 2], 3, "%02hhx", hash.hashData.bytes[i]);
}

std::string Hash128ToString(UInt32 d0, UInt32 d1, UInt32 d2, UInt32 d3)
{
    Hash128 val;
    val.hashData.u32[0] = d0;
    val.hashData.u32[1] = d1;
    val.hashData.u32[2] = d2;
    val.hashData.u32[3] = d3;
    return Hash128ToString(val);
}

static inline int HexToNumber(char c)
{
    if (c >= '0' && c <= '9')
        return c - '0';
    if (c >= 'a' && c <= 'f')
        return c - 'a' + 10;
    if (c >= 'A' && c <= 'F')
        return c - 'A' + 10;
    return 0;
}

Hash128 StringToHash128(std::string hexEncodedHash)
{
    return StringToHash128(hexEncodedHash.data(), hexEncodedHash.size());
}

Hash128 StringToHash128(const char* hexEncodedHash, size_t length)
{
    Hash128 digest;
    const size_t end = length > 32 ? 16 : length / 2;
    for (size_t i = 0; i < end; ++i)
    {
        digest.hashData.bytes[i] = HexToNumber(hexEncodedHash[i * 2]) * 16 + HexToNumber(hexEncodedHash[i * 2 + 1]);
    }
    return digest;
}
