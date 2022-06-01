//
// Created by VincentZhang on 4/26/2022.
//

#ifndef HUAHUOENGINE_BLOCKMEMORYCACHEWRITER_H
#define HUAHUOENGINE_BLOCKMEMORYCACHEWRITER_H

#include "Logging/LogAssert.h"
#include "CacheWriterBase.h"
#include "Memory/MemoryMacros.h"
#include <cstdlib>
#include <vector>

class BlockMemoryCacheWriter : public CacheWriterBase
{
protected:

    enum
    {
        kBlockCacherCacheSize = 256
    };

    enum
    {
        kNumBlockReservations = 256
    };

    size_t      m_Size;
    SInt32      m_LockCount;
    MemLabelId  m_AllocLabel;

    //  It is possible to use the custom allocator for this index as well -- however,
    //  using the tracking linear tempory allocator is most efficient, when deallocating
    //  in the exact opposite order of allocating, which can only be guaranteed, when all allocations
    //  are in our control.
    typedef std::vector<UInt8*>   BlockVector;
    BlockVector                 m_Blocks;

    std::string m_Filename;

public:

    BlockMemoryCacheWriter(MemLabelId label)
            : m_AllocLabel(label)
            // , m_Blocks(label)
    {
        m_Blocks.reserve(kNumBlockReservations);
        m_Size = 0;
        m_LockCount = 0;
    }

    ~BlockMemoryCacheWriter()
    {
        Assert(m_LockCount == 0);
        for (BlockVector::iterator i = m_Blocks.begin(); i != m_Blocks.end(); i++)
            HUAHUO_FREE(m_AllocLabel,*i);
    }

    void ResizeBlocks(size_t newBlockSize)
    {
        size_t oldBlockSize = m_Blocks.size();

        // free the excess blocks when resizing to a smaller blocksize
        if (oldBlockSize > newBlockSize)
            for (size_t block = newBlockSize; block < oldBlockSize; block++)
                HUAHUO_FREE(m_AllocLabel,m_Blocks[block]);

        if (m_Blocks.capacity() < newBlockSize)
            m_Blocks.reserve(m_Blocks.capacity() * 2);

        m_Blocks.resize(newBlockSize, NULL);

        for (size_t block = oldBlockSize; block < newBlockSize; block++)
            m_Blocks[block] = (UInt8*)HUAHUO_MALLOC(m_AllocLabel, kBlockCacherCacheSize);
    }

    virtual void LockCacheBlock(size_t block, UInt8** startPos, UInt8** endPos)
    {
        ResizeBlocks((int)std::max<SInt64>(block + 1, m_Blocks.size()));
        *startPos = m_Blocks[block];
        *endPos = *startPos + kBlockCacherCacheSize;
        m_LockCount++;
    }

    virtual bool CompleteWriting(size_t size)
    {
        m_Size = size;
        ResizeBlocks(m_Size / kBlockCacherCacheSize + 1);
        return true;
    }

    virtual void UnlockCacheBlock(size_t block)    { m_LockCount--; }
    virtual size_t GetFileLength()             { return m_Size; }
    virtual size_t GetCacheSize()              { return kBlockCacherCacheSize; }
    virtual std::string GetPathName() const { return m_Filename; }

    // Expose, so the internal data can be used with MemoryCacherReadBlocks (see below)
    UInt8** GetCacheBlocks()                   { return m_Blocks.empty() ? NULL : &*m_Blocks.begin(); }
};


#endif //HUAHUOENGINE_BLOCKMEMORYCACHEWRITER_H
