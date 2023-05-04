#pragma once
#include "Configuration/IntegerDefinitions.h"
UInt32 CRCBegin();
UInt32 CRCDone(UInt32 crc);
UInt32 CRCFeed(UInt32 crc, const UInt8* buffer, size_t len);

// CRCFeed is very slow. ComputeHash32 is preferable.
template<typename T>
inline UInt32 CRCFeed(UInt32 crc, const T& value)
{
    return CRCFeed(crc, reinterpret_cast<const UInt8*>(&value), sizeof(T));
}
