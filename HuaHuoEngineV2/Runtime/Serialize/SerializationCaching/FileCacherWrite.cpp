
#include "FileCacherWrite.h"
#include "Memory/MemoryMacros.h"
#include <limits>



FileCacherWrite::FileCacherWrite()
{
    m_Success = true;
    m_Block = std::numeric_limits<size_t>::max();
    m_Locked = false;
    m_CacheSize = 0;
    m_DataCache = NULL;
}

bool FileCacherWrite::InitWriteFile(const std::string& pathName, size_t cacheSize)
{
    // __FAKEABLE_METHOD__(FileCacherWrite, InitWriteFile, (pathName, cacheSize));

    m_Path = PathToAbsolutePath(pathName);
    m_Success = true;

    if (!m_File.Open(m_Path, kWritePermission))
    {
        m_Success = false;
        return false;
    }

//    // file we're writing to always is a temporary, non-indexable file
//    SetFileFlags(m_Path, kAllFileFlags, kFileFlagDontIndex | kFileFlagTemporary);

    m_Block = std::numeric_limits<size_t>::max();
    m_Locked = false;
    m_CacheSize = cacheSize;
    m_DataCache = (UInt8*)HUAHUO_MALLOC(kMemTempAlloc, m_CacheSize);

    return true;
}

bool FileCacherWrite::CompleteWriting(size_t size)
{
    // __FAKEABLE_METHOD__(FileCacherWrite, CompleteWriting, (size));

    Assert(m_Block != std::numeric_limits<size_t>::max());

    size_t remainingData = size - (m_Block * m_CacheSize);
    Assert(remainingData <= m_CacheSize);

    m_Success &= m_File.Write((UInt64)m_Block * m_CacheSize, m_DataCache, remainingData);

    return m_Success;
}

bool FileCacherWrite::WriteHeaderAndCloseFile(void* data, size_t position, size_t size)
{
    // __FAKEABLE_METHOD__(FileCacherWrite, WriteHeaderAndCloseFile, (data, position, size));

    Assert(position == 0);
    if (size != 0)
        m_Success &= m_File.Write((UInt64)position, data, size);
    m_Success &= m_File.Close();

    return m_Success;
}

FileCacherWrite::~FileCacherWrite()
{
    if (m_DataCache)
    {
        HUAHUO_FREE(kMemTempAlloc, m_DataCache);
        m_DataCache = NULL;
    }

    m_File.Close();
}

void FileCacherWrite::LockCacheBlock(size_t block, UInt8** startPos, UInt8** endPos)
{
    // __FAKEABLE_METHOD__(FileCacherWrite, LockCacheBlock, (block, startPos, endPos));

    Assert(block != -1);
    Assert(block == m_Block || m_Block + 1 == block);
    AssertMsg(!m_Locked, "File cache is already locked");

    if (m_Block != block)
    {
        if (m_Block != std::numeric_limits<size_t>::max())
            m_Success &= m_File.Write(m_DataCache, m_CacheSize);

        m_Block = block;
    }

    *startPos = m_DataCache;
    *endPos = m_DataCache + m_CacheSize;
    m_Locked = true;
}

void FileCacherWrite::UnlockCacheBlock(size_t block)
{
    Assert(block == m_Block);
    AssertMsg(m_Locked, "File cache is already unlocked");

    m_Locked = false;
}

std::string FileCacherWrite::GetPathName() const
{
    return m_Path;
}

