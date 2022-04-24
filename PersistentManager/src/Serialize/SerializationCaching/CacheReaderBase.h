//
// Created by VincentZhang on 4/24/2022.
//

#ifndef PERSISTENTMANAGER_CACHEREADERBASE_H
#define PERSISTENTMANAGER_CACHEREADERBASE_H
#include "baselib/include/IntegerDefinitions.h"
#include <cstdio>
#include <string>

class CacheReaderBase
{
public:

    virtual ~CacheReaderBase();

    virtual void DirectRead(void* data, size_t position, size_t size) = 0;
    virtual void LockCacheBlock(size_t block, UInt8** startPos, UInt8** endPos) = 0;
    virtual void UnlockCacheBlock(size_t block) = 0;

    virtual size_t GetCacheSize() const = 0;
    virtual std::string GetPathName() const = 0;
    virtual size_t GetFileLength() const = 0;
    virtual UInt8* GetAddressOfMemory();
};



#endif //PERSISTENTMANAGER_CACHEREADERBASE_H
