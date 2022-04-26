//
// Created by VincentZhang on 4/26/2022.
//

#include "CachedWriter.h"
#include "CacheWriterBase.h"
#include "Logging/LogAssert.h"
#include "memcpy_constrained.h"

void CachedWriter::SetPosition(size_t position)
{
    size_t cacheSize = m_ActiveWriter.cacheBase->GetCacheSize();
    size_t newBlock = position / (UInt64)cacheSize;
    if (newBlock != m_ActiveWriter.block)
    {
        AssertMsg(newBlock == m_ActiveWriter.block + 1, "CachedWriter::SetPosition trying to move more than one block ahead, which is not allowed");

        m_ActiveWriter.cacheBase->UnlockCacheBlock(m_ActiveWriter.block);
        m_ActiveWriter.block = newBlock;
        m_ActiveWriter.cacheBase->LockCacheBlock(m_ActiveWriter.block, &m_ActiveWriter.cacheStart, &m_ActiveWriter.cacheEnd);
    }
    m_ActiveWriter.cachePosition = m_ActiveWriter.cacheStart + (position - m_ActiveWriter.block * cacheSize);
}


void CachedWriter::UpdateWriteCache(const void* data, size_t size)
{
    Assert(m_ActiveWriter.cacheBase != NULL);
    AssertFormatMsg(size <= m_ActiveWriter.cacheBase->GetCacheSize(), "Trying to write %i bytes into a CachedWriter, but that's bigger than the underlying block size of %i", (int)size, (int)m_ActiveWriter.cacheBase->GetCacheSize());

    size_t position = GetPosition();
    size_t cacheSize = m_ActiveWriter.cacheBase->GetCacheSize();
    // copy data from oldblock
    memcpy_constrained_dst(m_ActiveWriter.cachePosition, data, size, m_ActiveWriter.cacheStart, m_ActiveWriter.cacheEnd);

    SetPosition(position + (UInt64)size);

    // copy data new block
    UInt8* cachePosition = m_ActiveWriter.cacheStart + (position - m_ActiveWriter.block * cacheSize);
    memcpy_constrained_dst(cachePosition, data, size, m_ActiveWriter.cacheStart, m_ActiveWriter.cacheEnd);
}

size_t CachedWriter::ActiveWriter::GetPosition() const
{
    return cachePosition - cacheStart + block * cacheBase->GetCacheSize();
}

void CachedWriter::InitWrite(CacheWriterBase& cacher)
{
    InitActiveWriter(m_ActiveWriter, cacher);

#if UNITY_EDITOR
    m_DefaultWriter = m_ActiveWriter;
    m_ActiveResourceImageMode = kResourceImageNotSupported;
#endif
}

void CachedWriter::InitActiveWriter(ActiveWriter& activeWriter, CacheWriterBase& cacher)
{
    Assert(activeWriter.block == -1);

    activeWriter.cacheBase = &cacher;
    activeWriter.block = 0;
    activeWriter.cacheBase->LockCacheBlock(activeWriter.block, &activeWriter.cacheStart, &activeWriter.cacheEnd);
    activeWriter.cachePosition = activeWriter.cacheStart;
}