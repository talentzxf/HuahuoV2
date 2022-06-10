#include "OpenFileCache.h"

OpenFileCache::OpenFileCache(MemLabelRef label)
    : m_MemLabel(label)
{
    m_TimeStamp = 0;
    for (int i = 0; i < kOpenedFileCacheCount; i++)
    {
        m_TimeStamps[i] = 0;
        // m_Filenames[i].set_memory_label(label);
        m_Cache[i].SetMemoryLabel(label);
    }
}

OpenFileCache::~OpenFileCache()
{
    ForceCloseAll();
}

File* OpenFileCache::OpenCached(const std::string& thePath)
{
    if (thePath.empty())
    {
        return nullptr;
    }

    m_TimeStamp++;

    // find cache, don't do anything if we are in the cache
    for (int i = 0; i < kOpenedFileCacheCount; i++)
    {
        if (thePath == m_Filenames[i])
        {
            m_TimeStamps[i] = m_TimeStamp;
            Assert(m_Cache[i].IsValid());
            return &m_Cache[i];
        }
    }

    // Find Least recently used cache entry
    UInt32 lruTimeStamp = m_TimeStamps[0];
    int lruIndex = 0;
    for (int i = 1; i < kOpenedFileCacheCount; i++)
    {
        if (m_TimeStamps[i] < lruTimeStamp)
        {
            lruTimeStamp = m_TimeStamps[i];
            lruIndex = i;
        }
    }

    // replace the least recently used cache entry
    if (m_Cache[lruIndex].IsValid())
    {
        m_Cache[lruIndex].Close();
    }

    if (!m_Cache[lruIndex].Open(thePath, kReadPermission))
    {
        ErrorString(Format("Could not open file %s for read", thePath.c_str()));
        m_TimeStamps[lruIndex] = 0;
        m_Filenames[lruIndex].clear();
        return NULL;
    }

    m_TimeStamps[lruIndex] = m_TimeStamp;
    m_Filenames[lruIndex] = thePath;

    return &m_Cache[lruIndex];
}

void OpenFileCache::ForceCloseAll()
{
    // Find and close cache
    for (int i = 0; i < kOpenedFileCacheCount; i++)
    {
        if (m_Cache[i].IsValid())
        {
            m_Cache[i].Close();
            m_Filenames[i].clear();
            m_TimeStamps[i] = 0;
        }
    }
}

void OpenFileCache::ForceClose(const std::string& path)
{
    if (path.empty())
    {
        return;
    }

    // Find and close cache
    for (int i = 0; i < kOpenedFileCacheCount; i++)
    {
        if (m_Filenames[i] == path)
        {
            Assert(m_Cache[i].IsValid());
            m_Cache[i].Close();
            m_Filenames[i].clear();
            m_TimeStamps[i] = 0;
            return;
        }
    }
}
