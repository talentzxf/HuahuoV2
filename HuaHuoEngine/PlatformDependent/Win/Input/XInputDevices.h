#pragma once

#include "xinput.h"

namespace win
{
    class XInputDevices
    {
    public:
        static bool Initialize();
        static void Shutdown();

        static bool GetUserIndex(DWORD bitset, DWORD& index);

        static bool IsXInputDevice(const WCHAR* deviceName);

        static DWORD GetInputState(DWORD dwUserIndex, XINPUT_STATE* pState);

        static void RefreshAllDevices();

    private:
        static HINSTANCE s_dll;

    private:
        typedef DWORD (WINAPI* XInputGetStateFunc)(DWORD dwUserIndex, XINPUT_STATE* pState);
        static XInputGetStateFunc m_getStateFunc;
    };
}
