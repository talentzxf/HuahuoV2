#pragma once

// Define special-case VirtualKey code used by KeyboardMapping without pulling in WinUser.h
#ifndef VK_PAUSE
#define VK_PAUSE          0x13
#endif

#ifndef VK_SNAPSHOT
#define VK_SNAPSHOT       0x2C
#endif

#ifndef VK_NUMLOCK
#define VK_NUMLOCK        0x90
#endif

namespace win
{
// Shared between Win32 and UWP (Metro)
namespace shared
{
    // Holds mapping between hardware scancodes and KeyCodes defined by input system
    class KeyboardMapping
    {
    public:
        KeyboardMapping()
        {
            memset(m_ScanCodeToKeyCode, 0, _countof(m_ScanCodeToKeyCode) * _countof(m_ScanCodeToKeyCode[0]) * sizeof(int));
            memset(m_KeyCodeToScanCode, 0, _countof(m_KeyCodeToScanCode) * sizeof(int));

            InitializeKeyMapInternal();
        }

        int GetScanCode(KeyboardInputState::KeyCode keyCode) const
        {
            AssertMsg(keyCode < _countof(m_KeyCodeToScanCode), "keyCode value is out of range");

            return m_KeyCodeToScanCode[keyCode];
        }

        int GetScanCode(void* keyCodeBuffer) const
        {
            if (keyCodeBuffer == NULL)
                return 0;

            auto keyCodeValue = static_cast<UInt32*>(keyCodeBuffer);
            if ((*keyCodeValue) >= KeyboardInputState::Count)
                return 0;

            return GetScanCode(static_cast<KeyboardInputState::KeyCode>(*keyCodeValue));
        }

        KeyboardInputState::KeyCode GetKeyCode(int scanCode, bool alternateKey, int virtualKey) const
        {
            // We need to use the Windows VirtualKey (if known) to disambiguate between certain special keys, i.e. Pause and PrintScreen
            //
            // IMPORTANT: Win32's RAWINPUT sends 2 messages for Pause key: first holds scancode 0x1D and second has scancode 0x45.
            // This is because Pause is special and has a longer Make code that doesn't fit in 1 byte.
            //
            // HOWEVER, this does NOT occur on UWP; the OnKey event fires only once with the first scancode value 0x1D. Furthermore,
            // we only receive a single "Extended" flag, which seems to be set if either E0 or E1 prefix is set, rather than
            // the individual RI_KEY_E0 and RI_KEY_E1 flags. This means on UWP it's impossible to distinguish between Pause and
            // RightControl by scancode alone.
            //
            // Similarly, PrintScreen returns 0x37 (with E0 prefix) when Shift/Ctrl keys pressed and 0x54 (no prefix) when Alt is pressed,
            // which cannot be directly mapped. Therefore, we'll employ logic to handle these cases
            //
            // NOTE: The Win32 message handler will need to trap/discard the second message for Pause key

            KeyboardInputState::KeyCode keyCode;
            if (scanCode == 0x1D && virtualKey == VK_PAUSE)
            {
                keyCode = KeyboardInputState::Pause;
            }
            else if (scanCode == 0x37 && alternateKey)
            {
                AssertMsg(virtualKey == VK_SNAPSHOT, "Expected VirtualKey VK_SNAPSHOT for 'PrtScn' key");   // Verify this is indeed PrintScreen
                keyCode = KeyboardInputState::PrintScreen;
            }
            else if (scanCode == 0x54 && !alternateKey)
            {
                AssertMsg(virtualKey == VK_SNAPSHOT, "Expected VirtualKey VK_SNAPSHOT for 'PrtScn' key");   // Verify this is indeed PrintScreen
                keyCode = KeyboardInputState::PrintScreen;
            }
            else if (scanCode < _countof(m_ScanCodeToKeyCode))
            {
                if (scanCode == 0x45 && virtualKey == VK_NUMLOCK)
                {
                    // The E0 prefix is NOT set for RAWINPUT but IS used for UWP; unify these cases by clearing alternateKey flag
                    alternateKey = false;
                }
                keyCode = m_ScanCodeToKeyCode[scanCode][alternateKey ? 1 : 0];
            }
            else
            {
                keyCode = KeyboardInputState::None;
            }

            return keyCode;
        }

    private:

        KeyboardInputState::KeyCode     m_ScanCodeToKeyCode[256][2];
        UInt32                          m_KeyCodeToScanCode[256];

        void InitializeKeyMapInternal()
        {
            #define MAP(key, scan, ...) \
                MapKeyToScanCode(KeyboardInputState:: ## key, scan, __VA_ARGS__);

            // This mapping can be found at: https://download.microsoft.com/download/1/6/1/161ba512-40e2-4cc9-843a-923143f3456c/scancode.doc
            // Top row.
            MAP(Escape,         0x01, "Escape");
            MAP(F1,             0x3B, "F1");
            MAP(F2,             0x3C, "F2");
            MAP(F3,             0x3D, "F3");
            MAP(F4,             0x3E, "F4");
            MAP(F5,             0x3F, "F5");
            MAP(F6,             0x40, "F6");
            MAP(F7,             0x41, "F7");
            MAP(F8,             0x42, "F8");
            MAP(F9,             0x43, "F9");
            MAP(F10,            0x44, "F10");
            MAP(F11,            0x57, "F11");
            MAP(F12,            0x58, "F12");

            // PrintScreen can return 3 different Make codes depending if Shift, Ctrl, or Alt are also pressed:
            // - Make: E0 2A when pressed by itself
            // - Make: E0 37 when Shift or Ctrl are pressed
            // - Make: 54 when Alt pressed
            // We cannot directly map all these cases using this simple schema and must use special-case logic
            // (See GetKeyCode() comments)
            MAP(PrintScreen,    0x2A, "PrtScn", true);  // Map PrtScn's default Make code
            MAP(ScrollLock,     0x46, "ScrLk");

            // The PAUSE key is a super-special case and a pain to deal with; it's Make code is too big to store in one byte
            // so we cannot directly map it with this simple layout. Also, similar with PrintScreen, it returns a different
            // Make code when Ctrl is pressed, i.e. the "Break" key. Furthermore it's key events are reported differently
            // on Win32 and UWP (see GetKeyCode() comments).
            //
            // To handle these cases we'll use the following strategy:
            // - "Pause" key is not directly mapped but handled with special-case logic within GetKeyCode()
            // - "Break" key is directly mapped to Pause instead (Make E0 46)
            // - Win32/UWP will need to handle different behavior of Pause/Break key rather than KeyboardMappings
            MAP(Pause,          0x46, "Pause/Break", true);

            // Digit row.
            MAP(Backquote,      0x29, "`");
            MAP(Digit1,         0x02, "1");
            MAP(Digit2,         0x03, "2");
            MAP(Digit3,         0x04, "3");
            MAP(Digit4,         0x05, "4");
            MAP(Digit5,         0x06, "5");
            MAP(Digit6,         0x07, "6");
            MAP(Digit7,         0x08, "7");
            MAP(Digit8,         0x09, "8");
            MAP(Digit9,         0x0A, "9");
            MAP(Digit0,         0x0B, "0");
            MAP(Minus,          0x0C, "-");
            MAP(Equals,         0x0D, "=");
            MAP(Backspace,      0x0E, "BACKSPACE");

            // Tab row.
            MAP(Tab,            0x0F, "TAB");
            MAP(Q,              0x10, "Q");
            MAP(W,              0x11, "W");
            MAP(E,              0x12, "E");
            MAP(R,              0x13, "R");
            MAP(T,              0x14, "T");
            MAP(Y,              0x15, "Y");
            MAP(U,              0x16, "U");
            MAP(I,              0x17, "I");
            MAP(O,              0x18, "O");
            MAP(P,              0x19, "P");
            MAP(LeftBracket,    0x1A, "[");
            MAP(RightBracket,   0x1B, "]");
            MAP(Backslash,      0x2B, "\\"); // Key 29 on US keyboards, key 42 on international keyboards.

            // Caps lock row.
            MAP(CapsLock,       0x3A, "CAPS");
            MAP(A,              0x1E, "A");
            MAP(S,              0x1F, "S");
            MAP(D,              0x20, "D");
            MAP(F,              0x21, "F");
            MAP(G,              0x22, "G");
            MAP(H,              0x23, "H");
            MAP(J,              0x24, "J");
            MAP(K,              0x25, "K");
            MAP(L,              0x26, "L");
            MAP(Semicolon,      0x27, ";");
            MAP(Quote,          0x28, "'");
            MAP(Enter,          0x1C, "ENTER");

            // Shift row.
            // NOTE: This row can have two additional keys on some keyboards (key 45 with code 0x56 and
            //       key 56 with code 0x73). In the old system we just mapped both of them to SDLK_BACKSLASH
            //       (VK_OEM_5 and VK_OEM_102) which I think is a bad solution. We map them to OEM keys here.
            MAP(LeftShift,      0x2A, "LSHIFT");
            MAP(OEM1,           0x56, "OEM1"); // Key 45. International keyboards only.
            MAP(Z,              0x2C, "Z");
            MAP(X,              0x2D, "X");
            MAP(C,              0x2E, "C");
            MAP(V,              0x2F, "V");
            MAP(B,              0x30, "B");
            MAP(N,              0x31, "N");
            MAP(M,              0x32, "M");
            MAP(Comma,          0x33, ",");
            MAP(Period,         0x34, ".");
            MAP(Slash,          0x35, "/");
            MAP(OEM2,           0x73, "OEM2"); // Key 56. Brazilian and some far east keyboards only.
            MAP(RightShift,     0x36, "RSHIFT");

            // Bottom row.
            MAP(LeftCtrl,       0x1D, "LCTRL");
            MAP(LeftMeta,       0x5B, "LMETA", true); // Left Windows key
            MAP(LeftAlt,        0x38, "LALT");
            MAP(Space,          0x39, "SPACE");
            MAP(RightAlt,       0x38, "RALT", true);
            MAP(RightMeta,      0x5C, "RMETA", true); // Right Windows key (not common)
            MAP(ContextMenu,    0x5D, "MENU", true); // "Menu" key
            MAP(RightCtrl,      0x1D, "RCTRL", true);

            // Numpad.
            ////TODO: test this out with an external numpad that has an equals key (or a Mac keyboard on a PC; they too have the equals key) and see what it comes out as
            // NumLock can return 0x45 Make code both with or without E0 prefix; UWP seems to use E0 flag while Win32's RAWINPUT does not
            // This will also be handled with special case logic (see GetKeyCode() comments)
            MAP(NumLock,        0x45, "NUMLOCK");
            MAP(NumpadDivide,   0x35, "NUM \\", true);
            MAP(NumpadMultiply, 0x37, "NUM *");
            MAP(Numpad7,        0x47, "NUM 7");
            MAP(Numpad8,        0x48, "NUM 8");
            MAP(Numpad9,        0x49, "NUM 9");
            MAP(NumpadMinus,    0x4A, "NUM -");
            MAP(Numpad4,        0x4B, "NUM 4");
            MAP(Numpad5,        0x4C, "NUM 5");
            MAP(Numpad6,        0x4D, "NUM 6");
            MAP(NumpadPlus,     0x4E, "NUM +");
            MAP(Numpad2,        0x50, "NUM 2");
            MAP(Numpad3,        0x51, "NUM 3");
            MAP(Numpad1,        0x4F, "NUM 1");
            MAP(Numpad0,        0x52, "NUM 0");
            MAP(NumpadPeriod,   0x53, "NUM .");
            MAP(NumpadEnter,    0x1C, "NUM ENTER", true);

            // Insert/delete block.
            MAP(Insert,         0x52, "INS", true);
            MAP(Delete,         0x53, "DEL", true);
            MAP(Home,           0x47, "HOME", true);
            MAP(End,            0x4F, "END", true);
            MAP(PageUp,         0x49, "PgUp", true);
            MAP(PageDown,       0x51, "PgDn", true);

            // Arrow block.
            MAP(UpArrow,        0x48, "UP", true);
            MAP(LeftArrow,      0x4B, "LEFT", true);
            MAP(DownArrow,      0x50, "DOWN", true);
            MAP(RightArrow,     0x4D, "RIGHT", true);

#undef MAP
        }

        void MapKeyToScanCode(KeyboardInputState::KeyCode keyCode, UInt32 scanCode, LPCSTR keyName, bool alternate = false)
        {
            m_ScanCodeToKeyCode[scanCode][alternate ? 1 : 0] = keyCode;

            // Input events gives us these as flags, but the hardware scancodes have these as part of the value.  We need to reattach them for GetKeyNameTextW.
            if (alternate)
            {
                scanCode |= 0x0000100;
            }
            m_KeyCodeToScanCode[keyCode] = scanCode;
        }
    };
} // namespace shared
} // namespace win
