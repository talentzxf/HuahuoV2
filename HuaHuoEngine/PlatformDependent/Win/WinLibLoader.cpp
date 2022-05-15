#include "UnityPrefix.h"

#include "WinLibLoader.h"

namespace winlibloader
{
    HMODULE impl::LoadDll(const char* name)
    {
        return LoadLibraryA(name);
    }

    HMODULE impl::LoadDll(const wchar_t* name)
    {
        return LoadLibraryW(name);
    }
}
