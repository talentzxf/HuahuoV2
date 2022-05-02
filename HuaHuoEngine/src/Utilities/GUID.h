#pragma once

#include "baselib/include/IntegerDefinitions.h"
#include "Serialize/SerializeUtility.h"
#include <string>
#include "HashFunctions.h"
#include "Hash.h"

#if UNITY_EDITOR
#include "Runtime/Scripting/ScriptingTypes.h"
#endif

enum { kGUIDStringLength = 32 };

// ADD_NEW_PLATFORM_HERE
#if !defined(UNITY_HAVE_GUID_INIT)
#define UNITY_HAVE_GUID_INIT (PLATFORM_WEBGL || PLATFORM_OSX || PLATFORM_WIN || PLATFORM_IOS || PLATFORM_TVOS || PLATFORM_ANDROID || PLATFORM_LINUX || PLATFORM_PLAYSTATION)
#endif // !defined(UNITY_HAVE_GUID_INIT)

// To setup the unique identifier use Init ().
// You can compare it against other unique identifiers
// It is guaranteed to be unique over space and time
//
// Called HuaHuoGUID because Visual Studio really does not like structs named GUID!
struct HuaHuoGUID
{
    // Be aware that because of endianness and because we store a GUID as four integers instead
    // of as the DWORD, WORD, and BYTE groupings as used by Microsoft, the individual bytes
    // may not end up where you expect them to be.  In particular both GUID and UUID specify Data4
    // (which is data[2] and data[3] here) as big endian but we split this value into two DWORDs
    // both of which we store as little endian.  So, if you're looking for the type bits in the GUID,
    // they are found in the least significant byte of data[2] instead of in the most significant
    // byte (where you would expect them).
    // Also, our text format neither conforms to the canonical format for GUIDs nor for UUIDs so
    // again the bits change place here (the group of type bits is found one character to the right).
    UInt32 data[4];

    // Used to be called GUID, so for serialization it has the old name
    DECLARE_SERIALIZE_OPTIMIZE_TRANSFER(GUID)

    HuaHuoGUID(UInt32 a, UInt32 b, UInt32 c, UInt32 d) { data[0] = a; data[1] = b; data[2] = c; data[3] = d; }
    HuaHuoGUID()  { data[0] = 0; data[1] = 0; data[2] = 0; data[3] = 0; }

    bool operator==(const HuaHuoGUID& rhs) const
    {
        return data[0] == rhs.data[0] && data[1] == rhs.data[1] && data[2] == rhs.data[2] && data[3] == rhs.data[3];
    }

    bool operator!=(const HuaHuoGUID& rhs) const { return !(*this == rhs); }

    bool operator<(const HuaHuoGUID& rhs) const { return CompareGUID(*this, rhs) == -1; }
    bool operator>(const HuaHuoGUID& rhs) const { return CompareGUID(*this, rhs) == 1; }
    friend int CompareGUID(const HuaHuoGUID& lhs, const HuaHuoGUID& rhs);

    // Use this instead of guid != HuaHuoGUID() -- Will often result in self-documented code
    bool IsValid() const { return data[0] != 0 || data[1] != 0 || data[2] != 0 || data[3] != 0; }

    static HuaHuoGUID CreateMinimumLexicographicalValue() { return HuaHuoGUID(); }

#if UNITY_HAVE_GUID_INIT
    void Init();
#endif
};

//BIND_MANAGED_TYPE_NAME(HuaHuoGUID, UnityEditor_GUID);

std::string GUIDToString(const HuaHuoGUID& guid/*, MemLabelRef label = kMemString*/);
void GUIDToString(const HuaHuoGUID& guid, char* string);
std::string GUIDToStringWithHyphens(const HuaHuoGUID& guid/*, MemLabelRef label = kMemString*/);

HuaHuoGUID StringToGUID(const std::string guidString);

#if UNITY_HAVE_GUID_INIT
HuaHuoGUID GenerateGUID();
HuaHuoGUID CreateGUIDFromSInt64(SInt64 value);
#endif

inline int CompareGUID(const HuaHuoGUID& lhs, const HuaHuoGUID& rhs)
{
    for (int i = 0; i < 4; i++)
    {
        if (lhs.data[i] < rhs.data[i])
            return -1;
        if (lhs.data[i] > rhs.data[i])
            return 1;
    }
    return 0;
}

bool CompareGUIDStringLess(const HuaHuoGUID& lhs, const HuaHuoGUID& rhs);

template<class TransferFunction>
void HuaHuoGUID::Transfer(TransferFunction& transfer)
{
    TRANSFER(data[0]);
    TRANSFER(data[1]);
    TRANSFER(data[2]);
    TRANSFER(data[3]);
}

namespace core
{
    template<>
    struct hash<HuaHuoGUID>
    {
        UInt32 operator()(const HuaHuoGUID& g) const
        {
            return ComputeHash32(g.data, sizeof(g.data));
        }
    };
}

#if ENABLE_NATIVE_TEST_FRAMEWORK
#include "External/UnitTest++/src/MemoryOutStream.h"
inline UnitTest::MemoryOutStream& operator<<(UnitTest::MemoryOutStream& stream, const HuaHuoGUID& guid)
{
    stream << GUIDToString(guid);
    return stream;
}

#endif
