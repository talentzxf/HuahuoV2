//
// Created by VincentZhang on 4/26/2022.
//

#include "MemoryCacherReadBlocks.h"
#include "Logging/LogAssert.h"

BlockReadInfo CalculateBlockReadInfo(UInt8* start, UInt8* end, size_t offset, size_t size)
{
    const size_t blockReadSize = (end - start - offset);
    const size_t copyAmount = std::min(blockReadSize, size);
    const UInt8 * const source = start + offset;

    return { source, copyAmount };
}

BlockRangeInfo CalculateBlockRange(size_t size, size_t position, size_t maxBlockSize)
{
    const size_t startBlock = position / maxBlockSize;
    const size_t offset = position % maxBlockSize;
    const size_t endBlock = ((position + size - 1) / maxBlockSize) + 1;

    return { startBlock, endBlock, offset };
}

void ReadFileCache(CacheReaderBase& cache, void* data, size_t position, size_t size)
{
    const size_t cacheMaxBlockSize = cache.GetCacheSize();

    BlockRangeInfo range = CalculateBlockRange(size, position, cacheMaxBlockSize);

    UInt8* dest = static_cast<UInt8*>(data);

    for (size_t blockIdx = range.startIndex; blockIdx < range.endIndex; ++blockIdx)
    {
        UInt8 *blockStart, *blockEnd;
        cache.LockCacheBlock(blockIdx, &blockStart, &blockEnd);

        BlockReadInfo readInfo = CalculateBlockReadInfo(blockStart, blockEnd, range.offset, size);

        memcpy(dest, readInfo.source, readInfo.length);

        dest += readInfo.length;
        size -= readInfo.length;
        range.offset = 0;

        cache.UnlockCacheBlock(blockIdx);
    }
}

MemoryCacherReadBlocks::MemoryCacherReadBlocks(UInt8** blocks, int size, size_t cacheBlockSize)
        :   m_Memory(blocks)
        ,   m_FileSize(size)
        ,   m_CacheBlockSize(cacheBlockSize)
{
}

MemoryCacherReadBlocks::~MemoryCacherReadBlocks()
{
}

void MemoryCacherReadBlocks::LockCacheBlock(size_t block, UInt8** startPos, UInt8** endPos)
{
    /// VERIFY OUT OF BOUNDS!!! ???
    Assert(block <= m_FileSize / m_CacheBlockSize);
    *startPos = m_Memory[block];
    *endPos = *startPos + std::min<int>(GetFileLength() - block * m_CacheBlockSize, m_CacheBlockSize);
}

void MemoryCacherReadBlocks::DirectRead(void* data, size_t position, size_t size)
{
    ReadFileCache(*this, data, (UInt64)position, size);
}