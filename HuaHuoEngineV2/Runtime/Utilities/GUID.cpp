#include "GUID.h"

#include "Math/Random/rand.h"

#if PLATFORM_WIN
#include <windef.h>
#include <ObjBase.h>
#endif

#if UNITY_APPLE
#include <CoreFoundation/CoreFoundation.h>
#endif

// Return true if the GUID as its type bits set to 10.  This is the standard type both
// for GUIDs as well as for UUIDs.  We care about this because we flip the second bit
// for scrambled GUIDs in non-Pro.
static bool IsStandardGUID(const HuaHuoGUID& guid)
{
    // Note unusual location of type bits.  See note in header.
    return ((guid.data[2] & 0xC0) == 0x80);
}

static void MarkAsStandardGUID(HuaHuoGUID& guid)
{
    guid.data[2] = (guid.data[2] & ~0xC0) | 0x80;
}

// #if UNITY_HAVE_GUID_INIT && !UNITY_USE_PLATFORM_GUID
void HuaHuoGUID::Init()
{
    // __FAKEABLE_METHOD__(HuaHuoGUID, Init, ());

#if UNITY_APPLE

    CFUUIDRef myUUID = CFUUIDCreate(kCFAllocatorDefault);
    CFUUIDBytes bytes = CFUUIDGetUUIDBytes(myUUID);
    CFRelease(myUUID);

    memcpy(data, &bytes, sizeof(CFUUIDBytes));

#elif PLATFORM_WIN

    GUID guid;
    ::CoCreateGuid(&guid);

    memcpy(data, &guid, sizeof(guid));

#else

    struct RandomDataGenerator
    {
        Rand rand;
#if SUPPORT_THREADS
        SimpleLock mutex;
#endif
        RandomDataGenerator() : rand(Rand::GetUniqueGenerator()) {}
        void operator()(HuaHuoGUID& guid)
        {
#if SUPPORT_THREADS
            SimpleLock::AutoLock lock(mutex);
#endif
            for (int i = 0; i < 4; i++)
                guid.data[i] = rand.Get();
        }
    };
    static RandomDataGenerator generateRandomData;

    generateRandomData(*this);
    MarkAsStandardGUID(*this);

#endif

    AssertMsg(IsStandardGUID(*this),
        "HuaHuoGUID::Init() must produce a standard GUID/UUID with type bits 10.");
}

// #endif // UNITY_HAVE_GUID_INIT && !UNITY_USE_PLATFORM_GUID

bool CompareGUIDStringLess(const HuaHuoGUID& lhsGUID, const HuaHuoGUID& rhsGUID)
{
    char lhs[32];
    char rhs[32];
    GUIDToString(lhsGUID, lhs);
    GUIDToString(rhsGUID, rhs);

    for (int i = 0; i < 32; i++)
    {
        if (lhs[i] != rhs[i])
            return lhs[i] < rhs[i];
    }

    return false;
}

std::string GUIDToString(const HuaHuoGUID& guid, MemLabelRef label)
{
    char name[kGUIDStringLength + 1];
    GUIDToString(guid, name);
    name[kGUIDStringLength] = '\0';
    return std::string(name);
}

//std::string GUIDToStringWithHyphens(const HuaHuoGUID& guid, MemLabelRef label)
//{
//    const std::string str = GUIDToString(guid, kMemTempAlloc);
//
//    std::string r(label);
//    core::FormatTo(r, "{0}-{1}-{2}-{3}-{4}",
//        str.substr(0,  8),
//        str.substr(8,  4),
//        str.substr(12,  4),
//        str.substr(16,  4),
//        str.substr(20, 12));
//    return r;
//}

extern const char kHexToLiteral[16];

void GUIDToString(const HuaHuoGUID& guid, char* name)
{
    for (int i = 0; i < 4; i++)
    {
        for (int j = 8; j--;)
        {
            UInt32 cur = guid.data[i];
            cur >>= (j * 4);
            cur &= 0xF;
            name[i * 8 + j] = kHexToLiteral[cur];
        }
    }
}

//this is pre initialized to make this function thread safe
const signed char s_LiteralToHex[255] = { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, -1, -1, -1, -1, -1, -1, -1, 10, 11, 12, 13, 14, 15, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 10, 11, 12, 13, 14, 15, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 };
HuaHuoGUID StringToGUID(std::string guidString)
{
    size_t length = guidString.size();
    if (length != kGUIDStringLength)
        return HuaHuoGUID();

    // Convert every hex char into an int [0...16]
    int hex[kGUIDStringLength];
    for (int i = 0; i < kGUIDStringLength; i++)
        hex[i] = (int)s_LiteralToHex[(int)guidString[i]];

    HuaHuoGUID guid;
    for (int i = 0; i < 4; i++)
    {
        UInt32 cur = 0;
        for (int j = 8; j--;)
        {
            int curHex = hex[i * 8 + j];
            if (curHex == -1)
                return HuaHuoGUID();

            cur |= (curHex << (j * 4));
        }
        guid.data[i] = cur;
    }
    return guid;
}

#if UNITY_HAVE_GUID_INIT && !UNITY_USE_PLATFORM_GUID
HuaHuoGUID GenerateGUID()
{
    HuaHuoGUID guid;
    guid.Init();
    return guid;
}

HuaHuoGUID CreateGUIDFromSInt64(SInt64 value)
{
    HuaHuoGUID guid;
    memcpy(&guid, &value, sizeof(value));
    MarkAsStandardGUID(guid);
    return guid;
}

#endif // UNITY_HAVE_GUID_INIT && !UNITY_USE_PLATFORM_GUID
