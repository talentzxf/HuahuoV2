#include "UnityPrefix.h"
#include "DetectProcessorCores.h"

#include <winerror.h>
#include <errhandlingapi.h>
#include <sysinfoapi.h>

#include "Runtime/Utilities/dynamic_array.h"

// Implement this outside of SystemInfo.cpp since other tools want to use this functionality without the libraries that systeminfo requires
int DetectNumberOfProcessorCores()
{
    int cores = 0;

    // The first call to GetLogicalProcessorInformationEx() is supposed to fail, but only to let us know how much data we need to allocate
    DWORD returnLength = 0;
    if (!::GetLogicalProcessorInformationEx(RelationProcessorCore, nullptr, &returnLength) && ::GetLastError() != ERROR_INSUFFICIENT_BUFFER)
        return 1;

    dynamic_array<uint8_t> buffer(returnLength, kMemTempAlloc);
    if (!::GetLogicalProcessorInformationEx(RelationProcessorCore, reinterpret_cast<PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX>(buffer.data()), &returnLength))
        return 1;

    DWORD offset = 0;
    while (offset < returnLength)
    {
        ++cores;

        PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX info = reinterpret_cast<PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX>(buffer.data() + offset);

        // Since SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX is of variable length, we can't use simple pointer arithmetic.
        offset += info->Size;
    }

    return cores;
}

int DetectNumberOfLogicalProcessors()
{
    int processors = 0;

    // The first call to GetLogicalProcessorInformationEx() is supposed to fail, but only to let us know how much data we need to allocate
    DWORD returnLength = 0;
    if (!::GetLogicalProcessorInformationEx(RelationGroup, nullptr, &returnLength) && ::GetLastError() != ERROR_INSUFFICIENT_BUFFER)
        return 1;

    dynamic_array<uint8_t> buffer(returnLength, kMemTempAlloc);
    if (!::GetLogicalProcessorInformationEx(RelationGroup, reinterpret_cast<PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX>(buffer.data()), &returnLength))
        return 1;

    DWORD offset = 0;
    while (offset < returnLength)
    {
        PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX info = reinterpret_cast<PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX>(buffer.data() + offset);

        for (int group = 0; group < info->Group.ActiveGroupCount; ++group)
            processors += info->Group.GroupInfo[group].ActiveProcessorCount;

        // Since SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX is of variable length, we can't use simple pointer arithmetic.
        offset += info->Size;
    }

    return processors;
}
