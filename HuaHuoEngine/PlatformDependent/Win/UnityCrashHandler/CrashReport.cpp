#include "Utilities.h"
#include "CrashReport.h"

static const unsigned short kMaxUserMetadataKeySize = 0xFF;
static const unsigned short kMaxUserMetadataValueSize = 1024;

std::string FixWindowsPath(std::string path)
{
    std::replace(path.begin(), path.end(), L'/', L'\\');
    return path;
}

CrashReport::CrashReport()
    : appName("<unknown>")
    , info("<not set>")
    , isEditor(false)
    , showDialog(true)
{
}

void CrashReport::SetBugReporterAppPath(const char* path)
{
    this->bugReporterAppPath = path;
}

void CrashReport::SetCrashReportPath(const char* crashRepPath)
{
    this->crashReportPath = FixWindowsPath(crashRepPath);
}

void CrashReport::AddFile(const std::string& filename, const std::string& desc)
{
    this->additionalFiles.push_back(CrashReportFile(FixWindowsPath(filename), desc));
}

void CrashReport::RemoveFile(const std::string& unfixedFile)
{
    std::string file = FixWindowsPath(unfixedFile);
    for (
        std::vector<CrashReportFile>::iterator i = additionalFiles.begin();
        i != additionalFiles.end();
    )
    {
        if (i->path.compare(file) == 0)
            i = additionalFiles.erase(i);
        else
            i++;
    }
}

void CrashReport::SetMonoPath(const char* path)
{
    monoPath = FixWindowsPath(path);
}

const char* CrashReport::GetMetaData(const char* key) const
{
    std::map<std::string, std::string>::const_iterator i = metaData.find(key);
    if (i == metaData.end())
        return nullptr;
    return i->second.c_str();
}

void CrashReport::SetUserMetaData(core::string key, const char* value)
{
    if (key.size() > kMaxUserMetadataKeySize)
        key.resize(kMaxUserMetadataKeySize);

    if (value != NULL)
    {
        core::string newValue(value);
        if (newValue.size() > kMaxUserMetadataValueSize)
            newValue.resize(kMaxUserMetadataValueSize);

        userMetadata[key] = newValue;
    }
    else
    {
        std::map<core::string, core::string>::iterator it = userMetadata.find(key);
        if (it != userMetadata.end())
            userMetadata.erase(it);
    }
}

void CrashReport::SetLogBufferSize(const char* logBufferSizeStr)
{
    UInt32 logBufferSize;
    if (StringToInteger<UInt32>(logBufferSizeStr, &logBufferSize))
    {
        logBuffer.SetSize(logBufferSize);
    }
}

void  CrashReport::RecordLogMessage(const char* message, const char* timestampStr, const char* framecountStr, const char* typeStr)
{
    UInt64 timestamp;
    if (!StringToInteger<UInt64>(timestampStr, &timestamp))
    {
        printf_console("Corrupt log message timestamp sent to crash handler: %s\n", timestampStr);
    }

    int framecount;
    if (!StringToInteger<int>(framecountStr, &framecount))
    {
        printf_console("Corrupt log message framecount sent to crash handler: %s\n", framecountStr);
    }

    int type;
    if (!StringToInteger<int>(typeStr, &type))
    {
        printf_console("Corrupt log message type sent to crash handler: %s\n", typeStr);
    }

    logBuffer.RecordLogMessage(message, timestamp, framecount, (LogType)type);
}

CrashReportMessageHandler::CrashReportMessageHandler(CrashReport* report)
    : m_Report(report)
{
}

void CrashReportMessageHandler::ProcessMessage(const char* payload, size_t payloadSize)
{
    if (!payload || !payloadSize)
        return;

    std::vector<const char*> payloadChunks;

    // This data will be a series of non-null, null-terminated strings
    size_t consumed = 0;
    while (consumed < payloadSize)
    {
        size_t chunkSize = strlen(payload);
        Assert(consumed + chunkSize + 1 <= payloadSize);

        payloadChunks.push_back(payload);

        payload += chunkSize + 1;
        consumed += chunkSize + 1;
    }

    ProcessMessage(payloadChunks.data(), payloadChunks.size());
}

void CrashReportMessageHandler::ProcessMessage(const char** payload, size_t payloadCount)
{
    if (payloadCount == 0)
    {
        ReportCorruptMessage("Empty message recieved by crash handler.", payload, payloadCount);
        return;
    }

    Mutex::AutoLock lock(m_Lock);

    // For debugging purposes, print the payload
    printf_console("Message payload [%d]: ", (unsigned int)payloadCount);
    for (size_t i = 0; i < payloadCount; ++i)
    {
        printf_console("'%s' ", payload[i]);
    }
    printf_console("\n");

    // Now parse it
    if (strcmp(payload[0], "AddFile") == 0)
    {
        if (payloadCount != 3)
        {
            ReportCorruptMessage("Expected two parameters 'file' and 'desc'", payload, payloadCount);
        }
        else
        {
            m_Report->AddFile(payload[1], payload[2]);
        }
    }
    else if (strcmp(payload[0], "RemoveFile") == 0)
    {
        if (payloadCount != 2)
        {
            ReportCorruptMessage("Expected parameter 'file'", payload, payloadCount);
        }
        else
        {
            m_Report->RemoveFile(payload[1]);
        }
    }
    else if (strcmp(payload[0], "SetIsEditor") == 0)
    {
        if (payloadCount != 2)
        {
            ReportCorruptMessage("Expected parameter 'isEditor'", payload, payloadCount);
        }
        else if (strcmp(payload[1], "true") == 0)
        {
            m_Report->SetIsEditor(true);
        }
        else if (strcmp(payload[1], "false") == 0)
        {
            m_Report->SetIsEditor(false);
        }
        else
        {
            ReportCorruptMessage("Parameter for 'isEditor' should be 'true' or 'false'", payload, payloadCount);
        }
    }
    else if (strcmp(payload[0], "SetMetaData") == 0)
    {
        if (payloadCount != 3)
        {
            ReportCorruptMessage("Expected two parameters 'key' and 'value'", payload, payloadCount);
        }
        else
        {
            m_Report->SetMetaData(payload[1], payload[2]);
        }
    }
    else if (strcmp(payload[0], "SetUserMetaData") == 0)
    {
        if (payloadCount != 3)
        {
            ReportCorruptMessage("Expected two parameters 'key' and 'value'", payload, payloadCount);
        }
        else
        {
            m_Report->SetUserMetaData(payload[1], payload[2]);
        }
    }
    else if (strcmp(payload[0], "SetAppName") == 0)
    {
        if (payloadCount != 2)
        {
            ReportCorruptMessage("Expected parameter 'name'", payload, payloadCount);
        }
        else
        {
            m_Report->SetAppName(payload[1]);
        }
    }
    else if (strcmp(payload[0], "SetVendor") == 0)
    {
        if (payloadCount != 2)
        {
            ReportCorruptMessage("Expected parameter 'vendor'", payload, payloadCount);
        }
        else
        {
            m_Report->SetVendor(payload[1]);
        }
    }
    else if (strcmp(payload[0], "SetInfo") == 0)
    {
        if (payloadCount != 2)
        {
            ReportCorruptMessage("Expected parameter 'info'", payload, payloadCount);
        }
        else
        {
            m_Report->SetInfoString(payload[1]);
        }
    }
    else if (strcmp(payload[0], "SetShowDialog") == 0)
    {
        if (payloadCount != 2)
        {
            ReportCorruptMessage("Expected parameter 'showDialog'", payload, payloadCount);
        }
        else if (strcmp(payload[1], "true") == 0)
        {
            m_Report->SetShowDialog(true);
        }
        else if (strcmp(payload[1], "false") == 0)
        {
            m_Report->SetShowDialog(false);
        }
        else
        {
            ReportCorruptMessage("Parameter for 'SetShowDialog' should be 'true' or 'false'", payload, payloadCount);
        }
    }
    else if (strcmp(payload[0], "SetBugReporterAppPath") == 0)
    {
        if (payloadCount != 2)
        {
            ReportCorruptMessage("Expected parameter 'path'", payload, payloadCount);
        }
        else
        {
            m_Report->SetBugReporterAppPath(payload[1]);
        }
    }
    else if (strcmp(payload[0], "SetCrashReportPath") == 0)
    {
        if (payloadCount != 2)
        {
            ReportCorruptMessage("Expected parameter 'path'", payload, payloadCount);
        }
        else
        {
            m_Report->SetCrashReportPath(payload[1]);
        }
    }
    else if (strcmp(payload[0], "SetMonoPath") == 0)
    {
        if (payloadCount != 2)
        {
            ReportCorruptMessage("Expected parameter 'path'", payload, payloadCount);
        }
        else
        {
            m_Report->SetMonoPath(payload[1]);
        }
    }
    else if (strcmp(payload[0], "SetLogBufferSize") == 0)
    {
        if (payloadCount != 2)
        {
            ReportCorruptMessage("Expected parameter 'logBufferSize'", payload, payloadCount);
        }
        else
        {
            m_Report->SetLogBufferSize(payload[1]);
        }
    }
    else if (strcmp(payload[0], "RecordLogMessage") == 0)
    {
        if (payloadCount != 5)
        {
            ReportCorruptMessage("Expected four parameters 'message', 'timestamp', 'framecount', and 'type'", payload, payloadCount);
        }
        else
        {
            m_Report->RecordLogMessage(payload[1], payload[2], payload[3], payload[4]);
        }
    }
    else
    {
        ReportCorruptMessage("Unknown message", payload, payloadCount);
    }
}

void CrashReportMessageHandler::ReportCorruptMessage(const char* error, const char** payload, size_t payloadCount)
{
    printf_console("Corrupt message sent to crash handler: %s\nPayload:\n", error);
    for (size_t i = 0; i < payloadCount; ++i)
    {
        printf_console("  [%d] %s\n", i, payload[i]);
    }
}
