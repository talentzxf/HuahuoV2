#pragma once

#if ENABLE_MONO

#include <vector>

#include "Runtime/Mono/MonoIncludes.h"
#include <windef.h>
#include <intsafe.h>

struct MonoStackFrameDetails
{
    char* methodName;
    size_t methodNameLen;
    char* className;
    size_t classNameLen;
    char* assemblyName;
    size_t assemblyNameLen;
};

class OutOfProcessMono
{
public:
#if UNITY_64
    struct DynamicFunctionTable
    {
        DWORD64 BeginAddress;
        DWORD64 EndAddress;
        std::vector<RUNTIME_FUNCTION> Functions;
    };
#endif

    class ReadException
    {
    public:
        HANDLE hProcess;
        const void* Address;
        size_t Size;

        ReadException(
            HANDLE process,
            const void* address,
            size_t size)
            : hProcess(process)
            , Address(address)
            , Size(size)
        {
        }
    };

    ~OutOfProcessMono();

    static OutOfProcessMono* Initialize(HANDLE hProcess, const wchar_t* monoDllPath, const MonoDomain* domain);

    bool GetStackFrameDetails(
        const void* address,
        MonoStackFrameDetails* details) const;

#if UNITY_64
    // Loads the FATs. Can throw a ReadException.
    bool LoadFATs(const void* monoFATs);

    const DynamicFunctionTable* FindFunctionTableForAddress(DWORD64 address) const;
    PRUNTIME_FUNCTION FindFunctionForAddress(DWORD64 address) const;
    PRUNTIME_FUNCTION FindFunctionForAddress(const DynamicFunctionTable* table, DWORD64 address) const;
    const std::vector<DynamicFunctionTable>& GetFunctionTables() const { return m_FunctionTables; }
#endif

private:
    OutOfProcessMono(HANDLE hProcess, HMODULE monoDll, const MonoDomain* domain);

    OutOfProcessMono(const OutOfProcessMono&); // = delete
    OutOfProcessMono& operator=(const OutOfProcessMono&);   // = delete

    HMODULE m_hMonoDLL;
    HANDLE m_hProcess;
    const MonoDomain* m_MonoDomain;

#if UNITY_64
    std::vector<DynamicFunctionTable> m_FunctionTables;
#endif
};

#endif // ENABLE_MONO
