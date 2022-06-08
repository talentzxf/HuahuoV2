#pragma once

#include "SwapEndianBytes.h"
#include "Logging/LogAssert.h"

inline void SwapEndianArray(void* data, int bytesPerComponent, size_t count)
{
    if (bytesPerComponent == 2)
    {
        UInt16* p = (UInt16*)data;
        for (size_t i = 0; i < count; i++)
            SwapEndianBytes(*p++);
    }
    else if (bytesPerComponent == 4)
    {
        UInt32* p = (UInt32*)data;
        for (size_t i = 0; i < count; i++)
            SwapEndianBytes(*p++);
    }
    else
    {
        Assert(bytesPerComponent == 1);
    }
}
