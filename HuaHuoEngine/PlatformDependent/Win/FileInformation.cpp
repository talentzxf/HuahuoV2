#include "UnityPrefix.h"
#include "FileInformation.h"
#include "WinUnicode.h"
#include <winver.h>

#pragma comment(lib, "version.lib")

static void GetFileInfo(void* fileInformationData, const DWORD* translation, const char* field, char* fieldValue)
{
    wchar_t key[2000];
    UINT length;
    swprintf_s(key,
        _countof(key),
        L"\\StringFileInfo\\%04x%04x\\%S",
        LOWORD(*translation),
        HIWORD(*translation),
        field);

    wchar_t* value;
    if (!::VerQueryValueW(fileInformationData, key, (void**)&value, &length))
        return;

    ::WideCharToMultiByte(CP_UTF8, 0, value, static_cast<int>(length), fieldValue, kDefaultPathBufferSize, NULL, NULL);
}

FileInformation::FileInformation(const wchar_t* filePath) :
    m_Valid(false)
{
    if (filePath == 0 || filePath[0] == 0)
        return;

    DWORD handle = 0;
    const DWORD size = ::GetFileVersionInfoSizeW(filePath, &handle);
    if (size == 0)
        return;

    void* fileInformationData = UNITY_MALLOC(kMemTempAlloc, size);
    memset(fileInformationData, 0, size);

    memset(m_ProductName, 0, sizeof(m_ProductName));
    memset(m_CompanyName, 0, sizeof(m_CompanyName));
    memset(m_FileDescription, 0, sizeof(m_FileDescription));

    if (!GetFileVersionInfoW(filePath, handle, size, fileInformationData))
    {
        UNITY_FREE(kMemTempAlloc, fileInformationData);
        return;
    }

    VS_FIXEDFILEINFO* fixedFileInfo = NULL;
    UINT length = 0;

    if (!VerQueryValueW(fileInformationData, L"\\", (void**)&fixedFileInfo, &length))
    {
        UNITY_FREE(kMemTempAlloc, fileInformationData);
        return;
    }

    m_FileVersion.v1 = HIWORD(fixedFileInfo->dwFileVersionMS);
    m_FileVersion.v2 = LOWORD(fixedFileInfo->dwFileVersionMS);
    m_FileVersion.v3 = HIWORD(fixedFileInfo->dwFileVersionLS);
    m_FileVersion.v4 = LOWORD(fixedFileInfo->dwFileVersionLS);

    m_ProductVersion.v1 = HIWORD(fixedFileInfo->dwProductVersionMS);
    m_ProductVersion.v2 = LOWORD(fixedFileInfo->dwProductVersionMS);
    m_ProductVersion.v3 = HIWORD(fixedFileInfo->dwProductVersionLS);
    m_ProductVersion.v4 = LOWORD(fixedFileInfo->dwProductVersionLS);

    // get translation - first translation in array is used
    DWORD* translation = NULL;
    if (!::VerQueryValueW(fileInformationData, L"\\VarFileInfo\\Translation", (void**)&translation, &length))
    {
        UNITY_FREE(kMemTempAlloc, fileInformationData);
        return;
    }

    GetFileInfo(fileInformationData, translation, "ProductName", m_ProductName);
    GetFileInfo(fileInformationData, translation, "CompanyName", m_CompanyName);
    GetFileInfo(fileInformationData, translation, "FileDescription", m_FileDescription);

    UNITY_FREE(kMemTempAlloc, fileInformationData);

    m_Valid = true;
}

bool FileInformation::ValidFileInformation() const
{
    return m_Valid;
}

const FileInformation::Version& FileInformation::GetFileVersion() const
{
    return m_FileVersion;
}

const FileInformation::Version& FileInformation::GetProductVersion() const
{
    return m_ProductVersion;
}

const char* FileInformation::GetProductName() const
{
    if (ValidFileInformation())
        return m_ProductName;
    else
        return NULL;
}

const char* FileInformation::GetCompanyName() const
{
    if (ValidFileInformation())
        return m_CompanyName;
    else
        return NULL;
}

const char* FileInformation::GetFileDescription() const
{
    if (ValidFileInformation())
        return m_FileDescription;
    else
        return NULL;
}

bool FileInformation::CopyString(char* dest, int dstSize, const char* src) const
{
    if (ValidFileInformation() && dest)
    {
        strncpy_s(dest, dstSize, src, dstSize - 1);
        return true;
    }
    else
        return false;
}

bool FileInformation::GetProductName(char* dstProductName, int size) const
{
    return CopyString(dstProductName, size, m_ProductName);
}

bool FileInformation::GetCompanyName(char* dstCompanyName, int size) const
{
    return CopyString(dstCompanyName, size, m_CompanyName);
}

bool FileInformation::GetFileDescription(char* dstFileDescription, int size) const
{
    return CopyString(dstFileDescription, size, m_FileDescription);
}
