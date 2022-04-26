#include "CacheWriterBase.h"
#include "Logging/LogAssert.h"

CacheWriterBase::~CacheWriterBase()
{
}

bool CacheWriterBase::WriteHeaderAndCloseFile(void* data, size_t position, size_t size)
{
    AssertString("Only used for writing serialized files");
    return false;
}

UInt8* CacheWriterBase::GetAddressOfMemory()
{
    // ErrorString("GetAddressOfMemory called on CacheWriterBase which does not support it");
    return NULL;
}
