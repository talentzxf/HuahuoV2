#pragma once

#include "Runtime/VirtualFileSystem/LocalFileSystemPosix.h"

class LocalFileSystemWebGL : public LocalFileSystemPosix
{
public:
    virtual     bool        Delete(FileEntryData& data, bool recursively);
    virtual     bool        Copy(FileEntryData& from, FileEntryData& to);
    virtual     bool        AtomicMove(FileEntryData& from, FileEntryData& to);
    virtual     bool        Close(FileEntryData& data);
};
