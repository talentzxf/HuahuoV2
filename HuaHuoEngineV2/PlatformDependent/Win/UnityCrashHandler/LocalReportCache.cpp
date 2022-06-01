#include "UnityPrefix.h"
#include "PlatformDependent/Win/WinUtils.h"
#include "CachedCrashReport.h"
#include "Utilities.h"
#include "Modules/CrashReporting/Events/CrashReport.h"
#include "External/zlib/src/zlib.h"
#include <fileapi.h>

static const wchar_t* kLocalCacheReportFolder = L"\\Unity\\CrashReports\\";
static const wchar_t* kLocalCacheReportExtension = L".json.gz";

#if DEBUGMODE
static const char* kPerformanceReportingEndpointLocal = "http://localhost:8080";
static const char* kPerformanceReportingEndpointDev = "https://dev-perf-events.cloud.unity3d.com";
#endif
static const char* kPerformanceReportingEndpoint = "https://perf-events.cloud.unity3d.com";

bool CompressBlob(std::vector<UInt8>& dest, const void* src, size_t srcSize)
{
    z_stream stream = {};
    stream.zalloc = Z_NULL;
    stream.zfree = Z_NULL;
    stream.opaque = Z_NULL;
    // todo: what are these magic numbers? (from CrashReporter.mm)
    int ret = deflateInit2(&stream, Z_DEFAULT_COMPRESSION, Z_DEFLATED, 15 + 16, 8, Z_DEFAULT_STRATEGY);
    if (ret != Z_OK)
        return false;

    stream.avail_in = static_cast<uLongf>(srcSize);
    stream.next_in = reinterpret_cast<Bytef*>(const_cast<void*>(src));

    size_t estSize = deflateBound(&stream, srcSize);
    dest.resize(estSize);

    uLongf bufferSize = static_cast<uLongf>(srcSize);
    uLongf compressedSize = 0;
    Bytef* out = reinterpret_cast<Bytef*>(dest.data());

    do
    {
        stream.avail_out = bufferSize - stream.total_out;
        stream.next_out = out + stream.total_out;
        ret = deflate(&stream, Z_FINISH);
        if (ret == Z_STREAM_ERROR)
            return false;
    }
    while (stream.avail_in != 0);

    deflateEnd(&stream);

    dest.resize(stream.total_out);
    return true;
}

std::wstring GetCachedCrashReportFolder()
{
    return GetAppDataLocalLowDirectory() + kLocalCacheReportFolder;
}

void GetCachedCrashReports(std::vector<std::wstring>& reportFiles)
{
    std::wstring path = GetCachedCrashReportFolder() + L"\\";
    std::wstring pathFilter = path + L"*" + kLocalCacheReportExtension;

    WIN32_FIND_DATAW fd;

    HANDLE hFind = FindFirstFileW(pathFilter.c_str(), &fd);
    if (hFind == INVALID_HANDLE_VALUE)
        return;

    do
    {
        if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
        {
            reportFiles.push_back(path + fd.cFileName);
        }
    }
    while (FindNextFileW(hFind, &fd) != 0);

    FindClose(hFind);
}

bool CacheCrashReport(const CrashReporting::CrashReport* crashReport)
{
    // Serialize the crash report data into a JSON string
    core::string reportJSON = crashReport->ToJsonString();

    // Compress the report
    std::vector<UInt8> reportBlob;
    if (!CompressBlob(reportBlob, reportJSON.c_str(), reportJSON.size()))
    {
        return false;
    }

    // Create the cached report
    CachedCrashReport cachedReport(reportBlob.data(), reportBlob.size());

    // Save it
    std::wstring reportFilename(GetCachedCrashReportFolder());
    reportFilename += L"\\CrashReport";
    reportFilename += GetTimeString();
    reportFilename += kLocalCacheReportExtension;

    return cachedReport.Write(reportFilename.c_str(), crashReport->m_ProjectID.c_str()) == kCacheOK;
}

int ReportCacheMain(int argc, wchar_t** argv)
{
    bool noDelete = PopCommandLineSwitch(L"--no-delete", argc, argv);

    const char* endpoint = kPerformanceReportingEndpoint;
#if DEBUGMODE
    if (PopCommandLineSwitch(L"--dev-endpoint", argc, argv))
        endpoint = kPerformanceReportingEndpointDev;
    if (PopCommandLineSwitch(L"--local-endpoint", argc, argv))
        endpoint = kPerformanceReportingEndpointLocal;
#endif

    std::vector<std::wstring> reports;
    GetCachedCrashReports(reports);

    printf_console("Found %u potential reports:\n", (UInt32)reports.size());

    for (std::vector<std::wstring>::const_iterator reportFile = reports.begin(); reportFile != reports.end(); ++reportFile)
    {
        printf_console(" .. %S: ", reportFile->c_str());

        CachedCrashReport report;
        CachedReportStatus status = report.Read(reportFile->c_str());
        if (status == kCacheOK)
        {
            status = report.Upload(endpoint);
        }

        // If the upload was OK, delete the report
        if (status == kCacheOK && !noDelete)
        {
            CachedCrashReport::Delete(reportFile->c_str());
        }

        printf_console("%s\n", CachedReportErrorAsString(status));
    }

    printf_console("Done.\n");
    return 0;
}
