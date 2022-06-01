#include "UnityPrefix.h"
#include "Input.h"
#include "RawInputHid.h"
#include "Runtime/Input/GetInput.h"
#include "Runtime/Input/SimulateInputEvents.h"
#include "Runtime/Graphics/ScreenManager.h"
#include "PlatformDependent/Win/WinUtils.h"
#include "Runtime/Interfaces/IVRDevice.h"

#if UNITY_EDITOR
#include "Editor/Platform/Interface/GUIView.h"
#include "Editor/Platform/Interface/RepaintController.h"
#include "Editor/Platform/Interface/HighDpi.h"
#endif
#include <windows.h>
#include <windowsx.h>
#include <dbt.h>

extern int kVirtKeyToKeyCode[256];

namespace win
{
    Input::Input(void) :
        window(NULL),
        active(false),
        m_Touch(GetScreenManager().GetDPI(), false)
    {
        for (int i = 0; i < _countof(this->joystickGlobalKeyMap); ++i)
        {
            char name[40];

            if (-1 != sprintf_s(name, "joystick button %d", i))
            {
                this->joystickGlobalKeyMap[i] = StringToKey(name);
            }
            else
            {
                this->joystickGlobalKeyMap[i] = -1;
            }
        }

        m_Touch.InitTouches();
    }

    bool Input::Open(HWND window)
    {
        // validate input arguments

        Assert(IsWindow(window));

        // make sure input is closed

        if (this->window)
            this->Close();

        // save window handle for later use

        this->window = window;

        // done

        return true;
    }

    void Input::Close(void)
    {
        this->window = NULL;
    }

    bool Input::GetJoystickNames(dynamic_array<core::string> &names)
    {
        names.clear_dealloc();
        return true;
    }

    bool Input::Activate(bool active)
    {
        this->active = active;

        if (this->active)
        {
            // reset mouse delta on activation because it tends to accumulate very large values (case 403674)
            this->mouse.ResetDelta();
        }
        else
        {
            // don't crash if input manager is not available (case 552881)
            InputManager* inputManager = GetInputManagerPtr();

            // reset mouse

            this->mouse.Reset(true);

            if (inputManager)
            {
                inputManager->SetMouseDelta(Vector4f(0.0f, 0.0f, 0.0f, 0.0f));

                for (int i = 0; i < kMouseButtonCount; ++i)
                {
                    inputManager->SetMouseButton(i, false);
                }
            }

            // reset keyboard

            this->keyboard.Reset(true);

            if (inputManager)
            {
                for (int i = 0; i < kKeyCount; ++i)
                {
                    inputManager->SetKeyState(i, false);
                }
            }

            // reset joysticks

            for (JoystickStates::iterator it = this->joysticks.begin(); it != this->joysticks.end(); ++it)
            {
                it->second->Reset(true);

                if (inputManager)
                {
                    // reset axes

                    const int id = (it->second->GetId() + 1);

                    for (int i = 0; i < kMaxJoyStickAxis; ++i)
                    {
                        inputManager->SetJoystickPosition(id, i, 0.0f);
                    }

                    // reset buttons

                    for (int i = 0; i < kMaxJoyStickButtons; ++i)
                    {
                        int globalKey = this->joystickGlobalKeyMap[i];

                        if (-1 != globalKey)
                        {
                            inputManager->SetKeyState(globalKey, false);
                        }

                        int localKey = it->second->MapKey(i);

                        if (-1 != localKey)
                        {
                            inputManager->SetKeyState(localKey, false);
                        }
                    }
                }
            }
        }

        return true;
    }

    bool Input::ToggleFullscreen(bool fullscreen, HWND window)
    {
        this->window = window;
        return true;
    }

    bool Input::Process(bool discard)
    {
        // always update mouse position

        if (!discard)
        {
            this->UpdateMousePosition();
        }

        // ignore input if not active

        if (!this->active &&
            !(GetIVRDevice() && GetIVRDevice()->IsDeviceWindowFocused()))
        {
            return true;
        }

        // update state

        if (!this->UpdateState())
        {
            return false;
        }

        // update input manager only if input data is not to be discarded

        if (!discard)
        {
            // get input manager

            InputManager &inputManager = GetInputManager();

            // set mouse delta

            static const float kMouseDeltaScale = 0.5f;

            Vector4f delta = this->mouse.GetDelta();

            delta.x *= kMouseDeltaScale;
            delta.y *= -kMouseDeltaScale;

            inputManager.SetMouseDelta(delta);

            // set mouse buttons

            const Buttons &mouseButtons = this->mouse.GetButtons();

            for (Buttons::const_iterator it = mouseButtons.begin(); it != mouseButtons.end(); ++it)
            {
                inputManager.SetMouseButton(it->GetKey(), it->GetState());
            }

            // set keyboard buttons

            const Buttons &keyboardButtons = this->keyboard.GetButtons();

            bool ignoreKeyPress = false;
            if (inputManager.GetEatKeyPressOnTextFieldFocus() && inputManager.GetTextFieldInput())
                ignoreKeyPress = true;

            for (Buttons::const_iterator it = keyboardButtons.begin(); it != keyboardButtons.end(); ++it)
            {
                if (!ignoreKeyPress)
                    inputManager.SetKeyState(it->GetKey(), it->GetState());
            }

            // process joysticks

            for (JoystickStates::const_iterator jit = this->joysticks.begin(); jit != this->joysticks.end(); ++jit)
            {
                // set values

                const int id = (jit->second->GetId() + 1);
                const float(&axes)[kMaxJoyStickAxis] = jit->second->GetAxes();

                for (int i = 0; i < _countof(axes); ++i)
                {
                    inputManager.SetJoystickPosition(id, i, axes[i]);
                }

                // set buttons

                const Buttons &joystickButtons = jit->second->GetButtons();

                for (Buttons::const_iterator bit = joystickButtons.begin(); bit != joystickButtons.end(); ++bit)
                {
                    int key = bit->GetKey();
                    bool state = bit->GetState();

                    // set global button

                    if ((key >= 0) && (key < _countof(this->joystickGlobalKeyMap)))
                    {
                        int globalKey = this->joystickGlobalKeyMap[key];

                        if (-1 != globalKey)
                        {
                            inputManager.SetKeyState(globalKey, state);
                        }
                    }

                    // set local button

                    int localKey = jit->second->MapKey(key);

                    if (-1 != localKey)
                    {
                        inputManager.SetKeyState(localKey, state);
                    }
                }
            }
        }

        // reset state

        this->mouse.Reset(false);
        this->keyboard.Reset(false);

        for (JoystickStates::iterator it = this->joysticks.begin(); it != this->joysticks.end(); ++it)
        {
            it->second->Reset(false);
        }

        // done

        return true;
    }

    bool Input::UpdateState(void)
    {
        PreprocessTouches();
        return true;
    }

    LRESULT Input::OnMessage(HWND window, UINT message, WPARAM wParam, LPARAM lParam, BOOL& handled)
    {
        switch (message)
        {
            case WM_LBUTTONDOWN:
            case WM_LBUTTONDBLCLK:
            {
                SetMouseCapture(window);
                GetInputManager().SetMouseButton(0, true);
            }
            break;

            case WM_LBUTTONUP:
            {
                GetInputManager().SetMouseButton(0, false);
                ReleaseMouseCapture();
            }
            break;

            case WM_RBUTTONDOWN:
            case WM_RBUTTONDBLCLK:
            {
                SetMouseCapture(window);
                GetInputManager().SetMouseButton(1, true);
            }
            break;
            case WM_MOUSEMOVE:
            {
                // Get mouse pos directly from the event itself
                POINT p;
                p.x = GET_X_LPARAM(lParam);
                p.y = GET_Y_LPARAM(lParam);

                Vector2f position;
                if (ClientToScreen(window, &p) && Input::ConvertPositionToClientAreaCoord(window, p, Input::kMouse, Input::PositionConvertData(), position))
                {
                    GetInputManager().SetMousePosition(position);

                    if (!GetScreenManager().GetCursorInsideWindow())
                    {
                        TRACKMOUSEEVENT tme = { sizeof(TRACKMOUSEEVENT) };
                        tme.dwFlags = TME_LEAVE;
                        tme.hwndTrack = window;
                        TrackMouseEvent(&tme);

                        GetScreenManager().SetCursorInsideWindow(true);
                    }
                }
            }
            break;
            case WM_MOUSELEAVE:
            {
                #if !UNITY_EDITOR
                GetScreenManager().SetCursorInsideWindow(false);
                #endif
            }
            break;
            case WM_RBUTTONUP:
            {
                GetInputManager().SetMouseButton(1, false);
                ReleaseMouseCapture();
            }
            break;

            case WM_MBUTTONDOWN:
            case WM_MBUTTONDBLCLK:
            {
                SetMouseCapture(window);
                GetInputManager().SetMouseButton(2, true);
            }
            break;

            case WM_MBUTTONUP:
            {
                GetInputManager().SetMouseButton(2, false);
                ReleaseMouseCapture();
            }
            break;

            case WM_XBUTTONDOWN:
            case WM_XBUTTONDBLCLK:
            {
                switch (HIWORD(wParam))
                {
                    case XBUTTON1:
                    {
                        SetMouseCapture(window);
                        GetInputManager().SetMouseButton(3, true);
                    }
                    break;

                    case XBUTTON2:
                    {
                        SetMouseCapture(window);
                        GetInputManager().SetMouseButton(4, true);
                    }
                    break;
                }
            }
            break;

            case WM_XBUTTONUP:
            {
                switch (HIWORD(wParam))
                {
                    case XBUTTON1:
                    {
                        GetInputManager().SetMouseButton(3, false);
                        ReleaseMouseCapture();
                    }
                    break;

                    case XBUTTON2:
                    {
                        GetInputManager().SetMouseButton(4, false);
                        ReleaseMouseCapture();
                    }
                    break;
                }
            }
            break;

            case WM_MOUSEWHEEL:
            {
                handled = TRUE;
                return 0;
            }

            case WM_CAPTURECHANGED:
            {
                ResetMouseCapture();
            }
            break;

            case WM_CHAR:
            {
                WCHAR wide = wParam;

                if ((wide >= L' ') || (L'\b' == wide) || (L'\n' == wide) || (L'\r' == wide))
                {
                    CHAR utf8[5] = {};

                    if (0 != WideCharToMultiByte(CP_UTF8, 0, &wide, 1, utf8, sizeof(utf8), NULL, NULL))
                    {
                        GetInputManager().GetInputString() += utf8;
                    }
                }
            }
            break;

            case WM_KEYDOWN:
            case WM_SYSKEYDOWN:
            case WM_KEYUP:
            case WM_SYSKEYUP:
            {
                OnKey(window, message, wParam, lParam);
                return 0;
            }

            case WM_TOUCH:
            {
                OnTouch(window, message, wParam, lParam);
                break;
            }

            case WM_INPUT:
            {
                OnInput(window, message, wParam, lParam);
                handled = FALSE;
            }
            break;

            case WM_DEVICECHANGE:
            {
                if ((DBT_DEVICEARRIVAL == wParam) || (DBT_DEVICEREMOVECOMPLETE == wParam))
                {
                    DEV_BROADCAST_DEVICEINTERFACE_W const* dbch = reinterpret_cast<DEV_BROADCAST_DEVICEINTERFACE_W const*>(lParam);

                    if (DBT_DEVTYP_DEVICEINTERFACE == dbch->dbcc_devicetype)
                    {
                        // Check that this is actually a HID interface
                        GUID hidGUID;
                        HidD_GetHidGuid(&hidGUID);

                        if (dbch->dbcc_classguid == hidGUID)
                        {
                            OnDeviceChange(dbch->dbcc_name, (DBT_DEVICEARRIVAL == wParam));
                            return TRUE;
                        }
                    }
                }
            }
            break;
        }

        return 0;
    }

    bool Input::UpdateMousePosition(void)
    {
        // pretend everything is ok if inputmanager is not ready yet

        if (!GetInputManagerPtr())
        {
            return true;
        }

        // get mouse position in screen coordinates

        POINT position;

        if (!GetCursorPos(&position))
        {
            // vista and later versions of windows returns access denied message when computer is locked.
            // ignore this message to not flood console log

            DWORD error = GetLastError();

            if (ERROR_ACCESS_DENIED != error)
            {
                core::string message = winutils::ErrorCodeToMsg(error);
                ErrorString(Format("<I> Failed to get cursor position:\r\n%s", message.c_str()));
            }

            return false;
        }

        // convert mouse position to client-area coordinates
        Vector2f newPos;
        if (ConvertPositionToClientAreaCoord(this->window, position, Input::kMouse, Input::PositionConvertData(), newPos))
        {
            GetInputManager().SetMousePosition(newPos);
        }
        else
        {
            // It is not necessary to set an error here, because the function may not
            // succeed for this simple reason that the given window is simply not visible.
            return false;
        }

        // done
        return true;
    }

    bool Input::ConvertPositionToClientAreaCoord(HWND activeWindow, POINT position, PointerDeviceType deviceType, const Input::PositionConvertData& data, Vector2f& newPos)
    {
        if (!IsWindowVisible(activeWindow))
        {
            return false;
        }

        if (!ScreenToClient(activeWindow, &position))
        {
            const char* kDeviceNames[kDeviceTypeCount] =
            {
                "Mouse",
                "Touch"
            };
            ErrorString(Format("<I.%s> Failed to convert screen coordinates to client:\r\n%s", kDeviceNames[deviceType], WIN_LAST_ERROR_TEXT));
            return false;
        }

#if UNITY_EDITOR
        position = DPIScaleHelper(activeWindow).FromNative(position);
#endif
        newPos = data.Convert(deviceType, Vector2f(position.x, position.y));

        return true;
    }

    static bool VKPressedDown(UINT vk)
    {
        return (GetAsyncKeyState(vk) & 0x8000u) != 0;
    }

    LRESULT Input::OnKey(HWND window, UINT message, WPARAM wParam, LPARAM lParam)
    {
        // Bit:
        // 31   The transition state. The value is always 0 for a WM_(SYS)KEYDOWN message.
        // 31   The transition state. The value is always 1 for a WM_(SYS)KEYUP message.
        bool down      = (static_cast<UINT>(lParam) & 0x80000000u) == 0;

        // Bit:
        // 24   Indicates whether the key is an extended key, such as the right-hand ALT and CTRL keys that appear on
        // an enhanced 101- or 102-key keyboard. The value is 1 if it is an extended key; otherwise, it is 0.
        bool extended  = (static_cast<UINT>(lParam) & 0x01000000u) != 0;

        // Bit:
        // 29   The context code. The value is 1 if the ALT key is down while the key is pressed;
        // it is 0 if the WM_SYSKEYDOWN message is posted to the active window because no window has the keyboard focus.
        bool context   = (static_cast<UINT>(lParam) & 0x20000000u) != 0;

        int key;
        switch (static_cast<UINT>(wParam))
        {
            case VK_SHIFT:
                this->keyboard.AddButton(SDLK_LSHIFT, VKPressedDown(VK_LSHIFT));
                this->keyboard.AddButton(SDLK_RSHIFT, VKPressedDown(VK_RSHIFT));
                return 0;

            case VK_CONTROL:
                if (extended)
                {
                    key = SDLK_RCTRL;
                }
                else
                {
                    key = SDLK_LCTRL;

                    // LCtlr pressed with Alt -> AltGr
                    if (context)
                    {
                        this->keyboard.AddButton(SDLK_MODE, down);
                    }
                }
                break;

            case VK_MENU:
                if (extended)
                {
                    key = SDLK_RALT;

                    // Scan codes for right alt and alt gray are identical, thus we set both keys at once
                    this->keyboard.AddButton(SDLK_MODE, down);
                }
                else
                {
                    key = SDLK_LALT;
                }
                break;

            case VK_RETURN:
                key = (extended ? SDLK_KP_ENTER : SDLK_RETURN);
                break;

            case VK_SNAPSHOT:
                // Windows doesn't send WM_KEYDOWN message for PrintScreen button
                // Yyou can use RegisterHotKey to register for WM_HOTKEY for PrintScreen button
                // But there doesn't seem any advantage in that.
                // So simply emulate key press, so user can do Input.GetKeyDown(KeyCode.SysReq))
                this->keyboard.AddButton(kVirtKeyToKeyCode[VK_SNAPSHOT], true);
                this->keyboard.AddButton(kVirtKeyToKeyCode[VK_SNAPSHOT], false);
                return 0;

            default:
                key = ((wParam < _countof(kVirtKeyToKeyCode)) ? kVirtKeyToKeyCode[wParam] : SDLK_UNKNOWN);
                break;
        }

        if (SDLK_UNKNOWN != key)
        {
            this->keyboard.AddButton(key, down);
        }

        return 0;
    }

    LRESULT Input::OnInput(HWND window, UINT message, WPARAM wParam, LPARAM lParam)
    {
        return 0;
    }

    LRESULT Input::OnDeviceChange(LPCWSTR name, bool add)
    {
        return TRUE;
    }

    void Input::ProcessTouchImpl(HWND hWnd, const TOUCHINPUT& ti, const Input::PositionConvertData& data)
    {
        // From MS docs: TOUCHINPUT returns coords in "hundredths of a pixel of physical screen coordinates"
        POINT ptInput;
        ptInput.x = ti.x / 100;
        ptInput.y = ti.y / 100;

        Vector2f pos; // Zero vector
        if (ConvertPositionToClientAreaCoord(hWnd, ptInput, Input::kTouch, data, pos))
        {
            float radius = 0;
            float radiusVariance = 0;

            if (ti.dwMask & TOUCHINPUTMASKF_CONTACTAREA)
            {
                ProcessTouchContactRadius(hWnd, data, ti.cxContact, ti.cxContact, radius, radiusVariance);
            }

            if (ti.dwFlags & TOUCHEVENTF_DOWN)
            {
                AddTouchEvent(ti.dwID, pos.x, pos.y, TouchPhaseEmulation::kTouchBegan, ti.dwTime, radius, radiusVariance);
            }
            else if (ti.dwFlags & TOUCHEVENTF_UP)
            {
                AddTouchEvent(ti.dwID, pos.x, pos.y, TouchPhaseEmulation::kTouchEnded, ti.dwTime, radius, radiusVariance);
            }
            else if (ti.dwFlags & TOUCHEVENTF_MOVE)
            {
                AddTouchEvent(ti.dwID, pos.x, pos.y, TouchPhaseEmulation::kTouchMoved, ti.dwTime, radius, radiusVariance);
            }
        } // ConvertPositionToClientAreaCoord
    }

    void Input::ProcessTouchContactRadius(HWND hWnd, const Input::PositionConvertData& data, DWORD cxContact, DWORD cyContact, float& radius, float& radiusVariance)
    {
        // From MS docs: TOUCHINPUT returns coords in "hundredths of a pixel of physical screen coordinates"
        Vector2f contactSize;
        contactSize.x = static_cast<float>(cxContact) / 100.0f;
        contactSize.y = static_cast<float>(cyContact) / 100.0f;

#if UNITY_EDITOR
        contactSize = DPIScaleHelper(hWnd).FromNative(contactSize);
#endif

        auto& scale = GetScreenManager().GetCoordinateScale();
        contactSize.x *= scale.x;
        contactSize.y *= scale.y;

        // Treat the contact size (width/height) as axes to an ellipse with the longer of the two being the major axis.
        // This way we can compute radius and radiusVariance the same way as on Android (its API returns contact size as an ellipse)
        float majorAxis;
        float minorAxis;

        if (contactSize.x > contactSize.y)
        {
            majorAxis = contactSize.x;
            minorAxis = contactSize.y;
        }
        else
        {
            minorAxis = contactSize.x;
            majorAxis = contactSize.y;
        }

        radius = (majorAxis + minorAxis) / 4.0f; // calculating medium radius
        radiusVariance = fabs(majorAxis / 2.0f - radius);
    }

    LRESULT Input::OnTouch(HWND window, UINT message, WPARAM wParam, LPARAM lParam)
    {
        UINT cInputs = LOWORD(wParam);

        TOUCHINPUT nativeTouch[32];
        if (cInputs > 32)
            cInputs = 32;
        const Input::PositionConvertData data;
        if (WinGetUserTouchInputInfo((HTOUCHINPUT)lParam, cInputs, nativeTouch, sizeof(TOUCHINPUT)))
        {
            for (INT i = 0; i < static_cast<INT>(cInputs); ++i)
            {
                TOUCHINPUT ti = nativeTouch[i];

                if (IsMultiTouchEnabled())
                {
                    // If multitouch is enabled, we process all the touches
                    ProcessTouchImpl(window, ti, data);
                }
                else if (ti.dwFlags & TOUCHEVENTF_PRIMARY)
                {
                    // If multitouch is not enabled, we process only the primary touch
                    // and then break the loop, since the primary can only be one
                    ProcessTouchImpl(window, ti, data);
                    break;
                }
            } // for each native touch

            // If you handled the message and don't want anything else done with it, you can close it
            WinCloseUserTouchInputHandle((HTOUCHINPUT)lParam);
        } // GetUserTouchInputInfo

        return 0;
    }

    void Input::AddTouchEvent(int pointerId, float x, float y, TouchPhaseEmulation::TouchPhase newPhase, long long timestamp, float radius, float radiusVariance)
    {
        m_Touch.AddTouchEvent(pointerId, x, y, newPhase, timestamp, TouchPhaseEmulation::kMilli, radius, radiusVariance);
    }

    std::size_t Input::ActiveTouchCount() const
    {
        return m_Touch.GetActiveTouchCount();
    }

    std::size_t Input::TouchCount() const
    {
        return m_Touch.GetTouchCount();
    }

    bool Input::GetTouch(unsigned index, Touch& touch) const
    {
        return m_Touch.GetTouch(index, touch);
    }

    bool Input::IsTouchPressureSupported() const
    {
        return m_Touch.IsTouchPressureSupported();
    }

    bool Input::IsMultiTouchEnabled() const
    {
        return m_Touch.IsMultiTouchEnabled();
    }

    bool Input::IsStylusTouchSupported() const
    {
        return m_Touch.IsStylusTouchSupported();
    }

    bool Input::GetMousePresent(void) const
    {
        return m_Touch.GetMousePresent();
    }

    void Input::SetMultiTouchEnabled(bool flag)
    {
        m_Touch.SetMultiTouchEnabled(flag);
    }

    void Input::PreprocessTouches()
    {
        m_Touch.PreprocessTouches();
        SimulateMouseInput();
    }

    void Input::PostprocessTouches()
    {
        m_Touch.PostprocessTouches();
    }

    Input::PositionConvertData::PositionConvertData()
    {
#if UNITY_EDITOR
        Rectf  cameraRect;
        bool hasFocus;
        RepaintController::GetScreenParamsFromGameView(GetGameView(), true, true, &hasFocus, &guiRect, &cameraRect, &targetSize);
#else
        screenHeight = GetScreenManager().GetHeight();
        screenWidth = GetScreenManager().GetWidth();
#endif
    }

    Vector2f Input::PositionConvertData::Convert(Input::PointerDeviceType deviceType, const Vector2f& position) const
    {
        Vector2f newPos = position;
#if UNITY_EDITOR
        // Apply Game View scale and translation and send to InputManager
        newPos.x -= guiRect.x;
        newPos.y = guiRect.height - newPos.y + guiRect.y;
        newPos.x *= targetSize.x / guiRect.width;
        newPos.y *= targetSize.y / guiRect.height;
#else
        RectInt rect = GetScreenManager().GetRepositionRect();
        Vector2f scale = GetScreenManager().GetCoordinateScale();
        newPos.x = RoundfToInt((position.x - rect.x) * scale.x);
        newPos.y = RoundfToInt((position.y - rect.y) * scale.y);

        newPos.y = (screenHeight - newPos.y - 1);
        // If the device is a mouse we leave its position and allow negative values when outside of the window. (case 827851)
        if (deviceType == Input::kTouch)
        {
            newPos.x = clamp<float>(newPos.x, 0, screenWidth);
            newPos.y = clamp<float>(newPos.y, 0, screenHeight);
        }
#endif
        return newPos;
    }
}

// These stubs are for Windows based players (web and stand alone), which do not (yet?) support this.
#if !UNITY_EDITOR
Vector3f GetAcceleration() { return Vector3f(0, 0, 0); }
#endif
