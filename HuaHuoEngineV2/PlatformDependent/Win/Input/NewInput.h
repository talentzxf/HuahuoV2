#pragma once

#include "WinInputBase.h"
#include "Modules/Input/InputSystem.h"
#include "Runtime/Utilities/RuntimeStatic.h"

#include "PlatformDependent/Win/Input/KeyboardMappings.h"

namespace win
{
    class NewInput final : public IInputBase
    {
    public:
        NewInput();

        // IInputBase.
        virtual bool Open(HWND window);
        virtual void Close(void);
        virtual bool Activate(bool active);
        virtual bool ToggleFullscreen(bool fullscreen, HWND window);
        virtual LRESULT OnMessage(HWND window, UINT message, WPARAM wParam, LPARAM lParam, BOOL& handled);

        static const win::shared::KeyboardMapping& GetKeyboardMapping() { return *s_KeyboardMapping; }

        int GetKeyboardId() { return m_Keyboard.state.deviceId; }
        void SetKeyboardIMEIsSelected(bool selected);

    protected:

        enum { kCompositionModeAuto = 0, kCompositionModeOn, kCompositionModeOff };

        struct Keyboard : InputDeviceCallbacks
        {
            StateInputEventData<KeyboardInputState> state;
            Keyboard() { memset(&state, 0, sizeof(state)); }
            virtual SInt64 IOCTL(int code, void* buffer, int bufferSize);
        };

        struct Mouse : InputDeviceCallbacks
        {
            StateInputEventData<MouseInputState> state;
            Mouse() { memset(&state, 0, sizeof(state)); }
            virtual SInt64 IOCTL(int code, void* buffer, int bufferSize);
        };

        struct Pen : InputDeviceCallbacks
        {
            StateInputEventData<PenInputState> state;
            Pen() { memset(&state, 0, sizeof(state)); }
            virtual SInt64 IOCTL(int code, void* buffer, int bufferSize);
        };

        ////TODO: support multiple mice
        Keyboard m_Keyboard;
        Mouse m_Mouse;
        Pen m_Pen;
        double m_LastClickTime;
        USHORT m_LastClickFlags;

        int m_TouchscreenDeviceId;
        bool m_MouseButtonsSwapped;

        static RuntimeStatic<win::shared::KeyboardMapping> s_KeyboardMapping;

        void InitializeIME();
        void InitializeKeyboard();
        void InitializeMouse();
        void InitializeTouch();
        void InitializePen();

        void RegisterRawInput(HWND window);

        void OnTouch(HWND window, UINT message, WPARAM wParam, LPARAM lParam);
        void OnInput(HWND window, UINT message, WPARAM wParam, LPARAM lParam, bool isFocused);
        void OnText(HWND window, UINT message, WPARAM wParam, LPARAM lParam);
        void OnPointer(HWND window, UINT message, WPARAM wParam, LPARAM lParam);

        void OnMouse(const RAWMOUSE &data);
        void OnKey(const RAWKEYBOARD &data);
    };

    double GetCurrentEventTimeInUnityTime();
}
