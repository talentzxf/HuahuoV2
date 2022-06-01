#include "UnityPrefix.h"
#include "XInputDevices.h"

#include <wbemidl.h>

namespace win
{
    HINSTANCE XInputDevices::s_dll = 0;
    XInputDevices::XInputGetStateFunc XInputDevices::m_getStateFunc = 0;


    bool XInputDevices::Initialize()
    {
        Assert(!s_dll);

        s_dll = LoadLibrary("xinput1_3.dll");
        if (!s_dll)
        {
            printf_console("XInput1_3.dll not found. Trying XInput9_1_0.dll instead...\n");

            s_dll = LoadLibrary("xinput9_1_0.dll");
            if (!s_dll)
            {
                printf_console("XInput9_1_0.dll not found either. XInput-based controllers will not have full functionality.\n");
                return false;
            }
        }

        m_getStateFunc = (XInputGetStateFunc)GetProcAddress(s_dll, "XInputGetState");
        if (!m_getStateFunc)
        {
            printf_console("XInputGetState not found in the XInput DLL.\n");
            FreeLibrary(s_dll);
            s_dll = 0;
            return false;
        }

        return true;
    }

    void XInputDevices::Shutdown()
    {
        m_getStateFunc = 0;

        if (s_dll)
        {
            FreeLibrary(s_dll);
            s_dll = 0;
        }
    }

    DWORD XInputDevices::GetInputState(DWORD dwUserIndex, XINPUT_STATE* pState)
    {
        if (m_getStateFunc)
        {
            return m_getStateFunc(dwUserIndex, pState);
        }
        else
        {
            return ERROR_DEVICE_NOT_CONNECTED;
        }
    }

    void XInputDevices::RefreshAllDevices()
    {
        // ping all device state - beware this is expensive (~2ms per controller) if the device is disconnected
        if (m_getStateFunc)
        {
            XINPUT_STATE fakeState;
            for (DWORD i = 0; i < 4; ++i)
            {
                m_getStateFunc(i, &fakeState);
            }
        }
    }

    //-----------------------------------------------------------------------------
    bool XInputDevices::IsXInputDevice(const WCHAR* deviceName)
    {
        // Check if the device ID contains "IG_".  If it does, then it's an XInput device
        return (wcsstr(deviceName, L"IG_") || wcsstr(deviceName, L"ig_"));
    }

    //-----------------------------------------------------------------------------
    bool XInputDevices::GetUserIndex(DWORD bitset, DWORD& index)
    {
        // todo: find a proper way to get user index. this works 100% reliably for single joystick only

        if (m_getStateFunc)
        {
            for (DWORD i = 0; i < 4; ++i)
            {
                if (0 == (bitset & (1 << i)))
                {
                    XINPUT_STATE state;

                    if (ERROR_SUCCESS == m_getStateFunc(i, &state))
                    {
                        index = i;
                        return true;
                    }
                }
            }
        }

        return false;
    }
} // namespace
