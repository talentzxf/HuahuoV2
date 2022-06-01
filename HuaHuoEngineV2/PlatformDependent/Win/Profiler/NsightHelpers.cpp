#include "UnityPrefix.h"

#if ENABLE_PROFILER
#include "Windows.h"
#include <tchar.h>
#include "Runtime/Utilities/PathNameUtility.h"

// Check if Unity was launched from Nvidia Nsight Graphics Profiler.
// This is done by scanning Unity's virtual address space for a module named
// "Nvda.Graphics.Interception.dll"
// When Nvidia Nsight Graphics launches an application, it will inject its graphics
// interception dll into the address space of the launched app.  Therefore we can
// know if our application was launched by nsight graphics if we can find the interception
// dll within our application's address space.
bool WasAppLaunchedUnderNsightGpuCapture()
{
    uintptr_t address = 0;

    for (;;)
    {
        MEMORY_BASIC_INFORMATION memoryInfo;
        if (!VirtualQuery(reinterpret_cast<void*>(address), &memoryInfo, sizeof(memoryInfo)))
            break;

        if (memoryInfo.Type & MEM_IMAGE)
        {
            dynamic_array<wchar_t> dllPath(kMemTempAlloc);
            size_t nextSize = MAX_PATH;

            // GetModuleFileNameW will fail if our dllPath buffer is not large enough to hold the contents of the filename.
            // In that case, we can double the size of our dllPath buffer until we reach a size that can hold the contents of the filename.
            // At that point, we can resize the dllPath buffer to the actual filename length.
            do
            {
                dllPath.resize_uninitialized(nextSize);
                nextSize *= 2;

                auto size = GetModuleFileNameW(reinterpret_cast<HMODULE>(memoryInfo.AllocationBase), &dllPath[0], static_cast<DWORD>(dllPath.size()));
                dllPath.resize_uninitialized(size);
            }
            while (GetLastError() == ERROR_INSUFFICIENT_BUFFER);

            const unsigned int dllFilePathLength = dllPath.size();
            core::string dllFilePathAsString(kMemTempAlloc);
            dllFilePathAsString.resize(dllFilePathLength);
            wcstombs(&dllFilePathAsString[0], dllPath.data(), dllFilePathLength);
            ConvertSeparatorsToUnity(dllFilePathAsString);

            if (GetLastPathNameComponent(dllFilePathAsString) == "Nvda.Graphics.Interception.dll")
                return true;
        }

        // Esnure that we never attempt to access memory outside of our application's address space.
        if (std::numeric_limits<uintptr_t>::max() - address < memoryInfo.RegionSize)
            break;

        address += memoryInfo.RegionSize;
    }

    return false;
}

#endif
