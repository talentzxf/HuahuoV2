//
// Created by VincentZhang on 6/4/2022.
//

#ifndef HUAHUOENGINEV2_FILECACHERREAD_H
#define HUAHUOENGINEV2_FILECACHERREAD_H
#include "CacheReaderBase.h"
#include "Memory/AllocatorLabels.h"
#include "File/AsyncReadManager.h"

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
    struct CacheBlock
    {
        UInt8*           data;
        int              block;
        int              locked;

        CacheBlock() { data = NULL; block = -1; locked = 0; }
    };
    void DebugLinearFileAccess(size_t position, size_t size);

    int RequestBlock(int block);
    void SyncReadCommandBlock(int index);
    bool Request(int block, int readCmdIndex, CacheBlock& cacheBlock, bool sync);
    void AllocateBlock(CacheBlock& block);
    void DeallocateBlock(CacheBlock& block);

    bool m_PrefetchNextBlock;
    MemLabelId m_MemLabel;
    size_t m_FileSize;
    size_t m_CacheSize;
    std::string m_Path;

    enum { kCacheCount = 2 };
    CacheBlock       m_ActiveBlocks[kCacheCount];
    AsyncReadCommand m_ReadCommands[kCacheCount];
    AsyncReadCommand m_DirectReadCommands;
};


#endif //HUAHUOENGINEV2_FILECACHERREAD_H
