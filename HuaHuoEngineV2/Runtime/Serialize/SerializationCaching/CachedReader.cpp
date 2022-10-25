//
// Created by VincentZhang on 4/24/2022.
//

#include "CachedReader.h"
#include "memcpy_constrained.h"
#include "CacheReaderBase.h"
#include "Logging/LogAssert.h"

CachedReader::CachedReader()
{
    m_Cacher = 0;
    m_Block = -1;
    m_OutOfBoundsRead = false;
}

CachedReader::~CachedReader()
{
    Assert(m_Block == -1);
}

void CachedReader::LockCacheBlockBounded()
{
    m_Cacher->LockCacheBlock(m_Block, &m_CacheStart, &m_CacheEnd);
    UInt8* maxPos = m_MaximumPosition - m_Block * m_CacheSize + m_CacheStart;
    m_CacheEnd = std::min(m_CacheEnd, maxPos);
}

void CachedReader::InitRead(CacheReaderBase& cacher, size_t position, size_t readSize)
{
    Assert(m_Block == -1);
    m_Cacher = &cacher;
    Assert(m_Cacher != NULL);
    m_CacheSize = m_Cacher->GetCacheSize();
    m_Block = position / m_CacheSize;
    m_MaximumPosition = position + readSize;
    m_MinimumPosition = position;

    LockCacheBlockBounded();

    SetPosition(position);
}

void CachedReader::InitResourceImages(ResourceImageGroup& resourceImageGroup)
{
    m_ResourceImageGroup = resourceImageGroup;
}

size_t CachedReader::End()
{
    Assert(m_Block != -1);
    size_t position = GetPosition();
    OutOfBoundsError(position, 0);

    m_Cacher->UnlockCacheBlock(m_Block);
    m_Block = -1;
    return position;
}

void CachedReader::SetPosition(size_t position)
{
    OutOfBoundsError(position, 0);
    if (m_OutOfBoundsRead)
        return;

    if (position / m_CacheSize != (size_t)m_Block)
    {
        m_Cacher->UnlockCacheBlock(m_Block);
        m_Block = position / m_CacheSize;
        m_Cacher->LockCacheBlock(m_Block, &m_CacheStart, &m_CacheEnd);
    }
    m_CachePosition = position - m_Block * m_CacheSize + m_CacheStart;
}

void CachedReader::OutOfBoundsError(size_t position, size_t size)
{
    if (m_OutOfBoundsRead)
        return;

    if (position + size > m_Cacher->GetFileLength())
    {
        FatalErrorString("The file \'" + m_Cacher->GetPathName() + "\' is corrupted! Refresh page!\n[Position out of bounds!]");
        m_OutOfBoundsRead = true;
    }

    if (position + size > m_MaximumPosition)
    {
        FatalErrorString("The file \'" + m_Cacher->GetPathName() + "\' is corrupted! Refresh page!\n[Position out of bounds!]");
        m_OutOfBoundsRead = true;
    }

    if (position < m_MinimumPosition)
    {
        FatalErrorString("The file \'" + m_Cacher->GetPathName() + "\' is corrupted! Refresh page!\n[Position out of bounds!]");
        m_OutOfBoundsRead = true;
    }
}

void CachedReader::UpdateReadCache(void* data, size_t size)
{
    Assert(m_Cacher != NULL);
    Assert(size <= m_CacheSize);

    size_t position = GetPosition();
    OutOfBoundsError(position, size);

    if (m_OutOfBoundsRead)
    {
        memset(data, 0, size);
        return;
    }

    // copy data oldblock
    SetPosition(position);
    memcpy_constrained_src(data, m_CachePosition, size, m_CacheStart, m_CacheEnd);

    // Read next cache block only if we actually need it.
    if (m_CachePosition + size > m_CacheEnd)
    {
        // Check if the cache block
        // copy data new block
        SetPosition(position + size);
        UInt8* cachePosition = position - m_Block * m_CacheSize + m_CacheStart;
        memcpy_constrained_src(data, cachePosition, size, m_CacheStart, m_CacheEnd);
    }
    else
    {
        m_CachePosition += size;
    }
}

void CachedReader::Align4Read()
{
    UInt32 offset = m_CachePosition - m_CacheStart;
    offset = ((offset + 3) >> 2) << 2;
    m_CachePosition = m_CacheStart + offset;
}

void CachedReader::Read(void* data, size_t size)
{
    if (m_CachePosition + size <= m_CacheEnd)
    {
        memcpy(data, m_CachePosition, size);
        m_CachePosition += size;
    }
    else
    {
        // Read some data directly if it is coming in big chunks and we are not hitting the end of the file!
        size_t position = GetPosition();
        OutOfBoundsError(position, size);

        if (m_OutOfBoundsRead)
        {
            memset(data, 0, size);
            return;
        }

        // Read enough bytes from the cache to align the position with the cache size
        if (position % m_CacheSize != 0)
        {
            size_t blockEnd = ((position / m_CacheSize) + 1) * m_CacheSize;
            size_t curReadSize = std::min<size_t>(size, blockEnd - position);
            memcpy_constrained_src(data, m_CachePosition, curReadSize, m_CacheStart, m_CacheEnd);
            m_CachePosition += curReadSize;
            position += curReadSize;
            (UInt8*&)data += curReadSize;
            size -= curReadSize;
        }

        // If we have a big block of data read directly without a cache, all aligned reads
        size_t physicallyLimitedSize = std::min<size_t>((position + size), m_Cacher->GetFileLength()) - position;
        size_t blocksToRead = physicallyLimitedSize / m_CacheSize;
        if (blocksToRead > 0)
        {
            size_t curReadSize = blocksToRead * m_CacheSize;
            m_Cacher->DirectRead((UInt8*)data, position, curReadSize);
            m_CachePosition += curReadSize;
            (UInt8*&)data += curReadSize;
            size -= curReadSize;
        }

        // Read the rest of the data from the cache!
        while (size != 0)
        {
            size_t curReadSize = std::min<size_t>(size, m_CacheSize);
            UpdateReadCache(data, curReadSize);
            (UInt8*&)data += curReadSize;
            size -= curReadSize;
        }
    }
}
