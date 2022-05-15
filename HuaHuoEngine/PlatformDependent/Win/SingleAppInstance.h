#pragma once

#include "UnityPrefix.h"
#include <windef.h>

class SingleAppInstance
{
public:
    SingleAppInstance(const char* messagePrefix);
    ~SingleAppInstance();

    bool FindOtherInstance();
    bool ActivateOtherInstance() const;
    void SendCommandToOtherInstance(const core::string& cmd, bool activateOtherInstance = false) const;

    /// Test if a given message is the identity message
    LRESULT CheckMessage(UINT message, WPARAM wParam, LPARAM lParam) const;

private:
    /// EnumWindows callback
    static BOOL CALLBACK SearchOtherInstance(HWND hWnd, LPARAM lParam);

    core::string m_MessagePrefix;
    core::string m_MutexName;
    HANDLE m_Mutex;
    UINT m_IdentityMessage; // message used to identify us
    HWND m_OtherWindow; // Handle of the window of the found other instance
};
