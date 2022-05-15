#pragma once


#include "Modules/Input/InputDeviceState.h"
#include "Modules/Input/InputEventData.h"

namespace webgl
{
    class NewInput
    {
    public:
        NewInput();

        bool Open();
        void Close();

        void KeyEvent(int eventType, const EmscriptenKeyboardEvent *event);
        void TextHandlerEvent(int eventType, const EmscriptenKeyboardEvent *event);

        void MouseMoveEvent(int eventType, const EmscriptenMouseEvent *event);
        void MouseButtonEvent(int eventType, const EmscriptenMouseEvent *event);
        void MouseWheelEvent(int eventType, const EmscriptenWheelEvent *event);

        void GamePadStatusEvent(int eventType, const EmscriptenGamepadEvent *event);

        void TouchEvent(int eventType, const EmscriptenTouchEvent *event);

        void Process();
    private:
        StateInputEventData<MouseInputState> m_MouseState;
        StateInputEventData<KeyboardInputState> m_KeyboardState;
        int m_TouchscreenDeviceId;
        double m_LastClickTime;
        UInt16 m_LastClickFlags;

        void InitializeKeyboard();
        void InitializeMouse();
        void InitializeTouch();
    };
}
