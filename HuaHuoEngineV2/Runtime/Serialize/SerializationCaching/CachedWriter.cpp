//
// Created by VincentZhang on 4/26/2022.
//

#include "CachedWriter.h"
#include "CacheWriterBase.h"
#include "Logging/LogAssert.h"
#include "memcpy_constrained.h"
#include "Utilities/Align.h"

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

bool CachedWriter::CompleteWriting()
{
    m_ActiveWriter.cacheBase->UnlockCacheBlock(m_ActiveWriter.block);

    bool success = m_ActiveWriter.cacheBase->CompleteWriting(m_ActiveWriter.GetPosition());

//#if UNITY_EDITOR
//    if (m_ActiveResourceImageMode != kResourceImageNotSupported)
//    {
//        for (int i = 0; i < kNbResourceImages; i++)
//        {
//            if (!m_ResourceImageWriters[i].cacheBase)
//                continue;
//            success &= m_ResourceImageWriters[i].cacheBase->CompleteWriting(m_ResourceImageWriters[i].GetPosition());
//            success &= m_ResourceImageWriters[i].cacheBase->WriteHeaderAndCloseFile(NULL, (UInt64)0, 0);
//        }
//    }
//#endif

    return success;
}

void CachedWriter::PreallocateForWrite(size_t sizeOfWrite)
{
    size_t position = GetPosition();
    m_ActiveWriter.cacheBase->PreallocateForWrite(m_ActiveWriter.block, &m_ActiveWriter.cacheStart, &m_ActiveWriter.cacheEnd, sizeOfWrite);
    m_ActiveWriter.cachePosition = position - m_ActiveWriter.block * m_ActiveWriter.cacheBase->GetCacheSize() + m_ActiveWriter.cacheStart;
}

void CachedWriter::Write(const void* data, size_t size)
{
    if (m_ActiveWriter.cachePosition + size < m_ActiveWriter.cacheEnd)
    {
        memcpy(m_ActiveWriter.cachePosition, data, size);
        m_ActiveWriter.cachePosition += size;
    }
    else
    {
        PreallocateForWrite(size);
        while (size != 0)
        {
            size_t curWriteSize = std::min(size, m_ActiveWriter.cacheBase->GetCacheSize());
            UpdateWriteCache(data, curWriteSize);
            (UInt8*&)data += curWriteSize;
            size -= curWriteSize;
        }
    }
}

void CachedWriter::Align4Write()
{
    UInt32 leftOver = Align4LeftOver(m_ActiveWriter.cachePosition - m_ActiveWriter.cacheStart);
    UInt8 value = 0;
    for (UInt32 i = 0; i < leftOver; i++)
        Write(value);
}