#include "CommonStringTable.h"
#include <cstring>
#include "CommonString.h"
#include "Utilities/ArrayUtility.h"
#include "Utilities/HashStringFunctions.h"
#include "Utilities/RegisterRuntimeInitializeAndCleanup.h"
#include "BaseClasses/BaseTypes.h"
#include "Memory/MemoryMacros.h"

static CommonStringTable* gCommonStringTable = NULL;


static UInt32 HashCommonString(const char* str, size_t len)
{
    // Normally we'd use ComputeStringHash, but that one uses different hash algorithm depending on
    // whether we're on 64 or 32 bit. For tests in the code below, we want consistent
    // hash values on all plaforms. So use FNV explicitly.
    return ComputeFNV1aHash(str, len);
}

CommonStringTable::CommonStringTable(MemLabelRef label)
    : m_MemLabel(label)
{
//    for (int i = 0; i < kHashTableSize; ++i)
//        m_Strings[i].set_memory_label(m_MemLabel);

    const char* str = HuaHuo::CommonString::BufferBegin;
    while (str < HuaHuo::CommonString::BufferEnd)
    {
        size_t length = std::strlen(str);
        Entry e =
        {
            HashCommonString(str, length),
            str
        };
        m_Strings[e.hash % kHashTableSize].push_back(e);

        str += length + 1;
    }

    for (int i = 0; i < kHashTableSize; ++i)
        m_Strings[i].shrink_to_fit();
}

CommonStringTable::~CommonStringTable()
{}

const char* CommonStringTable::FindCommonString(const char* str, size_t length) const
{
    if (IsCommonString(str))
        return str;

    StringHash strHash = HashCommonString(str, length);
    const std::vector<Entry>& bucket = m_Strings[strHash % kHashTableSize];
    for (size_t i = 0; i < bucket.size(); ++i)
    {
        const Entry& e = bucket[i];
        if (e.hash == strHash && std::strcmp(e.buffer, str) == 0)
            return e.buffer;
    }

    return NULL;
}

void CommonStringTable::StaticInitialize(void*)
{
    gCommonStringTable = HUAHUO_NEW_AS_ROOT(CommonStringTable, kMemString, "Managers", "SharedStrings") ();

#if DEBUGMODE
    // Notice: when you have added new strings to CommonStrings.h:
    // 1. Start Unity; you'll get break at the two asserts at the end of this function;
    // 2. Make a new line in bufferHashes like { the new last Id, the value of finalHash },
    // NEVER CHANGE EXISTING STRINGS!

    // the following code is for ensuring the existing strings are not changed
    UInt32 bufferHashes[][2] =
    {
        { Unity::CommonString::Id_Vector4f, 0xf1aacd1e },
        { Unity::CommonString::Id_m_ScriptingClassIdentifier, 0xb58392fb },
        { Unity::CommonString::Id_Gradient, 0x2cbfe13e },
        { Unity::CommonString::Id_TypePtr, 0xfda93efb },
        { Unity::CommonString::Id_int2_storage, 0x22379d91 },
        { Unity::CommonString::Id_int3_storage, 0x3c1612a8 },
        { Unity::CommonString::Id_BoundsInt, 0xe9aae2c1 },
        { Unity::CommonString::Id_m_CorrespondingSourceObject, 0x4c26efdf },
        { Unity::CommonString::Id_m_PrefabAsset, 0x9b564b6a },
        { Unity::CommonString::Id_FileSize, 0xAFF573A8}
    };

    // loop through already defined hashes
    UInt32 j = 0, hash = 0x141401CC;
    const char* str = Unity::CommonString::BufferBegin;
    for (size_t i = 0; i < ARRAY_SIZE(bufferHashes); ++i)
    {
        for (; j <= bufferHashes[i][0]; ++j)
        {
            size_t len = std::strlen(str);
            hash ^= HashCommonString(str, len);
            str += len + 1;
        }

        // HITTING THIS MEANS YOU HAVE CHANGED EXISTING STRINGS: NEVER DO THAT!
        // (or you changed the hashing function; in which case fine, update all the expected hashes -- their values are
        // only used at runtime for speeding up lookups)
        AssertFormatMsg(hash == bufferHashes[i][1], "CommonStringTable entries have changed? index %i got %x expected %x", (int)i, hash, bufferHashes[i][1]);
    }

    // computes a hash for all strings for updating
    for (; j < Unity::CommonString::IdEmpty; ++j)
    {
        size_t len = std::strlen(str);
        hash ^= HashCommonString(str, len);
        str += len + 1;
    }
    UInt32 finalHash = hash;

    // Hitting this means you have added new strings: see notice above.
    AssertMsg(bufferHashes[ARRAY_SIZE(bufferHashes) - 1][0] == Unity::CommonString::IdEmpty - 1, "CommonStringTable new string added, bufferHashes needs an update");
    AssertFormatMsg(bufferHashes[ARRAY_SIZE(bufferHashes) - 1][1] == finalHash, "CommonStringTable new string added, bufferHashes needs an update (final hash value got %x expected %x)", finalHash, bufferHashes[ARRAY_SIZE(bufferHashes) - 1][1]);
#endif // #if DEBUGMODE
}

void CommonStringTable::StaticCleanup(void*)
{
    HUAHUO_DELETE(gCommonStringTable, kMemString);
    gCommonStringTable = NULL;
}

static RegisterRuntimeInitializeAndCleanup s_CommonStringTableCallbacks(CommonStringTable::StaticInitialize, CommonStringTable::StaticCleanup, -1);

CommonStringTable& GetCommonStringTable()
{
    return *gCommonStringTable;
}
