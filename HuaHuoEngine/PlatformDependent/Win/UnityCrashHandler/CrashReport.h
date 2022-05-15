#pragma once

#include "UnityPrefix.h"
#include "CrashHandlerCommPipe.h"

#include <Runtime/Threads/Mutex.h>
#include <vector>
#include <string>
#include <map>

#include "Modules/CrashReporting/LogBuffer.h"

struct CrashReportFile
{
    CrashReportFile(const std::string& filepath, const std::string& desc)
        : path(filepath)
        , description(desc)
    {}

    std::string path;
    std::string description;
};

class CrashReport
{
public:
    CrashReport();

    void SetAppName(const char* name) { this->appName = name; }
    void SetVendor(const char* name) { this->vendor = name; }
    void SetInfoString(const char* info) { this->info = info; }
    void SetBugReporterAppPath(const char* crashRepPath);
    void SetCrashReportPath(const char* crashRepPath);
    void AddFile(const std::string& filename, const std::string& desc);
    void RemoveFile(const std::string& filename);
    void SetIsEditor(bool value) { this->isEditor = value; }
    void SetMetaData(const char* key, const char* value) { metaData[key] = value; }
    void SetUserMetaData(core::string key, const char* value);
    void SetLogBufferSize(const char* logBufferSizeStr);
    void RecordLogMessage(const char* message, const char* timestampStr, const char* framecountStr, const char* typeStr);
    void SetShowDialog(bool show) { showDialog = show; }
    void SetMonoPath(const char* path);

    const char* GetMetaData(const char* key) const;

    std::map<std::string, std::string> metaData;
    std::vector<CrashReportFile> additionalFiles;
    std::string bugReporterAppPath;
    std::string crashReportPath;
    std::string appName;
    std::string vendor;
    std::string info;
    std::string errLogFileName;
    std::string errDumpFileName;
    std::string monoPath;
    bool        showDialog;
    bool        isEditor;
    std::map<core::string, core::string> userMetadata;
    CrashReporting::LogBuffer logBuffer;
};

class CrashReportMessageHandler : public CommPipeMessageHandler
{
public:
    CrashReportMessageHandler(CrashReport* report);

    virtual void ProcessMessage(const char* payload, size_t payloadSize);

private:
    CrashReport * m_Report;

    void ProcessMessage(const char** payload, size_t payloadCount);
    void ReportCorruptMessage(const char* error, const char** payload, size_t payloadCount);

    Mutex m_Lock;
};
