#include "UnityPrefix.h"
#include "LocalFileSystemWindowsShared.h"

#include "Runtime/Utilities/FileUtilities.h"
#include "Runtime/Utilities/PathNameUtility.h"
#include "Runtime/Utilities/Word.h"
#include "Runtime/Threads/CurrentThread.h"
#include "Runtime/Threads/Thread.h"
#include "PlatformDependent/Win/PathUnicodeConversion.h"
#include "PlatformDependent/Win/WinUtils.h"
#include "Runtime/File/AsyncReadManagerMetrics.h"

#include <ShellAPI.h>
#include <direct.h>
#include <fstream>
#include <fileapi.h>

#if PLATFORM_WINRT
#include "PlatformDependent/WinRT/FileWinHelpers.h"
#elif !PLATFORM_FAMILY_WINDOWSGAMES
#include <Shlobj.h>
#include <Psapi.h>
#include <Shlwapi.h>
#endif

#include <WinBase.h>

// undo damage from windows headers
#undef CreateDirectory


static void WindowsFileTimeToUnityTime(const FILETIME& ft, DateTime& dateTime)
{
    // Windows FILETIME values are 100-nanoseconds since 1601.

    ULARGE_INTEGER timeValue;
    timeValue.LowPart = ft.dwLowDateTime;
    timeValue.HighPart = ft.dwHighDateTime;

    dateTime = DateTime(1601, 1, 1, 0, 0, 0);
    dateTime.ticks += timeValue.QuadPart;
}

namespace winutils
{
    static inline DWORD FileFlagsToWindowsAttributes(UInt32 flags)
    {
        DWORD v = 0;
        if (flags & kFileFlagTemporary)
            v |= FILE_ATTRIBUTE_TEMPORARY;
        if (flags & kFileFlagDontIndex)
            v |= FILE_ATTRIBUTE_NOT_CONTENT_INDEXED;
        if (flags & kFileFlagHidden)
            v |= FILE_ATTRIBUTE_HIDDEN;
        return v;
    }
}

LocalFileSystemWindowsShared::LocalFileSystemWindowsShared()
{
}

const char* LocalFileSystemWindowsShared::Filename(const FileEntryData& data) const
{
    const char* s = strrchr(data.m_path, '/');
    if (s == NULL)
    {
        return strrchr(data.m_path, '\\') + 1;
    }
    return s + 1;
}

bool LocalFileSystemWindowsShared::Parent(const FileEntryData& data, core::string& path) const
{
    core::string trimmed = InitPath(data.m_path);
    trimmed.erase(trimmed.find_last_not_of("\\/") + 1);
    size_t prevFolder = trimmed.find_last_of('/');
    if (prevFolder == core::string::npos)
    {
        return false;
    }
    path = trimmed.substr(0, prevFolder);
    if (path.length() < trimmed.length())
    {
        return true;
    }
    return false;
}

bool LocalFileSystemWindowsShared::IsDir(const FileEntryData& data) const
{
    TEMP_WSTRING wPath;
    UnityPathToWindowsPath(data.m_path, wPath);

    WIN32_FILE_ATTRIBUTE_DATA fileInfo;
    if (GetFileAttributesExW(wPath.c_str(), GetFileExInfoStandard, &fileInfo) == FALSE)
        return false;

    return (fileInfo.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ? true : false;
}

bool LocalFileSystemWindowsShared::IsFile(const FileEntryData& data) const
{
    TEMP_WSTRING wPath;
    UnityPathToWindowsPath(data.m_path, wPath);

    WIN32_FILE_ATTRIBUTE_DATA fileInfo;
    if (GetFileAttributesExW(wPath.c_str(), GetFileExInfoStandard, &fileInfo) == FALSE)
        return false;

    return (fileInfo.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ? false : true;
}

bool LocalFileSystemWindowsShared::IsSymLink(const FileEntryData& data) const
{
    TEMP_WSTRING wPath;
    UnityPathToWindowsPath(data.m_path, wPath);

    WIN32_FILE_ATTRIBUTE_DATA fileInfo;
    bool success = GetFileAttributesExW(wPath.c_str(), GetFileExInfoStandard, &fileInfo) == TRUE;
    data.m_lastError = GetLastFileSystemError(success);
    if (!success)
        return false;

    return (fileInfo.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) ? true : false;
}

DateTime LocalFileSystemWindowsShared::LastModified(const FileEntryData& data) const
{
    TEMP_WSTRING wPath;
    UnityPathToWindowsPath(data.m_path, wPath);

    DateTime retDate;
    WIN32_FILE_ATTRIBUTE_DATA    fileInfo;
    bool success = GetFileAttributesExW(wPath.c_str(), GetFileExInfoStandard, &fileInfo) == TRUE;
    data.m_lastError = GetLastFileSystemError(success);
    if (success)
        WindowsFileTimeToUnityTime(fileInfo.ftLastWriteTime, retDate);

    return retDate;
}

bool LocalFileSystemWindowsShared::CreateAsFile(FileEntryData& data)
{
    TempString fullFilename = GetFileSystem().ToAbsolute(data.m_path);

    TEMP_WSTRING wPath;
    UnityPathToWindowsPath(fullFilename.c_str(), wPath);

    HANDLE hfile = CreateFileW(wPath.c_str(), GENERIC_WRITE, 0, 0, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, 0);
    if (hfile != INVALID_HANDLE_VALUE)
    {
        CloseHandle(hfile);
        return true;
    }
    return false;
}

bool LocalFileSystemWindowsShared::CreateAsDir(FileEntryData& data)
{
    TEMP_WSTRING wPath;
    UnityPathToWindowsPath(data.m_path, wPath);

    bool success = ::CreateDirectoryW(wPath.c_str(), NULL) != 0;
    data.m_lastError = GetLastFileSystemError(success);

    return success;
}

bool LocalFileSystemWindowsShared::Delete(FileEntryData& data, bool recursively)
{
    TEMP_WSTRING wPath;
    UnityPathToWindowsPath(data.m_path, wPath);

    if (!IsDir(data))
    {
        // the following code will attempt to delete the file(s) up to "SHARING_VIOLATION_RETRY_COUNT" times, waiting 10 milliseconds between retries if
        // the call failed due to sharing violation
        // this is necessary to avoid file-locking hickups with the Windows file system due to anti-viruses, file indexing, or badly programmed Explorer extensions.
        for (int r = 0; r < SHARING_VIOLATION_RETRY_COUNT; r++)
        {
            bool success = DeleteFileW(wPath.c_str()) != 0;
            data.m_lastError = GetLastFileSystemError(success);

            if (success)
                return true;

            if (!FixReadOnlyFile(data, wPath.c_str()))
                return false;
        }

        return false;
    }

    // remove a directory
    if (recursively)
    {
        FileEntryInfoList childEntries;
        void* handlerData = NULL;
        Enumerate(data.m_path, &childEntries, false, &handlerData);
        for (FileEntryInfoList::iterator i = childEntries.begin(); i != childEntries.end(); i++)
        {
            FileSystemEntry item(i->m_path);
            if (!item.Delete(true))
                return false;
        }
    }

    for (int r = 0; r < SHARING_VIOLATION_RETRY_COUNT; r++)
    {
        bool success = RemoveDirectoryW(wPath.c_str()) != 0;
        data.m_lastError = GetLastFileSystemError(success);

        if (success)
            return true;

        if (!FixReadOnlyFile(data, wPath.c_str()))
            return false;
    }

    return false;
}

static inline bool IsSharingViolationErrorCode(FileSystemError errorCode)
{
    return (errorCode == kFileSystemErrorAccessDenied);
}

bool LocalFileSystemWindowsShared::FixReadOnlyFile(FileEntryData &data, const wchar_t* winPath)
{
    if (!IsSharingViolationErrorCode(data.m_lastError))
        return false;

    // perhaps the file is readonly, if so make it writable.
    DWORD attributes = GetFileAttributesW(winPath);
    if (attributes != INVALID_FILE_ATTRIBUTES)
    {
        attributes &= ~FILE_ATTRIBUTE_READONLY;
        SetFileAttributesW(winPath, attributes);
    }

    CurrentThread::SleepForSeconds(0.01);
    return true;
}

bool LocalFileSystemWindowsShared::SetFlags(FileEntryData& data, UInt32 attributeMask, UInt32 attributeValue)
{
    TEMP_WSTRING wPath;
    UnityPathToWindowsPath(data.m_path, wPath);

    const DWORD attr = GetFileAttributesW(wPath.c_str());
    if (attr == INVALID_FILE_ATTRIBUTES)
        return false;

    DWORD winMask = winutils::FileFlagsToWindowsAttributes(attributeMask);
    DWORD winValue = winutils::FileFlagsToWindowsAttributes(attributeValue);

    // only files and not directories can be temporary
    if (attr & FILE_ATTRIBUTE_DIRECTORY)
    {
        winMask &= ~FILE_ATTRIBUTE_TEMPORARY;
        winValue &= ~FILE_ATTRIBUTE_TEMPORARY;
    }

    BOOL ok = SetFileAttributesW(wPath.c_str(), (attr & (~winMask)) | winValue);
    return ok ? true : false;
}

bool LocalFileSystemWindowsShared::SetReadOnly(FileEntryData& data, bool readOnly)
{
    TEMP_WSTRING wPath;
    UnityPathToWindowsPath(data.m_path, wPath);

    DWORD attr = GetFileAttributesW(wPath.c_str());
    if (attr == INVALID_FILE_ATTRIBUTES)
        return false;

    if (readOnly)
        attr |= FILE_ATTRIBUTE_READONLY;
    else
        attr &= ~FILE_ATTRIBUTE_READONLY;

    BOOL ok = SetFileAttributesW(wPath.c_str(), attr);
    return ok ? true : false;
}

bool LocalFileSystemWindowsShared::Copy(FileEntryData& from, FileEntryData& to)
{
    TEMP_WSTRING wFromPath, wToPath;
    UnityPathToWindowsPath(from.m_path, wFromPath);
    UnityPathToWindowsPath(to.m_path, wToPath);

    return ::CopyFileW(wFromPath.c_str(), wToPath.c_str(), TRUE) != FALSE;
}

bool LocalFileSystemWindowsShared::AtomicMove(FileEntryData& from, FileEntryData& to)
{
    TEMP_WSTRING source, destination;
    UnityPathToWindowsPath(from.m_path, source);
    UnityPathToWindowsPath(to.m_path, destination);

    // refer to LocalFileSystemWindows::Delete for details surrounding the following retry loop
    for (int r = 0; r < SHARING_VIOLATION_RETRY_COUNT; r++)
    {
        bool success = MoveFileExW(source.c_str(), destination.c_str(), MOVEFILE_REPLACE_EXISTING) != 0;
        from.m_lastError = to.m_lastError = GetLastFileSystemError(success);

        if (success)
            return true;

        if (!FixReadOnlyFile(from, source.c_str()))
            return false;
        if (!FixReadOnlyFile(to, destination.c_str()))
            return false;
    }

    return false;
}

bool LocalFileSystemWindowsShared::IsAbsoluteFilePath(core::string_ref path)
{
    if (path.size() <= 1)
        return false;

    // C:/ style path.
    if (path[1] == ':')
    {
        return true;
    }

    // UNC path
    if ((path[0] == kPlatformPathNameSeparator && path[1] == kPlatformPathNameSeparator ||
         path[0] == kPathNameSeparator && path[1] == kPathNameSeparator) &&
        path[2] != '\0')
    {
        return true;
    }

    // / only root style
    if (path[0] == kPlatformPathNameSeparator || path[0] == kPathNameSeparator)
        return true;

    return false;
}

bool LocalFileSystemWindowsShared::Enumerate(const char* path, FileEntryInfoList* list, bool recurse, void** handlerData, FileEnumerationFlag flags) const
{
#if !UNITY_EDITOR
    AssertMsg((flags & kStoreSizeOnly) == 0, "Filesystem does not support storing size during enumeration outside of Unity Editor.");
#endif // UNITY_EDITOR
    WIN32_FIND_DATAW findData;
    HANDLE hFindFile;

    TempString fileSpec = path;
    TempString parentPath = path;

    if (EndsWith(fileSpec, "/"))
    {
        fileSpec += "*";
    }
    else
    {
        fileSpec += "/*";
        parentPath += "/";
    }

    TEMP_WSTRING wPath;
    UnityPathToWindowsPath(fileSpec.c_str(), wPath);

    // FindExInfoBasic can speedup file search by omitting 8.3 filenames generation
    hFindFile = FindFirstFileExW(wPath.c_str(), FindExInfoBasic, &findData, FindExSearchNameMatch, NULL, 0);
    // On WinXP FindExInfoBasic is unsupported, so we should fallback to old FindFirstFile proc
    if (hFindFile == INVALID_HANDLE_VALUE)
    {
        DWORD err = GetLastError();
        if (err == ERROR_INVALID_PARAMETER)
            hFindFile = FindFirstFileW(wPath.c_str(), &findData);
    }
    // Nothing was found
    if (hFindFile == INVALID_HANDLE_VALUE)
        return false;

    bool retValue = true;
    TempString utfFilename;
    do
    {
        utfFilename.clear();
        ConvertWideToUTF8String(findData.cFileName, utfFilename);

        // special case for . and .. directories
        bool isDir = findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY;
        if (isDir)
        {
            if (!strcmp(utfFilename.c_str(), ".") || !strcmp(utfFilename.c_str(), ".."))
                continue;
        }

        bool isHidden = ((findData.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) ? true : false) || (utfFilename[0] == '.' ? true : false);
        size_t parentPathLength = parentPath.size();
        parentPath += utfFilename;

        if (!FlaggedAsSkipped(isDir, isHidden, utfFilename, flags))
        {
            if (flags & kStoreSizeOnly)
            {
                Assert(handlerData != NULL);
                if (!isDir)
                {
                    LARGE_INTEGER li;
                    li.HighPart = findData.nFileSizeHigh;
                    li.LowPart = findData.nFileSizeLow;
                    (*(*(UInt64**)handlerData)) += (UInt64)li.QuadPart;
                }
            }
            else
            {
                FileEntryInfo& fileData = list->emplace_back();
                strcpy_truncate(fileData.m_path, parentPath);
                fileData.m_isDir = isDir;
                fileData.m_isSymLink = (findData.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) ? true : false;
                fileData.m_isHidden = isHidden;
                WindowsFileTimeToUnityTime(findData.ftLastWriteTime, fileData.m_lastModified);
            }

            if (isDir && recurse)
            {
                retValue = Enumerate(parentPath.c_str(), list, recurse, handlerData, flags);
                if (!retValue)
                    break;
            }
        }
        parentPath.resize(parentPathLength);
    }
    while (FindNextFileW(hFindFile, &findData));

    FindClose(hFindFile);

    return retValue;
}

bool LocalFileSystemWindowsShared::Exists(const FileEntryData& data) const
{
    TEMP_WSTRING wPath;
    UnityPathToWindowsPath(data.m_path, wPath);

    WIN32_FILE_ATTRIBUTE_DATA attrData;
    BOOL result = GetFileAttributesExW(wPath.c_str(), GetFileExInfoStandard, &attrData);

    return result != FALSE ? true : false;
}

bool LocalFileSystemWindowsShared::Lock(FileEntryData& data, FileLockMode lockMode)
{
    AssertMsg(lockMode != kShared, "Support for shared lock is not implemented on Windows");
    if (lockMode != kNoLock)
    {
        Close(data);
        return Open(data, kWritePermission, kSilentReturnOnOpenFail);
    }

    return Close(data);
}

bool LocalFileSystemWindowsShared::IsHidden(const FileEntryData& data) const
{
    TempString absPath = GetFileSystem().ToAbsolute(data.m_path);

    TEMP_WSTRING wPath;
    UnityPathToWindowsPath(absPath.c_str(), wPath);

    WIN32_FILE_ATTRIBUTE_DATA    fileInfo;
    bool success = GetFileAttributesExW(wPath.c_str(), GetFileExInfoStandard, &fileInfo) == TRUE;
    data.m_lastError = GetLastFileSystemError(success);
    if (!success)
        return false;

    return (fileInfo.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) ? true : false;
}

bool LocalFileSystemWindowsShared::IsLocked(const FileEntryData& data)
{
    if (!Exists(data))
        return false;

    if (IsDir(data))
    {
        // check each children file and recurse down directory tree
        FileEntryInfoList childEntries;
        void* handlerData = NULL;
        Enumerate(data.m_path, &childEntries, false, &handlerData);
        for (FileEntryInfoList::iterator i = childEntries.begin(); i != childEntries.end(); i++)
        {
            FileSystemEntry entry(i->m_path);
            if (entry.IsLocked())
                return true;
        }

        return false;
    }

    TEMP_WSTRING wPath;
    UnityPathToWindowsPath(data.m_path, wPath);

    // else, it's a file
    HANDLE hFile = CreateFileW(wPath.c_str(),
        FILE_GENERIC_READ | FILE_GENERIC_WRITE,
        0,
        NULL,
        OPEN_ALWAYS,
        NULL,
        NULL);

    bool isLocked = (hFile == INVALID_HANDLE_VALUE);
    if (!isLocked)
    {
        CloseHandle(hFile);
    }

    return isLocked;
}

bool LocalFileSystemWindowsShared::IsReadOnly(const FileEntryData& data) const
{
    TEMP_WSTRING wPath;
    UnityPathToWindowsPath(data.m_path, wPath);

    const DWORD attr = GetFileAttributesW(wPath.c_str());
    if (attr == INVALID_FILE_ATTRIBUTES)
        return false;

    return (attr & FILE_ATTRIBUTE_READONLY) != 0;
}

bool LocalFileSystemWindowsShared::Open(FileEntryData& data, FilePermission permissions,  FileAutoBehavior behavior)
{
    PROFILER_AUTO(s_ProfileFileOpen, core::string::create_from_external(data.m_path));

    DWORD accessMode, shareMode, createMode;
    switch (permissions)
    {
        case kReadPermission:
            accessMode = FILE_GENERIC_READ;
            shareMode = FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE;
            createMode = OPEN_EXISTING;
            break;
        case kWritePermission:
            accessMode = FILE_GENERIC_WRITE;
            shareMode = 0;
            createMode = CREATE_ALWAYS;
            break;
        case kAppendPermission:
            accessMode = FILE_GENERIC_WRITE;
            shareMode = 0;
            createMode = OPEN_ALWAYS;
            break;
        case kReadWritePermission:
            accessMode = FILE_GENERIC_READ | FILE_GENERIC_WRITE;
            shareMode = 0;
            createMode = OPEN_ALWAYS;
            break;
        default:
            AssertString("Unknown permission mode");
            return INVALID_HANDLE_VALUE;
    }

    HANDLE hFile = (HANDLE)data.m_AccessorData;
    if (hFile == 0 || hFile == INVALID_HANDLE_VALUE)
    {
        TEMP_WSTRING wPath;
        UnityPathToWindowsPath(data.m_path, wPath);

        hFile = CreateFileW(wPath.c_str(),
            accessMode,
            shareMode,
            NULL,
            createMode,
            NULL,
            NULL);
        data.m_AccessorData = hFile;
        data.m_lastError = GetLastFileSystemError(hFile != INVALID_HANDLE_VALUE);

        if (permissions == kAppendPermission)
        {
            if (SetFilePointer(hFile, 0, NULL, FILE_END) == INVALID_SET_FILE_POINTER)
            {
                Close(data);
                data.m_AccessorData = INVALID_HANDLE_VALUE;
                data.m_lastError = GetLastFileSystemError(false);
            }
        }
    }

    if (data.m_AccessorData == INVALID_HANDLE_VALUE)
        return false;

    // we were able to open / create the file, set this file's data handler to
    // this one so that subsequent calls to read/write and close are directly handled
    // but the same handler that opened it.
    data.m_handler = this;
    data.m_accessorHandler = this;

    return true;
}

bool LocalFileSystemWindowsShared::Read(FileEntryData& data, VFS::FileSize from, UInt64 count, void* buffer, UInt64* actual, FileReadFlags flags)
{
    HANDLE hFile = (HANDLE)data.m_AccessorData;
    if (hFile == 0 || hFile == INVALID_HANDLE_VALUE)
    {
        return false;
    }

    PROFILER_AUTO(s_ProfileFileRead, core::string::create_from_external(data.m_path), from.Cast<UInt64>(), count);

    LARGE_INTEGER distanceToMove;
    distanceToMove.QuadPart = from.Cast<UInt64>();
    if (SetFilePointerEx(hFile, distanceToMove, NULL, FILE_BEGIN) == FALSE)
    {
        data.m_lastError = GetLastFileSystemError(false);
        return false;
    }

    // ReadFile can only read max 4GB at once, so do a loop if we need to read more
    BOOL success = true;
    *actual = 0;
    while (count > 0 && success)
    {
        DWORD actualRead = 0;
        DWORD toRead = std::min<UInt64>(count, std::numeric_limits<DWORD>::max());
        success = ::ReadFile(hFile, (void*)buffer, toRead, &actualRead, NULL);
        data.m_lastError = GetLastFileSystemError(success != FALSE);
        RECORDFILEREAD(static_cast<UInt64>(actualRead), buffer);
        buffer = static_cast<char*>(buffer) + actualRead;
        *actual += actualRead;
        count -= toRead;
    }

    return success == TRUE && *actual > 0 ? true : false;
}

bool LocalFileSystemWindowsShared::Read(FileEntryData& data, UInt64 count, void* buffer, UInt64* actual, FileReadFlags flags)
{
    HANDLE hFile = (HANDLE)data.m_AccessorData;
    if (hFile == 0 || hFile == INVALID_HANDLE_VALUE)
    {
        return false;
    }

    PROFILER_AUTO(s_ProfileFileRead, core::string::create_from_external(data.m_path), 0, count);

    // ReadFile can only read max 4GB at once, so do a loop if we need to read more
    BOOL success = true;
    *actual = 0;
    while (count > 0 && success)
    {
        DWORD actualRead = 0;
        DWORD toRead = std::min<UInt64>(count, std::numeric_limits<DWORD>::max());
        success = ::ReadFile(hFile, (void*)buffer, toRead, &actualRead, NULL);
        data.m_lastError = GetLastFileSystemError(success != FALSE);
        RECORDFILEREAD(static_cast<UInt64>(actualRead), buffer);
        buffer = static_cast<char*>(buffer) + actualRead;
        *actual += actualRead;
        count -= toRead;
    }

    return success == TRUE && *actual > 0 ? true : false;
}

bool LocalFileSystemWindowsShared::Close(FileEntryData& data)
{
    HANDLE hFile = (HANDLE)data.m_AccessorData;
    if (hFile && hFile != INVALID_HANDLE_VALUE)
    {
        PROFILER_AUTO(s_ProfileFileClose, core::string::create_from_external(data.m_path));

        BOOL success = CloseHandle(hFile);
        data.m_lastError = GetLastFileSystemError(success != FALSE);
        data.m_AccessorData = NULL;
        return success != FALSE;
    }
    return true;
}

bool LocalFileSystemWindowsShared::Write(FileEntryData& data, VFS::FileSize at, UInt64 size, const void* buffer, UInt64* actual)
{
    HANDLE hFile = (HANDLE)data.m_AccessorData;
    if (hFile == 0 || hFile == INVALID_HANDLE_VALUE)
    {
        return false;
    }

    PROFILER_AUTO(s_ProfileFileWrite, core::string::create_from_external(data.m_path), at.Cast<UInt64>(), size);

    LARGE_INTEGER distanceToMove;
    distanceToMove.QuadPart = at.Cast<UInt64>();
    if (SetFilePointerEx(hFile, distanceToMove, NULL, FILE_BEGIN) == FALSE)
    {
        data.m_lastError = GetLastFileSystemError(false);
        return false;
    }

    // WriteFile can only write max 4GB at once, so do a loop if we need to write more
    BOOL success = TRUE;
    *actual = 0;
    UInt64 totalExpected = size;
    while (size > 0 && success)
    {
        DWORD actualWrite = 0;
        DWORD toWriteSize = std::min<UInt64>(size, std::numeric_limits<DWORD>::max());
        BOOL success = ::WriteFile(hFile, buffer, (DWORD)toWriteSize, &actualWrite, NULL);

        data.m_lastError = GetLastFileSystemError(success != FALSE);
        buffer = static_cast<const char*>(buffer) + actualWrite;
        size -= toWriteSize;
        *actual += actualWrite;
    }

    return success == TRUE ? (*actual == totalExpected) : false;
}

bool LocalFileSystemWindowsShared::Write(FileEntryData& data, UInt64 size, const void* buffer, UInt64* actual)
{
    HANDLE hFile = (HANDLE)data.m_AccessorData;
    if (hFile == 0 || hFile == INVALID_HANDLE_VALUE)
    {
        return false;
    }

    PROFILER_AUTO(s_ProfileFileWrite, core::string::create_from_external(data.m_path), 0, size);

    // WriteFile can only write max 4GB at once, so do a loop if we need to write more
    BOOL success = TRUE;
    *actual = 0;
    UInt64 totalExpected = size;
    while (size > 0 && success)
    {
        DWORD actualWrite = 0;
        DWORD toWriteSize = std::min<UInt64>(size, std::numeric_limits<DWORD>::max());
        BOOL success = ::WriteFile(hFile, buffer, (DWORD)toWriteSize, &actualWrite, NULL);

        data.m_lastError = GetLastFileSystemError(success != FALSE);
        buffer = static_cast<const char*>(buffer) + actualWrite;
        size -= toWriteSize;
        *actual += actualWrite;
    }
    return success == TRUE ? (*actual == totalExpected) : false;
}

bool LocalFileSystemWindowsShared::Seek(FileEntryData& data, VFS::FileOffset offset, FileOrigin origin)
{
    HANDLE hFile = (HANDLE)data.m_AccessorData;
    if (hFile == 0 || hFile == INVALID_HANDLE_VALUE)
    {
        return false;
    }

    PROFILER_AUTO(s_ProfileFileSeek, core::string::create_from_external(data.m_path), offset.Cast<SInt64>(), static_cast<SInt32>(origin));

    int originOs = SEEK_SET;
    switch (origin)
    {
        case kBeginning: originOs = FILE_BEGIN; break;
        case kCurrent: originOs = FILE_CURRENT; break;
        case kEnding: originOs = FILE_END; break;
        default: AssertMsg(false, "unknown file origin enum"); break;
    }

    LARGE_INTEGER distanceToMove;
    distanceToMove.QuadPart = offset.Cast<LONGLONG>();
    bool success = SetFilePointerEx(hFile, distanceToMove, NULL, originOs) != INVALID_SET_FILE_POINTER;
    data.m_lastError = GetLastFileSystemError(success);
    return success;
}

VFS::FileSize LocalFileSystemWindowsShared::Position(const FileEntryData& data) const
{
    VFS::FileSize pos;
    HANDLE hFile = (HANDLE)data.m_AccessorData;
    if (hFile == 0 || hFile == INVALID_HANDLE_VALUE)
        return pos;

    LARGE_INTEGER distanceToMove, newFilePointer;
    distanceToMove.QuadPart = 0;
    bool success = SetFilePointerEx(hFile, distanceToMove, &newFilePointer, FILE_CURRENT) == TRUE;
    data.m_lastError = GetLastFileSystemError(success);
    if (!success)
        return pos;

    pos = (UInt64)newFilePointer.QuadPart;
    return pos;
}

bool LocalFileSystemWindowsShared::SetLength(FileEntryData& data, VFS::FileSize newSize)
{
    HANDLE hFile = (HANDLE)data.m_AccessorData;
    if (hFile == 0 || hFile == INVALID_HANDLE_VALUE)
        return false;

    bool success = false;
    LARGE_INTEGER newPos, oldPos, endPos;
    // Get current pos
    newPos.QuadPart = 0;
    if (SetFilePointerEx(hFile, newPos, &oldPos, FILE_CURRENT) == TRUE)
    {
        // Set new pos
        newPos.QuadPart = newSize.Cast<UInt64>();
        if (SetFilePointerEx(hFile, newPos, &endPos, FILE_BEGIN) == TRUE)
        {
            // Truncate the file and get position back
            success = SetEndOfFile(hFile) == TRUE;
            SetFilePointerEx(hFile, oldPos, &endPos, FILE_BEGIN);
        }
    }

    data.m_lastError = GetLastFileSystemError(success);
    return success;
}

VFS::FileSize LocalFileSystemWindowsShared::Size(const FileEntryData& data) const
{
    TEMP_WSTRING wPath;
    UnityPathToWindowsPath(data.m_path, wPath);

    WIN32_FILE_ATTRIBUTE_DATA    fileInfo;
    bool success = GetFileAttributesExW(wPath.c_str(), GetFileExInfoStandard, &fileInfo) != 0;
    data.m_lastError = GetLastFileSystemError(success);
    if (!success)
        return (UInt64)0;

    if (!(fileInfo.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT))
    {
        LARGE_INTEGER li;
        li.HighPart = fileInfo.nFileSizeHigh;
        li.LowPart = fileInfo.nFileSizeLow;
        return (UInt64)li.QuadPart;
    }

    // for symlink, use GetFileSize(), which returns the attribute of the actual file
    HANDLE hFile = CreateFileW(wPath.c_str(),
        GENERIC_READ,
        FILE_SHARE_READ,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL);
    if (hFile != INVALID_HANDLE_VALUE)
    {
        LARGE_INTEGER newPos, oldPos, endPos;
        endPos.QuadPart = newPos.QuadPart = 0;
        if (SetFilePointerEx(hFile, newPos, &oldPos, FILE_CURRENT) == TRUE)
        {
            SetFilePointerEx(hFile, newPos, &endPos, FILE_END);
            SetFilePointerEx(hFile, oldPos, &newPos, FILE_BEGIN);
        }
        CloseHandle(hFile);
        return (UInt64)endPos.QuadPart;
    }

    data.m_lastError = GetLastFileSystemError(false);
    return (UInt64)0;
}

#if !PLATFORM_FAMILY_WINDOWSGAMES
bool LocalFileSystemWindowsShared::CreateFileWithContentAtomically(const FileEntryData& data, const void* buffer, UInt64 bufferSize) const
{
    TempString directory;
    if (!Parent(data, directory))
        return false;

    // We first write contents to a temporary file, which we rename to the desired file name atomically later
    TEMP_WSTRING wideDirectory;
    UnityPathToWindowsPath(directory.c_str(), wideDirectory);

    wchar_t tmpFile[MAX_PATH];
    if (GetTempFileNameW(wideDirectory.c_str(), L"tmp", 0, tmpFile) == 0)
    {
        data.m_lastError = GetLastFileSystemError(false);
        return false;
    }

    HANDLE hFile = CreateFileW(tmpFile, GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE)
    {
        data.m_lastError = GetLastFileSystemError(false);
        return false;
    }

    DWORD bytesWritten;
    AssertFormatMsg(bufferSize < std::numeric_limits<DWORD>::max(), "The size of %llu bytes is too large to write to a file atomically!", bufferSize);
    if (WriteFile(hFile, buffer, static_cast<DWORD>(bufferSize), &bytesWritten, NULL) == FALSE || bufferSize != bytesWritten)
    {
        data.m_lastError = GetLastFileSystemError(false);
        CloseHandle(hFile);
        return false;
    }

    FlushFileBuffers(hFile);
    CloseHandle(hFile);

    TEMP_WSTRING targetPath;
    UnityPathToWindowsPath(data.m_path, targetPath);

    // Does file exist?
    if (GetFileAttributesW(targetPath.c_str()) == INVALID_FILE_ATTRIBUTES)
    {
        // No, use MoveFileExW
        if (MoveFileExW(tmpFile, targetPath.c_str(), MOVEFILE_WRITE_THROUGH) != FALSE)
        {
            // Success!
            return true;
        }

        // Something went wrong.
        // Maybe the file got created before we had the chance to move to it? Fallback to ReplaceFileW
        // Otherwise, fail the operation
        if (GetLastError() != ERROR_ALREADY_EXISTS)
        {
            data.m_lastError = GetLastFileSystemError(false);
            return false;
        }
    }

    // File exists, use ReplaceFileW

    // We need to specify backup file name to preserve the original file in cases replacement fails
    // From MSDN:
    // ERROR_UNABLE_TO_MOVE_REPLACEMENT 1176 (0x498)
    // The replacement file could not be renamed. If lpBackupFileName was specified, the replaced and replacement files retain their original
    // file names. Otherwise, the replaced file no longer exists and the replacement file exists under its original name.
    TEMP_WSTRING backupFilePath = targetPath + L".bak";
    if (ReplaceFileW(targetPath.c_str(), tmpFile, backupFilePath.c_str(), REPLACEFILE_IGNORE_MERGE_ERRORS, NULL, NULL) == FALSE)
    {
        data.m_lastError = GetLastFileSystemError(false);
        return false;
    }

    return true;
}

#endif //!PLATFORM_FAMILY_WINDOWSGAMES

void LocalFileSystemWindowsShared::UnityPathToWindowsPath(const char* path, TEMP_WSTRING& wpath) const
{
    if (path == NULL || *path == '\0')
    {
        wpath.clear();
        return;
    }

    // To overcome MAX_PATH limitation (260 chars) we should use UNC path.
    // If we create folder the limit is MAX_PATH - 12.
    // See http://msdn.microsoft.com/en-us/library/windows/desktop/aa365247(v=vs.85).aspx#maxpath
    TempString fixedPath = const_cast<LocalFileSystemWindowsShared*>(this)->ToAbsolute(path);
    if (fixedPath.size() >= (MAX_PATH - 12))
    {
        static const char* kWindowsLongPathPrefix = "\\\\?\\";
        fixedPath = kWindowsLongPathPrefix + fixedPath;
        ConvertUnityPathName(fixedPath, wpath);

        // In some cases, the file path exceeds MAX_PATH because it contains too many relative components, i.e. "..\"
        // Calling GetFullPathNameW will convert this path into a condensed absolute path
        // See: https://docs.microsoft.com/en-us/windows/win32/api/fileapi/nf-fileapi-getfullpathnamew

        dynamic_array<WCHAR> tempPath(kMemTempAlloc);
        tempPath.resize_uninitialized(wpath.length() + 2);  // Resize buffer to hold entire path plus a wide null terminator

        if (GetFullPathNameW(wpath.c_str(), tempPath.size(), tempPath.data(), nullptr) != 0)
        {
            wpath = tempPath.data();
        }
    }
    else
    {
        ConvertUnityPathName(path, wpath);
    }
}

FileSystemError LocalFileSystemWindowsShared::GetLastFileSystemError(bool operationSuccessful) const
{
    DWORD error = GetAndUpdateLastError(operationSuccessful);
    return ConvertToFileSystemError(error);
}

FileSystemError LocalFileSystemWindowsShared::ConvertToFileSystemError(DWORD error)
{
    switch (error)
    {
        case ERROR_SUCCESS:
            return kFileSystemErrorSuccess;
        case ERROR_ACCESS_DENIED:
        case ERROR_SHARING_VIOLATION:
        case ERROR_LOCK_VIOLATION:
            return kFileSystemErrorAccessDenied;
        case ERROR_FILE_NOT_FOUND:
        case ERROR_PATH_NOT_FOUND:
            return kFileSystemErrorNotFound;
        case ERROR_NOT_SAME_DEVICE:
            return kFileSystemErrorCrossVolumeMove;
        case ERROR_NOT_SUPPORTED:
            return kFileSystemErrorNotSupported;
        case ERROR_ALREADY_EXISTS:
            return kFileSystemErrorAlreadyExists;
        default:
            return kFileSystemErrorUnknown;
    }
}

DWORD LocalFileSystemWindowsShared::GetAndUpdateLastError(bool operationSuccessful) const
{
    return m_LastError = operationSuccessful ? ERROR_SUCCESS : ::GetLastError();
}

core::string LocalFileSystemWindowsShared::InitPath(const core::string& path) const
{
    core::string cleaned = path;
    std::replace(cleaned.begin(), cleaned.end(), '\\', '/');
    return cleaned;
}

bool LocalFileSystemWindowsShared::Touch(FileEntryData& data)
{
    SYSTEMTIME systemTime;
    GetSystemTime(&systemTime);

    FILETIME fileTime;
    SystemTimeToFileTime(&systemTime, &fileTime);

    return SetFileTimeStamp(data, fileTime);
}

bool LocalFileSystemWindowsShared::SetFileTimeStamp(FileEntryData& data, const FILETIME &modifiedFileTime)
{
    TEMP_WSTRING wPath;
    UnityPathToWindowsPath(data.m_path, wPath);

    // Open file with only FILE_WRITE_ATTRIBUTES as we only need to modify its attributes, rather than its contents.
    // This way file can be opened for reading/writing without requiring share mode be set.
    HANDLE fileHandle = CreateFileW(wPath.c_str(),
        FILE_WRITE_ATTRIBUTES,
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
        0,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        0);
    data.m_lastError = GetLastFileSystemError(fileHandle != INVALID_HANDLE_VALUE);
    if (fileHandle == INVALID_HANDLE_VALUE)
        return false;

    bool success = SetFileTime(fileHandle, NULL, NULL, &modifiedFileTime) != 0;
    data.m_lastError = GetLastFileSystemError(success);

    CloseHandle(fileHandle);

    return success;
}

bool LocalFileSystemWindowsShared::SetFileTimeStamp(FileEntryData& data, const DateTime &modifiedTime)
{
    DateTime start = DateTime(1601, 1, 1, 0, 0, 0);
    ULARGE_INTEGER modifiedTimeLargeInteger;
    modifiedTimeLargeInteger.QuadPart = modifiedTime.ticks - start.ticks;

    FILETIME newFileTime{ modifiedTimeLargeInteger.LowPart, modifiedTimeLargeInteger.HighPart };

    return SetFileTimeStamp(data, newFileTime);
}

bool LocalFileSystemWindowsShared::GetAvailableDiskSpace(const char* path, UInt64& availableBytes) const
{
    UInt64 i64TotalBytes, i64TotalFreeBytes;
    // We are using the lpFreeBytesAvailable (to caller) parameter for our value since this
    // takes into account disk quota management - see https://support.microsoft.com/en-us/kb/231497

    core::wstring widePath(kMemTempAlloc);
    ConvertUTF8ToWideString(path, widePath);

    if (!GetDiskFreeSpaceExW(widePath.c_str(), (PULARGE_INTEGER)&availableBytes, (PULARGE_INTEGER)&i64TotalBytes,
        (PULARGE_INTEGER)&i64TotalFreeBytes))
    {
        const TempString lastError = winutils::ErrorCodeToMsg(GetLastError());
        printf_console("GetDiskFreeSpaceEx failed with err '%s'; path was %s\n", lastError.c_str(), path);
        return false;
    }
    return true;
}

core::string LocalFileSystemWindowsShared::GetApplicationPath() const
{
    if (m_CachedApplicationPath.empty())
    {
        dynamic_array<wchar_t> pathBuffer(kMemTempAlloc);
        DWORD dataRetrieved = 0;
        HMODULE appModule = GetHModuleForApplicationPath();
        do
        {
            pathBuffer.resize_initialized(pathBuffer.size() + MAX_PATH);
            dataRetrieved = GetModuleFileNameW(appModule, pathBuffer.begin(), pathBuffer.size());
        }
        while (dataRetrieved >= pathBuffer.size());

        pathBuffer.resize_initialized(dataRetrieved);
        ConvertWindowsPathName(pathBuffer.begin(), m_CachedApplicationPath);
    }
    return core::string(m_CachedApplicationPath, kMemTempAlloc);
}

core::string LocalFileSystemWindowsShared::GetApplicationFolder() const
{
    core::string applicationPath = GetApplicationPath();
    return TempString(DeleteLastPathNameComponent(applicationPath));
}

core::string LocalFileSystemWindowsShared::GetApplicationContentsPath() const
{
    return AppendPathName(GetApplicationFolder(), "Data");
}

core::string LocalFileSystemWindowsShared::GetUserAppCacheFolder() const
{
    TempString path = GetUserAppDataFolder();
    if (path.empty())
        return path;

    path = AppendPathName(path, "Caches");
    CreateDirectory(path);
    return path;
}

core::string LocalFileSystemWindowsShared::LastErrorMessage()
{
    return winutils::ErrorCodeToMsg(LastError());
}

int LocalFileSystemWindowsShared::LastError()
{
    return (int)m_LastError;
}
