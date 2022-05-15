#include "UnityPrefix.h"
#include "MonoOOP.h"
#include <windows.h>

#if ENABLE_MONO
typedef int(*PROC_READ_MEMORY)(void* buffer, size_t* readSize, const void* address, size_t size, void* userdata);
typedef void(*PROC_READ_EXCEPTION)(const void* address, size_t size, void* userdata);
typedef void(*PROC_INIT_OUT_OF_PROCESS_ACCESS)(PROC_READ_MEMORY, PROC_READ_EXCEPTION, void*);
typedef int(*PROC_GET_STACK_FRAME_DETAILS)(const MonoDomain* domain, const void* frameAddress, MonoStackFrameDetails* frameDetails);

#if UNITY_64
typedef const void*(*PROC_ITERATE_FUNCTION_ACCESS_TABLE_64)(const void*);
typedef int(*PROC_GET_FUNCTION_ACCESS_TABLE_64)(const void*, uintptr_t*, uintptr_t*, void**, size_t*);
#endif

static int ReadMemoryCallback(void* buffer, size_t* readSize, const void* address, size_t size, void* userData)
{
    HANDLE hProcess = reinterpret_cast<HANDLE>(userData);
    return ReadProcessMemory(hProcess, address, buffer, size, reinterpret_cast<SIZE_T*>(readSize));
}

static void ReadExceptionCallback(const void* address, size_t size, void* userData)
{
    throw OutOfProcessMono::ReadException(
        reinterpret_cast<HANDLE>(userData), // hProcess
        address,
        size);
}

OutOfProcessMono::OutOfProcessMono(HANDLE hProcess, HMODULE hDLL, const MonoDomain* domain)
    : m_hMonoDLL(hDLL)
    , m_hProcess(hProcess)
    , m_MonoDomain(domain)
{
}

OutOfProcessMono::~OutOfProcessMono()
{
    if (m_hMonoDLL)
    {
        FreeLibrary(m_hMonoDLL);
    }
}

OutOfProcessMono* OutOfProcessMono::Initialize(HANDLE hProcess, const wchar_t* monoDllPath, const MonoDomain* domain)
{
    HMODULE hDLL = LoadLibraryW(monoDllPath);
    if (!hDLL)
        return nullptr;

    PROC_INIT_OUT_OF_PROCESS_ACCESS initAccess = (PROC_INIT_OUT_OF_PROCESS_ACCESS)
        GetProcAddress(hDLL, "mono_unity_oop_init");
    if (!initAccess)
    {
        FreeLibrary(hDLL);
        return false;
    }

    initAccess(ReadMemoryCallback, ReadExceptionCallback, hProcess);

    return new OutOfProcessMono(hProcess, hDLL, domain);
}

bool OutOfProcessMono::GetStackFrameDetails(
    const void* address,
    MonoStackFrameDetails* details) const
{
    if (!m_MonoDomain || !details)
        return false;

    PROC_GET_STACK_FRAME_DETAILS getFrameDetails = (PROC_GET_STACK_FRAME_DETAILS)
        GetProcAddress(m_hMonoDLL, "mono_unity_oop_get_stack_frame_details");
    if (!getFrameDetails)
        return false;

    return getFrameDetails(m_MonoDomain, address, details) != 0;
}

#if UNITY_64
bool OutOfProcessMono::LoadFATs(const void* monoFATs)
{
    PROC_ITERATE_FUNCTION_ACCESS_TABLE_64 iterateFAT64 = (PROC_ITERATE_FUNCTION_ACCESS_TABLE_64)
        GetProcAddress(m_hMonoDLL, "mono_unity_oop_iterate_dynamic_function_access_tables64");
    PROC_GET_FUNCTION_ACCESS_TABLE_64 getFAT64 = (PROC_GET_FUNCTION_ACCESS_TABLE_64)
        GetProcAddress(m_hMonoDLL, "mono_unity_oop_get_dynamic_function_access_table64");

    if (!iterateFAT64 || !getFAT64)
    {
        return false;
    }

    // Loop over the tables
    for (const void* it = monoFATs; it != nullptr; it = iterateFAT64(it))
    {
        uintptr_t moduleStart = 0, moduleEnd = 0;
        PRUNTIME_FUNCTION table = nullptr;
        size_t tableSize = 0;
        if (getFAT64(it, &moduleStart, &moduleEnd, (void**)&table, &tableSize))
        {
            // todo: add it to the list
            DynamicFunctionTable t;
            t.BeginAddress = moduleStart;
            t.EndAddress = moduleEnd;
            if (tableSize > 0)
            {
                t.Functions.resize(tableSize);
                if (!ReadProcessMemory(m_hProcess, table, t.Functions.data(), sizeof(RUNTIME_FUNCTION) * tableSize, nullptr))
                    throw ReadException(m_hProcess, table, sizeof(RUNTIME_FUNCTION) * tableSize);
            }
            m_FunctionTables.push_back(t);
        }
    }

    return true;
}

const OutOfProcessMono::DynamicFunctionTable* OutOfProcessMono::FindFunctionTableForAddress(DWORD64 address) const
{
    for (size_t i = 0; i < m_FunctionTables.size(); ++i)
    {
        const DynamicFunctionTable& table = m_FunctionTables[i];
        if (address >= table.BeginAddress && address < table.EndAddress)
            return &table;
    }

    return nullptr;
}

PRUNTIME_FUNCTION OutOfProcessMono::FindFunctionForAddress(const DynamicFunctionTable* table, DWORD64 address) const
{
    if (table == nullptr)
        return nullptr;

    for (size_t i = 0; i < table->Functions.size(); ++i)
    {
        PRUNTIME_FUNCTION func = const_cast<PRUNTIME_FUNCTION>(&table->Functions[i]);

        DWORD64 funcStart = table->BeginAddress + func->BeginAddress;
        DWORD64 funcEnd = table->BeginAddress + func->EndAddress;

        if (address >= funcStart && address < funcEnd)
            return func;
    }

    return nullptr;
}

PRUNTIME_FUNCTION OutOfProcessMono::FindFunctionForAddress(DWORD64 address) const
{
    return FindFunctionForAddress(FindFunctionTableForAddress(address), address);
}

#endif // _M_X64

#endif // ENABLE_MONO
