//
// Created by VincentZhang on 4/24/2022.
//

#include "CacheReaderBase.h"
#include "Logging/LogAssert.h"

CacheReaderBase::~CacheReaderBase()
{}

UInt8* CacheReaderBase::GetAddressOfMemory()
{
    // ErrorString("GetAddressOfMemory called on CacheReaderBase which does not support it");
    return NULL;
}