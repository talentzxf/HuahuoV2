#pragma once

#include "WinInputBase.h"
#include "Runtime/Input/InputManager.h"
#include "PlatformDependent/Win/Resource.h"
#include "Runtime/Math/Vector4.h"
#include "Runtime/Math/Rect.h"

#include "Runtime/Input/GetInput.h" // Touch
#include "Runtime/Input/TouchPhaseEmulation.h"
#include "WinTouch.h"
#include <winuser.h>

namespace win
{
    class __declspec(novtable)Input : public IInputBase
    {
    protected:
        #pragma region Button

        class Button
        {
        private:
            int key;
            bool state;

        public:
            inline Button(int key, bool state) :
                key(key),
                state(state)
            {
                // dummy constuctor
            }

            inline int GetKey(void) const { return this->key; }
            inline bool GetState(void) const { return this->state; }
        };

        typedef std::vector<Button> Buttons;

        #pragma endregion

        #pragma region MouseState

        class MouseState
        {
        private:
            Vector4f delta;
            Buttons buttons;

        public:
            inline MouseState(void) :
                delta(0.0f, 0.0f, 0.0f, 0.0f)
            {
                this->buttons.reserve(256);
            }

            inline const Vector4f &GetDelta(void) const { return this->delta; }
            inline const Buttons &GetButtons(void) const { return this->buttons; }

            inline void AddDeltaX(float value) { this->delta.x += value; }
            inline void AddDeltaY(float value) { this->delta.y += value; }

            void AddDeltaZ(float value)
            {
                this->delta.z += clamp(value, -1.f, 1.f);
            }

            void AddDeltaW(float value)
            {
                this->delta.w += clamp(value, -1.f, 1.f);
            }

            void AddButton(int key, bool state)
            {
                if ((0 == key) || (1 == key))
                {
                    if (GetSystemMetrics(SM_SWAPBUTTON))
                    {
                        if (0 == key)
                        {
                            key = 1;
                        }
                        else if (1 == key)
                        {
                            key = 0;
                        }
                    }
                }

                this->buttons.push_back(Button(key, state));
            }

            inline void Reset(bool full)
            {
                this->delta.Set(0.0f, 0.0f, 0.0f, 0.0f);
                this->buttons.clear();
            }

            inline void ResetDelta()
            {
                this->delta.Set(0.0f, 0.0f, 0.0f, 0.0f);
            }
        };

        #pragma endregion

        #pragma region KeyboardState

        class KeyboardState
        {
        private:
            Buttons buttons;

        public:
            inline KeyboardState(void)
            {
                this->buttons.reserve(256);
            }

            inline const Buttons &GetButtons(void) const { return this->buttons; }
            inline void AddButton(int key, bool state) { this->buttons.push_back(Button(key, state)); }

            void Reset(bool full)
            {
                this->buttons.clear();
            }
        };

        #pragma endregion

        #pragma region JoystickState

        class JoystickState :
            public ResourceRoot<JoystickState>
        {
        public:
            enum Type
            {
                T_GENERIC               = 0,
                T_XINPUT_DEVICE,                // User id stored in typeData
            };

        private:
            const int id;
            Type type;
            DWORD typeData;

            int keyMap[kMaxJoyStickButtons];
            Buttons buttons;
            float axes[kMaxJoyStickAxis];

        public:
            inline JoystickState(const int id) :
                id(id),
                type(T_GENERIC),
                typeData(0)
            {
                for (int i = 0; i < _countof(this->keyMap); ++i)
                {
                    char name[40];

                    if (-1 != sprintf_s(name, "joystick %d button %d", (id + 1), i))
                    {
                        this->keyMap[i] = StringToKey(name);
                    }
                    else
                    {
                        this->keyMap[i] = -1;
                    }
                }

                this->buttons.reserve(256);

                for (int i = 0; i < _countof(this->axes); ++i)
                {
                    this->axes[i] = 0.0f;
                }
            }

            inline int GetId(void) const { return this->id; }

            inline void SetType(Type t, DWORD typeData = 0u) { this->type = t; this->typeData = typeData; }
            inline Type GetType() const { return type; }
            inline DWORD GetTypeData() const { return typeData; }

            int MapKey(int key) const
            {
                if ((key < 0) || (key >= _countof(this->keyMap)))
                {
                    return -1;
                }

                return this->keyMap[key];
            }

            inline const Buttons &GetButtons(void) const { return this->buttons; }
            inline void AddButton(int key, bool state) { this->buttons.push_back(Button(key, state)); }

            inline const float(&GetAxes(void) const)[kMaxJoyStickAxis] { return this->axes; }
            inline void SetAxis(int axis, float value)
            {
                if ((axis >= 0) && (axis < _countof(this->axes)))
                {
                    this->axes[axis] = value;
                }
            }

            void Reset(bool full)
            {
                this->buttons.clear();

                if (full)
                {
                    for (int i = 0; i < _countof(this->axes); ++i)
                    {
                        this->axes[i] = 0.0f;
                    }
                }
            }
        };

        typedef ResourcePtr<JoystickState> JoystickStatePtr;

        typedef std::map<int, JoystickStatePtr> JoystickStates;

        #pragma endregion

    protected:
        MouseState mouse;
        KeyboardState keyboard;
        JoystickStates joysticks;
        HWND window;

        TouchPhaseEmulation m_Touch;

    private:
        int joystickGlobalKeyMap[kMaxJoyStickButtons];
        bool active;

    public:
        Input(void);

        virtual ~Input(void)
        {
            // dummy destructor
        }

        virtual bool Open(HWND window);
        virtual void Close(void);

        virtual bool GetJoystickNames(dynamic_array<core::string> &names);

        virtual bool Activate(bool active);
        virtual bool ToggleFullscreen(bool fullscreen, HWND window);

        virtual bool Process(bool discard);

        virtual LRESULT OnMessage(HWND window, UINT message, WPARAM wParam, LPARAM lParam, BOOL& handled);

        virtual LRESULT OnKey(HWND window, UINT message, WPARAM wParam, LPARAM lParam);
        virtual LRESULT OnInput(HWND window, UINT message, WPARAM wParam, LPARAM lParam);
        virtual LRESULT OnDeviceChange(LPCWSTR name, bool add);
        virtual LRESULT OnTouch(HWND window, UINT message, WPARAM wParam, LPARAM lParam);

        std::size_t TouchCount() const;
        std::size_t ActiveTouchCount() const;
        bool GetTouch(unsigned index, Touch& touch) const;
        void AddTouchEvent(int pointerId, float x, float y, TouchPhaseEmulation::TouchPhase newPhase, long long timestamp, float radius, float radiusVariance);

        bool IsTouchPressureSupported() const;
        bool IsStylusTouchSupported() const;
        bool IsMultiTouchEnabled() const;
        void SetMultiTouchEnabled(bool flag);
        bool GetMousePresent(void) const;

        void PreprocessTouches();
        void PostprocessTouches();

    protected:
        virtual bool UpdateState(void);

    private:
        bool UpdateMousePosition(void);

    public:
        enum PointerDeviceType
        {
            kMouse,
            kTouch,
            kDeviceTypeCount
        };

        struct PositionConvertData
        {
#if UNITY_EDITOR
            Rectf guiRect;
            Vector2f targetSize;
#else
            int screenHeight;
            int screenWidth;
#endif
            PositionConvertData();

            Vector2f Convert(Input::PointerDeviceType deviceType, const Vector2f& position) const;
        };

        static bool ConvertPositionToClientAreaCoord(HWND activeWindow, POINT position, PointerDeviceType deviceType, const PositionConvertData& data, Vector2f& outPosition);
    private:
        void ProcessTouchImpl(HWND hWnd, const TOUCHINPUT& ti, const Input::PositionConvertData& data);
        void ProcessTouchContactRadius(HWND hWnd, const Input::PositionConvertData& data, DWORD cxContact, DWORD cyContact, float& radius, float& radiusVariance);
    };
}
