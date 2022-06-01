#include "CrashDumpUtilities.h"
#include "Utilities.h"
#include "PlatformDependent/Win/WinUnicode.h"
#include "PlatformDependent/Win/WinUtils.h"
#include "PlatformDependent/Win/WinLib.h"
#include "PlatformDependent/Win/PathUnicodeConversion.h"
#include "PlatformDependent/Win/FileInformation.h"

#include <winbase.h>
#include <winver.h>
#include <timezoneapi.h>
#include <memoryapi.h>
#include <fileapi.h>
#include <processthreadsapi.h>
#include <Psapi.h>
#include <DbgHelp.h>
#include <TlHelp32.h>

// ---------------------------------------------------------------------------
//  crash handling internals

// largely based on:
// Copyright (C) 1998 Bruce Dawson
//
// This source file contains the exception handler for recording error
// information after crashes.
//
// Author:       Bruce Dawson
//               brucedawson@cygnus-software.com
// Modified by:  Hans Dietrich
//               hdietrich2@hotmail.com
// Version 1.1:  - reformatted output for XP-like error report
//               - added ascii output to stack dump
// Version 1.3:  - Added minidump output
// Version 1.4:  - Added invocation of XCrashReport.exe
//
// Further modified by Aras Pranckevicius, OTEE
//
// A paper by the original author can be found at:
//     http://www.cygnus-software.com/papers/release_debugging.html

#include <tchar.h>
#include <dbghelp.h>
#include <strsafe.h>

#pragma warning(disable:4312)

const int REP_CODE_BYTES = 16;          // Number of code bytes to record.
const int REP_MAX_STACK_DUMP = 2048;    // Maximum number of DWORDS in stack dumps.
const int REP_MAX_STACK_COLS = 4;       // Number of columns in stack dump.

#define ONEK            1024
#define SIXTYFOURK      (64*ONEK)
#define ONEM            (ONEK*ONEK)
#define ONEG            (ONEK*ONEK*ONEK)

// ---------------------------------------------------------------------------


// hprintf behaves similarly to printf, with a few vital differences.
// It uses wvsprintf to do the formatting, which is a system routine,
// thus avoiding C run time interactions. For similar reasons it
// uses WriteFile rather than fwrite.
// The one limitation that this imposes is that wvsprintf, and
// therefore hprintf, cannot handle floating point numbers.

// Too many calls to WriteFile can take a long time, causing
// confusing delays when programs crash. Therefore I implemented
// a simple buffering scheme for hprintf

#define HPRINTF_BUFFER_SIZE (8*1024)                // must be at least 2048
static char hprintf_buffer[HPRINTF_BUFFER_SIZE];    // wvsprintf never prints more than one K.
static int  hprintf_index = 0;

void hflush(HANDLE file)
{
    if (file != INVALID_HANDLE_VALUE && hprintf_index > 0)
    {
        DWORD NumBytes;
        WriteFile(file, hprintf_buffer, lstrlen(hprintf_buffer), &NumBytes, 0);
        hprintf_index = 0;
    }
}

void hprintf(HANDLE file, LPCTSTR Format, ...)
{
    if (file == INVALID_HANDLE_VALUE || file == nullptr)
        return;

    if (hprintf_index > (HPRINTF_BUFFER_SIZE - 1024))
    {
        DWORD NumBytes;
        WriteFile(file, hprintf_buffer, lstrlen(hprintf_buffer), &NumBytes, 0);
        hprintf_index = 0;
    }

    va_list arglist;
    va_start(arglist, Format);
    //hprintf_index += wvsprintf(&hprintf_buffer[hprintf_index], Format, arglist);
    STRSAFE_LPSTR end = nullptr;
    if (SUCCEEDED(StringCchVPrintfEx(
        &hprintf_buffer[hprintf_index],
        _countof(hprintf_buffer) - hprintf_index,
        &end,
        nullptr,
        0,
        Format,
        arglist)))
    {
        hprintf_index = static_cast<int>(end - hprintf_buffer);
    }
    va_end(arglist);
}

// Format the specified FILETIME to output in a human readable format,
// without using the C run time.
void FormatTime(char* output, size_t outputSize, FILETIME TimeToPrint)
{
    output[0] = 0;
    WORD Date, Time;
    if (FileTimeToLocalFileTime(&TimeToPrint, &TimeToPrint) &&
        FileTimeToDosDateTime(&TimeToPrint, &Date, &Time))
    {
        StringCchPrintf(output, outputSize,
            "%d-%02d-%02d_%02d%02d%02d",
            (Date / 512) + 1980, (Date / 32) & 15, Date & 31,
            (Time >> 11), (Time >> 5) & 0x3F, (Time & 0x1F) * 2);
    }
}

bool DumpMiniDump(HANDLE file, HANDLE hProcess, DWORD threadID, PEXCEPTION_POINTERS excpInfo)
{
    if (file == INVALID_HANDLE_VALUE || excpInfo == nullptr)
        return false;

    // Try to load dbghelp.dll - it is included only starting from WinXP by default
    // To save space we don't redistribute it though. Win2k users just can't get minidumps!
    using namespace winlib;
    if (GetDbghelp().Load())
    {
        if (GetDbghelp().IsFunctionLoaded<DbghelpDll::Fn::MiniDumpWriteDump>())
        {
            printf_console("writing minidump\n");

            MINIDUMP_EXCEPTION_INFORMATION eInfo;
            eInfo.ThreadId = threadID;
            eInfo.ExceptionPointers = excpInfo;
            eInfo.ClientPointers = false;

            return GetDbghelp().Invoke<DbghelpDll::Fn::MiniDumpWriteDump>(
                hProcess,
                GetProcessId(hProcess),
                file,
                MiniDumpNormal,
                excpInfo ? &eInfo : nullptr,
                nullptr,
                nullptr) != 0;
        }
        else
        {
            printf_console("ERROR: DumpMiniDump: MiniDumpWriteDump function in dbghelp.dll not found\n");
            return false;
        }
    }
    else
    {
        printf_console("ERROR: DumpMiniDump: dbghelp.dll not found\n");
        return false;
    }
}

// Print information about a code module (DLL or EXE) such as its size,
// location, time stamp, etc.
bool DumpModuleInfo(HANDLE logFile, HANDLE hProcess, HMODULE ModuleHandle, int nModuleNo)
{
    bool rc = false;
    wchar_t szModName[kDefaultPathBufferSize];
    ZeroMemory(szModName, sizeof(szModName));

    __try
    {
        if (GetModuleFileNameExW(hProcess, ModuleHandle, szModName, kDefaultPathBufferSize - 1) > 0)
        {
            // If GetModuleFileName returns greater than zero then this must
            // be a valid code module address. Therefore we can try to walk
            // our way through its structures to find the link time stamp.
            IMAGE_DOS_HEADER DosHeader;
            if (!ReadProcessMemory(hProcess, ModuleHandle, &DosHeader, sizeof(DosHeader), nullptr))
                return false;
            if (IMAGE_DOS_SIGNATURE != DosHeader.e_magic)
                return false;

            IMAGE_NT_HEADERS NTHeader;
            if (!ReadProcessMemory(hProcess, (TCHAR*)ModuleHandle + DosHeader.e_lfanew, &NTHeader, sizeof(NTHeader), nullptr))
                return false;
            if (IMAGE_NT_SIGNATURE != NTHeader.Signature)
                return false;

            // open the code module file so that we can get its file date and size
            HANDLE ModuleFile = CreateFileW(szModName, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING,
                FILE_ATTRIBUTE_NORMAL, 0);

            char TimeBuffer[100];
            TimeBuffer[0] = _T('\0');

            DWORD FileSize = 0;
            if (ModuleFile != INVALID_HANDLE_VALUE)
            {
                FileSize = GetFileSize(ModuleFile, 0);
                FILETIME LastWriteTime;
                if (GetFileTime(ModuleFile, 0, 0, &LastWriteTime))
                {
                    FormatTime(TimeBuffer, _countof(TimeBuffer), LastWriteTime);
                }
                CloseHandle(ModuleFile);
            }
            hprintf(logFile, _T("Module %d\r\n"), nModuleNo);
            char modNameAscii[kDefaultPathBufferSize];
            WideCharToMultiByte(CP_ACP, 0, szModName, -1, modNameAscii, kDefaultPathBufferSize - 1, 0, 0);
            hprintf(logFile, _T("%s\r\n"), modNameAscii);
            hprintf(logFile, _T("Image Base: 0x%08x  Image Size: 0x%08x\r\n"),
                NTHeader.OptionalHeader.ImageBase, NTHeader.OptionalHeader.SizeOfImage);

            hprintf(logFile, _T("File Size:  %-10d  File Time:  %s\r\n"), FileSize, TimeBuffer);

            hprintf(logFile, _T("Version:\r\n"));

            FileInformation fileInfo(szModName);
            TCHAR szBuf[200];

            fileInfo.GetCompanyName(szBuf, sizeof(szBuf) - 1);
            hprintf(logFile, _T("   Company:    %s\r\n"), szBuf);
            fileInfo.GetProductName(szBuf, sizeof(szBuf) - 1);
            hprintf(logFile, _T("   Product:    %s\r\n"), szBuf);
            fileInfo.GetFileDescription(szBuf, sizeof(szBuf) - 1);
            hprintf(logFile, _T("   FileDesc:   %s\r\n"), szBuf);
            FileInformation::Version version = fileInfo.GetFileVersion();
            hprintf(logFile, _T("   FileVer:    %d.%d.%d.%d\r\n"),
                version.v1, version.v2, version.v3, version.v4);
            version = fileInfo.GetProductVersion();
            hprintf(logFile, _T("   ProdVer:    %d.%d.%d.%d\r\n"),
                version.v1, version.v2, version.v3, version.v4);

            hprintf(logFile, _T("\r\n"));
            rc = true;
        }
    }
    // Handle any exceptions by continuing from this point.
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
    }
    return rc;
}

// Scan memory looking for code modules (DLLs or EXEs). VirtualQuery is used
// to find all the blocks of address space that were reserved or committed,
// and ShowModuleInfo will display module information if they are code
// modules.
void DumpModuleList(HANDLE logFile, HANDLE hProcess)
{
    SYSTEM_INFO SystemInfo;
    GetSystemInfo(&SystemInfo);

    const size_t PageSize = SystemInfo.dwPageSize;

    // Set NumPages to the number of pages in the 4GByte address space,
    // while being careful to avoid overflowing ints
    const size_t NumPages = 4 * size_t(ONEG / PageSize);
    size_t pageNum = 0;
    void *LastAllocationBase = 0;

    int nModuleNo = 1;

    while (pageNum < NumPages)
    {
        MEMORY_BASIC_INFORMATION MemInfo;
        if (VirtualQueryEx(hProcess, (void*)(pageNum * PageSize), &MemInfo, sizeof(MemInfo)))
        {
            if (MemInfo.RegionSize > 0)
            {
                // Adjust the page number to skip over this block of memory
                pageNum += MemInfo.RegionSize / PageSize;
                if (MemInfo.State == MEM_COMMIT && MemInfo.AllocationBase >
                    LastAllocationBase)
                {
                    // Look for new blocks of committed memory, and try
                    // recording their module names - this will fail
                    // gracefully if they aren't code modules
                    LastAllocationBase = MemInfo.AllocationBase;
                    if (DumpModuleInfo(logFile, hProcess, (HMODULE)LastAllocationBase, nModuleNo))
                    {
                        nModuleNo++;
                    }
                }
            }
            else
                pageNum += SIXTYFOURK / PageSize;
        }
        else
            pageNum += SIXTYFOURK / PageSize;

        // If VirtualQuery fails we advance by 64K because that is the
        // granularity of address space doled out by VirtualAlloc()
    }
}

#define ROUND_TO_MB(x) (((x) + ONEM - 1) / ONEM)

// Record information about where the crash occurred
void DumpSystemInformation(HANDLE logFile, HANDLE hProcess)
{
    char timeBuf[100];
    FILETIME currentTime;
    GetSystemTimeAsFileTime(&currentTime);
    FormatTime(timeBuf, _countof(timeBuf), currentTime);

    hprintf(logFile, _T("Error occurred at %s.\r\n"), timeBuf);

    WCHAR szModuleName[kDefaultPathBufferSize];
    ZeroMemory(szModuleName, sizeof(szModuleName));
    if (GetModuleFileNameExW(hProcess, nullptr, szModuleName, ARRAY_SIZE(szModuleName) - 2) <= 0)
        StringCchCopyW(szModuleName, _countof(szModuleName), L"Unknown");

    WCHAR szUserName[200];
    ZeroMemory(szUserName, sizeof(szUserName));
    DWORD UserNameSize = ARRAY_SIZE(szUserName) - 2;
    if (!GetUserNameW(szUserName, &UserNameSize))
        StringCchCopyW(szUserName, _countof(szUserName), L"Unknown");

    // petele: todo: check this; username is likely to be PII

    hprintf(logFile, _T("%s, run by %s.\r\n\r\n"), WideToUtf8(szModuleName).c_str(), WideToUtf8(szUserName).c_str());

    // Get system memory usage information
    MEMORYSTATUSEX systemMemInfo;
    systemMemInfo.dwLength = sizeof(systemMemInfo);
    GlobalMemoryStatusEx(&systemMemInfo);

    // Get process memory information
    PROCESS_MEMORY_COUNTERS processMemInfo;
    ZeroMemory(&processMemInfo, sizeof(processMemInfo));
    processMemInfo.cb = sizeof(processMemInfo);
    GetProcessMemoryInfo(hProcess, &processMemInfo, sizeof(processMemInfo));

    hprintf(logFile, _T("%d%% physical memory in use.\r\n"), systemMemInfo.dwMemoryLoad);
    hprintf(logFile, _T("%d MB physical memory [%d MB free].\r\n"), ROUND_TO_MB(systemMemInfo.ullTotalPhys), ROUND_TO_MB(systemMemInfo.ullAvailPhys));
    hprintf(logFile, _T("%d MB process peak paging file [%d MB used].\r\n"), ROUND_TO_MB(processMemInfo.PeakPagefileUsage), ROUND_TO_MB(processMemInfo.PagefileUsage));
    hprintf(logFile, _T("%d MB process peak working set [%d MB used].\r\n"), ROUND_TO_MB(processMemInfo.PeakWorkingSetSize), ROUND_TO_MB(processMemInfo.WorkingSetSize));

    // Get system performance info
    PERFORMACE_INFORMATION systemPerfInfo;
    ZeroMemory(&systemPerfInfo, sizeof(systemPerfInfo));
    systemPerfInfo.cb = sizeof(systemPerfInfo);
    GetPerformanceInfo(&systemPerfInfo, sizeof(systemPerfInfo));

    hprintf(logFile, _T("System Commit Total/Limit/Peak: %dMB/%dMB/%dMB\r\n"), ROUND_TO_MB(systemPerfInfo.CommitTotal * systemPerfInfo.PageSize), ROUND_TO_MB(systemPerfInfo.CommitLimit * systemPerfInfo.PageSize), ROUND_TO_MB(systemPerfInfo.CommitPeak * systemPerfInfo.PageSize));
    hprintf(logFile, _T("System Physical Total/Available: %dMB/%dMB\r\n"), ROUND_TO_MB(systemPerfInfo.PhysicalTotal * systemPerfInfo.PageSize), ROUND_TO_MB(systemPerfInfo.PhysicalAvailable * systemPerfInfo.PageSize));
    hprintf(logFile, _T("System Process Count: %d\r\n"), systemPerfInfo.ProcessCount);
    hprintf(logFile, _T("System Thread Count: %d\r\n"), systemPerfInfo.ThreadCount);
    hprintf(logFile, _T("System Handle Count: %d\r\n"), systemPerfInfo.HandleCount);
}

// Translate the exception code into something human readable
const TCHAR *GetExceptionDescription(DWORD ExceptionCode)
{
    struct ExceptionNames
    {
        DWORD   ExceptionCode;
        TCHAR * ExceptionName;
    };

    // names from <winnt.h>
    ExceptionNames ExceptionMap[] =
    {
        { 0x40010005, _T("a Control-C") },
        { 0x40010008, _T("a Control-Break") },
        { 0x80000002, _T("a Datatype Misalignment") },
        { 0x80000003, _T("a Breakpoint") },
        { 0xc0000005, _T("an Access Violation") },
        { 0xc0000006, _T("an In Page Error") },
        { 0xc0000017, _T("a No Memory") },
        { 0xc000001d, _T("an Illegal Instruction") },
        { 0xc0000025, _T("a Noncontinuable Exception") },
        { 0xc0000026, _T("an Invalid Disposition") },
        { 0xc000008c, _T("a Array Bounds Exceeded") },
        { 0xc000008d, _T("a Float Denormal Operand") },
        { 0xc000008e, _T("a Float Divide by Zero") },
        { 0xc000008f, _T("a Float Inexact Result") },
        { 0xc0000090, _T("a Float Invalid Operation") },
        { 0xc0000091, _T("a Float Overflow") },
        { 0xc0000092, _T("a Float Stack Check") },
        { 0xc0000093, _T("a Float Underflow") },
        { 0xc0000094, _T("an Integer Divide by Zero") },
        { 0xc0000095, _T("an Integer Overflow") },
        { 0xc0000096, _T("a Privileged Instruction") },
        { 0xc00000fD, _T("a Stack Overflow") },
        { 0xc0000142, _T("a DLL Initialization Failed") },
        { 0xe06d7363, _T("a Microsoft C++ Exception") },
    };

    for (int i = 0; i < sizeof(ExceptionMap) / sizeof(ExceptionMap[0]); i++)
    {
        if (ExceptionCode == ExceptionMap[i].ExceptionCode)
            return ExceptionMap[i].ExceptionName;
    }

    return _T("an Unknown exception type");
}

void DumpStack(
    HANDLE logFile,
    HANDLE hProcess,
    const DWORD* pStack,
    const DWORD* pStackTop)
{
    hprintf(logFile, _T("Stack Memory [0x%p-0x%p]:\r\n"), pStack, pStackTop);

    __try
    {
        if (pStackTop > pStack + REP_MAX_STACK_DUMP)
            pStackTop = pStack + REP_MAX_STACK_DUMP;

        int Count = 0;

        const DWORD* pStackStart = pStack;
        DWORD dwStack;

        int nDwordsPrinted = 0;

        while (pStack + 1 <= pStackTop)
        {
            if ((Count % REP_MAX_STACK_COLS) == 0)
            {
                pStackStart = pStack;
                nDwordsPrinted = 0;
                hprintf(logFile, _T("0x%08x: "), pStack);
            }

            if ((++Count % REP_MAX_STACK_COLS) == 0 || pStack + 2 > pStackTop)
            {
                if (!ReadProcessMemory(hProcess, pStack, &dwStack, sizeof(dwStack), nullptr))
                    dwStack = 0xccccccccul;

                hprintf(logFile, _T("%08x "), dwStack);
                nDwordsPrinted++;

                int n = nDwordsPrinted;
                while (n < 4)
                {
                    hprintf(logFile, _T("         "));
                    n++;
                }

                for (int i = 0; i < nDwordsPrinted; i++)
                {
                    if (!ReadProcessMemory(hProcess, pStackStart, &dwStack, sizeof(dwStack), nullptr))
                        dwStack = 0xccccccccul;

                    for (int j = 0; j < 4; j++)
                    {
                        char c = (char)(dwStack & 0xFF);
                        if (c < 0x20 || c > 0x7E)
                            c = '.';
#ifdef _UNICODE
                        WCHAR w = (WCHAR)c;
                        hprintf(logFile, _T("%c"), w);
#else
                        hprintf(logFile, _T("%c"), c);
#endif
                        dwStack = dwStack >> 8;
                    }
                    pStackStart++;
                }

                hprintf(logFile, _T("\r\n"));
            }
            else
            {
                if (!ReadProcessMemory(hProcess, pStack, &dwStack, sizeof(dwStack), nullptr))
                    dwStack = 0xccccccccul;

                hprintf(logFile, _T("%08x "), dwStack);
                nDwordsPrinted++;
            }
            pStack++;
        }
        hprintf(logFile, _T("\r\n"));
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        hprintf(logFile, _T("Exception encountered during stack dump.\r\n"));
    }
}

void DumpRegisters(HANDLE logFile, PCONTEXT ctxRec)
{
    // Print out the register values in an XP error window compatible format.
#if defined(_AMD64_)
    hprintf(logFile, _T("\r\n"));
    hprintf(logFile, _T("Context:\r\n"));
    hprintf(logFile, _T("RDI:    0x%016llx  RSI: 0x%016llx  RAX:   0x%016llx\r\n"),
        ctxRec->Rdi, ctxRec->Rsi, ctxRec->Rax);
    hprintf(logFile, _T("RBX:    0x%016llx  RCX: 0x%016llx  RDX:   0x%016llx\r\n"),
        ctxRec->Rbx, ctxRec->Rcx, ctxRec->Rdx);
    hprintf(logFile, _T("RIP:    0x%016llx  RBP: 0x%016llx  SegCs: 0x%016llx\r\n"),
        ctxRec->Rip, ctxRec->Rbp, ctxRec->SegCs);
    hprintf(logFile, _T("EFlags: 0x%016llx  RSP: 0x%016llx  SegSs: 0x%016llx\r\n"),
        ctxRec->EFlags, ctxRec->Rsp, ctxRec->SegSs);
    hprintf(logFile, _T("R8:     0x%016llx  R9:  0x%016llx  R10:   0x%016llx\r\n"),
        ctxRec->R8, ctxRec->R9, ctxRec->R10);
    hprintf(logFile, _T("R11:    0x%016llx  R12: 0x%016llx  R13:   0x%016llx\r\n"),
        ctxRec->R11, ctxRec->R12, ctxRec->R13);
    hprintf(logFile, _T("R14:    0x%016llx  R15: 0x%016llx\r\n"),
        ctxRec->R14, ctxRec->R15);
#else
    hprintf(logFile, _T("\r\n"));
    hprintf(logFile, _T("Context:\r\n"));
    hprintf(logFile, _T("EDI:    0x%08x  ESI: 0x%08x  EAX:   0x%08x\r\n"),
        ctxRec->Edi, ctxRec->Esi, ctxRec->Eax);
    hprintf(logFile, _T("EBX:    0x%08x  ECX: 0x%08x  EDX:   0x%08x\r\n"),
        ctxRec->Ebx, ctxRec->Ecx, ctxRec->Edx);
    hprintf(logFile, _T("EIP:    0x%08x  EBP: 0x%08x  SegCs: 0x%08x\r\n"),
        ctxRec->Eip, ctxRec->Ebp, ctxRec->SegCs);
    hprintf(logFile, _T("EFlags: 0x%08x  ESP: 0x%08x  SegSs: 0x%08x\r\n"),
        ctxRec->EFlags, ctxRec->Esp, ctxRec->SegSs);
#endif
}

void GetCrashedModuleName(HANDLE hProcess, PCONTEXT pContext, std::wstring& moduleName)
{
    wchar_t szCrashModulePathName[MAX_PATH];
    ZeroMemory(szCrashModulePathName, sizeof(szCrashModulePathName));
    const wchar_t *pszCrashModuleFileName = L"Unknown";
    const void *ip = GetInstructionPointerFromContext(pContext);
    MEMORY_BASIC_INFORMATION MemInfo;
    // VirtualQuery can be used to get the allocation base associated with a
    // code address, which is the same as the ModuleHandle. This can be used
    // to get the filename of the module that the crash happened in.
    if (VirtualQueryEx(hProcess, ip, &MemInfo, sizeof(MemInfo)) &&
        (GetModuleFileNameExW(hProcess, (HINSTANCE)MemInfo.AllocationBase,
            szCrashModulePathName,
            ARRAY_SIZE(szCrashModulePathName) - 2) > 0))
    {
        pszCrashModuleFileName = GetFilePart(szCrashModulePathName);
    }

    moduleName = pszCrashModuleFileName;
}

const void* GetInstructionPointerFromContext(PCONTEXT pContext)
{
#if defined(_AMD64_)
    return (const void*)pContext->Rip;
#else
    return (const void*)pContext->Eip;
#endif
}

void DumpCodeBytes(HANDLE file, HANDLE hProcess, PCONTEXT pContext)
{
    hprintf(file, _T("\r\nBytes at CS:EIP:\r\n"));

    const BYTE* ip = reinterpret_cast<const BYTE*>(GetInstructionPointerFromContext(pContext));
    BYTE code;
    for (int codebyte = 0; codebyte < REP_CODE_BYTES; codebyte++)
    {
        if (!ReadProcessMemory(hProcess, ip + codebyte, &code, sizeof(code), nullptr))
        {
            hprintf(file, _T("?? "));
        }
        else
        {
            hprintf(file, _T("%02x "), code);
        }
    }

    hprintf(file, _T("\r\n"));
}
