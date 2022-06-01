#pragma once

enum CachedReportStatus
{
    kCacheOK,
    kCacheMissingMeta,
    kCacheMissingReport,
    kCacheCorruptMeta,
    kCacheCorruptReport,
    kCacheVersionUnsupported,
    kCacheFailedToWrite,
    kCacheMissingUploadEndPoint,
    kCacheFailedToUpload,
    kCacheReportTooLarge,
    kProjectIDEmpty
};

const char* CachedReportErrorAsString(CachedReportStatus status);

class CachedCrashReport
{
public:
    CachedCrashReport();

    // Copies a CrashReportEvent
    CachedCrashReport(const UInt8* reportBlob, size_t reportSize);

    // Load from disk
    CachedReportStatus Read(const wchar_t* reportFileName);

    // Save the report to the cache and update the meta on disk
    CachedReportStatus Write(const wchar_t* reportFileName, const char* projectID);

    // Upload it to the server
    CachedReportStatus Upload(const char* endPoint) const;

    // Remove the file and metadata
    static CachedReportStatus Delete(const wchar_t* reportFileName);

    bool IsValid() const { return m_reportBlob.size() > 0; }

private:
    struct MetaData;

    std::vector<UInt8> m_reportBlob;
    std::string m_projectID;

    CachedReportStatus GenerateReportFile(const wchar_t* reportFilename);
    CachedReportStatus GenerateMetaFile(const wchar_t* metaFilename, const UInt8 hash[16], const char* projectID);
    CachedReportStatus ValidateMetaData(const MetaData* metaData, UInt32 expectedSize, const UInt8 reportHash[16]);
};
