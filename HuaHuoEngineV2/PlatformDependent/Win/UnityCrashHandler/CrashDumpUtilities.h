#pragma once

#include "UnityPrefix.h"
#include <windef.h>

#include <string>
#include <vector>

void hflush(HANDLE file);
void hprintf(HANDLE file, LPCTSTR Format, ...);
void FormatTime(char* output, size_t outputSize, FILETIME TimeToPrint);
bool DumpMiniDump(HANDLE file, HANDLE hProcess, DWORD threadID, PEXCEPTION_POINTERS excpInfo);
void DumpModuleList(HANDLE logFile, HANDLE hProcess);
void DumpSystemInformation(HANDLE logFile, HANDLE hProcess);
const TCHAR *GetExceptionDescription(DWORD ExceptionCode);
void DumpStack(HANDLE logFile, HANDLE hProcess, const DWORD* pStack, const DWORD* pStackTop);
void DumpRegisters(HANDLE logFile, PCONTEXT ctxRec);
void GetCrashedModuleName(HANDLE hProcess, PCONTEXT pContext, std::wstring& moduleName);
const void* GetInstructionPointerFromContext(PCONTEXT pContext);
void DumpCodeBytes(HANDLE file, HANDLE hProcess, PCONTEXT pContext);
