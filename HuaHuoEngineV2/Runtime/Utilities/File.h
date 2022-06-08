//
// Created by VincentZhang on 6/4/2022.
//

#ifndef HUAHUOENGINEV2_FILE_H
#define HUAHUOENGINEV2_FILE_H
#include <string>
#include "NonCopyable.h"
#include "EnumFlags.h"

enum FilePermission     { kReadPermission = 0, kWritePermission = 1, kReadWritePermission = 2, kAppendPermission = 3 };
enum FileAutoBehavior   { kNormalBehavior = 0, kSilentReturnOnOpenFail = 1 << 0, kRetryOnOpenFail = 1 << 1, kBehaviorOpen = 1 << 16, kBehaviorCreate = 2 << 16, kBehaviorTruncate = 4 << 16 };
enum FileLockMode       { kShared = 1, kExclusive = 2, kNoLock = 8 };
enum FileOrigin         { kBeginning = 1, kCurrent = 2, kEnding = 3 };

enum FileEnumerationFlag
{
    kEnumerateAll = 0,
    kSkipHiddenFiles = 1 << 1,
    kSkipHiddenDirectories = 1 << 2,
    kSkipVisibleFiles = 1 << 3,
    kSkipVisibleDirectories = 1 << 4,
    kSkipBlacklist = 1 << 5,
    kSkipTmpFiles = 1 << 6,
    kStoreSizeOnly = 1 << 7,
    kSkipFiles = kSkipHiddenFiles | kSkipVisibleFiles,
    kSkipDirectories = kSkipHiddenDirectories | kSkipVisibleDirectories,
    kVisibleFilesOnly = kSkipDirectories | kSkipHiddenFiles,
    kValidProjectFilesOnly = kSkipBlacklist | kSkipHiddenDirectories | kSkipTmpFiles,
    kValidProjectFilesSizeOnly = kSkipBlacklist | kSkipHiddenDirectories | kSkipTmpFiles | kStoreSizeOnly,
    kAllFilesSizeOnly = kEnumerateAll | kStoreSizeOnly,
    kNoRecursion = kSkipDirectories
};

enum FileSystemError
{
    kFileSystemErrorSuccess = 0,
    kFileSystemErrorInvalidPath,
    kFileSystemErrorNotFound,
    kFileSystemErrorMediaNotFound,       // SD card may be damaged or ejected.
    kFileSystemErrorMediaInvalid,        // You need to close all files, unmount+remount the SDCARD, and try again.
    kFileSystemErrorMediaCorrupted,      // The archive has been tampered with; the save-data needs deleted and re-created.
    kFileSystemErrorMediaAccessError,         // You may have to re-insert the SDCARD and try again.
    kFileSystemErrorAlreadyExists,
    kFileSystemErrorWriteProtected,
    kFileSystemErrorAccessDenied,
    kFileSystemErrorCrossVolumeMove,
    kFileSystemErrorOutOfSpace,
    kFileSystemErrorOutOfMemory,
    kFileSystemErrorBusy,                // Only for "host" filesystem.
    kFileSystemErrorUnknown,
    kFileSystemErrorNotSupported         // Unspecified internal error.
};

enum FileReadFlags
{
    kFileReadNoFlags = 0,

    kFileReadDestinationBufferWriteOnly = 1 << 0, // Destination memory should not be read from (e.g. if it is GPU memory which is slow to read)
};

ENUM_FLAGS(FileEnumerationFlag);
ENUM_FLAGS(FileAutoBehavior);
ENUM_FLAGS(FileReadFlags);

bool CreateDirectory(std::string pathName);

std::string PathToAbsolutePath(std::string path);
inline std::string PathToAbsolutePath(const char* path) { return PathToAbsolutePath(std::string(path)); }
inline std::string PathToAbsolutePathFromScript(const char* path) { return PathToAbsolutePath(std::string(path)); }

bool IsFileCreated(std::string path);

bool IsDirectoryCreated(std::string path);
bool IsDirectoryEmpty(std::string path);

bool IsPathCreated(std::string path);

void CreateFile(std::string path);

size_t GetFileLength(std::string path);

class MemoryFileAccessor;
class File : public NonCopyable
{
//    FileSystemEntry* m_entry;
//    FileAccessor*    m_accessor;

    bool isMemoryFile;

    size_t    m_Position;
    std::string     m_Path;
    FILE* m_FileAccessor;
    MemoryFileAccessor* m_MemFileAccessor;

public:
    File();
    ~File();

    bool Open(std::string path, FilePermission perm, FileAutoBehavior behavior = kNormalBehavior);
    // bool OpenFileSystemEntry(const FileSystemEntry& fileSystemEntry, FilePermission permission, FileAutoBehavior behavior = kNormalBehavior);
    bool Close();

    bool IsValid();

    size_t Read(void* buffer, size_t size, FileReadFlags flags = kFileReadNoFlags);
    size_t Read(size_t position, void* buffer, size_t size, FileReadFlags flags = kFileReadNoFlags);

    bool Write(const void* buffer, size_t size);
    bool Write(size_t pos, const void* buffer, size_t size);
    bool SetFileLength(size_t size);
    size_t GetFileLength();
    size_t GetPosition() const { return m_Position; }
    bool Seek(size_t position, FileOrigin origin);

    void SetMemoryLabel(MemLabelId label) { /*m_Path.set_memory_label(label);*/ }

    enum LockMode
    {
        kNone = 8,
        kShared = 1,
        kExclusive = 2
    };

    bool Lock(File::LockMode mode, bool block);
};

// On Vista, its search indexer can wreak havoc on files that are written then renamed. Full story here:
// http://stackoverflow.com/questions/153257/random-movefileex-failures-on-vista-access-denied-looks-like-caused-by-search-i
// but in short, whenever a file is temporary or should be excluded from search indexing, we have to set those flags.
enum FileFlags
{
    kFileFlagTemporary = (1 << 0), // File is meant to be temporary.
    kFileFlagDontIndex = (1 << 1), // Exclude file from search indexing (Spotlight, Vista Search, ...)
    kFileFlagHidden    = (1 << 2), // Set file as hidden file.
    kFileFlagNoBackup  = (1 << 3), // Don't make backups of this file. So far used only on iOS.

    kAllFileFlags = kFileFlagTemporary | kFileFlagDontIndex | kFileFlagNoBackup
};

#if HUAHUO_EDITOR
std::string GenerateUniquePathSafe(std::string inPath);
std::string GenerateUniquePath(std::string inPath);
#endif
#endif //HUAHUOENGINEV2_FILE_H
