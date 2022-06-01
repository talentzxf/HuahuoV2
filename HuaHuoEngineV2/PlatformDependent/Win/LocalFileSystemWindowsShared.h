#pragma once

#include "Runtime/VirtualFileSystem/VirtualFileSystem.h"
#include "Runtime/VirtualFileSystem/LocalFileSystem.h"
#include "Runtime/Threads/ThreadSpecificValue.h"

// Note: This file is shared by both Windows Desktop targets, WinRT targets - like Windows Store Apps, Windows Phone Blue, and XBoxOne CGBatchPlugin (PlatformDependent\XboxOne\Jam\XboxOneCgBatchPlugin.jam)
// Many of these functions are identical on these platforms, so to avoid code duplication, shared code is placed here
// ToDo: Add more shared code

#define SHARING_VIOLATION_RETRY_COUNT   5

class LocalFileSystemWindowsShared : public LocalFileSystemHandler
{
public:
    LocalFileSystemWindowsShared();

    virtual const char* Filename(const FileEntryData& data) const;
    virtual bool        Parent(const FileEntryData& data, core::string& path) const;
    virtual bool        IsDir(const FileEntryData& data) const;
    virtual bool        IsFile(const FileEntryData& data) const;
    bool                IsSymLink(const FileEntryData& data) const;
    virtual DateTime    LastModified(const FileEntryData& data) const;
    virtual bool        CreateAsFile(FileEntryData& data);
    virtual bool        CreateAsDir(FileEntryData& data);
    virtual bool        Delete(FileEntryData& data, bool recursively);
    virtual bool        SetFlags(FileEntryData& data, UInt32 attributeMask, UInt32 attributeValue);
    virtual bool        SetReadOnly(FileEntryData& data, bool readOnly);
    virtual bool        Copy(FileEntryData& from, FileEntryData& to);
    virtual bool        AtomicMove(FileEntryData& from, FileEntryData& to);
    virtual bool        IsAbsoluteFilePath(core::string_ref path);
    virtual bool        Enumerate(const char* path, FileEntryInfoList*, bool recurse, void** handlerData, FileEnumerationFlag flags = kEnumerateAll) const;
    virtual bool        Exists(const FileEntryData& data) const;
    virtual bool        Lock(FileEntryData& data, FileLockMode lockMode);
    virtual bool        IsHidden(const FileEntryData& data) const;
    virtual bool        IsLocked(const FileEntryData& data);
    virtual bool        IsReadOnly(const FileEntryData& data) const;

    virtual bool        Open(FileEntryData& data, FilePermission permissions,  FileAutoBehavior behavior);
    virtual bool        Close(FileEntryData& data);
    virtual bool        Read(FileEntryData& data, VFS::FileSize from, UInt64 count, void* buffer, UInt64* actual, FileReadFlags flags);
    virtual bool        Read(FileEntryData& data, UInt64 size, void* buffer, UInt64* actual, FileReadFlags flags);
    virtual bool        Write(FileEntryData& data, VFS::FileSize at, UInt64 size, const void* buffer, UInt64* actual);
    virtual bool        Write(FileEntryData& data, UInt64 size, const void* buffer, UInt64* actual);
    virtual bool        Seek(FileEntryData& data, VFS::FileOffset offset, FileOrigin origin);
    virtual VFS::FileSize      Position(const FileEntryData& data) const;
    virtual bool        SetLength(FileEntryData& data, VFS::FileSize newSize);
    virtual VFS::FileSize   Size(const FileEntryData& data) const;
#if !PLATFORM_FAMILY_WINDOWSGAMES
    virtual bool        CreateFileWithContentAtomically(const FileEntryData& data, const void* buffer, UInt64 bufferSize) const;
#endif //!PLATFORM_FAMILY_WINDOWSGAMES

    virtual bool        Touch(FileEntryData& data);
    virtual bool        SetFileTimeStamp(FileEntryData& data, const DateTime &modifiedTime);
    virtual bool        GetAvailableDiskSpace(const char* path, UInt64& availableBytes) const;

    virtual core::string    GetApplicationPath() const;
    virtual core::string    GetApplicationFolder() const;
    virtual core::string    GetApplicationContentsPath() const;
    virtual core::string    GetUserAppCacheFolder() const;

    virtual core::string    LastErrorMessage();
    virtual int         LastError();
protected:
    // Convert Unity's path to Windows unicode path
    void UnityPathToWindowsPath(const char* path, TEMP_WSTRING& wpath) const;
    bool FixReadOnlyFile(FileEntryData &data, const wchar_t* winPath);

    // Convert WinAPI error code returned by GetAndUpdateLastError() to FileSystemError
    FileSystemError GetLastFileSystemError(bool operationSuccessful) const;
    static FileSystemError ConvertToFileSystemError(DWORD error);

    // Update last system error value and return it
    virtual DWORD GetAndUpdateLastError(bool operationSuccessful) const;
    core::string InitPath(const core::string& path) const;

    // Error code of the last WinAPI operation, stored as a TLS value to provide cached GetLastError result
    // in case GetLastError can be modified by further operation (clean up after failed operation, etc.)
    mutable UNITY_TLS_VALUE(DWORD) m_LastError;

    virtual HMODULE GetHModuleForApplicationPath() const = 0;

    bool SetFileTimeStamp(FileEntryData& data, const FILETIME &modifiedFileTime);

private:
    mutable core::string m_CachedApplicationPath;
};
