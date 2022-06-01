#pragma once
#include <cstdlib>
#include <string>
#include "BaseClasses/BaseTypes.h"

class CacheWriterBase
{
public:
    virtual ~CacheWriterBase();

    virtual bool CompleteWriting(size_t size) = 0;
    virtual bool WriteHeaderAndCloseFile(void* data, size_t position, size_t size);

    virtual void LockCacheBlock(size_t block, UInt8** startPos, UInt8** endPos) = 0;
    virtual void UnlockCacheBlock(size_t block) = 0;

    virtual void PreallocateForWrite(size_t block, UInt8** startPos, UInt8** endPos, size_t sizeOfWrite) {}

    virtual size_t GetCacheSize() = 0;      // Block size, not actual size

    virtual std::string GetPathName() const = 0;
    virtual UInt8* GetAddressOfMemory();
};
