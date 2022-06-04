//
// Created by VincentZhang on 6/4/2022.
//

#ifndef HUAHUOENGINEV2_FILECACHERREAD_H
#define HUAHUOENGINEV2_FILECACHERREAD_H
#include "CacheReaderBase.h"
#include "Memory/AllocatorLabels.h"

class FileCacherRead : public CacheReaderBase {
public:
    FileCacherRead(MemLabelId memLabel, const std::string& pathName, size_t cacheSize, bool prefetchNextBlock);
    ~FileCacherRead();

    virtual size_t GetFileLength() const         { /*__FAKEABLE_METHOD__(FileCacherRead, GetFileLength, ());*/  return m_FileSize; }
    virtual size_t GetCacheSize() const          { return m_CacheSize; }

    virtual void LockCacheBlock(size_t block, UInt8** startPos, UInt8** endPos);
    virtual void UnlockCacheBlock(size_t block);

    virtual void DirectRead(void* data, size_t position, size_t size);
    virtual std::string GetPathName() const;
private:
    MemLabelId m_MemLabel;
    size_t m_FileSize;
    size_t m_CacheSize;
    std::string m_Path;
};


#endif //HUAHUOENGINEV2_FILECACHERREAD_H
