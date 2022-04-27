//
// Created by VincentZhang on 4/26/2022.
//

#ifndef PERSISTENTMANAGER_MEMORYCACHERREADBLOCKS_H
#define PERSISTENTMANAGER_MEMORYCACHERREADBLOCKS_H

#include "CacheReaderBase.h"

class MemoryCacherReadBlocks : public CacheReaderBase
{
public:

    MemoryCacherReadBlocks(UInt8** blocks, int size, size_t cacheBlockSize);
    ~MemoryCacherReadBlocks();

    virtual void LockCacheBlock(size_t block, UInt8** startPos, UInt8** endPos);
    virtual void UnlockCacheBlock(size_t block) {}

    virtual void DirectRead(void* data, size_t position, size_t size);

    virtual size_t  GetFileLength() const            { return m_FileSize; }

    virtual size_t GetCacheSize() const              { return m_CacheBlockSize; }

    virtual std::string GetPathName() const  { return "none"; }

    virtual UInt8* GetAddressOfMemory()
    {
        return m_Memory[0];
    }

private:

    // Finishes all reading, deletes all caches
    void Flush();

    UInt8**     m_Memory;
    size_t      m_FileSize;
    size_t      m_CacheBlockSize;
};


struct BlockReadInfo
{
    const UInt8 * const source;
    size_t length;
};

struct BlockRangeInfo
{
    size_t startIndex;
    size_t endIndex;
    size_t offset;
};

BlockRangeInfo CalculateBlockRange(size_t size, size_t position, size_t cacheLineSize);
BlockReadInfo CalculateBlockReadInfo(UInt8* start, UInt8* end, size_t offset, size_t size);

void ReadFileCache(CacheReaderBase& cacher, void* data, size_t position, size_t size);

#endif //PERSISTENTMANAGER_MEMORYCACHERREADBLOCKS_H
