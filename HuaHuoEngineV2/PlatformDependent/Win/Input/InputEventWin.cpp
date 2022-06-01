#include "UnityPrefix.h"
#include "Input.h"
#include "Runtime/Misc/InputEvent.h"
#include "Runtime/Input/InputManager.h"
#include "Runtime/Graphics/ScreenManager.h"
#include "Runtime/Input/GUIEventManager.h"
#if UNITY_EDITOR
#include "Runtime/Input/GetInput.h"
#include "Editor/Platform/Interface/HighDpi.h"
#endif
#include <winuser.h>
#include <windowsx.h>
#include "Editor/Platform/Windows/WinMonitorCollection.h"

#ifndef WM_MOUSEWHEEL
#define WM_MOUSEWHEEL 0x020A
#endif
#ifndef GET_WHEEL_DELTA_WPARAM
#define GET_WHEEL_DELTA_WPARAM(wParam)  ((short)HIWORD(wParam))
#endif
#ifndef MK_ALT
#define MK_ALT  ( 0x20 )
#endif
#ifndef WM_MOUSEHWHEEL
#define WM_MOUSEHWHEEL 0x020E
#endif

int InputKeycodeFromVKEY(int vkey);   // GetInputDX.cpp
int GetModifierFlags();
Vector2f InputEvent::s_LastTouchDownPosition = Vector2f::zero;

extern bool s_wantsMouseJumping; // EditorGUIUtility.bindings


static Vector2f g_LastMousePosition;
static Vector2f GetInputMousePosition(WPARAM wParam, LPARAM lParam, HWND window)
{
    POINT p;
    p.x = GET_X_LPARAM(lParam);
    p.y = GET_Y_LPARAM(lParam);

    Vector2f position = g_LastMousePosition;

#if UNITY_EDITOR
    p = DPIScaleHelper(window).FromNative(p);
    position.x = p.x;
    position.y = p.y;
#else
    if (ClientToScreen(window, &p) && win::Input::ConvertPositionToClientAreaCoord(window, p, win::Input::kMouse, win::Input::PositionConvertData(), position))
        g_LastMousePosition = position;
#endif
    return position;
}

Vector2f InputEvent::GetCurrentRelativeMousePosition(HWND window)
{
    POINT mousePos;
    Vector2f modifiedPos;
    ::GetCursorPos(&mousePos);
    ::ScreenToClient(window, &mousePos);
#if UNITY_EDITOR
    mousePos = DPIScaleHelper(window).FromNative(mousePos);
    modifiedPos = Vector2f(mousePos.x, mousePos.y);
#else
    modifiedPos = win::Input::PositionConvertData().Convert(win::Input::kMouse, Vector2f(mousePos.x, mousePos.y));
#endif
    return modifiedPos;
}

bool InputEvent::IsMouseEventComingFromTouch()
{
    // Check if incoming mouse events are emulated from touches
#define MOUSEEVENTF_FROMTOUCH 0xFF515700
    return (GetMessageExtraInfo() & MOUSEEVENTF_FROMTOUCH) == MOUSEEVENTF_FROMTOUCH;
}

#if UNITY_EDITOR
InputEvent InputEvent::RepaintEvent(HWND window)
{
    InputEvent evt;
    evt.Init();
    evt.type = kRepaint;
    evt.modifiers = GetModifierFlags();

    Vector2f p = GetCurrentRelativeMousePosition(window);
    #if !UNITY_EDITOR
    p.y = GetScreenManager().GetHeight() - p.y;
    #endif
    evt.mousePosition = p;

    evt.button = 0;
    bool swapped = GetSystemMetrics(SM_SWAPBUTTON);
    if (GetAsyncKeyState(swapped ? VK_LBUTTON : VK_RBUTTON))
        evt.button = kRightButton;
    if (GetAsyncKeyState(swapped ? VK_RBUTTON : VK_LBUTTON))
        evt.button = kLeftButton;

    //evt.Debug();

    return evt;
}

InputEvent InputEvent::RepaintEventWithMousePosition(HWND window, Vector2f mousePosition)
{
    InputEvent evt = RepaintEvent(window);
    evt.mousePosition = mousePosition;
    return evt;
}

InputEvent InputEvent::SimulateKeyPressEvent(HWND window, int keyCode, int modifiers)
{
    InputEvent evt;
    evt.Init();
    evt.type = kKeyDown;

    evt.modifiers = modifiers;
    evt.keycode = keyCode;

    Vector2f p = GetCurrentRelativeMousePosition(window);
    #if !UNITY_EDITOR
    p.y = GetScreenManager().GetHeight() - p.y;
    #endif

    evt.mousePosition = p;

    evt.button = 0;

    return evt;
}

#endif

InputEvent::InputEvent(UINT message, WPARAM wParam, LPARAM lParam, HWND window)
    :   commandString(NULL)
    ,   delta(0.0f, 0.0f)
    ,   displayIndex(0)
{
    static Vector2f s_LastMousePos(0.0f, 0.0f);
    static HWND s_LastMousePosWindow = NULL;

    bool isMouse = false, isKey = false;

    // TODO:
    //if (modFlags & NSFunctionKeyMask)
    //  modifiers |= InputEvent::kFunctionKey;

    // Optional Members defaulted here:
    clickCount = 0;
    button = 0;

    // Figure out event type
    switch (message)
    {
        case WM_LBUTTONDOWN:
            type = InputEvent::kMouseDown;
            isMouse = true;
            button = kLeftButton;
            clickCount = 1;
            break;
        case WM_RBUTTONDOWN:
            type = InputEvent::kMouseDown;
            isMouse = true;
            button = kRightButton;
            clickCount = 1;
            break;
        case WM_MBUTTONDOWN:
            type = InputEvent::kMouseDown;
            isMouse = true;
            button = kMiddleButton;
            clickCount = 1;
            break;
        case WM_LBUTTONUP:
            type = InputEvent::kMouseUp;
            isMouse = true;
            button = kLeftButton;
            clickCount = 1;
            break;
        case WM_RBUTTONUP:
            type = InputEvent::kMouseUp;
            isMouse = true;
            button = kRightButton;
            clickCount = 1;
            break;
        case WM_MBUTTONUP:
            type = InputEvent::kMouseUp;
            isMouse = true;
            button = kMiddleButton;
            clickCount = 1;
            break;
        case WM_LBUTTONDBLCLK:
            type = InputEvent::kMouseDown;
            isMouse = true;
            button = kLeftButton;
            clickCount = 2;
            break;
        case WM_RBUTTONDBLCLK:
            type = InputEvent::kMouseDown;
            isMouse = true;
            button = kRightButton;
            clickCount = 2;
            break;
        case WM_MBUTTONDBLCLK:
            type = InputEvent::kMouseDown;
            isMouse = true;
            button = kMiddleButton;
            clickCount = 2;
            break;
        case WM_MOUSEMOVE:
            type = InputEvent::kMouseMove;
            isMouse = true;
            button = 0;
            if (wParam & MK_LBUTTON)
            {
                type = InputEvent::kMouseDrag;
                button |= kLeftButton;
            }
            if (wParam & MK_RBUTTON)
            {
                type = InputEvent::kMouseDrag;
                button |= kRightButton;
            }
            if (wParam & MK_MBUTTON)
            {
                type = InputEvent::kMouseDrag;
                button |= kMiddleButton;
            }
            break;
        case WM_MOUSEHOVER:
            type = InputEvent::kMouseEnterWindow;
            isMouse = true;
            break;
        case WM_MOUSELEAVE:
            type = InputEvent::kMouseLeaveWindow;
            isMouse = true;
            break;
        case WM_KEYUP:
        case WM_SYSKEYUP:
            type = InputEvent::kKeyUp;
            isKey = true;
            break;
        case WM_KEYDOWN:
        case WM_SYSKEYDOWN:
            type = InputEvent::kKeyDown;
            isKey = true;
            break;
        case WM_CHAR:
            // Only process WM_CHAR if:
            //   1) High-order bit is 0 (key was PRESSED)
            //   2) High-order bit is 1 (key was RELEASED) AND scan code is 0x38 (Universal ALT key scan code)
            //
            //  In case 2, we assume an Alt+numpad Unicode pattern was entered. Unlike regular WM_CHAR messaages, which are
            //    preceded by a WM_KEYDOWN and with the high-order bit unset (signifies key pressed), messages sent via
            //    Alt+numpad entry *do not* have a corresponding WM_KEYDOWN event. Instead, they're preceded by the WM_KEYUP
            //    event for the ALT key (VK_MENU) followed by a WM_CHAR with the universal ALT scan code (0x38) and high-order
            //    bit unset (signfies key released). The scan code is in the 3rd byte of the WM_CHAR lParam, so check against
            //    0x380000 instead of 0x38. The wParam contains the actual character, as with a standard WM_CHAR message.
            if (!(lParam & (1 << 31)) || (lParam & 0xFF0000) == 0x380000)
            {
                type = InputEvent::kKeyDown;
                isKey = true;
            }
            break;
        case WM_MOUSEWHEEL:
            delta.y = -GET_WHEEL_DELTA_WPARAM(wParam) / float(WHEEL_DELTA) * 3.0f;

            type = InputEvent::kScrollWheel;
            isMouse = true;
            break;
        case WM_MOUSEHWHEEL:
            delta.x = GET_WHEEL_DELTA_WPARAM(wParam) / float(WHEEL_DELTA) * 3.0f;

            type = InputEvent::kScrollWheel;
            isMouse = true;
            break;
        default:
            type = InputEvent::kIgnore;
            break;
    }

    // Handle modifiers
    modifiers = 0;
    if (type != InputEvent::kIgnore)
    {
        if (GetKeyState(VK_SHIFT) < 0)
            modifiers  |= InputEvent::kShift;
        if (GetKeyState(VK_CONTROL) < 0)
            modifiers  |= InputEvent::kControl;
        if (GetKeyState(VK_MENU) < 0)
            modifiers  |= InputEvent::kAlt;
        if (GetKeyState(VK_LWIN) < 0 || GetKeyState(VK_RWIN) < 0)
            modifiers |= InputEvent::kCommand;
        if (GetKeyState(VK_CAPITAL) < 0)
            modifiers |= InputEvent::kCapsLock;
    }

    // set up mouse values
    if (isMouse)
    {
        // For now it only tells if it comes either from mouse or touch (finger or pen). We choose pen by default like MacOs
        pointerType = InputEvent::IsMouseEventComingFromTouch() ? InputEvent::kPen : InputEvent::kMouse;

        Vector2f p;
        if ((pointerType == kTouch || pointerType == kPen) && type == kMouseDown)
        {
            POINT ptInput;
            ptInput.x = InputEvent::s_LastTouchDownPosition.x;
            ptInput.y = InputEvent::s_LastTouchDownPosition.y;
            ScreenToClient(window, &ptInput);
#if UNITY_EDITOR
            ptInput = DPIScaleHelper(window).FromNative(ptInput);
#endif
            p = Vector2f(ptInput.x, ptInput.y);
        }
        else
        {
            // from windows documentation, these events should not be used because the param is wrong
            if (type == InputEvent::kScrollWheel    // in that case the lparam contains coordinates relative to the screen
                || type == InputEvent::kMouseLeaveWindow) // in that case the lparam should not be used
                p = GetCurrentRelativeMousePosition(window);
            else
                p = GetInputMousePosition(wParam, lParam, window);
        }

        // TODO: We should invert ScreenPointToRay to be top-left (I think)
        // For now, we use the old versions & just invert the Y coordinate later

        #if !UNITY_EDITOR
        p.y = GetScreenManager().GetHeight() - p.y;
        #endif

        mousePosition = p;

        pressure = 1.0f; // TODO ?

        // mouse wheel events already captured scroll distance in delta
        if (type == InputEvent::kScrollWheel && message == WM_MOUSEWHEEL)
        {
            // nothing
        }
        // other events are real mouse ones. Compute delta
        else
        {
            if (s_LastMousePosWindow == NULL) // for the first time, set a delta of zero
                delta = Vector2f::zero;
            else
            {
                // convert the last mouse pos from old window to new window
                if (s_LastMousePosWindow != window)
                {
                    POINT pt;

                    #if UNITY_EDITOR
                    pt = DPIScaleHelper(s_LastMousePosWindow).ConvertToNative(s_LastMousePos);
                    #else
                    pt.x = s_LastMousePos.x;
                    pt.y = s_LastMousePos.y;
                    #endif

                    ::ClientToScreen(s_LastMousePosWindow, &pt);
                    ::ScreenToClient(window, &pt);
                    #if UNITY_EDITOR
                    pt = DPIScaleHelper(window).FromNative(pt);
                    #endif
                    s_LastMousePos.x = pt.x;
                    s_LastMousePos.y = pt.y;
                }
                delta = mousePosition - s_LastMousePos;
            }

            #if UNITY_EDITOR
            if (type == kMouseDrag && pointerType == InputEvent::kMouse && s_wantsMouseJumping) // The mouse jumping works only for mouse
                DoMouseJumpingThroughScreenEdges(window, mousePosition, s_LastMousePos);
            else
            #endif
            s_LastMousePos = mousePosition;
            s_LastMousePosWindow = window;

            // We get quite a lot of move/drag events that have delta of zero. Filter those
            // out.
            if ((type == kMouseMove || type == kMouseDrag) && delta.x == 0.0f && delta.y == 0.0f)
                type = InputEvent::kIgnore;
        }
    }
    // Zero out the values for non-mouse events
    else
    {
        Vector2f p = GetCurrentRelativeMousePosition(window);

        pressure = 0.0f;
        delta = Vector2f(0.0f, 0.0f);

        // TODO: We should invert ScreenPointToRay to be top-left (I think)
        // For now, we use the old versions & just invert the Y coordinate later

        #if !UNITY_EDITOR
        p.y = GetScreenManager().GetHeight() - p.y;
        #endif

        mousePosition = p;
    }

    // handle keyboard
    character = keycode = 0;
    keyRepeat = false;

    if (isKey)
    {
        if (message == WM_CHAR)
        {
            character = wParam;
            if (character == 127 || character < ' ' && character != 13 && character != 10 && character != 9) // ignore control characters other than Enter or Tab
                type = InputEvent::kIgnore;
        }
        else
        {
            switch (wParam)
            {
                case VK_CONTROL:
                {
                    keycode = InputKeycodeFromVKEY((lParam & (1 << 24)) ? VK_RCONTROL : VK_LCONTROL);
                }
                break;
                case VK_MENU:
                {
                    keycode = InputKeycodeFromVKEY((lParam & (1 << 24)) ? VK_RMENU : VK_LMENU);
                }
                break;
                case VK_RETURN:
                {
                    // There is no VK_Keypad_Enter, so we need to jump directly to setting the SDL keycode.
                    keycode = (lParam & (1 << 24)) ? SDLK_KP_ENTER : SDLK_RETURN;
                }
                break;
                case VK_SHIFT:
                {
                    // Left/Right Shift key can only be determined from scancode
                    UINT scancode = (lParam & 0x00ff0000) >> 16;
                    keycode = InputKeycodeFromVKEY(MapVirtualKey(scancode, MAPVK_VSC_TO_VK_EX));
                }
                break;
                default:
                {
                    keycode = InputKeycodeFromVKEY(wParam);
                }
                break;
            }

            if (keycode == SDLK_UNKNOWN)
                type = InputEvent::kIgnore;
            if (((wParam >= VK_F1) || (wParam >= VK_PRIOR && wParam <= VK_HELP)) && (wParam < VK_OEM_1 || wParam > VK_OEM_8))
                modifiers |= InputEvent::kFunctionKey;
        }

        // The unity convention is that function keys are anything that requires special handling
        // when dealing with a text entry field. Hence, backspace is
        if (keycode == SDLK_BACKSPACE)
            modifiers |= InputEvent::kFunctionKey;

        // "Return" gives an ASCII-13 (Carriage return). For sanity, we turn that into ASCII-10(linefeed, \n)
        if (character == 13)
            character = 10;

        bool previousKeyState = ((lParam & (1 << 30)) != 0);
        keyRepeat = previousKeyState;
    }
}

#if UNITY_EDITOR
void InputEvent::DoMouseJumpingThroughScreenEdges(HWND window, Vector2f mousePos, Vector2f& lastMousePos)
{
    const int kMargin = 10;
    POINT cursorPos;
    Rectf bounds;
    bool doSet = false;

    DPIScaleHelper dpi(window);

    cursorPos = dpi.ConvertToNative(lastMousePos);
    ClientToScreen(window, &cursorPos);

    const MonitorDesc* monitor = MonitorCollection::Get()->FindMonitorFromNativePosition(cursorPos);

    if (monitor != NULL)
    {
        bounds = Rectf(monitor->monitorArea.left, monitor->monitorArea.top, monitor->monitorArea.right - monitor->monitorArea.left, monitor->monitorArea.bottom - monitor->monitorArea.top);
    }
    else
    {
        bounds = GetScreenManager().GetBoundsOfDesktopAtPoint(Vector2f(cursorPos.x, cursorPos.y));
        bounds = dpi.ToNativeMultiMonitor(bounds);
    }

    cursorPos = dpi.ConvertToNative(mousePos);
    ClientToScreen(window, &cursorPos);

    if (cursorPos.x < bounds.x + kMargin)
    {
        cursorPos.x = bounds.GetXMax() - kMargin - (kMargin - (cursorPos.x - bounds.x));
        doSet = true;
    }
    else if (cursorPos.x > bounds.GetXMax() - kMargin)
    {
        cursorPos.x = bounds.x + kMargin + (kMargin - (bounds.GetXMax() - cursorPos.x));
        doSet = true;
    }
    if (cursorPos.y < bounds.y + kMargin)
    {
        cursorPos.y = bounds.GetYMax() - kMargin - (kMargin - (cursorPos.y - bounds.y));
        doSet = true;
    }
    else if (cursorPos.y > bounds.GetYMax() - kMargin)
    {
        cursorPos.y = bounds.y + kMargin + (kMargin - (bounds.GetYMax() - cursorPos.y));
        doSet = true;
    }

    if (doSet)
    {
        SetCursorPos(cursorPos.x, cursorPos.y);
        ScreenToClient(window, &cursorPos);
        lastMousePos = dpi.ConvertFromNative(cursorPos);
    }
    else
        lastMousePos = mousePos;
}

#endif

#if UNITY_EDITOR
InputEvent::InputEvent(int x, int y, DWORD keyFlags, int typ, HWND window)
    :   commandString(NULL)
    ,   delta(0.0f, 0.0f)
    ,   displayIndex(0)
    ,   keyRepeat(false)
{
    type = static_cast<Type>(typ);

    POINT pos = { x, y };
    ::ScreenToClient(window, &pos);
    pos = DPIScaleHelper(window).FromNative(pos);
    mousePosition = Vector2f(pos.x, pos.y);

    button = 0;
    character = 0;
    keycode = 0;
    modifiers = 0;
    pressure = 0;
    clickCount = 0;

    if (keyFlags & MK_CONTROL)
        modifiers |= kControl;
    if (keyFlags & MK_SHIFT)
        modifiers |= kShift;
    if (keyFlags & MK_ALT)
        modifiers |= kAlt;
}

InputEvent InputEvent::CommandStringEvent(const core::string& editorCommand, bool execute, HWND window)
{
    InputEvent event;
    event.Init();

    event.mousePosition = GetCurrentRelativeMousePosition(window);

    if (execute)
        event.type = InputEvent::kExecuteCommand;
    else
        event.type = InputEvent::kValidateCommand;
    event.commandString = new char[editorCommand.size() + 1];
    memcpy(event.commandString, editorCommand.c_str(), editorCommand.size() + 1);
    return event;
}

int GetModifierFlags()
{
    int modifiers = 0;

    if (GetKeyState(VK_SHIFT) < 0)
        modifiers  |= InputEvent::kShift;
    if (GetKeyState(VK_CONTROL) < 0)
        modifiers  |= InputEvent::kControl;
    if (GetKeyState(VK_MENU) < 0)
        modifiers  |= InputEvent::kAlt;
    if (GetKeyState(VK_LWIN) < 0 || GetKeyState(VK_RWIN) < 0)
        modifiers |= InputEvent::kCommand;
    if (GetKeyState(VK_CAPITAL) < 0)
        modifiers |= InputEvent::kCapsLock;

    return modifiers;
}

#endif


void ProcessMessageForInput(HWND window, UINT message, WPARAM wParam, LPARAM lParam)
{
    InputEvent ie(message, wParam, lParam, window);
    GetGUIEventManager().QueueEvent(ie);
}
