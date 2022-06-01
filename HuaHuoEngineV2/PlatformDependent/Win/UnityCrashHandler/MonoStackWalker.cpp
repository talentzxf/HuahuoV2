#include "UnityPrefix.h"
#include "MonoStackWalker.h"
#include "MonoOOP.h"

#include "PlatformDependent/Win/WinUtils.h"

#include <windows.h>
#include <DbgHelp.h>

MonoStackWalker::MonoStackWalker(
    DWORD dwProcessId,
    HANDLE hProcess,
    OutOfProcessMono* oopMono,
    Report* report)
    : StackWalker(dwProcessId, hProcess)
    , m_OoPMono(oopMono)
    , m_Report(report)
{
}

void MonoStackWalker::OnDbgHelpErr(LPCSTR szFuncName, DWORD gle, DWORD64 addr)
{
    if (m_Report)
    {
        core::string lastErrorMsg = winutils::ErrorCodeToMsg(gle);
        size_t l = -1;
        if ((l = lastErrorMsg.find_last_of('.')) != core::string::npos)
        {
            lastErrorMsg.resize(l + 1);
        }
        m_Report->OnDbgHelpErr(szFuncName, lastErrorMsg.c_str(), (LPVOID)addr);
    }
}

void MonoStackWalker::OnOutput(LPCSTR buffer)
{
    if (m_Report)
        m_Report->OnOutput(buffer);
}

void MonoStackWalker::OnCallstackEntry(CallstackEntryType eType, CallstackEntry &entry)
{
    StackWalker::OnCallstackEntry(eType, entry);

    if (m_Report)
    {
        m_Report->OnCallstackEntry(eType, entry);
    }
}

BOOL MonoStackWalker::TryGetManagedStackFrame(DWORD_PTR AddrPCOffset, CallstackEntry &callstackEntry)
{
#if !ENABLE_MONO
    return false;
#else
    if (!m_OoPMono)
        return false;

    char methodName[1024] = { 0 };
    char className[1024] = { 0 };

    MonoStackFrameDetails details = {};
    details.methodName = methodName;
    details.methodNameLen = _countof(methodName);
    details.className = className;
    details.classNameLen = _countof(className);
    details.assemblyName = callstackEntry.moduleName;
    details.assemblyNameLen = _countof(callstackEntry.moduleName);
    if (!m_OoPMono->GetStackFrameDetails(reinterpret_cast<const void*>(AddrPCOffset), &details))
    {
#if UNITY_64
        // GetStackFrameDetails is lossy. The FAT is truth.
        const OutOfProcessMono::DynamicFunctionTable* fat = m_OoPMono->FindFunctionTableForAddress(AddrPCOffset);
        if (fat == nullptr)
            return false;
        PRUNTIME_FUNCTION func = m_OoPMono->FindFunctionForAddress(fat, AddrPCOffset);
        if (func == nullptr)
            return false;
#else
        return false;
#endif
    }

    if (!methodName[0])
        sprintf_s(methodName, _countof(methodName), "<unknown method>");
    if (!className[0])
        sprintf_s(className, _countof(className), "<unknown class>");
    if (!callstackEntry.moduleName[0])
        sprintf_s(callstackEntry.moduleName, _countof(callstackEntry.moduleName), "Mono JIT Code");

    sprintf_s(callstackEntry.name, _countof(callstackEntry.name), "%s.%s()", className, methodName);

    callstackEntry.isManaged = true;

    return true;
#endif
}

// Install dynamic "modules" into the symbol handler
BOOL MonoStackWalker::LoadVirtualModules()
{
#if ENABLE_MONO && UNITY_64
    if (!m_OoPMono)
        return false;

    char err[1024];

    const std::vector<OutOfProcessMono::DynamicFunctionTable>& fats = m_OoPMono->GetFunctionTables();
    for (size_t i = 0; i < fats.size(); ++i)
    {
        const OutOfProcessMono::DynamicFunctionTable& fat = fats[i];
        DWORD fatSize = static_cast<DWORD>(fat.EndAddress - fat.BeginAddress);
        if (SymLoadModuleEx(m_hProcess, nullptr, nullptr, "Mono JIT Code", fat.BeginAddress, fatSize, nullptr, SLMFLAG_NO_SYMBOLS | SLMFLAG_VIRTUAL))
        {
            sprintf_s(err, _countof(err), "Loaded virtual Mono module at 0x%llx-0x%llx[%d].\r\n", fat.BeginAddress, fat.EndAddress, fatSize);
            OnOutput(err);
        }
        else
        {
            sprintf_s(err, _countof(err), "Failed to load virtual Mono module at 0x%llx-0x%llx[%d].\r\n", fat.BeginAddress, fat.EndAddress, fatSize);
            OnOutput(err);
        }
    }

    if (!SymRegisterFunctionEntryCallback64(m_hProcess, SymbolFunctionEntryCallback, reinterpret_cast<ULONG64>(m_OoPMono)))
    {
        sprintf_s(err, _countof(err), "Failed to register the managed symbol callback.\n");
        OnOutput(err);
        return FALSE;
    }

    return true;
#else
    return false;
#endif
}

MonoStackWalker::MonoStackWalker()
{
    AssertMsg(m_szSymPath == NULL, "InternalStackWalker: Symbol path should not be set by parent class");
}

#if ENABLE_MONO && UNITY_64
PVOID CALLBACK MonoStackWalker::SymbolFunctionEntryCallback(
    _In_ HANDLE hProcess,
    _In_ ULONG64 AddrBase,
    _In_ ULONG64 UserContext)
{
    const OutOfProcessMono* oopMono = reinterpret_cast<const OutOfProcessMono*>(UserContext);
    return oopMono->FindFunctionForAddress(AddrBase);
}

#endif // ENABLE_MONO && UNITY_64
