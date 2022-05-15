#include "UnityPrefix.h"
#include "CrashUI.h"

#include "Runtime/Threads/Mutex.h"

#include "resource.h"
#include <windows.h>
#include <CommCtrl.h>
#include <ShellApi.h>

#include "Utilities.h"

static const int kDialogWidth = 340;
static const int kAppIconSize = 128;
static const int kErrorIconSize = 32;
static const int kFrameSize = 25;
static const int kProgressBarHeight = 8;

struct AutoModule
{
    AutoModule(HMODULE h) : handle(h) {}
    ~AutoModule()
    {
        FreeLibrary(handle);
    }

    HMODULE handle;
};

BOOL CALLBACK GroupIconEnumerationProc(HMODULE hModule, LPCSTR lpszType, LPSTR lpszName, LONG_PTR lParam)
{
    *((LPCSTR*)lParam) = lpszName;
    return false; // stop searching
}

static HICON GetImageFromImageLibrary(const wchar_t* library, int desiredWidth, int desiredHeight)
{
    AutoModule hLibrary(LoadLibraryExW(library, nullptr, LOAD_LIBRARY_AS_DATAFILE));
    if (hLibrary.handle == nullptr)
        return nullptr;

    // Enumerate the icons and pick the first one
    LPCSTR iconName = nullptr;
    EnumResourceNamesA(hLibrary.handle, RT_GROUP_ICON, GroupIconEnumerationProc, (ULONG_PTR)&iconName);
    if (iconName == nullptr)
        return nullptr;

    HRSRC hRsrc = FindResourceA(hLibrary.handle, iconName, RT_GROUP_ICON);
    if (hRsrc == nullptr)
        return nullptr;

    HGLOBAL hGlobal = LoadResource(hLibrary.handle, hRsrc);
    if (hGlobal == nullptr)
        return nullptr;

    LPVOID pRes = LockResource(hGlobal);
    if (pRes == nullptr)
        return nullptr;

    int id = LookupIconIdFromDirectoryEx((BYTE*)pRes, true, 256, 256, 0);

    hRsrc = FindResourceA(hLibrary.handle, MAKEINTRESOURCEA(id), RT_ICON);
    if (hRsrc == nullptr)
        return nullptr;

    hGlobal = LoadResource(hLibrary.handle, hRsrc);
    if (hGlobal == nullptr)
        return nullptr;

    pRes = LockResource(hGlobal);
    if (pRes == nullptr)
        return nullptr;

    static const DWORD dwIconVersion = 0x00030000;
    return CreateIconFromResourceEx((BYTE*)pRes, SizeofResource(hLibrary.handle, hRsrc), true, dwIconVersion, desiredWidth, desiredHeight, 0);
}

float GetDpiScalingForWindow(HWND hWnd)
{
    static const UINT kDefaultDPI = 96;

    float dpi = 1.0f;

    AutoModule hUser32(LoadLibrary("User32.dll"));
    if (hUser32.handle != nullptr)
    {
        UINT(WINAPI * fnGetDpiForWindow)(HWND) = (UINT(WINAPI*)(HWND))(GetProcAddress(hUser32.handle, "GetDpiForWindow"));
        if (fnGetDpiForWindow != nullptr)
        {
            dpi = fnGetDpiForWindow(hWnd) / 96.0f;
        }
    }
    return dpi;
}

class CrashUIImpl : public CrashUI
{
public:
    CrashUIImpl()
        : CrashUI()
        , m_hDlg(nullptr)
        , m_hThread(nullptr)
        , m_hDialogCreatedEvent(nullptr)
    {
    }

    enum CustomWindowMessage
    {
        WMU_SET_PROGRESS = WM_USER + 0x100, // fixes case 957816
        WMU_CONFIGURE,
        WMU_COMPLETE
    };

    static CrashUIImpl& Get()
    {
        static CrashUIImpl impl;
        return impl;
    }

    void NotifyProgress(int progress)
    {
        if (!IsInitialized())
            return;

        if (progress < 0) progress = 0;
        if (progress > 100) progress = 100;

        Mutex::AutoLock lock(m_Lock);
        SendMessageW(m_hDlg, WMU_SET_PROGRESS, (WPARAM)progress, 0);
    }

    bool Show(
        const wchar_t* applicationPath,
        const wchar_t* applicationName,
        const wchar_t* versionInfo)
    {
        if (IsInitialized())
            return false;

        m_ExecutablePath = applicationPath;
        m_ApplicationName = applicationName;
        if (!m_ApplicationName.size())
            m_ApplicationName = GetFilePart(applicationPath);
        m_VersionInfo = versionInfo;

        m_hDialogCreatedEvent = CreateEvent(nullptr, false, false, nullptr);

        // Create the dialog
        m_hThread = CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)CrashDialogThread, nullptr, 0, nullptr);

        // Wait until the dialog notifies that it exists
        WaitForSingleObject(m_hDialogCreatedEvent, INFINITE);

        // Configure the dialog
        ConfigureDialog();

        // Set the dialog as the foreground window
        SetForegroundWindow(m_hDlg);

        return true;
    }

    void ConfigureDialog()
    {
        Mutex::AutoLock lock(m_Lock);

        float dpi = GetDpiScalingForWindow(m_hDlg);

        // Configure the error icon
        HICON hIcon = GetImageFromImageLibrary(m_ExecutablePath.c_str(), (int)(kAppIconSize * dpi), (int)(kAppIconSize * dpi));

        // Set the dialog title
        wchar_t wideTitle[512];
        swprintf_s(wideTitle, _countof(wideTitle),
            L"%s - %s",
            m_ApplicationName.c_str(),
            m_VersionInfo.c_str());

        // Configure the dialog
        SendMessageW(m_hDlg, WMU_CONFIGURE, (WPARAM)hIcon, (LPARAM)wideTitle);
    }

    bool Hide()
    {
        if (!IsInitialized())
            return false;

        {
            Mutex::AutoLock lock(m_Lock);
            SendMessageW(m_hDlg, WM_CLOSE, 0, 0);
        }

        WaitForSingleObject(m_hThread, INFINITE);
        return true;
    }

    bool Wait()
    {
        if (!IsInitialized())
            return false;

        {
            Mutex::AutoLock lock(m_Lock);
            SendMessageW(m_hDlg, WMU_COMPLETE, 0, 0);
        }

        WaitForSingleObject(m_hThread, INFINITE);
        return true;
    }

    void NotifyDialogCreated(HWND hDlg)
    {
        {
            Mutex::AutoLock lock(m_Lock);
            m_hDlg = hDlg;
        }
        SetEvent(m_hDialogCreatedEvent);
    }

    bool IsInitialized() const
    {
        return m_hDlg != nullptr;
    }

    bool IsShown() const
    {
        if (!IsInitialized())
            return false;

        DWORD exitCode = STILL_ACTIVE;
        GetExitCodeThread(m_hThread, &exitCode);
        if (exitCode == STILL_ACTIVE)
            return true;

        return false;
    }

    static int CALLBACK CrashDialogThread(void*)
    {
        return DialogBox(nullptr, MAKEINTRESOURCE(IDD_CRASHDIALOG), nullptr, CrashDialogProc);
    }

    static INT_PTR CALLBACK CrashDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

private:
    Mutex m_Lock;
    HANDLE m_hThread;
    HANDLE m_hDialogCreatedEvent;
    std::wstring m_ExecutablePath;
    std::wstring m_ApplicationName;
    std::wstring m_VersionInfo;
    HWND m_hDlg;
};

//-------------------------------------------------------------------------

void CrashUI::NotifyProgress(int progress)
{
    CrashUIImpl::Get().NotifyProgress(progress);
}

CrashUI& CrashUI::Get()
{
    return CrashUIImpl::Get();
}

//-------------------------------------------------------------------------

static void CalibrateProgressBar(HWND hDlg)
{
    SendDlgItemMessage(hDlg, IDC_PROGRESS, PBM_SETRANGE, 0, MAKELONG(0, 100));
    SendDlgItemMessage(hDlg, IDC_PROGRESS, PBM_SETPOS, 0, 0);
}

static bool GetIconImageSize(HANDLE hIcon, SIZE* out)
{
    ICONINFO i;
    if (!GetIconInfo((HICON)hIcon, &i))
        return false;

    BITMAP bm;
    GetObject(i.hbmColor, sizeof(BITMAP), &bm);

    out->cx = bm.bmWidth;
    out->cy = bm.bmHeight;
    return true;
}

static void ResetLayout(HWND hDlg, int dialogWidth, int progressBarHeight, int frameSize)
{
    HANDLE hAppIcon = (HANDLE)SendDlgItemMessage(hDlg, IDC_APP_ICON, STM_GETIMAGE, IMAGE_ICON, 0);
    HANDLE hErrorIcon = (HANDLE)SendDlgItemMessage(hDlg, IDC_ERROR_ICON, STM_GETIMAGE, IMAGE_ICON, 0);

    //
    // Get the image sizes
    //
    SIZE appIconSize = { kAppIconSize, kAppIconSize };
    SIZE errorIconSize = { kErrorIconSize, kErrorIconSize };
    if (hAppIcon != nullptr)
    {
        SIZE s;
        if (GetIconImageSize(hAppIcon, &s))
        {
            appIconSize = s;
        }
    }
    if (hErrorIcon != nullptr)
    {
        SIZE s;
        if (GetIconImageSize(hErrorIcon, &s))
        {
            errorIconSize = s;
        }
    }

    //
    // Construct the layout
    //
    RECT appIconRect = { 0, 0, appIconSize.cx, appIconSize.cy };
    RECT errorIconRect = { 0, 0, errorIconSize.cx, errorIconSize.cy };
    RECT progressBarRect = { 0, 0, std::max(dialogWidth, (int)appIconSize.cx), progressBarHeight };
    RECT clientRect;

    OffsetRect(&appIconRect, frameSize + (dialogWidth - appIconSize.cx) / 2, frameSize);
    OffsetRect(&errorIconRect, appIconRect.left, appIconRect.top);
    OffsetRect(&progressBarRect, frameSize, frameSize + appIconRect.bottom);
    UnionRect(&clientRect, &appIconRect, &progressBarRect);
    InflateRect(&clientRect, frameSize, frameSize);

    //
    // Set the window size
    //
    DWORD windowStyle = GetWindowLong(hDlg, GWL_STYLE);
    AdjustWindowRect(&clientRect, windowStyle, false);
    SetWindowPos(hDlg, nullptr, 0, 0, clientRect.right - clientRect.left, clientRect.bottom - clientRect.top, SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOZORDER);

    //
    // Set the layout
    //
    HWND hAppIconWnd = GetDlgItem(hDlg, IDC_APP_ICON);
    HWND hBangIconWnd = GetDlgItem(hDlg, IDC_ERROR_ICON);
    SetWindowPos(hAppIconWnd, nullptr, appIconRect.left, appIconRect.top, appIconSize.cx, appIconSize.cy, 0);
    SetWindowPos(hBangIconWnd, hAppIconWnd, errorIconRect.left, errorIconRect.top, errorIconSize.cx, errorIconSize.cy, 0);
    SetWindowPos(GetDlgItem(hDlg, IDC_PROGRESS), nullptr, progressBarRect.left, progressBarRect.top, progressBarRect.right - progressBarRect.left, progressBarRect.bottom - progressBarRect.top, 0);
}

INT_PTR CALLBACK CrashUIImpl::CrashDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
        case WM_INITDIALOG:
        {
            // Set up the progress bar.
            CalibrateProgressBar(hDlg);

            float dpiScale = GetDpiScalingForWindow(hDlg);
            int appIconSize = (int)(kAppIconSize * dpiScale);
            int errorIconSize = (int)(kErrorIconSize * dpiScale);

            // Set the placeholder icons up properly. The VS dialog designer is terrible and doesn't support resizing of icons.
            // As such, I have to programmatically request the right size and set it. *rolleyes*
            HANDLE hPlaceholderAppIcon = LoadImage(GetModuleHandle(nullptr), MAKEINTRESOURCE(IDI_UNITYICON), IMAGE_ICON, appIconSize, appIconSize, LR_DEFAULTCOLOR);
            if (hPlaceholderAppIcon != nullptr)
            {
                SendDlgItemMessage(hDlg, IDC_APP_ICON, STM_SETIMAGE, IMAGE_ICON, (LPARAM)hPlaceholderAppIcon);
            }
            HANDLE hErrorIcon = LoadImage(GetModuleHandle(nullptr), MAKEINTRESOURCE(IDI_UNITYERROR), IMAGE_ICON, errorIconSize, errorIconSize, LR_DEFAULTCOLOR);
            if (hErrorIcon != nullptr)
            {
                SendDlgItemMessage(hDlg, IDC_ERROR_ICON, STM_SETIMAGE, IMAGE_ICON, (LPARAM)hErrorIcon);
            }

            ResetLayout(hDlg, (int)(kDialogWidth * dpiScale), (int)(kProgressBarHeight * dpiScale), (int)(kFrameSize * dpiScale));

            // Notify the main thread that the dialog is created.
            CrashUIImpl::Get().NotifyDialogCreated(hDlg);
            return (INT_PTR)TRUE;
        }

        case WMU_SET_PROGRESS:
            // Set the progress.
            SendDlgItemMessage(hDlg, IDC_PROGRESS, PBM_SETPOS, wParam, 0);
            break;

        case WMU_COMPLETE:
            SendDlgItemMessage(hDlg, IDC_PROGRESS, PBM_SETPOS, 100, 0);
            break;

        case WMU_CONFIGURE:
            if (wParam != 0)
                SendDlgItemMessage(hDlg, IDC_APP_ICON, STM_SETIMAGE, IMAGE_ICON, (LPARAM)wParam);
            if (lParam != 0)
                SetWindowTextW(hDlg, (LPCWSTR)lParam);
            break;

        case WM_CLOSE:
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
    }
    return (INT_PTR)FALSE;
}
