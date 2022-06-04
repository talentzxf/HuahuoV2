#pragma once


#include "Utilities/File.h"
#include "CacheWriterBase.h"

/// Used by SerializedFile to write to disk.
/// Currently it doesn't allow any seeking that is, you can only write blocks in consecutive order
class FileCacherWrite : public CacheWriterBase
{
public:
    FileCacherWrite();
    bool InitWriteFile(const std::string& pathName, size_t cacheSize);

    virtual ~FileCacherWrite();

    virtual void LockCacheBlock(size_t block, UInt8** startPos, UInt8** endPos);
    virtual void UnlockCacheBlock(size_t block);

    virtual bool WriteHeaderAndCloseFile(void* data, size_t position, size_t size);

    virtual bool CompleteWriting(size_t size);

    virtual size_t  GetCacheSize() { return m_CacheSize; }
    virtual std::string GetPathName() const;

private:
    size_t m_Block;
    UInt8* m_DataCache;
    size_t m_CacheSize;

    File m_File;
    bool m_Success;
    bool m_Locked;
    std::string m_Path;
};

