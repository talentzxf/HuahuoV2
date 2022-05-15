#include "LocalFileSystemWebGL.h"
#include "JSBridge.h"

bool LocalFileSystemWebGL::Delete(FileEntryData& data, bool recursively)
{
    bool result = LocalFileSystemPosix::Delete(data, recursively);
    JS_FileSystem_Sync();
    return result;
}

bool LocalFileSystemWebGL::Copy(FileEntryData& from, FileEntryData& to)
{
    bool result = LocalFileSystemPosix::Copy(from, to);
    JS_FileSystem_Sync();
    return result;
}

bool LocalFileSystemWebGL::AtomicMove(FileEntryData& from, FileEntryData& to)
{
    bool result = LocalFileSystemPosix::AtomicMove(from, to);
    JS_FileSystem_Sync();
    return result;
}

bool LocalFileSystemWebGL::Close(FileEntryData& data)
{
    bool result = LocalFileSystemPosix::Close(data);
    JS_FileSystem_Sync();
    return result;
}
