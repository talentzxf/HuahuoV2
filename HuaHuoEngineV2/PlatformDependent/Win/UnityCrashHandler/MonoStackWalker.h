#pragma once

#include "PlatformDependent/Win/StackWalker.h"

class OutOfProcessMono;

class MonoStackWalker : public StackWalker
{
public:
    class Report
    {
    public:
        virtual void OnDbgHelpErr(const char* funcName, const char* errorMsg, const void* addr) {}
        virtual void OnOutput(const char* text) {}
        virtual void OnCallstackEntry(CallstackEntryType type, CallstackEntry& entry) {}
    };

    MonoStackWalker(DWORD dwProcessId, HANDLE hProcess, OutOfProcessMono* oopMono, Report* report);

    void SetReport(Report* report)
    {
        m_Report = report;
    }

    virtual void OnDbgHelpErr(LPCSTR szFuncName, DWORD gle, DWORD64 addr);
    virtual void OnOutput(LPCSTR buffer);
    virtual void OnCallstackEntry(CallstackEntryType eType, CallstackEntry &entry);

    virtual BOOL TryGetManagedStackFrame(DWORD_PTR AddrPCOffset, CallstackEntry &callstackEntry);

    // Install dynamic "modules" into the symbol handler
    BOOL LoadVirtualModules();

private:
    OutOfProcessMono* m_OoPMono;
    Report* m_Report;

    MonoStackWalker();

#if ENABLE_MONO && UNITY_64
    static PVOID CALLBACK SymbolFunctionEntryCallback(
        _In_ HANDLE hProcess,
        _In_ ULONG64 AddrBase,
        _In_ ULONG64 UserContext);
#endif // ENABLE_MONO && UNITY_64
};
