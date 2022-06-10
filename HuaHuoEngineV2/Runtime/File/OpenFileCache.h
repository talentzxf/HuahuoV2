#pragma once

#include "Utilities/File.h"

class OpenFileCache
{
    enum { kOpenedFileCacheCount = 10 };

    MemLabelId    m_MemLabel;
    File          m_Cache[kOpenedFileCacheCount];
    std::string   m_Filenames[kOpenedFileCacheCount];
    UInt32        m_TimeStamps[kOpenedFileCacheCount];
    UInt32        m_TimeStamp;

public:
    explicit OpenFileCache(MemLabelRef label);
    ~OpenFileCache();

    File* OpenCached(const std::string& thePath);
    void ForceCloseAll();
    void ForceClose(const std::string& thePath);
};
