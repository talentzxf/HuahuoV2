#include "UnityPrefix.h"
#include "LocalFileSystemWindows.h"
#include "Runtime/Utilities/FileUtilities.h"
#include "Runtime/Utilities/PathNameUtility.h"
#include "Runtime/Misc/SystemInfo.h"
#include "PlatformDependent/Win/PathUnicodeConversion.h"
#include <ShellAPI.h>
#include <Shlobj.h>
#include <Shlwapi.h>
#include <direct.h>
// undo damage from windows headers
#undef CreateDirectory
#undef SetCurrentDirectory

LocalFileSystemWindows::LocalFileSystemWindows()
{
}

bool LocalFileSystemWindows::MoveToTrash(FileEntryData& data)
{
    if (!IsFileCreated(data.m_path) && !IsDirectoryCreated(data.m_path))
        return false;

    // From MSDN:
    // If pFrom is set to a file name without a full path, deleting the file with FO_DELETE does not move it to the Recycle Bin,
    // even if the FOF_ALLOWUNDO flag is set. You must provide a full path to delete the file to the Recycle Bin.
    TEMP_WSTRING wPath;
    UnityPathToWindowsPath(ToAbsolute(data.m_path).c_str(), wPath);

    SHFILEOPSTRUCTW op;
    op.hwnd = NULL;
    op.wFunc = FO_DELETE;
    wPath += L'\0';     // op.pFrom has to be double null terminated (as it's actually a list of strings)
    op.pFrom = wPath.c_str();
    op.pTo = NULL;
    op.fFlags = FOF_NOCONFIRMATION | FOF_NOERRORUI | FOF_SILENT | FOF_ALLOWUNDO;
    op.hNameMappings = NULL;
    op.lpszProgressTitle = NULL;
    SHFileOperationW(&op);  // note: GetLastError() is not compatible with SHFileOperationW()

    // The above doesn't work when performed over the network.
    // In that case we just kill it immediately.
    if (IsFileCreated(data.m_path) || IsDirectoryCreated(data.m_path))
    {
        DeleteFileOrDirectory(data.m_path);
    }

    return !IsFileCreated(data.m_path) && !IsDirectoryCreated(data.m_path);
}

bool LocalFileSystemWindows::Target(const FileEntryData& data, FileSystemEntry& out_symLink) const
{
    TEMP_WSTRING wPath;
    UnityPathToWindowsPath(data.m_path, wPath);

    wchar_t widePath[kDefaultPathBufferSize + 1];
    memset(widePath, 0, sizeof(widePath));
    if (PathCanonicalizeW(widePath, wPath.c_str()))
    {
        TempString utfPath;
        ConvertWindowsPathName(widePath, utfPath);
        out_symLink = FileSystemEntry(utfPath);
        return true;
    }
    return false;
}

core::string LocalFileSystemWindows::GetUserAppDataFolder() const
{
    if (m_CachedUserAppDataFolder.empty())
    {
        wchar_t widePath[MAX_PATH];
        if (SUCCEEDED(SHGetFolderPathW(NULL, CSIDL_APPDATA, NULL, SHGFP_TYPE_CURRENT, widePath)))
        {
            TempString cachedUserAppDataFolder;
            ConvertWindowsPathName(widePath, cachedUserAppDataFolder);
            m_CachedUserAppDataFolder = AppendPathName(cachedUserAppDataFolder, "Unity");
            CreateDirectory(m_CachedUserAppDataFolder);
        }
    }

    return core::string(m_CachedUserAppDataFolder, kMemTempAlloc);
}

core::string LocalFileSystemWindows::GetApplicationManagedPath() const
{
    return AppendPathName(GetApplicationContentsPath(), "Managed");
}

HMODULE LocalFileSystemWindows::GetHModuleForApplicationPath() const
{
    return GetModuleHandleW(nullptr);
}

core::string LocalFileSystemWindows::GetSharedApplicationDataFolder() const
{
#if !UNITY_EXTERNAL_TOOL
    return systeminfo::GetCommonPersistentDataPath();
#else
    return "";
#endif
}
