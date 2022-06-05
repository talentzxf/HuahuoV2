#pragma once

#include "CacheWriterBase.h"
#include <vector>
#include "Logging/LogAssert.h"

static const size_t kMegabyte = 1024 * 1024;
static const size_t kShrinkToFitRatio = 8;

class MemoryCacheWriter : public CacheWriterBase
{
protected:
    enum
    {
        kCacheSize = 256
    };

    std::vector<UInt8>& m_Memory;
    SInt32                m_LockCount;

public:
    MemoryCacheWriter(std::vector<UInt8>& mem) : m_Memory(mem), m_LockCount(0) {}
    virtual ~MemoryCacheWriter() { Assert(m_LockCount == 0); }

    virtual void LockCacheBlock(size_t block, UInt8** startPos, UInt8** endPos)
    {
        m_Memory.resize((block + 1) * kCacheSize);

        *startPos = &m_Memory[block * kCacheSize];
        *endPos = *startPos + kCacheSize;

        m_LockCount++;
    }

    virtual void PreallocateForWrite(size_t block, UInt8** startPos, UInt8** endPos, size_t sizeOfWrite)
    {
        size_t newSize = m_Memory.size() + sizeOfWrite;
        newSize = newSize + (newSize % kMegabyte);
        m_Memory.resize(newSize);

        *startPos = &m_Memory[block * kCacheSize];
        *endPos = *startPos + kCacheSize;
    }

    virtual bool CompleteWriting(size_t size)
    {
        m_Memory.resize(size);

        // Only shrink to fit if we're wasting more than a ratio specified in kShrinkToFitRatio
        // This balances speed vs wasted memory, if m_Memory is large shrink_to_fit can take a very long time (think seconds)
        if ((m_Memory.capacity() - size) > (size / kShrinkToFitRatio))
            m_Memory.shrink_to_fit();

        return true;
    }

    virtual void UnlockCacheBlock(size_t block)    { m_LockCount--; }

    virtual size_t GetCacheSize()                  { return kCacheSize; }
    virtual std::string GetPathName() const   { return "MemoryStream"; }
};
