#include "UnityPrefix.h"
#include <windows.h>
#include <windowsx.h>
#include <dbt.h>

#include "Runtime/Input/GetInput.h"
#include "Input.h"
#include "Runtime/Misc/InputEvent.h"
#include "RawInput.h"
#include "Runtime/BaseClasses/ManagerContext.h"
#include "Runtime/Input/InputManager.h"
#include "Runtime/Graphics/ScreenManager.h"
#include "Runtime/Misc/Player.h"
#include "Runtime/Misc/PlayerSettings.h"
#include "Runtime/Misc/SystemInfo.h"
#include "PlatformDependent/Win/Input/WinIME.h"
#include "PlatformDependent/Win/WinUtils.h"
#include "PlatformDependent/Win/Registry.h"
#if UNITY_EDITOR
#include "Editor/Src/RemoteInput/GenericRemote.h"
#endif
#include "NewInput.h"

using namespace win;


int kVirtKeyToKeyCode[256];


namespace
{
    Input* g_OldInput;
    NewInput* g_NewInput;

    HDEVNOTIFY g_DeviceNotification = NULL;
    UINT g_MouseTrails = 0;

    #pragma region InputInitVKTable

    void InputInitVKTable(void)
    {
        static bool kVirtKeyToKeyCodeInitialized = false;

        if (!kVirtKeyToKeyCodeInitialized)
        {
            for (int i = 0; i < _countof(kVirtKeyToKeyCode); ++i)
            {
                kVirtKeyToKeyCode[i] = SDLK_UNKNOWN;
            }

            kVirtKeyToKeyCode[VK_ESCAPE] = SDLK_ESCAPE;
            kVirtKeyToKeyCode['1'] = SDLK_1;
            kVirtKeyToKeyCode['2'] = SDLK_2;
            kVirtKeyToKeyCode['3'] = SDLK_3;
            kVirtKeyToKeyCode['4'] = SDLK_4;
            kVirtKeyToKeyCode['5'] = SDLK_5;
            kVirtKeyToKeyCode['6'] = SDLK_6;
            kVirtKeyToKeyCode['7'] = SDLK_7;
            kVirtKeyToKeyCode['8'] = SDLK_8;
            kVirtKeyToKeyCode['9'] = SDLK_9;
            kVirtKeyToKeyCode['0'] = SDLK_0;
            kVirtKeyToKeyCode[VK_OEM_MINUS] = SDLK_MINUS;
            kVirtKeyToKeyCode[VK_OEM_PLUS] = SDLK_EQUALS;
            kVirtKeyToKeyCode[VK_BACK] = SDLK_BACKSPACE;
            kVirtKeyToKeyCode[VK_TAB] = SDLK_TAB;
            kVirtKeyToKeyCode['Q'] = SDLK_q;
            kVirtKeyToKeyCode['W'] = SDLK_w;
            kVirtKeyToKeyCode['E'] = SDLK_e;
            kVirtKeyToKeyCode['R'] = SDLK_r;
            kVirtKeyToKeyCode['T'] = SDLK_t;
            kVirtKeyToKeyCode['Y'] = SDLK_y;
            kVirtKeyToKeyCode['U'] = SDLK_u;
            kVirtKeyToKeyCode['I'] = SDLK_i;
            kVirtKeyToKeyCode['O'] = SDLK_o;
            kVirtKeyToKeyCode['P'] = SDLK_p;
            kVirtKeyToKeyCode[VK_OEM_4] = SDLK_LEFTBRACKET; // TODO
            kVirtKeyToKeyCode[VK_OEM_6] = SDLK_RIGHTBRACKET; // TODO
            kVirtKeyToKeyCode[VK_RETURN] = SDLK_RETURN;
            kVirtKeyToKeyCode[VK_LCONTROL] = SDLK_LCTRL;
            kVirtKeyToKeyCode[VK_CONTROL] = SDLK_LCTRL;
            kVirtKeyToKeyCode['A'] = SDLK_a;
            kVirtKeyToKeyCode['S'] = SDLK_s;
            kVirtKeyToKeyCode['D'] = SDLK_d;
            kVirtKeyToKeyCode['F'] = SDLK_f;
            kVirtKeyToKeyCode['G'] = SDLK_g;
            kVirtKeyToKeyCode['H'] = SDLK_h;
            kVirtKeyToKeyCode['J'] = SDLK_j;
            kVirtKeyToKeyCode['K'] = SDLK_k;
            kVirtKeyToKeyCode['L'] = SDLK_l;
            kVirtKeyToKeyCode[VK_OEM_1] = SDLK_SEMICOLON; // TODO
            kVirtKeyToKeyCode[VK_OEM_7] = SDLK_QUOTE; // TODO
            kVirtKeyToKeyCode[VK_OEM_3] = SDLK_BACKQUOTE; // TODO
            kVirtKeyToKeyCode[VK_OEM_8] = SDLK_BACKQUOTE; // TODO
            kVirtKeyToKeyCode[VK_LSHIFT] = SDLK_LSHIFT;
            kVirtKeyToKeyCode[VK_OEM_5] = SDLK_BACKSLASH; // TODO
            kVirtKeyToKeyCode[VK_OEM_102] = SDLK_BACKSLASH; // TODO
            kVirtKeyToKeyCode['Z'] = SDLK_z;
            kVirtKeyToKeyCode['X'] = SDLK_x;
            kVirtKeyToKeyCode['C'] = SDLK_c;
            kVirtKeyToKeyCode['V'] = SDLK_v;
            kVirtKeyToKeyCode['B'] = SDLK_b;
            kVirtKeyToKeyCode['N'] = SDLK_n;
            kVirtKeyToKeyCode['M'] = SDLK_m;
            kVirtKeyToKeyCode[VK_OEM_COMMA] = SDLK_COMMA;
            kVirtKeyToKeyCode[VK_OEM_PERIOD] = SDLK_PERIOD;
            kVirtKeyToKeyCode[VK_OEM_2] = SDLK_SLASH;
            kVirtKeyToKeyCode[VK_RSHIFT] = SDLK_RSHIFT;
            kVirtKeyToKeyCode[VK_MULTIPLY] = SDLK_KP_MULTIPLY;
            kVirtKeyToKeyCode[VK_LMENU] = SDLK_LALT;
            kVirtKeyToKeyCode[VK_SPACE] = SDLK_SPACE;
            kVirtKeyToKeyCode[VK_CAPITAL] = SDLK_CAPSLOCK;
            kVirtKeyToKeyCode[VK_F1] = SDLK_F1;
            kVirtKeyToKeyCode[VK_F2] = SDLK_F2;
            kVirtKeyToKeyCode[VK_F3] = SDLK_F3;
            kVirtKeyToKeyCode[VK_F4] = SDLK_F4;
            kVirtKeyToKeyCode[VK_F5] = SDLK_F5;
            kVirtKeyToKeyCode[VK_F6] = SDLK_F6;
            kVirtKeyToKeyCode[VK_F7] = SDLK_F7;
            kVirtKeyToKeyCode[VK_F8] = SDLK_F8;
            kVirtKeyToKeyCode[VK_F9] = SDLK_F9;
            kVirtKeyToKeyCode[VK_F10] = SDLK_F10;
            kVirtKeyToKeyCode[VK_NUMLOCK] = SDLK_NUMLOCK;
            kVirtKeyToKeyCode[VK_SCROLL] = SDLK_SCROLLOCK;
            kVirtKeyToKeyCode[VK_NUMPAD7] = SDLK_KP7;
            kVirtKeyToKeyCode[VK_NUMPAD8] = SDLK_KP8;
            kVirtKeyToKeyCode[VK_NUMPAD9] = SDLK_KP9;
            kVirtKeyToKeyCode[VK_SUBTRACT] = SDLK_KP_MINUS;
            kVirtKeyToKeyCode[VK_NUMPAD4] = SDLK_KP4;
            kVirtKeyToKeyCode[VK_NUMPAD5] = SDLK_KP5;
            kVirtKeyToKeyCode[VK_NUMPAD6] = SDLK_KP6;
            kVirtKeyToKeyCode[VK_ADD] = SDLK_KP_PLUS;
            kVirtKeyToKeyCode[VK_NUMPAD1] = SDLK_KP1;
            kVirtKeyToKeyCode[VK_NUMPAD2] = SDLK_KP2;
            kVirtKeyToKeyCode[VK_NUMPAD3] = SDLK_KP3;
            kVirtKeyToKeyCode[VK_NUMPAD0] = SDLK_KP0;
            kVirtKeyToKeyCode[VK_DECIMAL] = SDLK_KP_PERIOD;
            kVirtKeyToKeyCode[VK_F11] = SDLK_F11;
            kVirtKeyToKeyCode[VK_F12] = SDLK_F12;

            kVirtKeyToKeyCode[VK_F13] = SDLK_F13;
            kVirtKeyToKeyCode[VK_F14] = SDLK_F14;
            kVirtKeyToKeyCode[VK_F15] = SDLK_F15;

            kVirtKeyToKeyCode[VK_RCONTROL] = SDLK_RCTRL;
            kVirtKeyToKeyCode[VK_DIVIDE] = SDLK_KP_DIVIDE;
            kVirtKeyToKeyCode[VK_SNAPSHOT] = SDLK_SYSREQ; // PrintSrc/SysReq
            kVirtKeyToKeyCode[VK_RMENU] = SDLK_RALT;
            kVirtKeyToKeyCode[VK_PAUSE] = SDLK_PAUSE;
            kVirtKeyToKeyCode[VK_HOME] = SDLK_HOME;
            kVirtKeyToKeyCode[VK_UP] = SDLK_UP;
            kVirtKeyToKeyCode[VK_PRIOR] = SDLK_PAGEUP;
            kVirtKeyToKeyCode[VK_LEFT] = SDLK_LEFT;
            kVirtKeyToKeyCode[VK_RIGHT] = SDLK_RIGHT;
            kVirtKeyToKeyCode[VK_END] = SDLK_END;
            kVirtKeyToKeyCode[VK_DOWN] = SDLK_DOWN;
            kVirtKeyToKeyCode[VK_NEXT] = SDLK_PAGEDOWN;
            kVirtKeyToKeyCode[VK_INSERT] = SDLK_INSERT;
            kVirtKeyToKeyCode[VK_DELETE] = SDLK_DELETE;
            kVirtKeyToKeyCode[VK_LWIN] = SDLK_LMETA;
            kVirtKeyToKeyCode[VK_RWIN] = SDLK_RMETA;
            kVirtKeyToKeyCode[VK_APPS] = SDLK_MENU;

            kVirtKeyToKeyCodeInitialized = true;
        }
    }

    #pragma endregion
}

namespace win
{
    #pragma region Mouse Capture

    static HWND g_MouseCaptureWindow = NULL;
    static int g_MouseCaptureCount = 0;

    void ResetMouseCapture(void)
    {
        g_MouseCaptureWindow = NULL;
        g_MouseCaptureCount = 0;
    }

    void SetMouseCapture(HWND window)
    {
        if (window != g_MouseCaptureWindow)
        {
            ReleaseCapture();
            ResetMouseCapture();
        }

        if (g_MouseCaptureCount > 0)
        {
            ++g_MouseCaptureCount;
        }
        else
        {
            g_MouseCaptureWindow = window;
            g_MouseCaptureCount = 1;

            SetCapture(window);
        }
    }

    void ReleaseMouseCapture(void)
    {
        if (0 == --g_MouseCaptureCount)
        {
            ReleaseCapture();
            ResetMouseCapture();
        }
    }

    #pragma endregion
}

void InputInitTouch(HWND window)
{
    // Register the Window to receive touch input
    typedef BOOL (WINAPI *RegisterTouchWindowProc)(
        _In_  HWND hWnd,
        _In_  ULONG ulFlags
    );

    static RegisterTouchWindowProc RegisterTouchWindow_ = NULL;
    if (RegisterTouchWindow_ == NULL)
    {
        RegisterTouchWindow_ = reinterpret_cast<RegisterTouchWindowProc>(
            GetProcAddress(GetModuleHandle("User32.dll"), "RegisterTouchWindow"));
    }

    if (RegisterTouchWindow_ != NULL)
    {
        if (RegisterTouchWindow_(window, 0))
        {
            printf_console("<RI> Initialized touch support.\r\n");
        }
        else
        {
            const core::string lastError = winutils::ErrorCodeToMsg(GetLastError());
            printf_console("<RI> Touch support initialization failed: %s.\r\n", lastError.c_str());
        }
    }
}

void InputInit(HWND window)
{
    // make sure input has not been initialized yet

    if (g_OldInput != NULL)
    {
        return;
    }

    // initialize virtual key table

    InputInitVKTable();

    // initialize input

    printf_console("<RI> Initializing input.\r\n");

    if (GetPlayerSettings().GetEnableNewInputSystem())
    {
        g_NewInput = new NewInput();
        if (!g_NewInput->Open(window))
        {
            printf_console("Could not initialize new input system\n");
            delete g_NewInput;
            g_NewInput = NULL;
        }
    }

    if (!GetPlayerSettings().GetDisableOldInputManagerSupport())
    {
        g_OldInput = new RawInput();

        if (!g_OldInput->Open(window))
        {
            delete g_OldInput;
            g_OldInput = NULL;

            printf_console("<RI> Input initialization failed.\r\n");
            return;
        }
    }

    InputInitWindow(window);
}

void InputShutdown(void)
{
    InputShutdownWindow();

    if (g_NewInput != NULL)
    {
        g_NewInput->Close();
        delete g_NewInput;
        g_NewInput = NULL;
    }

    if (g_OldInput != NULL)
    {
        g_OldInput->Close();
        delete g_OldInput;
        g_OldInput = NULL;
    }
}

void InputInitWindow(HWND window)
{
    // register for device notifications
    InputShutdownWindow();

    DEV_BROADCAST_DEVICEINTERFACE_W dbch =
    {
        sizeof(dbch),
        DBT_DEVTYP_DEVICEINTERFACE,
        0,
        0,
        L'\0'
    };

    // Register for all device interface classes, even though we only want HID devices; on Win7 and later,
    // requesting only HID devices appears to miss things, possibly because they're children of USB devices.
    g_DeviceNotification = RegisterDeviceNotificationW(window, &dbch, DEVICE_NOTIFY_WINDOW_HANDLE | DEVICE_NOTIFY_ALL_INTERFACE_CLASSES);
    Assert(NULL != g_DeviceNotification);
}

void InputShutdownWindow()
{
    if (NULL != g_DeviceNotification)
    {
        BOOL const result = UnregisterDeviceNotification(g_DeviceNotification);
        Assert(FALSE != result);

        g_DeviceNotification = NULL;
    }
}

bool IsInputInitialized(void)
{
    return (g_OldInput != NULL || g_NewInput != NULL);
}

void InputProcess(void)
{
#if UNITY_EDITOR
    bool RemoteIsConnected();
    bool RemoteShouldOverrideLocalJoystickInput();
    if (RemoteIsConnected() && RemoteShouldOverrideLocalJoystickInput())
        return;
#endif

    if (g_OldInput != NULL)
    {
        bool discard = (kPlayerPaused == GetPlayerPause());
        g_OldInput->Process(discard);
    }
}

void InputPostprocess()
{
    if (g_OldInput != NULL)
    {
        g_OldInput->PostprocessTouches();
    }
}

void GetJoystickNames(dynamic_array<core::string> &names)
{
#if UNITY_EDITOR
    bool RemoteIsConnected();
    bool RemoteShouldOverrideLocalJoystickInput();
    if (RemoteIsConnected() && RemoteShouldOverrideLocalJoystickInput())
    {
        void RemoteGetJoystickNames(dynamic_array<core::string>&names);
        RemoteGetJoystickNames(names);
        return;
    }
#endif

    if (g_OldInput != NULL)
    {
        g_OldInput->GetJoystickNames(names);
    }
}

void ResetInput(void)
{
    if (g_OldInput != NULL)
    {
        g_OldInput->Process(true);
    }

    InputManager *input = static_cast<InputManager *>(GetManagerPtrFromContext(ManagerContext::kInputManager));

    if (NULL != input)
    {
        input->ResetInputAxes();
    }
}

void ResetInputAfterPause(void)
{
    ResetInput();
}

void InputActivate(void)
{
    if (g_OldInput != NULL)
    {
        g_OldInput->Activate(true);
    }
    if (g_NewInput != NULL)
    {
        g_NewInput->Activate(true);
    }
}

void InputPassivate(void)
{
    if (g_OldInput != NULL)
    {
        g_OldInput->Activate(false);
    }
    if (g_NewInput != NULL)
    {
        g_NewInput->Activate(false);
    }
}

void InputSetWindow(HWND window, bool fullscreen)
{
    if (g_NewInput != NULL)
    {
        g_NewInput->ToggleFullscreen(fullscreen, window);
    }
    if (g_OldInput != NULL)
    {
        g_OldInput->ToggleFullscreen(fullscreen, window);
    }

#if !UNITY_EDITOR
    InputInitWindow(window);
#endif

    // disable cursor trail in fullscreen because it makes cursor disappear altogether (case 392872)

    if (!fullscreen)
    {
        if (0 != g_MouseTrails)
        {
            const BOOL setMouseTrailsResult = SystemParametersInfoW(SPI_SETMOUSETRAILS, g_MouseTrails, NULL, 0);
            Assert(FALSE != setMouseTrailsResult);

            g_MouseTrails = 0;
        }
    }
    else
    {
        g_MouseTrails = 0;

        UINT mouseTrails;

        const BOOL getMouseTrailsResult = SystemParametersInfoW(SPI_GETMOUSETRAILS, 0, &mouseTrails, 0);
        Assert(FALSE != getMouseTrailsResult);

        if (FALSE != getMouseTrailsResult)
        {
            if (mouseTrails > 1)
            {
                g_MouseTrails = mouseTrails;

                const BOOL setMouseTrailsResult = SystemParametersInfoW(SPI_SETMOUSETRAILS, 0, NULL, 0);
                Assert(FALSE != setMouseTrailsResult);
            }
        }
    }
}

#if UNITY_EDITOR
// New input system process input in edit mode, too.
LRESULT ProcessInputMessageInEditor(HWND window, UINT message, WPARAM wParam, LPARAM lParam)
{
    if (g_NewInput)
    {
        BOOL handled = false;
        return g_NewInput->OnMessage(window, message, wParam, lParam, handled);
    }

    return 0;
}

#endif

LRESULT ProcessInputMessage(HWND window, UINT message, WPARAM wParam, LPARAM lParam, BOOL &handled)
{
    // For the new input system in the editor, we've already given the message to IInput above.
    if (g_NewInput && !UNITY_EDITOR)
    {
        g_NewInput->OnMessage(window, message, wParam, lParam, handled);
    }

    if (g_OldInput)
    {
        return g_OldInput->OnMessage(window, message, wParam, lParam, handled);
    }

    return 0;
}

LRESULT ProcessIMEMessage(HWND window, UINT message, WPARAM wParam, LPARAM lParam, BOOL &handled)
{
    LRESULT result;
    handled = ProcessIMEMessages(window, message, wParam, lParam, result, g_NewInput);
    return result;
}

static void TranslateAndDispatchEventMsg(const MSG& msg)
{
    TranslateMessage(&msg);
    DispatchMessage(&msg);
}

bool TranslateAndDispatchFilteredMessage(const MSG& msg)
{
    // According to Microsoft, the official way you are supposed to detect
    // and ignore fake touch messages is like this:
    // http://msdn.microsoft.com/en-us/library/windows/desktop/dd693088(v=vs.85).aspx
    #define MOUSEEVENTF_FROMTOUCH 0xFF515700
    bool dispatch = !InputEvent::IsMouseEventComingFromTouch();

#if PLATFORM_STANDALONE
    if (!dispatch)
    {
        // We always dispatch touch messages for windows other than the main Unity standalone player (see bug 704815)
        ScreenManagerPlatform* manager = GetScreenManagerPtr();
        HWND mainWindow = NULL;
        if (manager)
            mainWindow = manager->GetWindow();
        if (msg.hwnd != mainWindow)
            dispatch = true;
    }
#endif

    if (dispatch)
    {
        TranslateAndDispatchEventMsg(msg);
    }
    else // Simulated mouse message from touch input
    {
        // We need to take special care of the touch messages.
        // We want to filter out the simulated messages only and
        // only if they fall within the client area of the window
        // otherwise the user won't be able to interact with window
        // borders or window buttons, like close button or minimize button.
        DWORD dwStyle = (DWORD)GetWindowLong(msg.hwnd, GWL_STYLE);
        if (dwStyle & WS_CAPTION) // Window has borders and the caption
        {
            RECT client;
            if (GetClientRect(msg.hwnd, &client))
            {
                // GetClientRect returns the area of the client region and not the actual position of the rect
                // We need to convert the relative client rect into screen coordinates
                ClientToScreen(msg.hwnd, reinterpret_cast<POINT*>(&client.left));   // convert top-left
                ClientToScreen(msg.hwnd, reinterpret_cast<POINT*>(&client.right));  // convert bottom-right

                POINT pt;
                pt.x = GET_X_LPARAM(msg.lParam);
                pt.y = GET_Y_LPARAM(msg.lParam);

                if (!PtInRect(&client, pt))
                {
                    dispatch = true;
                    TranslateAndDispatchEventMsg(msg);
                }
            }
            else // Failed to get the client area; simply dispatch the message
            {
                dispatch = true;
                TranslateAndDispatchEventMsg(msg);
            }
        } // has borders
    }
    return dispatch;
}

BOOL WinGetUserTouchInputInfo(
    __in HTOUCHINPUT hTouchInput,               // input event handle; from touch message lParam
    __in UINT cInputs,                          // number of elements in the array
    __out_ecount(cInputs) PTOUCHINPUT pInputs,  // array of touch inputs
    __in int cbSize)                            // sizeof(TOUCHINPUT)
{
    typedef BOOL (WINAPI *GetTouchInputInfoProc)(
        __in HTOUCHINPUT hTouchInput,               // input event handle; from touch message lParam
        __in UINT cInputs,                          // number of elements in the array
        __out_ecount(cInputs) PTOUCHINPUT pInputs,  // array of touch inputs
        __in int cbSize);                           // sizeof(TOUCHINPUT)

    static GetTouchInputInfoProc GetTouchInputInfo_ = NULL;

    if (GetTouchInputInfo_ == NULL)
    {
        GetTouchInputInfo_ = reinterpret_cast<GetTouchInputInfoProc>(
            GetProcAddress(GetModuleHandle("User32.dll"), "GetTouchInputInfo"));
    }

    BOOL result = FALSE;
    if (GetTouchInputInfo_ != NULL)
    {
        result = GetTouchInputInfo_(hTouchInput, cInputs, pInputs, cbSize);
    }
    return result;
}

BOOL WinCloseUserTouchInputHandle(__in HTOUCHINPUT hTouchInput)
{
    typedef BOOL (WINAPI *CloseTouchInputHandleProc)(__in HTOUCHINPUT hTouchInput);

    static CloseTouchInputHandleProc CloseTouchInputHandle_ = NULL;

    if (CloseTouchInputHandle_ == NULL)
    {
        CloseTouchInputHandle_ = reinterpret_cast<CloseTouchInputHandleProc>(
            GetProcAddress(GetModuleHandle("User32.dll"), "CloseTouchInputHandle"));
    }

    BOOL result = FALSE;
    if (CloseTouchInputHandle_ != NULL)
    {
        result = CloseTouchInputHandle_(hTouchInput);
    }
    return result;
}

size_t GetTouchCount()
{
#if UNITY_EDITOR
    if (RemoteIsConnected() || RemoteTouchesAvailable())
        return RemoteGetTouchCount();
#endif
    return g_OldInput == NULL ? 0u : g_OldInput->TouchCount();
}

bool GetTouch(unsigned index, Touch& touch)
{
#if UNITY_EDITOR
    if (RemoteIsConnected() || RemoteTouchesAvailable())
        return RemoteGetTouch(index, touch);
#endif
    return g_OldInput != NULL && g_OldInput->GetTouch(index, touch);
}

bool IsTouchPressureSupported()
{
#if UNITY_EDITOR
    if (RemoteIsConnected() || RemoteTouchesAvailable())
        return RemoteIsTouchPressureSupported();
#endif
    return false;
}

bool IsStylusTouchSupported()
{
#if UNITY_EDITOR
    if (RemoteIsConnected() || RemoteTouchesAvailable())
        return RemoteIsStylusTouchSupported();
#endif
    return false;
}

bool GetMousePresent(void)
{
#if UNITY_EDITOR
    if (RemoteIsConnected() || RemoteTouchesAvailable())
        return RemoteGetMousePresent();
#endif
    return g_OldInput != NULL && g_OldInput->GetMousePresent();
}

bool IsMultiTouchEnabled()
{
#if UNITY_EDITOR
    if (RemoteIsConnected() || RemoteTouchesAvailable())
        return true;
#endif
    return g_OldInput != NULL && g_OldInput->IsMultiTouchEnabled();
}

void SetMultiTouchEnabled(bool flag)
{
#if UNITY_EDITOR
    if (RemoteIsConnected() || RemoteTouchesAvailable())
        return;
#endif
    if (g_OldInput)
        g_OldInput->SetMultiTouchEnabled(flag);
}

size_t GetActiveTouchCount()
{
#if UNITY_EDITOR
    if (RemoteIsConnected() || RemoteTouchesAvailable())
        return RemoteGetActiveTouchCount();
#endif
    return g_OldInput == NULL ? 0u : g_OldInput->ActiveTouchCount();
}

int InputKeycodeFromVKEY(int vkey)
{
    return kVirtKeyToKeyCode[vkey];
}

bool IsTouchSupported()
{
    static int isTouchSupported = -1;

    if (isTouchSupported == -1)
    {
        int value = GetSystemMetrics(SM_DIGITIZER);
        isTouchSupported = value & NID_MULTI_INPUT ? 1 : 0;
    }

    return isTouchSupported == 1;
}

#if !UNITY_EDITOR

void InputSwitchKeyConfig(HWND window)
{
    if (g_OldInput != NULL)
    {
        g_OldInput->ToggleFullscreen(false, window);
        g_OldInput->Process(true);
    }

    GetInputManager().InputEndFrame();
}

void ConsumeAnyBufferedInput(void)
{
    if (g_OldInput != NULL)
    {
        g_OldInput->Process(true);
    }

    GetInputManager().InputEndFrame();
}

#endif
