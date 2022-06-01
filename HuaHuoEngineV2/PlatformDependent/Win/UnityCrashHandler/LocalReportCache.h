#pragma once

namespace CrashReporting
{
    class CrashReport;
}

std::wstring GetCachedCrashReportFolder();
bool CacheCrashReport(const CrashReporting::CrashReport* crashReport);
int ReportCacheMain(int argc, wchar_t** argv);
