#include "UnityPrefix.h"
#include "PlatformDependent/Win/WinUtils.h"

#include "CachedCrashReport.h"
#include "Utilities.h"

#include <wtypes.h>
#include <WinInet.h>
#include <ShlObj.h>
#include <WinCrypt.h>

#include <sstream>
#include <unordered_map>
#include <tchar.h>
#include <strsafe.h>

static const UInt32 kHashSalt[] =
{
    0x3312e011, 0x22d43d6, 0xdf4b7506, 0x8d4eb39f,
    0xd31d1615, 0xd8dc0b9, 0x9ea12162, 0x66414aca
};

static const UInt32 kCrashReportMetaDataMagic = 0x4d524355;
static const UInt8  kCrashReportMetaDataVersionMajor = 2;
static const UInt8  kCrashReportMetaDataVersionMinor = 2;
static const UInt16 kCrashReportMetaDataVersion = ((UInt16)kCrashReportMetaDataVersionMajor << 8) | (UInt16)kCrashReportMetaDataVersionMinor;

static const size_t kMaxReportBlobSize = 0x7fffffffu;
static const char* kMultipartBoundary = "----------V2ymHFg03ehbqgZCaKO6jy";

#pragma pack(push, 1)
struct CachedCrashReport::MetaData
{
    UInt32 Magic;                   // kCrashReportMetaDataMagic
    UInt32 Size;                    // sizeof(CachedCrashReportMetaDataHeader)
    UInt16 Version;                 // kCrashReportMetaDataVersion
    UInt16 Reserved;                // padding/reserved for future use
    UInt8  Hash[16];                // MD5 hash of the report
    char ProjectID[37];

    // Add fields after this line and bump kCrashReportMetaDataVersion.
};
#pragma pack(pop)

// Use open source md5 implementation.
void md5(unsigned char *input, int ilen, unsigned char output[16]);

// Reads a file, adds salt and MD5 hashes the contents
bool ReadFileSaltAndHash(const wchar_t* filename, std::vector<UInt8>& data, UInt8 hash[16])
{
    // Clear the data
    data.resize(0);

    // Read the entire file
    if (!ReadEntireFile(filename, data))
        return false;

    // Add the salt
    size_t originalSize = data.size();
    size_t saltOffset = AlignSize(originalSize, sizeof(UInt32));
    data.resize(saltOffset + sizeof(kHashSalt));
    memcpy(data.data() + saltOffset, kHashSalt, sizeof(kHashSalt));

    // Hash it
    md5(data.data(), data.size(), hash);

    // Remove the salt
    data.resize(originalSize);

    return true;
}

CachedCrashReport::CachedCrashReport()
{
}

CachedCrashReport::CachedCrashReport(const UInt8* reportBlob, size_t reportSize)
    : m_reportBlob(reportSize)
{
    memcpy(m_reportBlob.data(), reportBlob, reportSize);
}

CachedReportStatus CachedCrashReport::Read(const wchar_t* reportFilename)
{
    Assert(reportFilename != nullptr && *reportFilename);

    // Read the entire file from disk into a string
    std::vector<UInt8> reportData;
    UInt8 hashOfFileOnDisk[16];
    if (!ReadFileSaltAndHash(reportFilename, reportData, hashOfFileOnDisk))
        return kCacheMissingReport;

    // Read the meta file
    std::wstring metaFilename(ChangeExtension(reportFilename, L".meta"));
    std::vector<UInt8> rawMetaData;
    if (!ReadEntireFile(metaFilename.c_str(), rawMetaData))
        return kCacheMissingMeta;

    // Validate the meta data
    if (rawMetaData.size() < sizeof(MetaData))
        return kCacheCorruptMeta;
    if (rawMetaData.size() > 0x7fffffff)
        return kCacheCorruptMeta;

    const MetaData* metaData = reinterpret_cast<const MetaData*>(rawMetaData.data());
    CachedReportStatus status = ValidateMetaData(metaData, (UInt32)rawMetaData.size(), hashOfFileOnDisk);
    if (status != kCacheOK)
        return status;

    // Meta-data checks out. Store the data.
    m_reportBlob = std::move(reportData);
    m_projectID = metaData->ProjectID;
    return kCacheOK;
}

CachedReportStatus CachedCrashReport::Write(const wchar_t* reportFilename, const char* projectID)
{
    Assert(reportFilename != nullptr && *reportFilename);

    if (!IsValid())
        return kCacheMissingReport;

    // Get the meta filename
    std::wstring metaFilename(ChangeExtension(reportFilename, L".meta"));

    // Create the cache folder if it doesn't exist
    ::SHCreateDirectoryExW(NULL, GetDirectoryPart(reportFilename).c_str(), NULL);

    CachedReportStatus status = GenerateReportFile(reportFilename);
    if (status != kCacheOK)
        return status;

    // Generate a hash of the report file
    UInt8 hash[16];
    std::vector<UInt8> fileData;
    if (!ReadFileSaltAndHash(reportFilename, fileData, hash))
        return kCacheMissingReport;

    // Generate the metadata file
    return GenerateMetaFile(metaFilename.c_str(), hash, projectID);
}

struct InternetHandle
{
    InternetHandle(HINTERNET hInternet)
        : handle(hInternet)
    {
    }

    ~InternetHandle()
    {
        InternetCloseHandle(handle);
    }

    bool IsValid() const { return handle != nullptr; }

    HINTERNET handle;
};

struct URL
{
    std::string scheme;
    std::string hostName;
    std::string userName;
    std::string password;
    std::string urlPath;
    std::string extra;
    UInt16 port;

    URL(const char* endpoint)
        : port(0)
    {
        URL_COMPONENTSA urlComponents;
        ZeroMemory(&urlComponents, sizeof(urlComponents));
        urlComponents.dwStructSize = sizeof(urlComponents);
        urlComponents.dwSchemeLength = 1;
        urlComponents.dwHostNameLength = 1;
        urlComponents.dwUserNameLength = 1;
        urlComponents.dwPasswordLength = 1;
        urlComponents.dwUrlPathLength = 1;
        urlComponents.dwExtraInfoLength = 1;

        if (InternetCrackUrlA(
            endpoint,
            0,
            0,
            &urlComponents))
        {
            scheme.assign(urlComponents.lpszScheme, urlComponents.dwSchemeLength);
            hostName.assign(urlComponents.lpszHostName, urlComponents.dwHostNameLength);
            userName.assign(urlComponents.lpszUserName, urlComponents.dwUserNameLength);
            password.assign(urlComponents.lpszPassword, urlComponents.dwPasswordLength);
            urlPath.assign(urlComponents.lpszUrlPath, urlComponents.dwUrlPathLength);
            extra.assign(urlComponents.lpszExtraInfo, urlComponents.dwExtraInfoLength);
            port = static_cast<UInt16>(urlComponents.nPort);
        }
    }
};

static void SerializeRequestBytes(std::vector<UInt8>& request, const void* data, size_t dataSize)
{
    if (dataSize == 0)
        return;

    request.insert(
        request.end(),
        reinterpret_cast<const UInt8*>(data),
        reinterpret_cast<const UInt8*>(data) + dataSize);
}

static void SerializeRequestStringF(std::vector<UInt8>& request, const char* format, ...)
{
    static const size_t kScratchSizeIncrement = 1024;
    static std::vector<char> scratch(kScratchSizeIncrement); // reserve some working memory for StringCchVPrintfEx

    va_list arglist;
    va_start(arglist, format);

    // Format the string into the scratch memory
    HRESULT hr;
    STRSAFE_LPSTR scratchEnd = nullptr;
    do
    {
        hr = StringCchVPrintfEx(scratch.data(), scratch.size(), &scratchEnd, nullptr, STRSAFE_IGNORE_NULLS, format, arglist);
        if (hr == ERROR_INSUFFICIENT_BUFFER)
            scratch.resize(scratch.size() + kScratchSizeIncrement);
    }
    while (hr == ERROR_INSUFFICIENT_BUFFER);

    Assert(scratchEnd >= scratch.data());

    // Serialize the resulting string
    SerializeRequestBytes(request, scratch.data(), scratchEnd - scratch.data());

    va_end(arglist);
}

static void SerializeRequestBody(std::vector<UInt8>& requestBody, const std::vector<UInt8>& report, const char* boundary)
{
    std::unordered_map<std::string, std::string> contentParams;
    contentParams["ver"] = "1.0";
    contentParams["lan"] = "en";

    requestBody.reserve(report.size() + 2048); // reserve some space up-front

    // Serialize the content parameters into the response body
    for (std::unordered_map<std::string, std::string>::const_iterator i = contentParams.begin(); i != contentParams.end(); ++i)
    {
        SerializeRequestStringF(requestBody,
            "--%s\r\n"
            "Content-Disposition: form-data; name=\"%s\"\r\n\r\n"
            "%s\r\n",
            boundary,
            i->first.c_str(),
            i->second.c_str());
    }

    // Add data to the request body
    SerializeRequestStringF(requestBody,
        "--%s\r\n"
        "Content-Disposition: form-data; name=\"file\"; filename=\"data.crashreportevent.gz\"\r\n"
        "Content-Type: application/octet-stream\r\n\r\n",
        boundary);

    SerializeRequestBytes(requestBody, report.data(), report.size());
    SerializeRequestStringF(requestBody, "\r\n--%s--\r\n", boundary);
}

CachedReportStatus CachedCrashReport::Upload(const char* endpoint) const
{
    if (!m_reportBlob.size())
        return kCacheMissingReport;
    if (endpoint == nullptr || !*endpoint)
        return kCacheMissingUploadEndPoint;
    if (m_projectID.empty())
        return kProjectIDEmpty;

    core::string formattedEndpoint = Format("%s/api/v2/projects/%s/reports", endpoint, m_projectID.c_str());
    const char* reportEndpoint = formattedEndpoint.c_str();

    // Construct the report body
    std::vector<UInt8> requestBody;
    SerializeRequestBody(requestBody, m_reportBlob, kMultipartBoundary);

    if (requestBody.size() > kMaxReportBlobSize)
        return kCacheReportTooLarge;
    DWORD reportSize = static_cast<DWORD>(requestBody.size());

    URL url(reportEndpoint);

    // Pulling in Unity's WebRequest class pulls in all kinds of dependencies we don't want.  So we'll just use WinInet's request APIs directly.
    InternetHandle hInternet(InternetOpenA(
        "UnityCrashHandler",
        INTERNET_OPEN_TYPE_PRECONFIG,
        nullptr,                          // proxy name
        nullptr,                          // proxy bypass
        0));
    if (!hInternet.IsValid())
    {
        printf_console("Failed to open the Internet with code %d\n", GetLastError());
        return kCacheFailedToUpload;
    }

    InternetHandle hSession(InternetConnectA(
        hInternet.handle,
        url.hostName.c_str(),
        static_cast<INTERNET_PORT>(url.port),
        url.userName.empty() ? nullptr : url.userName.c_str(),
        url.password.empty() ? nullptr : url.password.c_str(),
        INTERNET_SERVICE_HTTP,
        0,                         // flags
        (DWORD_PTR)nullptr));
    if (!hSession.IsValid())
    {
        printf_console("Failed to open internet session to server '%s' (code %d)\n", reportEndpoint, GetLastError());
        return kCacheFailedToUpload;
    }

    DWORD internetFlags =
        INTERNET_FLAG_SECURE |
        INTERNET_FLAG_NO_CACHE_WRITE |
        INTERNET_FLAG_NO_COOKIES |
        INTERNET_FLAG_NO_UI;

    if (url.scheme != "https")
    {
#if DEBUGMODE
        printf_console("Warning: URL scheme '%s' will not work in production.\n", url.scheme.c_str());
        internetFlags &= ~INTERNET_FLAG_SECURE;
#else
        printf_console("Invalid URL scheme '%s' (expected 'https').\n", url.scheme.c_str());
        return kCacheFailedToUpload;
#endif
    }

    InternetHandle hRequest(HttpOpenRequestA(
        hSession.handle,
        "POST",
        url.urlPath.c_str(),                         // object name
        "HTTP/1.1",
        nullptr,                         // referrer
        nullptr,                         // accept types (N/A b/c POST)
        internetFlags,
        (DWORD_PTR)nullptr));
    if (!hRequest.IsValid())
    {
        printf_console("Failed to open HTTP request to server '%s' (code %d)\n", reportEndpoint, GetLastError());
        return kCacheFailedToUpload;
    }

    // Construct the headers and send the request
    std::stringstream headers;
    headers << "Content-Type: multipart/form-data; boundary=" << kMultipartBoundary << "\r\n";
    headers << "Content-Length: " << reportSize << "\r\n";
    std::string headersString = headers.str();

    if (!HttpSendRequestA(
        hRequest.handle,
        headersString.c_str(),
        static_cast<DWORD>(headersString.size()),
        (LPVOID)requestBody.data(),
        reportSize))
    {
        printf_console("Failed to upload %u bytes to server '%s' (code %d)\n", reportSize, reportEndpoint, GetLastError());
        return kCacheFailedToUpload;
    }

    // Get the response from the server
    DWORD statusCode = 0;
    DWORD statusCodeSize = static_cast<DWORD>(sizeof(statusCode));
    DWORD statusCodeIndex = 0;
    if (!HttpQueryInfoA(
        hRequest.handle,
        HTTP_QUERY_STATUS_CODE | HTTP_QUERY_FLAG_NUMBER,
        &statusCode,
        &statusCodeSize,
        &statusCodeIndex))
    {
        printf_console("Upload of %u bytes to server '%s' completed, but received no response (Win32 code %d)\n", reportSize, reportEndpoint, GetLastError());
        return kCacheFailedToUpload;
    }

    // Check the status code
    if (statusCodeIndex != 0 && statusCodeIndex != ERROR_HTTP_HEADER_NOT_FOUND)
        printf_console("Warning: Upload of %u bytes to server '%s' completed, but received multiple status codes! (first code: %u)\n", reportSize, reportEndpoint, statusCode);
    if (statusCode < 200 || statusCode >= 300)
    {
        printf_console("Upload of %u bytes to server '%s' completed, the server responded with code %u\n", reportSize, reportEndpoint, statusCode);
        return kCacheFailedToUpload;
    }

    return kCacheOK;
}

CachedReportStatus CachedCrashReport::Delete(const wchar_t* reportFilename)
{
    std::wstring metaFilename(ChangeExtension(reportFilename, L".meta"));

    CachedReportStatus status = kCacheOK;
    if (DeleteFileW(reportFilename) == 0)
        status = kCacheMissingReport;

    if (DeleteFileW(metaFilename.c_str()) == 0 && status == kCacheOK)
        status = kCacheMissingMeta;

    return status;
}

CachedReportStatus CachedCrashReport::GenerateReportFile(const wchar_t* reportFilename)
{
    // Open the JSON file
    winutils::AutoHandle reportFile(CreateFileW(
        reportFilename,
        GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL | FILE_FLAG_WRITE_THROUGH, 0));
    if (!reportFile.IsValid())
    {
        return kCacheFailedToWrite;
    }

    // Write the report event
    DWORD bytesWritten = 0;
    if (!WriteFile(reportFile.Get(), m_reportBlob.data(), static_cast<DWORD>(m_reportBlob.size()), &bytesWritten, nullptr))
        return kCacheFailedToWrite;

    return kCacheOK;
}

CachedReportStatus CachedCrashReport::GenerateMetaFile(const wchar_t* metaFilename, const UInt8 hash[16], const char* projectID)
{
    MetaData metaHeader;
    memset(&metaHeader, 0, sizeof(metaHeader));

    metaHeader.Magic = kCrashReportMetaDataMagic;
    metaHeader.Size = sizeof(metaHeader);
    metaHeader.Version = kCrashReportMetaDataVersion;
    memcpy(metaHeader.Hash, hash, sizeof(metaHeader.Hash));
    strcpy(metaHeader.ProjectID, projectID);

    winutils::AutoHandle metaFile(CreateFileW(
        metaFilename,
        GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL | FILE_FLAG_WRITE_THROUGH, 0));
    if (!metaFile.IsValid())
    {
        return kCacheFailedToWrite;
    }

    DWORD bytesWritten = 0;
    if (WriteFile(metaFile.Get(), &metaHeader, sizeof(metaHeader), &bytesWritten, nullptr))
        return kCacheOK;
    else
        return kCacheFailedToWrite;
}

CachedReportStatus CachedCrashReport::ValidateMetaData(const MetaData* metaData, UInt32 expectedSize, const UInt8 reportHash[16])
{
    // Verify the basics
    if (metaData->Magic != kCrashReportMetaDataMagic)
        return kCacheCorruptMeta;
    if (metaData->Size != expectedSize)
        return kCacheCorruptMeta;
    if (metaData->Version > kCrashReportMetaDataVersion)
        return kCacheVersionUnsupported;

    std::string projectID(metaData->ProjectID);
    if (projectID.empty())
        return kProjectIDEmpty;

    // Check the hash
    if (memcmp(metaData->Hash, reportHash, sizeof(UInt8) * 16) != 0)
        return kCacheCorruptReport;

    return kCacheOK;
}

const char* CachedReportErrorAsString(CachedReportStatus status)
{
    switch (status)
    {
        case kCacheOK: return "OK";
        case kCacheMissingMeta: return "Missing metadata";
        case kCacheMissingReport: return "Missing report";
        case kCacheCorruptMeta: return "Metadata corrupt or wrong version";
        case kCacheCorruptReport: return "Corrupt report";
        case kCacheVersionUnsupported: return "Unknown version";
        case kCacheFailedToWrite: return "Failed to write";
        case kCacheFailedToUpload: return "Failed to upload";
        case kProjectIDEmpty: return "ProjectID is empty";
        default: return "Unknown error";
    }
}
