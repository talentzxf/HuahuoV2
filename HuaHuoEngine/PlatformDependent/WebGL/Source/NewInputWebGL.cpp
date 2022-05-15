#include "UnityPrefix.h"
#include "NewInputWebGL.h"
#include "Runtime/Graphics/ScreenManager.h"
#include "Runtime/Input/InputManager.h"
#include "Runtime/Input/GUIEventManager.h"
#include "Runtime/Input/TimeManager.h"
#include "Runtime/Input/SimulateInputEvents.h"
#include "Modules/Input/InputDeviceUtilities.h"
#include "Runtime/Misc/SystemInfo.h"
#include "External/Unicode/UTF8.h"
#include <emscripten/html5.h>
#include "Runtime/Interfaces/IInput.h"
#include "Runtime/Core/Containers/StringBuilder.h"

double GetStartupTimestamp();

namespace webgl
{
    double EmscriptenEventTimeToUnityTime(double emscriptenTimestamp)
    {
        const double timeInMsSinceMessageCreated = emscripten_get_now() - emscriptenTimestamp;
        return GetTimeSinceStartup() - static_cast<double>(timeInMsSinceMessageCreated) / Baselib_MillisecondsPerSecond;
    }

    #define kDistributionSize 500
    #define kMouseWheelScale 1.0f
    static float NormalizeWheelInput(float value)
    {
        // It appears that the scale of scroll wheel input values varies greatly across
        // browsers, different versions of the same browser, OSes and input devices.
        // Trying the approach proposed here to normalize input values:
        // http://jsbin.com/toyaqegumu/edit?html,css,js,output
        if (value == 0)
            return value;
        static dynamic_array<float> distribution;
        distribution.reserve(kDistributionSize);
        if (distribution.size() < kDistributionSize)
        {
            float avalue = fabs(value);
            dynamic_array<float>::iterator i = distribution.begin();
            for (; i != distribution.end() && *i < avalue; i++)
            {
            }
            distribution.insert(i, avalue);
        }
        return value * kMouseWheelScale / distribution[distribution.size() / 3];
    }

    static int HTML5MouseButtonToUnity(int sdl)
    {
        switch (sdl)
        {
            case 0: return 0;
            case 1: return 2;
            case 2: return 1;
            default: return 3;
        }
    }

    struct EmscriptenGamepadDevice
    {
        dynamic_array<char> eventBuffer;
        int axisCount;
        int buttonsCount;

        StateInputEventData<char>* GetEventBuffer()
        {
            return (StateInputEventData<char>*)eventBuffer.data();
        }

        const StateInputEventData<char>* GetEventBuffer() const
        {
            return (const StateInputEventData<char>*)eventBuffer.data();
        }
    };

    static core::hash_map<core::string, KeyboardInputState::KeyCode> g_HTML5CodeToKey;

    static dynamic_array<EmscriptenGamepadDevice> g_HTML5GamepadArray(4, kMemInput);
    static uint64_t g_HTML5GamepadActiveFlags = 0;

    inline bool HTML5GamepadIsActive(uint32_t indx)
    {
        return !!(g_HTML5GamepadActiveFlags & (1ULL << indx));
    }

    inline void HTML5GamepadSetActive(uint32_t indx, bool active)
    {
        if (active)
            g_HTML5GamepadActiveFlags |=   1ULL << indx;    // 0000000 |  0000010 = 0000010
        else
            g_HTML5GamepadActiveFlags &= ~(1ULL << indx);   // 0010000 & ~0010000 = 0010000 & 1101111 = 0000000
    }

    void HTML5GamepadMinSize(int size)
    {
        if (size > 64 || size < 0)
        {
            printf_console("[ERROR] HTML5GamepadMinSize sanity check failed, size = %d \n");
            return;
        }

        if (g_HTML5GamepadArray.size() < size)
            g_HTML5GamepadArray.resize_initialized(size, EmscriptenGamepadDevice());
    }

    NewInput::NewInput()
    {
        #define REG_KEY_HASH(K) (g_HTML5CodeToKey[#K] = KeyboardInputState::KeyCode::K)

        //https://developer.mozilla.org/en-US/docs/Web/API/KeyboardEvent/code#Code_values

        g_HTML5CodeToKey[""] = KeyboardInputState::KeyCode::None;
        g_HTML5CodeToKey["Unidentified"] = KeyboardInputState::KeyCode::None;

        REG_KEY_HASH(Space);
        REG_KEY_HASH(Enter);
        REG_KEY_HASH(Tab);
        REG_KEY_HASH(Backquote);
        REG_KEY_HASH(Quote);
        REG_KEY_HASH(Semicolon);
        REG_KEY_HASH(Comma);
        REG_KEY_HASH(Period);
        REG_KEY_HASH(Slash);
        REG_KEY_HASH(Backslash);
        g_HTML5CodeToKey["BracketLeft"] = KeyboardInputState::KeyCode::LeftBracket;             // different
        g_HTML5CodeToKey["BracketRight"] = KeyboardInputState::KeyCode::RightBracket;           // different
        REG_KEY_HASH(Minus);
        g_HTML5CodeToKey["Equal"] = KeyboardInputState::KeyCode::Equals;                        // different

        #define REG_KEY_HASH_PREKEY(K) (g_HTML5CodeToKey["Key" #K] = KeyboardInputState::KeyCode::K)

        REG_KEY_HASH_PREKEY(A);
        REG_KEY_HASH_PREKEY(B);
        REG_KEY_HASH_PREKEY(C);
        REG_KEY_HASH_PREKEY(D);
        REG_KEY_HASH_PREKEY(E);
        REG_KEY_HASH_PREKEY(F);
        REG_KEY_HASH_PREKEY(G);
        REG_KEY_HASH_PREKEY(H);
        REG_KEY_HASH_PREKEY(I);
        REG_KEY_HASH_PREKEY(J);
        REG_KEY_HASH_PREKEY(K);
        REG_KEY_HASH_PREKEY(L);
        REG_KEY_HASH_PREKEY(M);
        REG_KEY_HASH_PREKEY(N);
        REG_KEY_HASH_PREKEY(O);
        REG_KEY_HASH_PREKEY(P);
        REG_KEY_HASH_PREKEY(Q);
        REG_KEY_HASH_PREKEY(R);
        REG_KEY_HASH_PREKEY(S);
        REG_KEY_HASH_PREKEY(T);
        REG_KEY_HASH_PREKEY(U);
        REG_KEY_HASH_PREKEY(V);
        REG_KEY_HASH_PREKEY(W);
        REG_KEY_HASH_PREKEY(X);
        REG_KEY_HASH_PREKEY(Y);
        REG_KEY_HASH_PREKEY(Z);

        #undef REG_KEY_HASH_PREKEY

        REG_KEY_HASH(Digit0);
        REG_KEY_HASH(Digit1);
        REG_KEY_HASH(Digit2);
        REG_KEY_HASH(Digit3);
        REG_KEY_HASH(Digit4);
        REG_KEY_HASH(Digit5);
        REG_KEY_HASH(Digit6);
        REG_KEY_HASH(Digit7);
        REG_KEY_HASH(Digit8);
        REG_KEY_HASH(Digit9);

        g_HTML5CodeToKey["ShiftLeft"] = KeyboardInputState::KeyCode::LeftShift;                 // different
        g_HTML5CodeToKey["ShiftRight"] = KeyboardInputState::KeyCode::RightShift;               // different
        g_HTML5CodeToKey["AltLeft"] = KeyboardInputState::KeyCode::LeftAlt;                     // different
        g_HTML5CodeToKey["AltRight"] = KeyboardInputState::KeyCode::RightAlt;                   // different
        g_HTML5CodeToKey["ControlLeft"] = KeyboardInputState::KeyCode::LeftCtrl;                // different
        g_HTML5CodeToKey["ControlRight"] = KeyboardInputState::KeyCode::RightCtrl;              // different
        g_HTML5CodeToKey["OSLeft"] = KeyboardInputState::KeyCode::LeftMeta;                     // different
        g_HTML5CodeToKey["OSRight"] = KeyboardInputState::KeyCode::RightMeta;                   // different
        g_HTML5CodeToKey["MetaLeft"] = KeyboardInputState::KeyCode::LeftMeta;                   // different. same as OSLeft
        g_HTML5CodeToKey["MetaRight"] = KeyboardInputState::KeyCode::RightMeta;                 // different. same as OSRight
        REG_KEY_HASH(ContextMenu);
        REG_KEY_HASH(Escape);
        g_HTML5CodeToKey["ArrowLeft"] = KeyboardInputState::KeyCode::LeftArrow;                 // different
        g_HTML5CodeToKey["ArrowRight"] = KeyboardInputState::KeyCode::RightArrow;               // different
        g_HTML5CodeToKey["ArrowUp"] = KeyboardInputState::KeyCode::UpArrow;                     // different
        g_HTML5CodeToKey["ArrowDown"] = KeyboardInputState::KeyCode::DownArrow;                 // different
        REG_KEY_HASH(Backspace);
        REG_KEY_HASH(PageDown);
        REG_KEY_HASH(PageUp);
        REG_KEY_HASH(Home);
        REG_KEY_HASH(End);
        REG_KEY_HASH(Insert);
        REG_KEY_HASH(Delete);
        REG_KEY_HASH(CapsLock);
        REG_KEY_HASH(NumLock);
        REG_KEY_HASH(PrintScreen);
        REG_KEY_HASH(ScrollLock);
        REG_KEY_HASH(Pause);

        REG_KEY_HASH(NumpadEnter);
        REG_KEY_HASH(NumpadDivide);
        REG_KEY_HASH(NumpadMultiply);
        g_HTML5CodeToKey["NumpadAdd"] = KeyboardInputState::KeyCode::NumpadPlus;                // different
        g_HTML5CodeToKey["NumpadSubtract"] = KeyboardInputState::KeyCode::NumpadMinus;          // different
        g_HTML5CodeToKey["NumpadDecimal"] = KeyboardInputState::KeyCode::NumpadPeriod;          // different
        g_HTML5CodeToKey["NumpadEqual"] = KeyboardInputState::KeyCode::NumpadEquals;            // different
        REG_KEY_HASH(Numpad0);
        REG_KEY_HASH(Numpad1);
        REG_KEY_HASH(Numpad2);
        REG_KEY_HASH(Numpad3);
        REG_KEY_HASH(Numpad4);
        REG_KEY_HASH(Numpad5);
        REG_KEY_HASH(Numpad6);
        REG_KEY_HASH(Numpad7);
        REG_KEY_HASH(Numpad8);
        REG_KEY_HASH(Numpad9);

        REG_KEY_HASH(F1);
        REG_KEY_HASH(F2);
        REG_KEY_HASH(F3);
        REG_KEY_HASH(F4);
        REG_KEY_HASH(F5);
        REG_KEY_HASH(F6);
        REG_KEY_HASH(F7);
        REG_KEY_HASH(F8);
        REG_KEY_HASH(F9);
        REG_KEY_HASH(F10);
        REG_KEY_HASH(F11);
        REG_KEY_HASH(F12);

        g_HTML5CodeToKey["IntlBackslash"] = KeyboardInputState::KeyCode::OEM1;
        // OEM2,
        // OEM3,
        // OEM4,
        // OEM5,

        #undef REG_KEY_HASH
    }

    bool NewInput::Open()
    {
        if (!GetIInput())
            return false;

        printf_console("[NewInput] Open\n");

        GetIInput()->InitInput();

        InitializeKeyboard();
        InitializeMouse();
        InitializeTouch();

        return true;
    }

    void NewInput::Close()
    {
        if (!GetIInput())
            return;
        GetIInput()->InputShutdown();
    }

    void NewInput::Process()
    {
        int numGamePads = emscripten_get_num_gamepads();
        HTML5GamepadMinSize(numGamePads);

        for (int i = 0; i < numGamePads; i++)
        {
            if (HTML5GamepadIsActive(i) == false) continue;

            EmscriptenGamepadEvent gamepad;
            if (emscripten_get_gamepad_status(i, &gamepad) == EMSCRIPTEN_RESULT_SUCCESS)
            {
                EmscriptenGamepadDevice& device = g_HTML5GamepadArray[i];
                StateInputEventData<char>* e = device.GetEventBuffer();

                double newTimestamp = EmscriptenEventTimeToUnityTime(gamepad.timestamp);

                if (e->time < newTimestamp)
                {
                    e->time = newTimestamp;

                    float* writer = (float*)&e->stateData;

                    for (int i = 0; i < device.axisCount; ++i, ++writer)
                    {
                        *writer = (float)gamepad.axis[i];
                    }
                    for (int i = 0; i < device.buttonsCount; ++i, ++writer)
                    {
                        *writer = (float)gamepad.analogButton[i];
                    }

                    GetIInput()->QueueInputEvent(*e);
                }
            }
        }
    }

    void NewInput::KeyEvent(int eventType, const EmscriptenKeyboardEvent *event)
    {
        if (eventType != EMSCRIPTEN_EVENT_KEYDOWN && eventType != EMSCRIPTEN_EVENT_KEYUP)
            return;

        const auto timestamp = GetIInput()->GetInputEventTimeNow();

        // A string that identifies the physical key being pressed.
        // The value is not affected by the current keyboard layout or modifier state, so a particular key will always return the same value.
        const core::string utf8code = event->code;
        const auto it = g_HTML5CodeToKey.find(utf8code);
        const auto keyCode = (it == g_HTML5CodeToKey.end()) ? KeyboardInputState::KeyCode::None : it->second;

        if (keyCode != KeyboardInputState::KeyCode::None)
        {
            bool pressed = eventType == EMSCRIPTEN_EVENT_KEYDOWN;

            m_KeyboardState.stateData.SetKey(keyCode, pressed);
            m_KeyboardState.time = timestamp;

            GetIInput()->QueueInputEvent(m_KeyboardState);
        }
    }

    void NewInput::TextHandlerEvent(int eventType, const EmscriptenKeyboardEvent *event)
    {
        const auto timestamp = GetIInput()->GetInputEventTimeNow();

        // The printed representation of the pressed key.
        const core::string keycode = event->key;
        dynamic_array<UTF16> keycodeUtf16;

        ConvertUTF8toUTF16(keycode, keycodeUtf16);
        for (int i = 0; i < keycodeUtf16.size(); ++i)
        {
            GetIInput()->QueueTextInputEvent(kInputEventText, m_KeyboardState.deviceId, timestamp, keycodeUtf16[i]);
        }
    }

    void NewInput::TouchEvent(int eventType, const EmscriptenTouchEvent *event)
    {
        if (!GetIInput())
        {
            return;
        }

        float time = GetTimeSinceStartup();
        ScreenManagerWebGL &screenManager = GetScreenManager();

        for (int i = 0; i < event->numTouches; ++i)
        {
            const EmscriptenTouchPoint& touch = event->touches[i];
            // Ignore touches that haven't changed
            if (!touch.isChanged)
            {
                continue;
            }

            StateInputEventData<TouchInputState> touchEvent(m_TouchscreenDeviceId, time, kInputTouchState);
            TouchInputState& touchState = touchEvent.stateData;
            memset(&touchState, 0, sizeof(TouchInputState));

            switch (eventType)
            {
                case EMSCRIPTEN_EVENT_TOUCHSTART:
                    touchState.phase = TouchInputState::kBegan;
                    break;

                case EMSCRIPTEN_EVENT_TOUCHEND:
                    touchState.phase = TouchInputState::kEnded;
                    break;

                case EMSCRIPTEN_EVENT_TOUCHMOVE:
                    touchState.phase = TouchInputState::kMoved;
                    break;

                case EMSCRIPTEN_EVENT_TOUCHCANCEL:
                    touchState.phase = TouchInputState::kCancelled;
                    break;

                default:
                    touchState.phase = TouchInputState::kNone;
                    break;
            }

            // HTML5 touch event id's can have a 0 value, but the new input system doesn't allow 0.
            touchState.touchId = touch.identifier + 1;

            Vector2f pos = Vector2f(screenManager.CssPixelsToPhysicalPixels(touch.canvasX),
                GetScreenManager().GetHeight() - 1 - screenManager.CssPixelsToPhysicalPixels(touch.canvasY));
            touchState.positionX = pos.x;
            touchState.positionY = pos.y;
            touchState.pressure = 1; // HTML5 touch events don't support pressure

            GetIInput()->QueueInputEvent(touchEvent);
        }
    }

    void NewInput::MouseMoveEvent(int eventType, const EmscriptenMouseEvent *event)
    {
        ScreenManagerWebGL &screenManager = GetScreenManager();
        m_MouseState.stateData.positionX = screenManager.CssPixelsToPhysicalPixels(event->canvasX);
        m_MouseState.stateData.positionY = GetScreenManager().GetHeight() - 1 - screenManager.CssPixelsToPhysicalPixels(event->canvasY);
        m_MouseState.stateData.deltaX = screenManager.CssPixelsToPhysicalPixels(event->movementX);
        m_MouseState.stateData.deltaY = -screenManager.CssPixelsToPhysicalPixels(event->movementY);

        m_MouseState.stateData.buttons = event->buttons;

        m_MouseState.time = EmscriptenEventTimeToUnityTime(event->timestamp);

        if (eventType == EMSCRIPTEN_EVENT_MOUSEDOWN)
        {
            // In WebGL, we have no way to query actual OS double click time. So we use the windows default of 0.5s.
            // The proper way to detect double clicks, respecting OS preferences, would be to listen for
            // EMSCRIPTEN_EVENT_DBLCLICK events. But the problem with this is that they are fired after mosue up events.
            // So we'd have no way of knowing at mouse up event time if the event belongs to a single or double click,
            // and we'd need an additional event, which would be inconsisten with other platforms. We also cannot only
            // listen for EMSCRIPTEN_EVENT_DBLCLICK and EMSCRIPTEN_EVENT_CLICK events, as those are not fired when the
            // mouse is release outside of the canvas, which we want to know to reliably track mouse button state.
            if (m_LastClickFlags == event->buttons && m_MouseState.time - m_LastClickTime <= 0.5f)
                m_MouseState.stateData.clickCount++;
            else
                m_MouseState.stateData.clickCount = 1;
            m_LastClickTime = m_MouseState.time;
            m_LastClickFlags = event->buttons;
        }
        GetIInput()->QueueInputEvent(m_MouseState);
    }

    void NewInput::MouseButtonEvent(int eventType, const EmscriptenMouseEvent *event)
    {
        MouseMoveEvent(eventType, event);
    }

    void NewInput::MouseWheelEvent(int eventType, const EmscriptenWheelEvent *event)
    {
        m_MouseState.stateData.scrollX = NormalizeWheelInput(event->deltaX);
        m_MouseState.stateData.scrollY = -NormalizeWheelInput(event->deltaY);

        MouseMoveEvent(eventType, &event->mouse);

        m_MouseState.stateData.scrollX = m_MouseState.stateData.scrollY = 0;
    }

    void NewInput::GamePadStatusEvent(int eventType, const EmscriptenGamepadEvent *e)
    {
        HTML5GamepadMinSize(e->index);

        if (e->connected)
        {
            const int dataEventSizeInBytes = sizeof(float) * (e->numAxes + e->numButtons);
            const int fullEventSizeInBytes = dataEventSizeInBytes + sizeof(StateInputEventBase);

            // to avoid linking whole JSON library
            core::StringBuilder jsonBuild(kMemTempAlloc);
            jsonBuild.append("{");
            jsonBuild.append("\"interface\":\"webgl\"");
            jsonBuild.append(",\"type\":\"Gamepad\"");
            jsonBuild.append(",\"product\":\"");
            jsonBuild.append(e->id);
            jsonBuild.append("\"");
            jsonBuild.append(",\"capabilities\":");
            {
                jsonBuild.append("\"{\\\"numAxes\\\":");
                jsonBuild.append(e->numAxes);
                jsonBuild.append(",\\\"numButtons\\\":");
                jsonBuild.append(e->numButtons);
                jsonBuild.append(",\\\"mapping\\\":\\\"");
                jsonBuild.append(e->mapping);
                jsonBuild.append("\\\"}\"");
            }
            jsonBuild.append("}");

            core::string descriptorJSON = jsonBuild.ToString();

            EmscriptenGamepadDevice device;
            device.axisCount = e->numAxes;
            device.buttonsCount = e->numButtons;
            device.eventBuffer.resize_uninitialized(fullEventSizeInBytes);

            StateInputEventData<char>* gamepadState = device.GetEventBuffer();
            gamepadState->type = kInputEventState;
            gamepadState->sizeInBytes = fullEventSizeInBytes;
            gamepadState->stateFormat = 'HTML';
            gamepadState->time = EmscriptenEventTimeToUnityTime(e->timestamp);
            gamepadState->deviceId = GetIInput()->ReportNewInputDevice(descriptorJSON);

            if (HTML5GamepadIsActive(e->index))
                printf_console("[ERROR] Just connected gamepad #%d was active! \n", e->index);

            g_HTML5GamepadArray[e->index] = device;
            HTML5GamepadSetActive(e->index, true);

            printf_console("Gamepad added %d \n", e->index);
        }
        else
        {
            if (HTML5GamepadIsActive(e->index))
            {
                const auto& device = g_HTML5GamepadArray[e->index];
                const StateInputEventData<char>* gamepadState = device.GetEventBuffer();

                GetIInput()->ReportInputDeviceRemoved(gamepadState->deviceId);
                printf_console("Gamepad removed %d \n", e->index);
            }
            else
                printf_console("[ERROR] Disconnected gamepad #%d was not active! \n", e->index);

            HTML5GamepadSetActive(e->index, false);
        }
    }

    void NewInput::InitializeKeyboard()
    {
        if (!GetIInput())
            return;

        InputDeviceDescriptor descriptor;
        descriptor.type = "Keyboard";
        descriptor.interfaceName = "html5";

        memset(&m_KeyboardState, 0, sizeof(m_KeyboardState));

        m_KeyboardState.deviceId = GetIInput()->ReportNewInputDevice(descriptor);

        m_KeyboardState.sizeInBytes = sizeof(m_KeyboardState);
        m_KeyboardState.type = kInputEventState;
        m_KeyboardState.stateFormat = kInputKeyboardState;
    }

    void NewInput::InitializeMouse()
    {
        if (!GetIInput())
            return;

        InputDeviceDescriptor descriptor;
        descriptor.type = "Mouse";
        descriptor.interfaceName = "html5";

        memset(&m_MouseState, 0, sizeof(m_MouseState));

        m_MouseState.deviceId = GetIInput()->ReportNewInputDevice(descriptor);

        m_MouseState.sizeInBytes = sizeof(m_MouseState);
        m_MouseState.type = kInputEventState;
        m_MouseState.stateFormat = kInputMouseState;

        m_LastClickFlags = 0;
        m_LastClickTime = -1;
    }

    void NewInput::InitializeTouch()
    {
        if (!GetIInput())
        {
            return;
        }

        TouchDeviceDescriptor descriptor;
        descriptor.type = "Touchscreen";
        descriptor.interfaceName = "html5";
        descriptor.maxTouches = 32;
        descriptor.supportsPressure = false;

        m_TouchscreenDeviceId = GetIInput()->ReportNewInputDevice(descriptor);
    }
}
