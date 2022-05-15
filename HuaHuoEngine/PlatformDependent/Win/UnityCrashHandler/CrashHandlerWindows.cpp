#include "CrashHandlerWindows.h"
#include "Utilities.h"
#include "PlatformDependent/Win/WinUnicode.h"
#include "PlatformDependent/Win/WinUtils.h"
#include "PlatformDependent/Win/SharedCrashData.h"
#include "PlatformDependent/Win/ProcessThreadSnapshot.h"
#include "Editor/Platform/Windows/BugReporterCrashReporter.h"
#include "Modules/CrashReporting/Events/CrashReport.h"
#include "Runtime/Logging/LogAssert.h"
#include "Runtime/Logging/LogSystem.h"
#include "Runtime/Threads/Mutex.h"
#include "CrashReport.h"
#include "CrashHandlerCommPipe.h"
#include "CrashDumpUtilities.h"
#include "LocalReportCache.h"
#include "MonoOOP.h"
#include "CrashUI.h"
#include "MonoStackWalker.h"

#if !_HAS_EXCEPTIONS
#   error UnityCrashHandler requires _HAS_EXCEPTIONS=1 (dependency on <regex>)
#endif

#include <winbase.h>
#include <winuser.h>
#include <winver.h>
#include <DbgHelp.h>
#include <Psapi.h>
#include <tchar.h>
#include <strsafe.h>
#include <regex>
#include <sstream>

class OutOfProcessMono;

#define CRASH_RPT_ERROR_LOG_NAME    L"error.log"
#define CRASH_RPT_MINI_DUMP_NAME    L"crash.dmp"

enum ExitReturnCodes
{
    // Success
    k_ExitOK = winutils::kCrashHandlerExitCodeNormal,
    k_ExitBugReporter = winutils::kCrashHandlerExitCodeBugReporterStarted,

    // Error codes
    k_ExitIncorrectUsage = -900,
    k_ExitFailedToAttach,
    k_ExitCreateSharedEventFailure,
    k_ExitFailedToWait,
    k_ExitAbnormalExit,
    k_ExitFailedToReadCrashData,
    k_ExitCorruptCrashData,
    k_ExitFailedToMiniDump,
    k_ExitFailedToOpenPipe,
    k_ExitCommunicationError,
    k_ExitNonFatalError,
};

int Usage(const wchar_t* argv0)
{
    printf_console("Usage: %S <process ID> <shared region address>\n");
    return k_ExitIncorrectUsage;
}

int HandleProcessExit(DWORD processID, HANDLE hProc)
{
    // The external process has ended. Let's see how it exited.
    // If the exit code is an HRESULT or some other known exit code, we
    // should report this as a crash and try and gather as much information as we can.
    DWORD exitCode = 0;
    if (GetExitCodeProcess(hProc, &exitCode))
    {
        printf_console("Process PID %d exited with code %d\n", processID, exitCode);
        return k_ExitOK;
    }
    else
    {
        // Something weird has happened - we're unable to query the exit code for some reason.
        printf_console("Process PID %d was terminated but exit code is not available.\n", processID);
        return k_ExitAbnormalExit;
    }
}

void DumpCrashReportConfiguration(HANDLE file, const CrashReport& report)
{
    hprintf(file, "Crash Report configuration:\r\n");
    hprintf(file, " * App Name: %s\r\n", report.appName.c_str());
    hprintf(file, " * App Version: %s\r\n", report.info.c_str());
    hprintf(file, " * Mono DLL: %s\r\n", report.monoPath.c_str());
    hprintf(file, " * Bug Reporter App Path: %s\r\n", report.bugReporterAppPath.c_str());
    hprintf(file, " * Crash Report Path: %s\r\n", report.crashReportPath.c_str());
    hprintf(file, " * Is Editor: %s\r\n", report.isEditor ? "true" : "false");

    // dump report values
    hprintf(file, "\r\nCrash Report metadata:\r\n");
    for (
        std::map<std::string, std::string>::const_iterator kvp = report.metaData.begin();
        kvp != report.metaData.end();
        ++kvp)
    {
        hprintf(file, " * '%s' = '%s'\r\n", kvp->first.c_str(), kvp->second.c_str());
    }

    // Dump file names attached to the report
    hprintf(file, "\r\nAdditional report files:\r\n");
    for (
        std::vector<CrashReportFile>::const_iterator f = report.additionalFiles.begin();
        f != report.additionalFiles.end();
        ++f)
    {
        core::wstring wideF = Utf8ToWide(f->path);
        hprintf(file, " * %s\"%s\" (%s)\r\n", FileExists(wideF.c_str()) ? "" : "MISSING: ", f->path.c_str(), f->description.c_str());
    }

    hprintf(file, "\r\n\r\n");
}

// This walks the stack and logs the stack frames to a file
class StackWalkLogReport : public MonoStackWalker::Report
{
public:
    StackWalkLogReport(
        HANDLE hReportFile)
        : m_ReportFile(hReportFile)
    {
    }

    void OnDbgHelpErr(const char* funcName, const char* lastErrorMsg, const void* addr)
    {
        hprintf(m_ReportFile, "ERROR: %s, GetLastError: '%s' (Address: %p)\r\n", funcName, lastErrorMsg, addr);
    }

    void OnOutput(const char* buffer)
    {
        hprintf(m_ReportFile, "%s", buffer);
    }

private:
    HANDLE m_ReportFile;
};

// Walks the stack and copies stack frame entries into a CrashReport
class StackWalkCrashReport : public MonoStackWalker::Report
{
public:
    StackWalkCrashReport(
        CrashReporting::CrashReport* crashReport)
        : m_CrashReport(crashReport)
    {
        m_CrashReport->m_HasNativeCrash = true;
        m_CurrentCrashReportThread = NULL;
    }

    void OnCallstackEntry(StackWalker::CallstackEntryType eType, StackWalker::CallstackEntry &entry)
    {
        if (m_CurrentCrashReportThread)
        {
            if (entry.isManaged)
            {
                CrashReporting::StackFrame* stackFrame = UNITY_NEW(CrashReporting::StackFrame, kMemDefault) (
                    "", "", 0,
                    "", false, 0, 0, true, entry.undFullName);
                m_CurrentCrashReportThread->AddFrame(*stackFrame);
            }
            else
            {
                CrashReporting::StackFrame* stackFrame = UNITY_NEW(CrashReporting::StackFrame, kMemDefault) (
                    entry.moduleGUID, entry.moduleName, entry.baseOfImage,
                    entry.pdbName, false, entry.offset, entry.offset - entry.baseOfImage, false, "");
                m_CurrentCrashReportThread->AddFrame(*stackFrame);
            }
        }
    }

    void BeginReportThread(UInt32 threadNumber, const core::string& threadName, bool crashed)
    {
        if (m_CrashReport && !m_CurrentCrashReportThread)
        {
            m_CurrentCrashReportThread = UNITY_NEW(CrashReporting::Thread, kMemDefault) (threadNumber, threadName, crashed);
        }
    }

    void EndReportThread()
    {
        if (m_CrashReport && m_CurrentCrashReportThread)
        {
            m_CrashReport->m_NativeCrash.AddThread(*m_CurrentCrashReportThread);
            m_CurrentCrashReportThread = NULL;
        }
    }

private:
    CrashReporting::CrashReport* m_CrashReport;
    CrashReporting::Thread* m_CurrentCrashReportThread;
};

enum MonoLoadStatus
{
    kMonoLoadOK,
    kMonoLoadFailedMissingPath,
    kMonoLoadFailedNoOutOfProcessSupport,
    kMonoLoadFailedMissingDLL,
    kMonoFailedReadingFATs,
    kMonoFailedReadingMemory,
    kMonoDisabled
};

static void PrintMonoStatusToLog(HANDLE logFile, MonoLoadStatus status, const wchar_t* monoDllPath)
{
    switch (status)
    {
        case kMonoDisabled:
            // Do nothing.
            break;
        case kMonoLoadOK:
            hprintf(logFile, "Mono DLL loaded successfully at '%S'.\r\n", monoDllPath);
            break;
        case kMonoLoadFailedMissingPath:
            hprintf(logFile, "No mono DLL path set. This app is either running IL2CPP, or the crash occurred before mono was initialized.\r\n");
            break;
        case kMonoLoadFailedNoOutOfProcessSupport:
            hprintf(logFile, "*** WARNING ***\r\n"
                "Managed call stack frames will be incorrect or missing.\r\n"
                "The Mono DLL at '%S' does not provide out-of-process stack information support.\r\n"
                "Upgrade to a newer version of the Mono Scripting Runtime for more detailed debug information.\r\n",
                monoDllPath);
            break;
        case kMonoLoadFailedMissingDLL:
            hprintf(logFile, "*** WARNING ***\r\n"
                "Managed call stack frames will be incorrect or missing because the Mono DLL at '%S' could not be found.\r\n",
                monoDllPath);
            break;
        case kMonoFailedReadingFATs:
            hprintf(logFile, "*** WARNING ***\r\n"
                "Mono DLL loaded successfully at '%S' but failed to read Mono Function Access Tables. Stack frames may be missing or corrupt.\r\n",
                monoDllPath);
            break;
        case kMonoFailedReadingMemory:
            hprintf(logFile, "*** WARNING ***\r\n"
                "Mono DLL loaded successfully at '%S' but failed to read Mono stack frame memory. Stack frames may be missing or corrupt.\r\n",
                monoDllPath);
            break;
        default:
            hprintf(logFile, "*** WARNING ***\r\n"
                "Managed call stack frames will be incorrect or missing because the Mono DLL at '%S' could not be loaded (unknown error).\r\n",
                monoDllPath);
            break;
    }
}

#if ENABLE_MONO
static MonoLoadStatus InitializeOutOfProcessMono(HANDLE hProcess, const wchar_t* monoDllPath, const void* monoDomain, OutOfProcessMono** out)
{
    assert(out);
    *out = nullptr;

    if (!monoDllPath || !*monoDllPath)
    {
        return kMonoLoadFailedMissingPath;
    }

    OutOfProcessMono* oopMono = OutOfProcessMono::Initialize(hProcess, monoDllPath, reinterpret_cast<const MonoDomain*>(monoDomain));
    if (oopMono == nullptr)
    {
        if (FileExists(monoDllPath))
        {
            return kMonoLoadFailedNoOutOfProcessSupport;
        }
        else
        {
            return kMonoLoadFailedMissingDLL;
        }
    }

    *out = oopMono;
    return kMonoLoadOK;
}

#if UNITY_64
MonoLoadStatus InitializeFunctionAccessTables64(OutOfProcessMono& oopMono, const void* monoFAT64)
{
    try
    {
        if (!oopMono.LoadFATs(monoFAT64))
        {
            return kMonoFailedReadingFATs;
        }
    }
    catch (const OutOfProcessMono::ReadException&)
    {
        return kMonoFailedReadingMemory;
    }
    return kMonoLoadOK;
}

#endif // UNITY_64
#endif // ENABLE_MONO

bool IsValidExceptionRecord(const EXCEPTION_RECORD* record)
{
    // This simply determines if the entire structure is zero or not
    static_assert((sizeof(record) % sizeof(DWORD)) == 0, "EXCEPTION_RECORD must be DWORD aligned.");
    for (const DWORD *cur = reinterpret_cast<const DWORD*>(record), *end = reinterpret_cast<const DWORD*>(record + 1);
         cur != end; ++cur)
    {
        if (*cur != 0)
            return true;
    }
    return false;
}

bool SafeShowCallstack(MonoStackWalker& stackWalk, HANDLE hThread, const CONTEXT* context = nullptr, int maxFrames = 1024)
{
    __try
    {
        stackWalk.ShowCallstack(hThread, context, maxFrames);
        return true;
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        return false;
    }
}

bool GenerateReportSummary(
    const wchar_t* reportSummaryFilePath,
    HANDLE hProcess,
    const CrashReport& report,
    const ProcessThreadsSnapshot& threads,
    MonoLoadStatus monoLoadStatus,
    MonoStackWalker& stackWalk,
    const winutils::CrashDataHeader& crashData)
{
    winutils::AutoHandle reportSummaryFile(CreateFileW(
        reportSummaryFilePath,
        GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL | FILE_FLAG_WRITE_THROUGH, 0));
    // We deliberately continue here, even if CreateFileW fails. We still want
    // to go through the report procedure here anyway, because this function
    // also uploads the report to the Crash Report service.
    // (hprintf is resilient to the file handle being invalid.)

    hprintf(reportSummaryFile.Get(), "\xEF\xBB\xBF"); // UTF-8 BOM
    hprintf(reportSummaryFile.Get(), "%s by %s [version: %s]\r\n\r\n", report.appName.c_str(), report.vendor.c_str(), report.info.c_str());

    // Get the name of the crashed module
    std::wstring crashedModuleName;
    GetCrashedModuleName(hProcess, (PCONTEXT)&crashData.Context, crashedModuleName);

    // Print out the beginning of the error log in a Win95 error window compatible format.
    if (IsValidExceptionRecord(&crashData.ExceptionRecord))
    {
        hprintf(reportSummaryFile.Get(), "%s caused %s (0x%08x)\r\n  in module %s at %04x:%08x.\r\n\r\n",
            WideToUtf8(crashedModuleName).c_str(), GetExceptionDescription(crashData.ExceptionRecord.ExceptionCode),
            crashData.ExceptionRecord.ExceptionCode, WideToUtf8(crashedModuleName).c_str(), crashData.Context.SegCs,
            GetInstructionPointerFromContext((PCONTEXT)&crashData.Context));
    }
    else
    {
        hprintf(reportSummaryFile.Get(), "%s %s in module %s at %04x:%08x.\r\n\r\n",
            WideToUtf8(crashedModuleName).c_str(), crashData.Description,
            WideToUtf8(crashedModuleName).c_str(), crashData.Context.SegCs,
            GetInstructionPointerFromContext((PCONTEXT)&crashData.Context));
    }

    DumpSystemInformation(reportSummaryFile.Get(), hProcess);

    // Dump free disk space on the report drive
    ULARGE_INTEGER diskFreeBytes = { 0, 0 },
                   diskTotalBytes = { 0, 0 };
    std::wstring reportDirectory = GetDirectoryPart(reportSummaryFilePath);
    if (!reportDirectory.empty() && GetDiskFreeSpaceExW(reportDirectory.c_str(), &diskFreeBytes, &diskTotalBytes, nullptr))
    {
        hprintf(reportSummaryFile.Get(), "Disk space data for '%s': %llu bytes free of %llu total.\r\n\r\n", WideToUtf8(reportDirectory).c_str(), diskFreeBytes.QuadPart, diskTotalBytes.QuadPart);
    }
    else
    {
        hprintf(reportSummaryFile.Get(), "Disk space data for '%s' is unavailable.\r\n\r\n", WideToUtf8(reportDirectory).c_str());
    }

    // If the exception was an access violation, print out some additional
    // information, to the error log and the debugger.
    if (crashData.ExceptionRecord.ExceptionCode == STATUS_ACCESS_VIOLATION && crashData.ExceptionRecord.NumberParameters >= 2)
    {
        TCHAR szDebugMessage[1000];
        const char * readwrite = "Read from";
        if (crashData.ExceptionRecord.ExceptionInformation[0])
            readwrite = "Write to";
        StringCchPrintf(
            szDebugMessage, _countof(szDebugMessage),
            "%s location %08p caused an access violation.\r\n",
            readwrite, crashData.ExceptionRecord.ExceptionInformation[1]);

        hprintf(reportSummaryFile.Get(), "%s", szDebugMessage);
    }

    DumpRegisters(reportSummaryFile.Get(), (PCONTEXT)&crashData.Context);
    hprintf(reportSummaryFile.Get(), "\r\n");

    // Print out the bytes of code at the instruction pointer.
    DumpCodeBytes(reportSummaryFile.Get(), hProcess, (PCONTEXT)&crashData.Context);
    hprintf(reportSummaryFile.Get(), "\r\n");

    // Load Mono
    PrintMonoStatusToLog(reportSummaryFile.Get(), monoLoadStatus, Utf8ToWide(report.monoPath).c_str());

    StackWalkLogReport stackWalkReport(reportSummaryFile.Get());
    stackWalk.SetReport(&stackWalkReport);

    // Print the crashing stack to the report
    HANDLE hThread = OpenThread(THREAD_GET_CONTEXT | THREAD_SUSPEND_RESUME | THREAD_QUERY_INFORMATION, false, crashData.ThreadID);
    if (hThread)
    {
        hprintf(reportSummaryFile.Get(), "\r\n\r\nStack Trace of Crashed Thread %d:\r\n", crashData.ThreadID);
        if (!SafeShowCallstack(stackWalk, hThread, &crashData.Context))
            hprintf(reportSummaryFile.Get(), _T("Exception encountered during stack walk.\r\n"));
        CloseHandle(hThread);
    }
    else
    {
        stackWalk.OnOutput("<Unable to access crash thread information>\r\n");
    }

    // Walk all stacks
    hprintf(reportSummaryFile.Get(), "\r\nStacks for Running Threads:\r\n\r\n");
    const ProcessThreadsSnapshot::HandleList& threadHandles = threads.GetThreads();
    for (ProcessThreadsSnapshot::HandleList::const_iterator i = threadHandles.begin();
         i != threadHandles.end(); ++i)
    {
        DWORD id = GetThreadId(*i);
        if (id != crashData.ThreadID)
        {
            hprintf(reportSummaryFile.Get(), "Call Stack for Thread %d:\r\n", id);
            if (!SafeShowCallstack(stackWalk, *i))
                hprintf(reportSummaryFile.Get(), _T("Exception encountered during stack walk.\r\n"));
            hprintf(reportSummaryFile.Get(), "\r\n\r\n");
        }
    }
    hprintf(reportSummaryFile.Get(), "\r\n\r\n");

    // Time to print part or all of the stack to the error log. This allows
    // us to figure out the call stack, parameters, local variables, etc.
    if (crashData.StackTop != nullptr)
    {
#if defined(_AMD64_)
        DWORD* pStack = reinterpret_cast<DWORD*>(crashData.Context.Rsp);
#else
        DWORD* pStack = reinterpret_cast<DWORD*>(crashData.Context.Esp);
#endif
        DumpStack(reportSummaryFile.Get(), hProcess, pStack, reinterpret_cast<const DWORD*>(crashData.StackTop));
    }
    else
    {
        hprintf(reportSummaryFile.Get(), "No call stack available.\r\n");
    }

    // Dump the module list
    DumpModuleList(reportSummaryFile.Get(), hProcess);

    // Dump crash report configuration
    DumpCrashReportConfiguration(reportSummaryFile.Get(), report);

    hprintf(reportSummaryFile.Get(), "\r\n== [end of %S] ==\r\n", CRASH_RPT_ERROR_LOG_NAME);
    hflush(reportSummaryFile.Get());
    reportSummaryFile.Close();

    return true;
}

bool GenerateMiniDump(const wchar_t* errDumpFileName, HANDLE hProcess, DWORD threadID, PEXCEPTION_POINTERS ep)
{
    const HANDLE miniDumpFile = CreateFileW(errDumpFileName, GENERIC_WRITE,
        0, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_WRITE_THROUGH, 0);
    if (miniDumpFile != INVALID_HANDLE_VALUE)
    {
        bool ok = DumpMiniDump(miniDumpFile, hProcess, threadID, ep);
        CloseHandle(miniDumpFile);
        if (!ok)
        {
            DeleteFileW(errDumpFileName);
        }
        return ok;
    }
    return false;
}

// This function is shamelessly stolen from PlatformWrapper.cpp
UInt64 GetCurrentMillisecondsInUTC()
{
    FILETIME ft;
    GetSystemTimeAsFileTime(&ft);
    UInt64 t = ((((UInt64)ft.dwHighDateTime << 32) | ft.dwLowDateTime) / 10000);
    t -= 11644473600000ULL; // Converting file time to unix epoch
    return t;
}

CrashReporting::CrashReport* InitializeCrashReport(const CrashReport& report)
{
    CrashReporting::CrashReport* crashReport = NULL;

    const char* reportJson = report.GetMetaData("ReportJson");

    if (reportJson)
    {
        crashReport = UNITY_NEW(CrashReporting::CrashReport, kMemDefault) (CrashReporting::CrashReport::FromJsonString(reportJson));

        // Make sure we update the timestamp, as the serialized one we used as the starting point was likely
        // created much earlier than now
        crashReport->m_ClientTimestamp = GetCurrentMillisecondsInUTC();

        crashReport->m_IsEditor = report.isEditor;

        crashReport->m_LogMessages = report.logBuffer.GetLogMessages();

        crashReport->m_UserMetadata.clear_dealloc();
        crashReport->m_UserMetadata.reserve(report.userMetadata.size());
        for (std::map<core::string, core::string>::const_iterator  it = report.userMetadata.begin(); it != report.userMetadata.end(); ++it)
            crashReport->m_UserMetadata.push_back(CrashReporting::UserMetadata(it->first, it->second));

        // Thread and stack frame data is added later in StackWalkCrashReport::OnCallstackEntry()
    }

    return crashReport;
}

bool StartBugReporter(std::wstring& cmd)
{
    STARTUPINFOW startupInfo;
    ::ZeroMemory(&startupInfo, sizeof(startupInfo));
    startupInfo.cb = sizeof(startupInfo);
    startupInfo.dwFlags = STARTF_USESHOWWINDOW;
    startupInfo.wShowWindow = SW_SHOW;

    PROCESS_INFORMATION processInfo;
    ::ZeroMemory(&processInfo, sizeof(processInfo));

    return ::CreateProcessW(
        nullptr, // appName
        const_cast<wchar_t*>(cmd.c_str()),
        0, // processSecurity
        0, // threadSecurity
        FALSE, // inheritHandles
        0, // processFlags
        0, // environment
        nullptr, // workingDir
        &startupInfo,
        &processInfo) == TRUE;
}

template<class TString> TString& SubstituteString(TString& s, const TString& replace, const TString& with)
{
    for (size_t pos = s.find(replace); pos != std::string::npos; pos = s.find(replace))
    {
        s.replace(pos, replace.size(), with);
    }
    return s;
}

template<class TString> TString SubstituteString(const TString& s, const TString& replace, const TString& with)
{
    TString s2(s);
    SubstituteString(s2, replace, with);
    return s2;
}

void SubstituteMetaDataStrings(std::string& str, const CrashReport& report)
{
    std::regex metaRegex("<meta:(\\w+)>", std::regex_constants::icase);
    std::smatch matches;
    //
    // Replace <meta:string> with report.metaData[string]
    //
    while (std::regex_search(str, matches, metaRegex))
    {
        if (matches.size() != 2)
        {
            printf_console("Malformed meta request in bug reporter path: %s\n", matches.str().c_str());
        }
        else
        {
            std::string replaceWith;
            std::map<std::string, std::string>::const_iterator meta = report.metaData.find(matches[1]);
            if (meta == report.metaData.end())
            {
                printf_console("Warning: couldn't find metadata '%s'\n", matches[1].str().c_str());
            }
            else
            {
                replaceWith = meta->second;
            }
            SubstituteString(str, matches[0].str(), replaceWith);
        }
    }
}

void SubstituteAttachReportFiles(std::wstring& str, const std::vector<std::wstring>& files)
{
    std::wregex attachRegex(L"<attachFile:(.*<file>.*)>", std::regex_constants::icase);
    std::wsmatch matches;
    if (std::regex_search(str, matches, attachRegex))
    {
        if (matches.size() != 2)
        {
            printf_console("Malformed attachFiles request in bug reporter path: %s\n", matches.str().c_str());
        }
        else
        {
            const std::wstring replaceIn(matches[1].str());
            const std::wstring replace(L"<file>");
            std::wstringstream replaceWith;
            for (size_t i = 0; i < files.size(); ++i)
            {
                replaceWith << SubstituteString(replaceIn, replace, files[i]);
                if (i != files.size() - 1)
                    replaceWith << " ";
            }
            SubstituteString(str, matches[0].str(), replaceWith.str());
        }
    }
}

// Replace known strings in the bug reporter command line
std::wstring GetBugReporterCommandLine(const std::string& bufReporterPath, const CrashReport& report, const std::vector<std::wstring>& bugReportFiles)
{
    std::string cmdLine = bufReporterPath;

    // Replace meta: tags and other info
    SubstituteMetaDataStrings(cmdLine, report);
    SubstituteString(cmdLine, std::string("<appName>"), report.appName);
    SubstituteString(cmdLine, std::string("<vendor>"), report.vendor);
    SubstituteString(cmdLine, std::string("<version>"), report.info);
    SubstituteString(cmdLine, std::string("<crashReportPath>"), report.crashReportPath);

    // Replace attachFile: tags
    std::wstring cmdLineW(Utf8ToWide(cmdLine));
    SubstituteAttachReportFiles(cmdLineW, bugReportFiles);

    return cmdLineW;
}

int HandleProcessCrash(DWORD processID, HANDLE hProc, const void* sharedRegionAddress, const CrashReport& report, bool terminate, bool waitForUser, HANDLE hResumeEvent)
{
    // Take a snapshot of the threads. The crashing app will have (theoretically) suspended them.
    ProcessThreadsSnapshot threadSnapshot(
        processID,
        THREAD_GET_CONTEXT | THREAD_QUERY_INFORMATION,
        false,  // suspend
        false); // auto-resume

    // Read the shared area data
    winutils::CrashDataHeader crashData = {};
    SIZE_T numBytesRead = 0;
    if (!ReadProcessMemory(hProc, sharedRegionAddress, (LPVOID)&crashData, sizeof(crashData), &numBytesRead))
    {
        printf_console("Failed to read PID %d's memory at %p (%d bytes)\n", processID, sharedRegionAddress, sizeof(crashData));
        return k_ExitFailedToReadCrashData;
    }

    // Validate it
    if (crashData.Magic != winutils::CrashDataHeader::k_Magic)
    {
        printf_console("Crash data sent by PID %d is corrupt: expected magic '0x%llx' but got '0x%llx'\n", processID, winutils::CrashDataHeader::k_Magic, crashData.Magic);
        return k_ExitCorruptCrashData;
    }
    if (crashData.HeaderSize != sizeof(crashData))
    {
        printf_console("Crash data sent by PID %d is corrupt: expected size %d but got %d\n", processID, sizeof(crashData), crashData.HeaderSize);
        return k_ExitCorruptCrashData;
    }
    if (crashData.ProcessID != processID)
    {
        printf_console("Crash data sent by PID %d is corrupt: expected to agree on PID but got %d\n", processID, crashData.ProcessID);
        return k_ExitCorruptCrashData;
    }

    // Create the dump directory
    std::wstring dumpDirectory;
    if (!CreateDumpDirectory(Utf8ToWide(report.crashReportPath), dumpDirectory))
    {
        printf_console("Couldn't create crash dump directory: %S\n", dumpDirectory.c_str());
    }

    // Get the process file name
    wchar_t processFileName[2048];
    GetModuleFileNameExW(hProc, nullptr, processFileName, _countof(processFileName));

    CrashUI& crashUI = CrashUI::Get();

    // Show the crash UI
    if (report.showDialog)
    {
        crashUI.Show(
            processFileName,
            Utf8ToWide(report.appName).c_str(),
            Utf8ToWide(report.info).c_str());
    }

    std::wstring reportSummaryFilePath = dumpDirectory + L"\\" CRASH_RPT_ERROR_LOG_NAME;
    std::wstring crashDumpFilePath = dumpDirectory + L"\\" CRASH_RPT_MINI_DUMP_NAME;
    CrashReporting::CrashReport* crashReport = InitializeCrashReport(report);

    std::vector<std::wstring> bugReportFiles;

    // Load mono
    OutOfProcessMono* oopMono = nullptr;
    MonoLoadStatus monoStatus = kMonoDisabled;
#if ENABLE_MONO
    monoStatus = InitializeOutOfProcessMono(hProc, Utf8ToWide(report.monoPath).c_str(), crashData.MonoDomain, &oopMono);
#if UNITY_64
    if (oopMono)
    {
        monoStatus = InitializeFunctionAccessTables64(*oopMono, crashData.MonoFAT64);
    }
#endif
#endif

    // Initialize the stack walker
    MonoStackWalker stackWalk(GetProcessId(hProc), hProc, oopMono, nullptr);
    stackWalk.LoadModules();
    stackWalk.LoadVirtualModules();

    crashUI.NotifyProgress(20);

    // Dump minidump
    EXCEPTION_POINTERS exInfo = { &crashData.ExceptionRecord, &crashData.Context };
    if (GenerateMiniDump(crashDumpFilePath.c_str(), hProc, crashData.ThreadID, &exInfo))
    {
        bugReportFiles.push_back(crashDumpFilePath);
    }
    else
    {
        printf_console("Failed to generate minidump at '%S' (%x)\n", crashDumpFilePath.c_str(), HRESULT_FROM_WIN32(GetLastError()));
    }

    crashUI.NotifyProgress(40);

    if (crashReport)
    {
        // Get the call stacks for all the threads
        StackWalkCrashReport stackWalkReport(crashReport);
        stackWalk.SetReport(&stackWalkReport);

        int threadNum = 0;
        const ProcessThreadsSnapshot::HandleList& threadHandles = threadSnapshot.GetThreads();
        for (ProcessThreadsSnapshot::HandleList::const_iterator i = threadHandles.begin();
             i != threadHandles.end(); ++i)
        {
            DWORD id = GetThreadId(*i);
            stackWalkReport.BeginReportThread(threadNum++, Format("Thread %d", id), id == crashData.ThreadID);
            if (!SafeShowCallstack(stackWalk, *i))
                printf_console("Exception encountered during stack walk of thread %d\n", id);
            stackWalkReport.EndReportThread();
        }

        // Save the crash report to disk for upload later
        if (!CacheCrashReport(crashReport))
        {
            printf_console("Failed to cache crash report.\n");
        }
    }

    crashUI.NotifyProgress(60);

    // Dump report summary
    if (GenerateReportSummary(
        reportSummaryFilePath.c_str(),
        hProc,
        report,
        threadSnapshot,
        monoStatus,
        stackWalk,
        crashData))
    {
        bugReportFiles.push_back(reportSummaryFilePath);
    }
    else
    {
        printf_console("Failed to generate report summary at '%S'\n", reportSummaryFilePath.c_str());
    }

    crashUI.NotifyProgress(80);

    if (terminate && crashData.Flags.IsFatal)
    {
        // Terminate the process with the exception code, just as it would normally terminate
        // in this scenario.
        TerminateProcess(hProc, (UINT)crashData.ExceptionRecord.ExceptionCode);
    }
    else
    {
        // Not a fatal error - signal the host process that it should resume executing
        SetEvent(hResumeEvent);
    }

    // Copy additional files into the report folder and add them to the crash reporter
    for (
        std::vector<CrashReportFile>::const_iterator f = report.additionalFiles.begin();
        f != report.additionalFiles.end();
        ++f)
    {
        std::wstring sourceFile = Utf8ToWide(f->path);
        if (!FileExists(sourceFile.c_str()))
        {
            printf_console("Failed to find file \"%s\"\n", f->path.c_str());
        }
        else
        {
            std::wstring targetFile = MakeUniqueFileNameInTargetPath(sourceFile.c_str(), dumpDirectory.c_str());
            if (!CopyFileExW(sourceFile.c_str(), targetFile.c_str(), nullptr, nullptr, nullptr, COPY_FILE_FAIL_IF_EXISTS))
            {
                printf_console("Failed to copy \"%s\" to crash report folder at \"%S\"\n",
                    f->path.c_str(), dumpDirectory.c_str());
                bugReportFiles.push_back(sourceFile); // fall back to source path if target path couldn't be created
            }
            else
            {
                bugReportFiles.push_back(targetFile);
            }
        }
    }

    crashUI.NotifyProgress(100);

    bool shouldGoToWER = true;

    // Start the bug reporter if applicable
    if (!report.bugReporterAppPath.empty())
    {
        std::wstring cmdLine = GetBugReporterCommandLine(report.bugReporterAppPath, report, bugReportFiles);

        printf_console("Launching custom bug reporter with command line:\n  %S\n", cmdLine.c_str());

        if (StartBugReporter(cmdLine))
            shouldGoToWER = false;
        else
            printf_console("Failed to launch bug reporter: \"%s\"", report.bugReporterAppPath.c_str());
    }

    if (waitForUser)
        crashUI.Wait();
    else
        crashUI.Hide();

    return crashData.Flags.IsFatal ? (shouldGoToWER ? k_ExitOK : k_ExitBugReporter) : k_ExitNonFatalError;
}

int HandleCommPipeTermination(CrashHandlerCommPipe& commPipe)
{
    int errorCode = commPipe.Stop();
    if (errorCode != 0)
    {
        printf_console("Inter-process communication between Unity and the crash handler prematurely exited with code %d\n", errorCode);
        return k_ExitCommunicationError;
    }
    return 0;
}

int CrashHandlerMain(int argc, wchar_t** argv)
{
    bool terminateCrashedProcess = false;
    if (PopCommandLineSwitch(L"--terminate", argc, argv))
    {
        terminateCrashedProcess = true;
    }
    bool waitForUser = false;
    if (PopCommandLineSwitch(L"--wait-for-user", argc, argv))
    {
        waitForUser = true;
    }

    if (argc != 3)
    {
        return Usage(argv[0]);
    }

    wchar_t* processIdString = argv[1];
    wchar_t* sharedAreaVirtualAddressString = argv[2];

    DWORD processId = 0;
    size_t sharedAreaVirtualAddress = 0;
    if (!StringToInteger(processIdString, &processId) ||
        !StringToInteger(sharedAreaVirtualAddressString, &sharedAreaVirtualAddress))
    {
        return Usage(argv[0]);
    }

    // Attempt to open the process handle
    // Only use the access flags that we actually require, as using blanket PROCESS_ALL_ACCESS interferes with certain anti-cheat software (case 1204191)
    winutils::AutoHandle hProc(OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION | SYNCHRONIZE, false, processId));
    if (!hProc.IsValid())
    {
        printf_console("Couldn't attach to process PID %d\n", processId);
        return k_ExitFailedToAttach;
    }

    // Attempt to open the shared event
    winutils::SharedCrashNotificationEvents events;
    if (!winutils::CreateSharedCrashNotificationEvents(processId, events))
    {
        printf_console("Couldn't create shared event for PID %d\n", processId);
        return k_ExitCreateSharedEventFailure;
    }

    // Validate the stdin pipe
    HANDLE hStdIn = GetStdHandle(STD_INPUT_HANDLE);
    if (hStdIn == nullptr || hStdIn == INVALID_HANDLE_VALUE)
    {
        printf_console("Invalid pipe specified for STDIN.\n");
        return k_ExitFailedToOpenPipe;
    }

    // Set up the crash reporter
    CrashReport report;
    CrashReportMessageHandler reportMessageHandler(&report);

    // Start the communication thread
    CrashHandlerCommPipe commPipe(hStdIn, &reportMessageHandler);

    while (true)
    {
        // Wait for either the application to signal the event, or for the application to terminate
        HANDLE waitOn[] = { hProc.Get(), events.HostDataReady.Get(), commPipe.ThreadHandle(), events.RequestTerminate.Get() };
        DWORD waitResponse = WaitForMultipleObjectsEx(_countof(waitOn), waitOn, false, INFINITE, false);

        switch (waitResponse)
        {
            case WAIT_OBJECT_0:
                // The application has exited. We'll deal with this specially, then exit.
                return HandleProcessExit(processId, hProc.Get());
            case WAIT_OBJECT_0 + 1:
            {
                // The event was signaled, which means the host would like us to take a capture (e.g. because it crashed)

                // Reopen the process with full access rights
                winutils::AutoHandle hProcFullAccess(OpenProcess(PROCESS_ALL_ACCESS, false, processId));
                if (!hProcFullAccess.IsValid())
                {
                    printf_console("Couldn't attach to process PID %d\n", processId);
                    return k_ExitFailedToAttach;
                }

                // Handle the crash
                int result = HandleProcessCrash(processId, hProcFullAccess.Get(), reinterpret_cast<const void*>(sharedAreaVirtualAddress), report, terminateCrashedProcess, waitForUser, events.HostCanContinue.Get());

                if (result == k_ExitNonFatalError)
                    continue;

                // Shut down the pipe thread
                int commExitCode = commPipe.Stop();
                if (commExitCode != 0)
                    printf_console("Warning: IPC thread exited with code %d.\n", commExitCode);
                return result;
            }
            case WAIT_OBJECT_0 + 2:
                return HandleCommPipeTermination(commPipe);
            case WAIT_OBJECT_0 + 3:
                return k_ExitOK; // host requested we shut down gracefully
            case WAIT_FAILED:
                printf_console("Waiting on crash event failed with code %d\n", GetLastError());
                return k_ExitFailedToWait;
            default:
                printf_console("Warning: Wait on crash event threw unexpected error: %d. GetLastError is %d.\n", waitResponse, GetLastError());
                return k_ExitFailedToWait;
        }
    }

    // shouldn't get here
    return k_ExitOK;
}
