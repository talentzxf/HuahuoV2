#pragma once

#define ENABLE_RAW_INPUT_FOR_HID    1

#include "Input.h"
#include "RawInputHid.h"
#include "PlatformDependent/Win/Handle.h"
#include "PlatformDependent/Win/Resource.h"
#include "Runtime/Math/Vector2.h"


namespace win
{
    class RawInput :
        public Input
    {
    private:
        #pragma region HidDevice

        class HidDevice :
            public ResourceRoot<HidDevice>
        {
        private:
            #pragma region Data

            typedef void (HidDevice::*DataCallback)(const HIDP_DATA &data, ULONG index, LONG minimum, LONG maximum);

            class Data
            {
            private:
                DataCallback callback;
                ULONG index;
                LONG minimum;
                LONG maximum;

            public:
                Data(DataCallback callback, ULONG index, LONG minimum = 0, LONG maximum = 0) :
                    callback(callback),
                    index(index),
                    minimum(minimum),
                    maximum(maximum)
                {
                    // dummy constructor
                }

                inline void Invoke(HidDevice *device, const HIDP_DATA &data) const { (device->*this->callback)(data, this->index, this->minimum, this->maximum); }
            };

            typedef std::map<USHORT, Data> DataMap;

            #pragma endregion

            #pragma region Value

            enum ValueType : USHORT
            {
                kValueTypeAxis,
                kValueTypeSlider,
                kValueTypeHatSwitch,
                kValueTypeUnknown = 0xffff
            };

            class Value
            {
            private:
                ValueType type;
                USHORT index;
                USHORT dataIndex;
                LONG minimum;
                LONG maximum;

            public:
                Value(ValueType type, USHORT index, USHORT dataIndex, LONG minimum, LONG maximum) :
                    type(type),
                    index(index),
                    dataIndex(dataIndex),
                    minimum(minimum),
                    maximum(maximum)
                {
                    // dummy constructor
                }

                inline static bool IsSlider(const Value &value) { return (kValueTypeSlider == value.type); }
                inline static bool IsHatSwitch(const Value &value) { return (kValueTypeHatSwitch == value.type); }

                static bool Less(const Value &left, const Value &right)
                {
                    ULONG leftWeight = ((left.type << 16) | left.index);
                    ULONG rightWeight = ((right.type << 16) | right.index);

                    return (leftWeight < rightWeight);
                }

                void AddData(ULONG &valueIndex, DataMap &dataMap)
                {
                    DataCallback callback;
                    ULONG index = valueIndex;

                    switch (this->type)
                    {
                        case kValueTypeAxis:
                        case kValueTypeSlider:
                        {
                            callback = &HidDevice::OnValue;
                            ++valueIndex;
                        }
                        break;

                        case kValueTypeHatSwitch:
                        {
                            callback = &HidDevice::OnHatSwitch;
                            valueIndex += 2;
                        }
                        break;

                        default:
                            return;
                    }

                    Data data(callback, index, this->minimum, this->maximum);
                    dataMap.insert(DataMap::value_type(this->dataIndex, data));
                }
            };

            typedef std::vector<Value> Values;

            #pragma endregion

            typedef std::vector<CHAR> Report;
            typedef std::vector<HIDP_DATA> DataList;
            typedef std::vector<bool> ButtonStates;
            typedef std::map<USHORT, LONG> HatSwitchStates;

        private:
            static const Vector2f hatSwitchDirections[];

            const int id;
            FileHandle handle;
            PHIDP_PREPARSED_DATA preparsedData;
            core::wstring rawName;
            core::string name;
            Report inputReport;
            EventHandle inputEvent;
            OVERLAPPED inputOverlapped;
            DataMap dataMap;
            DataList dataList;
            ButtonStates newButtonStates;
            ButtonStates oldButtonStates;
            HatSwitchStates newHatSwitchStates;
            HatSwitchStates oldHatSwitchStates;
            JoystickStatePtr state;
            bool reading;
            bool latentConnectionPoll;

        public:
            inline HidDevice(int id) : id(id), preparsedData(NULL), reading(false), latentConnectionPoll(false) { ZeroMemory(&this->inputOverlapped, sizeof(this->inputOverlapped)); }
            inline ~HidDevice(void) { this->Close(); }

            inline int GetId(void) const { return this->id; }
            inline core::wstring const& GetRawName(void) const { return this->rawName; }
            inline core::string const& GetName(void) const { return this->name; }
            inline JoystickStatePtr const& GetState(void) const { return this->state; }
            inline bool GetConnected(void) const { return this->handle.IsOpen(); }
            inline bool GetLatentConnectionPoll(void) const { return this->latentConnectionPoll; }

            void StopLatentConnectionPoll(void) { this->latentConnectionPoll = false; }

            bool Open(LPCWSTR name, DWORD userIndexBitset);
            void Close(void);

            bool Activate(bool active);
            bool OnInput(PCHAR report, UINT reportLength);
            bool UpdateState(void);

        private:
            void AddButton(USAGE usage, USHORT dataIndex, ULONG &buttonIndex);
            static void AddValue(USAGE usagePage, USAGE usage, USHORT dataIndex, LONG minimum, LONG maximum, Values &values);

            void OnButton(const HIDP_DATA &data, ULONG index, LONG minimum, LONG maximum);
            void OnValue(const HIDP_DATA &data, ULONG index, LONG minimum, LONG maximum);
            void OnHatSwitch(const HIDP_DATA &data, ULONG index, LONG minimum, LONG maximum);

            void UpdateStateFromXInput();
            void ResetButtonAndJoystickValues(void);
        };

        typedef ResourcePtr<HidDevice> HidDevicePtr;
        typedef std::vector<HidDevicePtr> Devices;

        #pragma endregion

    private:
        #if !ENABLE_RAW_INPUT_FOR_HID
        static GUID hidGuid;
        #endif

        Devices devices;
        int lastDeviceId;

    public:
        static void InputInitVKTable(void);

        RawInput(void);
        virtual ~RawInput(void);

        virtual bool Open(HWND window);
        virtual void Close(void);

        virtual bool GetJoystickNames(dynamic_array<core::string> &names);

        virtual bool Activate(bool active);

        virtual bool ToggleFullscreen(bool fullscreen, HWND window);

        virtual bool Process(bool discard);

        virtual LRESULT OnInput(HWND window, UINT message, WPARAM wParam, LPARAM lParam);
        virtual LRESULT OnDeviceChange(LPCWSTR name, bool add);

    protected:
        virtual bool UpdateState(void);

    private:
        static bool IsSupportedHidDevice(USHORT usagePage, USHORT usage);
        static bool IsKeyboardorDevice(USHORT usagePage, USHORT usage);
        static bool CompareHidDevicePtr(HidDevicePtr const& left, HidDevicePtr const& right);

        bool AddDevice(LPCWSTR name, bool reopen);
        bool OnMouse(const RAWINPUTHEADER &header, const RAWMOUSE &data);
    };
}
