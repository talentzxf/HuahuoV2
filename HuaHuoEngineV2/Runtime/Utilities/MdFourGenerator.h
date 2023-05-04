#pragma once

#include "Math/Color.h"
#include "Utilities/Hash128.h"

struct MD4state_st;
Hash128 MdFourFile(const std::string& filename);


// Utility class for computing MD4 hashes of data.
// NOTE: do not use for new code! Use appropriate hasher from Runtime/Utilities/HashFunctions.h instead.
// MD4 has none of the advantages of more modern hashing algorithms:
// - if you want to checksum data or detect uniqueness, use a non-crypto hasher, will be much faster.
// - if you want crypto security, MD4 is nowhere near being strong enough. Even MD5 is not considered strong, and MD4 is much weaker.
class MdFourGenerator
{
public:
    MdFourGenerator();
    virtual ~MdFourGenerator();

    void Feed(const void* bytes, UInt64 length);
    void Feed(SInt32 data);
    void Feed(UInt32 data);
    void Feed(UInt64 data);
    void Feed(SInt64 data);
    void Feed(ColorRGBA32 data); // used to feed ColorRGBA32s to the hash, which are stored as *unswapped* UINT32s
    void Feed(float data);
    void Feed(double data);
    void Feed(const std::string& data);
    void Feed(char const* string);
    void Feed(Hash128 hash);
    UInt64 FeedFromFile(const std::string& pathName);
    Hash128 Finish();

private:
    MD4state_st * m_Acc;
    bool m_Done;
};
