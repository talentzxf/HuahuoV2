#include "UnityPrefix.h"
#include "Runtime/Graphics/ScreenManager.h"
#include "Runtime/Input/InputManager.h"
#include "Runtime/Input/GUIEventManager.h"
#include "Runtime/Input/TimeManager.h"
#include "Runtime/Input/SimulateInputEvents.h"
#include "Runtime/Misc/PlayerSettings.h"
#include "Runtime/Misc/SystemInfo.h"
#include "External/Unicode/UTF8.h"
#include "EmscriptenCallback.h"
#include <emscripten/html5.h>

#include "NewInputWebGL.h"
#include "Runtime/Interfaces/IInput.h"

#define kHTML5KeymapSize 225
int g_HTML5Keymap[kHTML5KeymapSize];
int g_NumTouches = 0;
#define kMaxTouches 32
Touch g_Touches[kMaxTouches];
float g_TouchTimes[kMaxTouches];
bool g_MultiTouchEnabled = true;
std::set<int> g_KeysPressed;
EmscriptenDeviceMotionEvent g_Acceleration;

webgl::NewInput* g_NewInput;
static bool g_NewInputSystemRunning = false;
static bool g_OldInputSystemRunning = false;
static bool g_InputInitialized = false;

enum CaptureAllKeyboardInputMode
{
    kCaptureAllKeyboardInputNotInitialized = 0,
    kCaptureAllKeyboardInputEnabled,
    kCaptureAllKeyboardInputDisabled
};

CaptureAllKeyboardInputMode g_CaptureAllKeyboardInput = kCaptureAllKeyboardInputNotInitialized;

enum TouchPhase
{
    kTouchBegan = 0,
    kTouchMoved = 1,
    kTouchStationary = 2,
    kTouchEnded = 3,
    kTouchCanceled = 4
};

static Vector2f GetMousePositionFromInputManager()
{
    Vector2f pos = GetInputManager().GetMousePosition();
    return Vector2f(pos.x, GetScreenManager().GetHeight() - 1 - pos.y);
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

static int HTML5MouseButtonsStateToUnityButton(int buttons)
{
    return buttons & 1 ? 0 : buttons & 2 ? 1 : buttons & 4 ? 2 : 3;
}

// key callback
static int KeyHandler(int eventType, const EmscriptenKeyboardEvent *event, void *userData)
{
    bool consumed = false;

    int keyCode = event->keyCode;
    if (keyCode < kHTML5KeymapSize)
        keyCode = g_HTML5Keymap[keyCode];

    if (event->location == DOM_KEY_LOCATION_RIGHT)
    {
        if (keyCode >= SDLK_LSHIFT && keyCode <= SDLK_LGUI)
            keyCode--;
    }

    if (g_CaptureAllKeyboardInput == kCaptureAllKeyboardInputEnabled)
    {
        switch (keyCode)
        {
            case SDLK_BACKSPACE:
            case SDLK_TAB:
            case SDLK_UP:
            case SDLK_DOWN:
            case SDLK_RIGHT:
            case SDLK_LEFT:
                // This will prevent the browser from processing these keydown events, so that the page does not scroll, browser does not open previous url, etc...
                consumed = true;
                break;
        }
    }

    if (g_NewInput != NULL)
    {
        g_NewInput->KeyEvent(eventType, event);
    }

    InputEvent ie;
    ie.Init();
    ie.type = eventType == EMSCRIPTEN_EVENT_KEYDOWN ? InputEvent::kKeyDown : InputEvent::kKeyUp;
    if (event->ctrlKey)
        ie.modifiers |= InputEvent::kControl;
    if (event->shiftKey)
        ie.modifiers |= InputEvent::kShift;
    if (event->altKey)
        ie.modifiers |= InputEvent::kAlt;
    if (event->metaKey)
        ie.modifiers |= InputEvent::kCommand;
    ie.keycode = keyCode;
    ie.mousePosition = GetMousePositionFromInputManager();
    SanitizeKeyEvent(ie);
    GetGUIEventManager().QueueEvent(ie);

    if (g_OldInputSystemRunning == false)
        return consumed;

    InputManager& im = GetInputManager();

    if (!im.GetTextFieldInput() || !im.GetEatKeyPressOnTextFieldFocus())
    {
        bool state = eventType == EMSCRIPTEN_EVENT_KEYDOWN;

        if (systeminfo::GetOperatingSystemFamily() == kMacOSXFamily)
        {
            // Workarounds for Cmd Key on OS X.
            if (state)
            {
                // We don't get any KeyUp events while Cmd is pressed.
                // So to avoid keys being stuck forever, remember pressed keys,
                // so we can reset them when Cmd is released.
                g_KeysPressed.insert(keyCode);

                // If we receive a keydown event for a key which is already pressed, while
                // Cmd is down, assume the key has been released in between, so release it so
                // that GetKeyDown will be true again.
                if (event->metaKey && im.GetKey(keyCode))
                    im.SetKeyState(keyCode, false);
            }
            else if (!state && (keyCode == SDLK_LMETA || keyCode == SDLK_RMETA))
            {
                // Release all pressed keys when Cmd is released, since we don't receive keyUp events,
                // so we cannot know if they are still pressed.
                for (std::set<int>::iterator i = g_KeysPressed.begin(); i != g_KeysPressed.end(); i++)
                    im.SetKeyState(*i, false);
                g_KeysPressed.clear();
            }
        }

        // Ignore unknown keys, which we may get in some browsers for international keyboard layouts
        if (keyCode != SDLK_UNKNOWN)
            im.SetKeyState(keyCode, state);
    }

    if (g_CaptureAllKeyboardInput == kCaptureAllKeyboardInputEnabled && keyCode == SDLK_BACKSPACE && eventType == EMSCRIPTEN_EVENT_KEYDOWN)
    {
        im.GetInputString() += "\b";
    }

    // In a few cases (e.g.: Backspace and Tab) we want the key event to only be processed here.
    // Thus by returning 'true', TextHandler will not be executed for this key event.
    return consumed;
}

// key press callback
static int TextHandler(int eventType, const EmscriptenKeyboardEvent *event, void *userData)
{
    if (g_NewInput != NULL)
    {
        g_NewInput->TextHandlerEvent(eventType, event);
    }

    if (g_OldInputSystemRunning == false)
        return true;

    unsigned long charCode = event->charCode;
    if (charCode == 0)
    {
        // On Firefox, charCode is zero upon Return/Enter keypress (case 733062)
        if (event->keyCode == SDLK_RETURN)
        {
            charCode = event->keyCode;
        }
        else
        {
            return true;
        }
    }

    InputEvent ie;
    ie.Init();
    ie.type = InputEvent::kKeyDown;

    // On Windows, RightAlt generates events for RightAlt+LeftControl keys.
    // This is how Windows expresses that key under layouts in which it acts as AltGr.
    // So, we ignore Control and Alt keys if they are both pressed (case 730097)
    if (event->ctrlKey && !event->altKey)
        ie.modifiers |= InputEvent::kControl;
    if (event->shiftKey)
        ie.modifiers |= InputEvent::kShift;
    if (event->altKey && !event->ctrlKey)
        ie.modifiers |= InputEvent::kAlt;
    if (event->metaKey)
        ie.modifiers |= InputEvent::kCommand;
    ie.mousePosition = GetMousePositionFromInputManager();
    ie.character = charCode;
    SanitizeKeyEvent(ie);
    GetGUIEventManager().QueueEvent(ie);

    if (g_OldInputSystemRunning == false)
        return true;

    core::string utf8;
    if (ConvertUTF16toUTF8(charCode, utf8))
        GetInputManager().GetInputString() += utf8;

    return true;
}

// mouse up/down callback
static int MouseButtonHandler(int eventType, const EmscriptenMouseEvent *event, void *userData)
{
    if (g_NewInput != NULL)
    {
        g_NewInput->MouseButtonEvent(eventType, event);
    }

    if (g_OldInputSystemRunning == false)
        return false;

    ScreenManagerWebGL &screenManager = GetScreenManager();
    int canvasX = screenManager.CssPixelsToPhysicalPixels(event->canvasX);
    int canvasY = screenManager.CssPixelsToPhysicalPixels(event->canvasY);

    int button = HTML5MouseButtonToUnity(event->button);
    InputEvent ie;
    ie.Init();
    ie.button = button;
    ie.mousePosition = Vector2f(canvasX, canvasY);
    ie.clickCount = 1;

    if (eventType == EMSCRIPTEN_EVENT_MOUSEDOWN)
    {
        static float lastTimestamp = 0;
        static int lastX = 0;
        static int lastY = 0;
        static int lastClickCount = 0;

        float t = GetTimeManager().GetRealtime();
        ie.type = InputEvent::kMouseDown;
        if (canvasX == lastX && canvasY == lastY && t - lastTimestamp <= 1.0f)
            ie.clickCount = lastClickCount + 1;

        lastX = canvasX;
        lastY = canvasY;
        lastClickCount = ie.clickCount;
        lastTimestamp = t;

        // Try restoring the cursor lock on mouse down if it's still required.
        // Browsers may drop the lock on ESC, minimize, task switch, etc.
        GetScreenManager().RestoreCursorLock();

        // Safari will start an Audio Context as suspended and requires us to resume it
        // from a user event handler, to ensure that audio playback is desired. So we
        // check if the context is suspended and resume it inside the mouse down handler.
        JS_Sound_ResumeIfNeeded();
    }
    else
        ie.type = InputEvent::kMouseUp;

    if (event->ctrlKey)
        ie.modifiers |= InputEvent::kControl;
    if (event->shiftKey)
        ie.modifiers |= InputEvent::kShift;
    if (event->altKey)
        ie.modifiers |= InputEvent::kAlt;
    if (event->metaKey)
        ie.modifiers |= InputEvent::kCommand;
    GetGUIEventManager().QueueEvent(ie);

    if (g_OldInputSystemRunning == false)
        return false;

    InputManager& im = GetInputManager();

    im.SetMouseButton(button, eventType != EMSCRIPTEN_EVENT_MOUSEUP);

    return false;
}

// mouse move callback
static int MouseMoveHandler(int eventType, const EmscriptenMouseEvent *event, void *userData)
{
    if (g_NewInput != NULL)
    {
        g_NewInput->MouseMoveEvent(eventType, event);
    }

    if (g_OldInputSystemRunning == false)
        return false;

    ScreenManagerWebGL &screenManager = GetScreenManager();
    int canvasX = screenManager.CssPixelsToPhysicalPixels(event->canvasX);
    int canvasY = screenManager.CssPixelsToPhysicalPixels(event->canvasY);
    int movementX = screenManager.CssPixelsToPhysicalPixels(event->movementX);
    int movementY = screenManager.CssPixelsToPhysicalPixels(event->movementY);

    InputEvent ie;
    ie.Init();

    if (event->buttons)
        ie.type = InputEvent::kMouseDrag;
    else
        ie.type = InputEvent::kMouseMove;

    ie.delta = Vector2f(movementX, movementY);
    ie.mousePosition = Vector2f(canvasX, canvasY);

    // The value of MouseEvent.button only guarantees to indicate which buttons are pressed during events caused by pressing or releasing one or multiple buttons.
    // In order to handle MouseDrag events correctly, the currently pressed mouse button can be determined based on the MouseEvent.buttons value.
    // Note: InputEvent.button represents the currently pressed mouse button, while the precedence of the mouse buttons has been chosen to match other platforms.
    // For this reason InputEvent.button does not necessarily represent the mouse button which was originally pressed to initiate the MouseDrag event.
    ie.button = HTML5MouseButtonsStateToUnityButton(event->buttons);

    if (event->ctrlKey)
        ie.modifiers |= InputEvent::kControl;
    if (event->shiftKey)
        ie.modifiers |= InputEvent::kShift;
    if (event->altKey)
        ie.modifiers |= InputEvent::kAlt;
    if (event->metaKey)
        ie.modifiers |= InputEvent::kCommand;
    GetGUIEventManager().QueueEvent(ie);

    if (g_OldInputSystemRunning == false)
        return false;

    InputManager& im = GetInputManager();
    im.SetMouseDelta(im.GetMouseDelta() + Vector4f(movementX, -movementY, 0, 0));
    im.SetMousePosition(Vector2f(canvasX, GetScreenManager().GetHeight() - 1 - canvasY));

    // Return false here to allow the browser to keep processing the event.
    // Since we handle MouseMoved events for the whole page, we'd otherwise
    // events required by other elements on the page (case 831631)
    return false;
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

// mouse wheel callback
static int MouseWheelHandler(int eventType, const EmscriptenWheelEvent *event, void *userData)
{
    if (g_NewInput != NULL)
    {
        g_NewInput->MouseWheelEvent(eventType, event);
    }

    ScreenManagerWebGL &screenManager = GetScreenManager();
    int canvasX = screenManager.CssPixelsToPhysicalPixels(event->mouse.canvasX);
    int canvasY = screenManager.CssPixelsToPhysicalPixels(event->mouse.canvasY);

    if (g_OldInputSystemRunning == false)
        return true;

    float deltaX = NormalizeWheelInput(event->deltaX);
    float deltaY = NormalizeWheelInput(event->deltaY);

    InputEvent ie;
    ie.Init();
    ie.type = InputEvent::kScrollWheel;
    ie.delta = Vector2f(deltaX, deltaY);
    ie.mousePosition = Vector2f(canvasX, canvasY);
    GetGUIEventManager().QueueEvent(ie);

    if (g_OldInputSystemRunning == false)
        return true;

    InputManager& im = GetInputManager();
    Vector4f delta = im.GetMouseDelta();
    im.SetMouseDelta(Vector4f(delta.x, delta.y, delta.z + deltaX, delta.w - deltaY));

    return true;
}

static int TouchHandler(int eventType, const EmscriptenTouchEvent *event, void *userData)
{
    if (g_NewInput != NULL)
    {
        g_NewInput->TouchEvent(eventType, event);
    }

    ScreenManagerWebGL &screenManager = GetScreenManager();
    g_NumTouches = event->numTouches;
    for (int i = 0; i < event->numTouches; i++)
    {
        if (event->touches[i].isChanged)
        {
            Vector2f pos = Vector2f(screenManager.CssPixelsToPhysicalPixels(event->touches[i].canvasX), GetScreenManager().GetHeight() - 1 - screenManager.CssPixelsToPhysicalPixels(event->touches[i].canvasY));
            float t = GetTimeSinceStartup();
            g_Touches[i].id = event->touches[i].identifier;
            g_Touches[i].deltaPos = g_Touches[i].pos - pos;
            g_Touches[i].pos = pos;
            g_Touches[i].rawPos = pos;
            g_Touches[i].deltaTime = t - g_TouchTimes[i];
            switch (eventType)
            {
                case EMSCRIPTEN_EVENT_TOUCHSTART:
                    g_Touches[i].phase = kTouchBegan;
                    break;

                case EMSCRIPTEN_EVENT_TOUCHEND:
                    g_Touches[i].phase = kTouchEnded;
                    break;

                case EMSCRIPTEN_EVENT_TOUCHMOVE:
                    g_Touches[i].phase = kTouchMoved;
                    break;

                case EMSCRIPTEN_EVENT_TOUCHCANCEL:
                    g_Touches[i].phase = kTouchCanceled;
                    break;
            }
            g_TouchTimes[i] = t;
        }
    }

    // Safari will start an Audio Context as suspended and requires us to resume it
    // from a user event handler, to ensure that audio playback is desired. So we
    // check if the context is suspended and resume it inside the touch handler.
    JS_Sound_ResumeIfNeeded();
    return true;
}

static int MotionHandler(int eventType, const EmscriptenDeviceMotionEvent *event, void *userData)
{
    g_Acceleration = *event;
    return true;
}

static int OrientationHandler(int eventType, const EmscriptenDeviceOrientationEvent *event, void *userData)
{
    // Currently not used. But emscripten html5 wrappers have a bug where the orientation callback needs to be set or the motion callback may crash.
    return true;
}

bool IsCaptureAllKeyboardInputEnabled()
{
    return g_CaptureAllKeyboardInput == kCaptureAllKeyboardInputEnabled;
}

void SetCaptureAllKeyboardInputEnabled(bool flag)
{
    CaptureAllKeyboardInputMode newValue = flag ? kCaptureAllKeyboardInputEnabled : kCaptureAllKeyboardInputDisabled;

    if (newValue != g_CaptureAllKeyboardInput)
    {
        if (g_CaptureAllKeyboardInput != kCaptureAllKeyboardInputNotInitialized)
        {
            // need to uninstall callbacks before installing new ones
            SET_EMSCRIPTEN_CALLBACK(emscripten_set_keydown_callback, NULL, NULL, false, NULL);
            SET_EMSCRIPTEN_CALLBACK(emscripten_set_keyup_callback, NULL, NULL, false, NULL);
            SET_EMSCRIPTEN_CALLBACK(emscripten_set_keypress_callback, NULL, NULL, false, NULL);
        }

        if (flag)
        {
            SET_EMSCRIPTEN_CALLBACK(emscripten_set_keydown_callback, 0, NULL, false, KeyHandler);
            SET_EMSCRIPTEN_CALLBACK(emscripten_set_keyup_callback, 0, NULL, false, KeyHandler);
            SET_EMSCRIPTEN_CALLBACK(emscripten_set_keypress_callback, 0, NULL, true, TextHandler);
        }
        else
        {
            SET_EMSCRIPTEN_CALLBACK(emscripten_set_keydown_callback, "#canvas", NULL, false, KeyHandler);
            SET_EMSCRIPTEN_CALLBACK(emscripten_set_keyup_callback, "#canvas", NULL, false, KeyHandler);
            SET_EMSCRIPTEN_CALLBACK(emscripten_set_keypress_callback, "#canvas", NULL, true, TextHandler);
        }

        g_CaptureAllKeyboardInput = newValue;
    }
}

EM_BOOL GamePadStatusCallback(int eventType, const EmscriptenGamepadEvent *e, void *userData)
{
    if (g_NewInput != NULL)
    {
        g_NewInput->GamePadStatusEvent(eventType, e);
    }

    return 0;
}

void InputInit()
{
    SetCaptureAllKeyboardInputEnabled(true);

    SET_EMSCRIPTEN_CALLBACK(emscripten_set_mouseup_callback, 0 /*"#document"*/, NULL, false, MouseButtonHandler);
    SET_EMSCRIPTEN_CALLBACK(emscripten_set_mousedown_callback, "#canvas", NULL, false, MouseButtonHandler);
    SET_EMSCRIPTEN_CALLBACK(emscripten_set_mousemove_callback, 0, NULL, false, MouseMoveHandler);
    SET_EMSCRIPTEN_CALLBACK(emscripten_set_wheel_callback, "#canvas", NULL, false, MouseWheelHandler);

    SET_EMSCRIPTEN_CALLBACK(emscripten_set_touchstart_callback, "#canvas", NULL, false, TouchHandler);
    SET_EMSCRIPTEN_CALLBACK(emscripten_set_touchend_callback, "#canvas", NULL, false, TouchHandler);
    SET_EMSCRIPTEN_CALLBACK(emscripten_set_touchmove_callback, "#canvas", NULL, false, TouchHandler);
    SET_EMSCRIPTEN_CALLBACK(emscripten_set_touchcancel_callback, "#canvas", NULL, false, TouchHandler);

    SET_EMSCRIPTEN_CALLBACK(emscripten_set_devicemotion_callback, NULL, false, MotionHandler);
    SET_EMSCRIPTEN_CALLBACK(emscripten_set_deviceorientation_callback, NULL, false, OrientationHandler);

    SET_EMSCRIPTEN_CALLBACK(emscripten_set_gamepadconnected_callback, NULL, false, GamePadStatusCallback);
    SET_EMSCRIPTEN_CALLBACK(emscripten_set_gamepaddisconnected_callback, NULL, false, GamePadStatusCallback);

    for (int i = 0; i < kHTML5KeymapSize; i++)
        g_HTML5Keymap[i] = i;
    for (int i = 'A'; i <= 'Z'; i++)
        g_HTML5Keymap[i] = i + 32;

    g_HTML5Keymap[16] = SDLK_LSHIFT;
    g_HTML5Keymap[17] = SDLK_LCTRL;
    g_HTML5Keymap[18] = SDLK_LALT;
    g_HTML5Keymap[20] = SDLK_CAPSLOCK;

    g_HTML5Keymap[33] = SDLK_PAGEUP;
    g_HTML5Keymap[34] = SDLK_PAGEDOWN;
    g_HTML5Keymap[35] = SDLK_END;
    g_HTML5Keymap[36] = SDLK_HOME;
    g_HTML5Keymap[37] = SDLK_LEFT;
    g_HTML5Keymap[38] = SDLK_UP;
    g_HTML5Keymap[39] = SDLK_RIGHT;
    g_HTML5Keymap[40] = SDLK_DOWN;
    g_HTML5Keymap[44] = SDLK_PRINT;
    g_HTML5Keymap[45] = SDLK_INSERT;
    g_HTML5Keymap[46] = SDLK_DELETE;

    g_HTML5Keymap[91] = SDLK_LMETA;

    g_HTML5Keymap[96] = SDLK_KP0;
    g_HTML5Keymap[97] = SDLK_KP1;
    g_HTML5Keymap[98] = SDLK_KP2;
    g_HTML5Keymap[99] = SDLK_KP3;
    g_HTML5Keymap[100] = SDLK_KP4;
    g_HTML5Keymap[101] = SDLK_KP5;
    g_HTML5Keymap[102] = SDLK_KP6;
    g_HTML5Keymap[103] = SDLK_KP7;
    g_HTML5Keymap[104] = SDLK_KP8;
    g_HTML5Keymap[105] = SDLK_KP9;
    g_HTML5Keymap[106] = SDLK_KP_MULTIPLY;
    g_HTML5Keymap[107] = SDLK_KP_PLUS;
    g_HTML5Keymap[109] = SDLK_KP_MINUS;
    g_HTML5Keymap[110] = SDLK_KP_PERIOD;
    g_HTML5Keymap[111] = SDLK_KP_DIVIDE;

    g_HTML5Keymap[112] = SDLK_F1;
    g_HTML5Keymap[113] = SDLK_F2;
    g_HTML5Keymap[114] = SDLK_F3;
    g_HTML5Keymap[115] = SDLK_F4;
    g_HTML5Keymap[116] = SDLK_F5;
    g_HTML5Keymap[117] = SDLK_F6;
    g_HTML5Keymap[118] = SDLK_F7;
    g_HTML5Keymap[119] = SDLK_F8;
    g_HTML5Keymap[120] = SDLK_F9;
    g_HTML5Keymap[121] = SDLK_F10;
    g_HTML5Keymap[122] = SDLK_F11;
    g_HTML5Keymap[123] = SDLK_F12;
    g_HTML5Keymap[124] = SDLK_F13;
    g_HTML5Keymap[125] = SDLK_F14;
    g_HTML5Keymap[126] = SDLK_F15;

    g_HTML5Keymap[144] = SDLK_NUMLOCK;

    g_HTML5Keymap[160] = SDLK_CARET;
    g_HTML5Keymap[161] = SDLK_EXCLAIM;
    g_HTML5Keymap[162] = SDLK_QUOTEDBL;
    g_HTML5Keymap[163] = SDLK_HASH;
    g_HTML5Keymap[164] = SDLK_DOLLAR;
    g_HTML5Keymap[165] = SDLK_PERCENT;
    g_HTML5Keymap[166] = SDLK_AMPERSAND;
    g_HTML5Keymap[167] = SDLK_UNDERSCORE;
    g_HTML5Keymap[168] = SDLK_LEFTPAREN;
    g_HTML5Keymap[169] = SDLK_RIGHTPAREN;
    g_HTML5Keymap[170] = SDLK_ASTERISK;
    g_HTML5Keymap[171] = SDLK_PLUS;
    g_HTML5Keymap[173] = SDLK_MINUS;
    g_HTML5Keymap[174] = SDLK_LEFTBRACKET;
    g_HTML5Keymap[175] = SDLK_RIGHTBRACKET;

    g_HTML5Keymap[186] = SDLK_SEMICOLON;
    g_HTML5Keymap[187] = SDLK_EQUALS;
    g_HTML5Keymap[188] = SDLK_COMMA;
    g_HTML5Keymap[189] = SDLK_MINUS;
    g_HTML5Keymap[190] = SDLK_PERIOD;
    g_HTML5Keymap[191] = SDLK_SLASH;
    g_HTML5Keymap[192] = SDLK_BACKQUOTE;
    g_HTML5Keymap[219] = SDLK_LEFTBRACKET;
    g_HTML5Keymap[220] = SDLK_BACKSLASH;
    g_HTML5Keymap[221] = SDLK_RIGHTBRACKET;
    g_HTML5Keymap[222] = SDLK_QUOTE;

    g_HTML5Keymap[224] = SDLK_LMETA;
}

void GetJoystickNames(dynamic_array<core::string> &names)
{
    names.resize_initialized(0);
    int numGamePads = emscripten_get_num_gamepads();

    for (int i = 0; i < numGamePads; i++)
    {
        EmscriptenGamepadEvent gamepad;
        if (emscripten_get_gamepad_status(i, &gamepad) == EMSCRIPTEN_RESULT_SUCCESS)
            names.push_back(gamepad.id);
    }
}

void InputProcess()
{
    if (g_InputInitialized == false)
    {
        PlayerSettings* settings = GetPlayerSettingsPtr();
        if (settings)
        {
            g_InputInitialized = true;

            g_NewInputSystemRunning = settings->GetEnableNewInputSystem() && GetIInput() != NULL;
            g_OldInputSystemRunning = settings->GetDisableOldInputManagerSupport() == false;

            if (g_OldInputSystemRunning)
                printf_console("Input Manager initialize...\n");

            if (g_NewInputSystemRunning)
                printf_console("Input System initialize...\n");

            if (g_NewInputSystemRunning && g_NewInput == NULL) // InitInput is too early for this code - it is before RuntimeStatic<InputSystemState> g_InputSystemState . Initialize
            {
                g_NewInput = new webgl::NewInput();
                if (!g_NewInput->Open())
                {
                    printf_console("Could not initialize new Input System\n");
                    delete g_NewInput;
                    g_NewInput = NULL;
                    g_NewInputSystemRunning = false;
                }
            }
        }
    }

    if (g_NewInput != NULL)
    {
        g_NewInput->Process();
    }

    if (g_OldInputSystemRunning == false)
        return;

    InputManager& im = GetInputManager();
    int numGamePads = emscripten_get_num_gamepads();

    int id = 0;
    for (int i = 0; i < numGamePads; i++)
    {
        EmscriptenGamepadEvent gamepad;
        if (emscripten_get_gamepad_status(i, &gamepad) == EMSCRIPTEN_RESULT_SUCCESS)
        {
            // Joystick IDx is 1 based
            const int port = id + 1;
            if (!strncmp(gamepad.mapping, "standard", sizeof(gamepad.mapping)))
            {
                // see http://www.w3.org/TR/gamepad for details about the standard layout

                // axis values
                float ltXAxis = gamepad.axis[0];
                float ltYAxis = gamepad.axis[1];
                float ltZAxis = gamepad.analogButton[6]; // left trigger
                float rtXAxis = gamepad.axis[2];
                float rtYAxis = gamepad.axis[3];
                float rtZAxis = gamepad.analogButton[7]; // right trigger

                im.SetJoystickPosition(port, 0, ltXAxis);
                im.SetJoystickPosition(port, 1, ltYAxis);
                im.SetJoystickPosition(port, 2, ltZAxis - rtZAxis); // windows xbox 360 compatibility
                im.SetJoystickPosition(port, 3, rtXAxis);
                im.SetJoystickPosition(port, 4, rtYAxis);

                float DPadY = 0.0f;
                if (gamepad.digitalButton[12])
                    DPadY = 1.0f;
                else if (gamepad.digitalButton[13])
                    DPadY = -1.0f;

                float DPadX = 0.0f;
                if (gamepad.digitalButton[14])
                    DPadX = -1.0f;
                else if (gamepad.digitalButton[15])
                    DPadX = 1.0f;

                im.SetJoystickPosition(port, 6, DPadY);
                im.SetJoystickPosition(port, 5, DPadX);

                im.SetJoystickPosition(port, 8, ltZAxis);
                im.SetJoystickPosition(port, 9, rtZAxis);

                static const int numStandardLayoutButtons = 16;
                static const int standardLayoutRemap[numStandardLayoutButtons] =
                {
                    0, // A
                    1, // B
                    2, // X
                    3, // Y
                    4, // Left Shoulder
                    5, // Right Shoulder
                    -1,
                    -1,
                    6, // Back
                    7, // Start
                    8, // Left Thumbstick
                    9, // Right Thumbstick
                    12, // D-Pad Up
                    13, // D-Pad Down
                    14, // D-Pad Left
                    15  // D-Pad Right
                };

                for (int j = 0; j < numStandardLayoutButtons; j++)
                {
                    if (standardLayoutRemap[j] != -1)
                    {
                        char buttonName[100];
                        sprintf(buttonName, "joystick %d button %d", port, standardLayoutRemap[j]);
                        im.SetKeyState(StringToKey(buttonName), gamepad.digitalButton[j]);
                        sprintf(buttonName, "joystick button %d", standardLayoutRemap[j]);
                        im.SetKeyState(StringToKey(buttonName), gamepad.digitalButton[j]);
                    }
                }
            }
            else
            {
                for (int j = 0; j < gamepad.numAxes; j++)
                    im.SetJoystickPosition(port, j, gamepad.axis[j]);
                for (int j = 0; j < gamepad.numButtons; j++)
                {
                    char buttonName[100];
                    sprintf(buttonName, "joystick %d button %d", port, j);
                    im.SetKeyState(StringToKey(buttonName), gamepad.digitalButton[j]);
                    sprintf(buttonName, "joystick button %d", j);
                    im.SetKeyState(StringToKey(buttonName), gamepad.digitalButton[j]);
                }
            }

            id++;
        }
    }

    for (int i = 0; i < g_NumTouches; i++)
    {
        if (g_Touches[i].phase == kTouchCanceled || g_Touches[i].phase == kTouchEnded)
        {
            if (g_TouchTimes[i])
            {
                g_TouchTimes[i] = 0;
                continue;
            }

            // clear finished touches so they don't keep showing up.
            g_NumTouches--;
            memmove(g_Touches + i, g_Touches + i + 1, sizeof(Touch) * (g_NumTouches - i));
            i--;
        }
    }

    SimulateMouseInput();
}

Vector3f GetAcceleration()
{
    return Vector3f(g_Acceleration.accelerationIncludingGravityX, g_Acceleration.accelerationIncludingGravityY, g_Acceleration.accelerationIncludingGravityZ);
}

size_t GetTouchCount()
{
    return g_NumTouches;
}

bool GetTouch(unsigned index, Touch& touch)
{
    if (index >= g_NumTouches)
        return false;

    touch = g_Touches[index];
    return true;
}

size_t GetActiveTouchCount()
{
    return GetTouchCount();
}

bool IsTouchPressureSupported()
{
    return false;
}

bool IsStylusTouchSupported()
{
    return false;
}

bool IsMultiTouchEnabled()
{
    return g_MultiTouchEnabled;
}

void SetMultiTouchEnabled(bool flag)
{
    g_MultiTouchEnabled = flag;
}

bool GetMousePresent(void)
{
    return true;
}

void InputShutdown()
{
    if (g_NewInput != NULL)
    {
        g_NewInput->Close();
        delete g_NewInput;
        g_NewInput = NULL;
    }

    SET_EMSCRIPTEN_CALLBACK(emscripten_set_keydown_callback, NULL, NULL, false, NULL);
    SET_EMSCRIPTEN_CALLBACK(emscripten_set_keyup_callback, NULL, NULL, false, NULL);
    SET_EMSCRIPTEN_CALLBACK(emscripten_set_keypress_callback, NULL, NULL, false, NULL);

    SET_EMSCRIPTEN_CALLBACK(emscripten_set_mouseup_callback, NULL, NULL, false, NULL);
    SET_EMSCRIPTEN_CALLBACK(emscripten_set_mousedown_callback, NULL, NULL, false, NULL);
    SET_EMSCRIPTEN_CALLBACK(emscripten_set_dblclick_callback, NULL, NULL, false, NULL);
    SET_EMSCRIPTEN_CALLBACK(emscripten_set_mousemove_callback, NULL, NULL, false, NULL);
    SET_EMSCRIPTEN_CALLBACK(emscripten_set_wheel_callback, NULL, NULL, false, NULL);

    SET_EMSCRIPTEN_CALLBACK(emscripten_set_touchstart_callback, NULL, NULL, false, NULL);
    SET_EMSCRIPTEN_CALLBACK(emscripten_set_touchend_callback, NULL, NULL, false, NULL);
    SET_EMSCRIPTEN_CALLBACK(emscripten_set_touchmove_callback, NULL, NULL, false, NULL);
    SET_EMSCRIPTEN_CALLBACK(emscripten_set_touchcancel_callback, NULL, NULL, false, NULL);

    SET_EMSCRIPTEN_CALLBACK(emscripten_set_devicemotion_callback, NULL, false, NULL);
    SET_EMSCRIPTEN_CALLBACK(emscripten_set_deviceorientation_callback, NULL, false, NULL);

    SET_EMSCRIPTEN_CALLBACK(emscripten_set_gamepadconnected_callback, NULL, false, NULL);
    SET_EMSCRIPTEN_CALLBACK(emscripten_set_gamepaddisconnected_callback, NULL, false, NULL);
}

void ResetInput()
{
}

void ResetInputAfterPause()
{
}
