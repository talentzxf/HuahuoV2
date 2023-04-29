#pragma once
#include "Serialize/SerializeUtility.h"

struct StreamingInfo
{
    size_t   offset;
    UInt32          size;
    std::string        path;

    bool IsValid() const { return !path.empty(); }

    void Reset() { size = 0; path = ""; }

    StreamingInfo() { Reset(); }

    DECLARE_SERIALIZE_NO_PPTR(StreamingInfo)
};

template<class T>
void StreamingInfo::Transfer(T& transfer)
{
    transfer.SetVersion(2);

    if (transfer.IsReading() && transfer.IsVersionSmallerOrEqual(1))
    {
        UInt32 value;
        transfer.Transfer(value, "offset");
        offset = (UInt64)value;
    }
    else
    {
        UInt64 x = offset;
        transfer.Transfer(x, "offset");
        offset = x;
    }

    transfer.Transfer(size, "size");
    transfer.Transfer(path, "path");
}
