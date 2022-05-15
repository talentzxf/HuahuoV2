#include "PlatformDependent/Win/SingleAppInstance.h"
#include <windows.h>

SingleAppInstance::SingleAppInstance(const char* messagePrefix)
    :   m_MessagePrefix(messagePrefix)
    ,   m_Mutex(NULL)
    ,   m_OtherWindow(NULL)
{
    m_IdentityMessage = ::RegisterWindowMessageA((m_MessagePrefix + "IdentityMessage").c_str());
}

SingleAppInstance::~SingleAppInstance()
{
    if (m_Mutex)
        ::CloseHandle(m_Mutex);
}

bool SingleAppInstance::FindOtherInstance()
{
    if (m_IdentityMessage == 0)
        return false;

    // Based on method by Joseph M. Newcomer to avoid multiple instances of an application:
    // http://www.codeproject.com/cpp/avoidmultinstance.asp
    // We limit instances to the desktop.
    m_MutexName = core::Join(m_MessagePrefix, "-SingleInstanceMutex-");
    HDESK desktop = ::GetThreadDesktop(::GetCurrentThreadId());
    DWORD len = 0;
    BOOL result = ::GetUserObjectInformation(desktop, UOI_NAME, NULL, 0, &len);
    if (result == 0 && ::GetLastError() == ERROR_INSUFFICIENT_BUFFER)
    {
        char* info;
        ALLOC_TEMP_AUTO(info, len);
        ::GetUserObjectInformation(desktop, UOI_NAME, info, len, &len);
        m_MutexName += info;
    }

    // Try to create a mutex. If it already exists, then we have another instance running.
    m_Mutex = ::CreateMutex(NULL, FALSE, m_MutexName.c_str());
    DWORD err = ::GetLastError();
    if (err == ERROR_ALREADY_EXISTS || err == ERROR_ACCESS_DENIED)
    {
        ::EnumWindows(SearchOtherInstance, reinterpret_cast<LPARAM>(this));
        // Even if we don't find the other window, we want to return true here. The other window may not be created yet,
        // or the other process is zombied, in any case, we are not the only instance if mutex creation fails.
        return true;
    }

    return m_OtherWindow != NULL;
}

bool SingleAppInstance::ActivateOtherInstance() const
{
    if (m_OtherWindow == NULL)
        return false;

    // On Win2k, windows can't get focus by themselves,
    // so it is the responsibility of the new process to bring the window
    // to foreground.
    // Restore the other app and bring to foreground.
    if (::IsIconic(m_OtherWindow))
        ::ShowWindow(m_OtherWindow, SW_RESTORE);

    if (::SetForegroundWindow(m_OtherWindow) == FALSE)
        return false;

    return true;
}

void SingleAppInstance::SendCommandToOtherInstance(const core::string& cmd, bool activateOtherInstance /* = false*/) const
{
    if (activateOtherInstance)
        ActivateOtherInstance();

    COPYDATASTRUCT cds;
    cds.dwData = 0;
    cds.cbData = cmd.size() + 1;
    cds.lpData = reinterpret_cast<void*>(const_cast<char*>(cmd.c_str()));
    ::SendMessageA(m_OtherWindow, WM_COPYDATA, 0, reinterpret_cast<LPARAM>(&cds));
}

LRESULT SingleAppInstance::CheckMessage(UINT message, WPARAM wParam, LPARAM lParam) const
{
    if (message == m_IdentityMessage)
        return m_IdentityMessage;
    return 0;
}

BOOL CALLBACK SingleAppInstance::SearchOtherInstance(HWND hWnd, LPARAM lParam)
{
    DWORD_PTR result;

    SingleAppInstance* self = reinterpret_cast<SingleAppInstance *>(lParam);

    //@TODO:  Don't send messages to ourselves
    if (true /*hWnd != reinterpret_cast<HWND>(ui->stw->MainHWND())*/)
    {
        // Send a message and see if it responds in expected way.
        // Use a timeout so we aren't blocked by hung processes.
        LRESULT found = ::SendMessageTimeout(hWnd, self->m_IdentityMessage, 0, 0, SMTO_BLOCK | SMTO_ABORTIFHUNG, 200, &result);
        if (found != 0 && result == static_cast<DWORD_PTR>(self->m_IdentityMessage))
        {
            self->m_OtherWindow = hWnd;
            return FALSE; // stop enumeration
        }
    }

    return TRUE;
}
