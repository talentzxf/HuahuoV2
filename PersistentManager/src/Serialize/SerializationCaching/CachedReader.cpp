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
        FatalErrorString("The file \'" + m_Cacher->GetPathName() + "\' is corrupted! Remove it and launch unity again!\n[Position out of bounds!]");
        m_OutOfBoundsRead = true;
    }

    if (position + size > m_MaximumPosition)
    {
        FatalErrorString("The file \'" + m_Cacher->GetPathName() + "\' is corrupted! Remove it and launch unity again!\n[Position out of bounds!]");
        m_OutOfBoundsRead = true;
    }

    if (position < m_MinimumPosition)
    {
        FatalErrorString("The file \'" + m_Cacher->GetPathName() + "\' is corrupted! Remove it and launch unity again!\n[Position out of bounds!]");
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