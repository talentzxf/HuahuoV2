#pragma once
#include "Memory/AllocatorLabels.h"
#include "CommonString.h"
#include <vector>

// CommonStringTable is initialized at load time with all common strings and
// stays completely constant during runtime, thus it is thread safe.

class CommonStringTable
{
public:
    typedef unsigned int StringHash;

    explicit CommonStringTable(MemLabelRef label);
    ~CommonStringTable();

    const char* FindCommonString(const char* str, size_t length) const;

    // Init
    static void StaticInitialize(void*);
    static void StaticCleanup(void*);

private:
    enum { kHashTableSize = HuaHuo::CommonString::Count / 5 };
    struct Entry { StringHash hash; const char* buffer; };
    MemLabelId m_MemLabel;
    std::vector<Entry> m_Strings[kHashTableSize];
};

CommonStringTable& GetCommonStringTable();
