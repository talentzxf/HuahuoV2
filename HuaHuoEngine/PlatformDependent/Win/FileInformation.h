#pragma once

#include "PathUnicodeConversion.h"

// FileInformation is designed to work inside an application crash handler
// using SEH try/catch and therefore doesn't have a destructor as this isn't supported.
// Since a destructor can't be used, manual clean-up is avoided by copying
// string info to internal char arrays in constructor.
class FileInformation
{
public:
    struct Version
    {
        Version() : v1(0), v2(0), v3(0), v4(0) {}
        unsigned short v1;
        unsigned short v2;
        unsigned short v3;
        unsigned short v4;
    };

    FileInformation(const wchar_t* filePath);

    bool ValidFileInformation() const;

    const Version& GetFileVersion() const;
    const Version& GetProductVersion() const;

    const char* GetProductName() const;
    const char* GetCompanyName() const;
    const char* GetFileDescription() const;

    bool GetProductName(char* dstProductName, int size) const;
    bool GetCompanyName(char* dstCompanyName, int size) const;
    bool GetFileDescription(char* dstFileDescription, int size) const;

private:
    bool CopyString(char* dest, int dstSize, const char* src) const;

private:
    bool m_Valid;

    Version m_FileVersion;
    Version m_ProductVersion;

    char m_ProductName[kDefaultPathBufferSize];
    char m_CompanyName[kDefaultPathBufferSize];
    char m_FileDescription[kDefaultPathBufferSize];
};
