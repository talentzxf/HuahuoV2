#include "UnityPrefix.h"
#include <windows.h>
#include "RawInput.h"
#include "XInputDevices.h"
#include "Runtime/Input/InputManager.h"
#include "PlatformDependent/Win/AutoPtr.h"
#include "PlatformDependent/Win/Handle.h"
#include "PlatformDependent/Win/WinUtils.h"
#include "PlatformDependent/Win/WinUnicode.h"

namespace win
{
    #pragma region HidDevice

    #define ZERO_HAT_SWITCH 0.0f
    #define HALF_HAT_SWITCH 0.707106781f
    #define FULL_HAT_SWITCH 1.0f

    const Vector2f RawInput::HidDevice::hatSwitchDirections[] =
    {
        Vector2f(ZERO_HAT_SWITCH, FULL_HAT_SWITCH),
        Vector2f(HALF_HAT_SWITCH, HALF_HAT_SWITCH),
        Vector2f(FULL_HAT_SWITCH, ZERO_HAT_SWITCH),
        Vector2f(HALF_HAT_SWITCH, -HALF_HAT_SWITCH),
        Vector2f(ZERO_HAT_SWITCH, -FULL_HAT_SWITCH),
        Vector2f(-HALF_HAT_SWITCH, -HALF_HAT_SWITCH),
        Vector2f(-FULL_HAT_SWITCH, ZERO_HAT_SWITCH),
        Vector2f(-HALF_HAT_SWITCH, HALF_HAT_SWITCH),
    };

    bool RawInput::HidDevice::Open(LPCWSTR name, DWORD userIndexBitset)
    {
        NTSTATUS status;
        HIDP_CAPS caps = {};

        // make sure device is closed

        this->Close();

        // store raw name

        this->rawName = name;

        // open device handle

        // In the Windows NT/Windows 2000 system, file handles cannot be opened on mice and keyboards.
        // However, all other HID devices will be available and recognized for testing purposes.
        HANDLE dFileHandle = CreateFileW(name, GENERIC_READ, (FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE), NULL, OPEN_EXISTING, 0, NULL);
        if (INVALID_HANDLE_VALUE == dFileHandle)
        {
            DWORD const dw = GetLastError();
            if (ERROR_ACCESS_DENIED != dw)
            {
                ErrorString(Format("<RI.Hid> Failed to create device file:\r\n %x %s", dw, winutils::ErrorCodeToMsg(dw).c_str()));
            }
            return false;
        }

        this->handle.Attach(dFileHandle);

        if (!this->handle.IsOpen())
        {
            #if ENABLE_RAW_INPUT_FOR_HID
            ErrorString(Format("<RI.Hid> Failed to open device handle:\r\n%s", WIN_LAST_ERROR_TEXT));
            #endif
            goto error;
        }

        // get preparsed data

        if (!HidD_GetPreparsedData(this->handle, &this->preparsedData))
        {
            // If the error code is INVALID_FUNCTION or INVALID_PARAMETER, just silently ignore this device as
            // it's not something we can work with (probably not a top-level HID collection device)
            DWORD const dw = GetLastError();
            if (ERROR_INVALID_FUNCTION != dw && ERROR_INVALID_PARAMETER != dw)
            {
                ErrorString(Format("<RI.Hid> Failed to get preparsed data: %s", WIN_LAST_ERROR_TEXT));
            }
            goto error;
        }

        // get device name

        WCHAR wideName[512];

        if (HidD_GetProductString(this->handle, wideName, _countof(wideName)))
        {
            ConvertWideToUTF8String(wideName, this->name);
        }

        // get device caps

        if (HIDP_STATUS_SUCCESS != (status = HidP_GetCaps(this->preparsedData, &caps)))
        {
            ErrorString(Format("<RI.Hid> Failed to get device caps (0x%.8x) for %s.", status, this->name.c_str()));
            goto error;
        }

        // we are only interested in joysticks and game pads
        if (RawInput::IsKeyboardorDevice(caps.UsagePage, caps.Usage))
        {
            #if ENABLE_RAW_INPUT_FOR_HID
            //LogString(Format("<RI.Hid> Keyboard has been attached (we will not manage it): %s.", this->name.c_str()));
            // The statement above has been commented out, because the less users know, the better they sleep.
            #endif

            // Even though we say "goto error;" here, it does not necessarily mean that a genuine
            // error has occured. Keyboards simply are ignored by the HID code and righly so -- they are managed
            // by the operating system and we don't want to specifically query them.
            goto error; // Ignore the device
        }

        if (!RawInput::IsSupportedHidDevice(caps.UsagePage, caps.Usage))
        {
            #if ENABLE_RAW_INPUT_FOR_HID
            // The statement below has been commented out, because it's not really an error when unsupported device is connected and it's annoying (case 607802).
            //LogString(Format("<RI.Hid> Device is not supported: %s (%.4d, 0x%.4x).", this->name.c_str(), caps.UsagePage, caps.Usage));
            #endif
            goto error;
        }

        // initialize input report buffer

        this->inputReport.resize(caps.InputReportByteLength);

        // buttons

        {
            // get button caps

            std::vector<HIDP_BUTTON_CAPS> buttonCaps;
            buttonCaps.resize(caps.NumberInputButtonCaps);

            USHORT buttonCapsLength = caps.NumberInputButtonCaps;
            ULONG buttonCount = 0;

            if (buttonCapsLength > 0)
            {
                if (HIDP_STATUS_SUCCESS != (status = HidP_GetButtonCaps(HidP_Input, &buttonCaps.front(), &buttonCapsLength, this->preparsedData)))
                {
                    ErrorString(Format("<RI.Hid> Failed to get button caps (0x%.8x) for %s.", status, this->name.c_str()));
                    goto error;
                }
            }

            // enumerate buttons
            buttonCaps.resize(buttonCapsLength);
            for (std::vector<HIDP_BUTTON_CAPS>::const_iterator it = buttonCaps.begin(); it != buttonCaps.end(); ++it)
            {
                if (it->IsRange)
                {
                    USAGE usage = it->Range.UsageMin;

                    for (USHORT dataIndex = it->Range.DataIndexMin; dataIndex <= it->Range.DataIndexMax; ++dataIndex)
                    {
                        this->AddButton(usage, dataIndex, buttonCount);

                        if (usage < it->Range.UsageMax)
                        {
                            ++usage;
                        }
                    }
                }
                else
                {
                    this->AddButton(it->NotRange.Usage, it->NotRange.DataIndex, buttonCount);
                }
            }

            // initialize button states

            if (kMaxJoyStickButtons > buttonCount)
            {
                buttonCount = kMaxJoyStickButtons;
            }

            this->newButtonStates.resize(buttonCount, false);
            this->oldButtonStates.resize(buttonCount, false);
        }

        // values

        {
            // get value caps

            std::vector<HIDP_VALUE_CAPS> valueCaps;
            valueCaps.resize(caps.NumberInputValueCaps);

            USHORT valueCapsLength = caps.NumberInputValueCaps;

            if (valueCapsLength > 0)
            {
                if (HIDP_STATUS_SUCCESS != (status = HidP_GetValueCaps(HidP_Input, &valueCaps.front(), &valueCapsLength, this->preparsedData)))
                {
                    ErrorString(Format("<RI.Hid> Failed to get value caps (0x%.8x) for %s.", status, this->name.c_str()));
                    goto error;
                }
            }

            // enumerate values

            Values values;
            values.reserve(kMaxJoyStickAxis);

            valueCaps.resize(valueCapsLength);
            for (std::vector<HIDP_VALUE_CAPS>::const_iterator it = valueCaps.begin(); it != valueCaps.end(); ++it)
            {
                if (it->IsRange)
                {
                    USAGE usage = it->Range.UsageMin;

                    for (USHORT dataIndex = it->Range.DataIndexMin; dataIndex <= it->Range.DataIndexMax; ++dataIndex)
                    {
                        AddValue(caps.UsagePage, usage, dataIndex, it->LogicalMin, it->LogicalMax, values);

                        if (usage < it->Range.UsageMax)
                        {
                            ++usage;
                        }
                    }
                }
                else
                {
                    AddValue(caps.UsagePage, it->NotRange.Usage, it->NotRange.DataIndex, it->LogicalMin, it->LogicalMax, values);
                }
            }

            // sort values

            stable_sort(values.begin(), values.end(), Value::Less);

            // add values to data map

            ULONG valueIndex = 0;

            for (Values::iterator it = values.begin(); it != values.end(); ++it)
            {
                it->AddData(valueIndex, this->dataMap);
            }

            // initialize hat switch states

            for (Values::size_type i = 0; i < values.size(); ++i)
            {
                if (Value::IsHatSwitch(values[i]))
                {
                    this->newHatSwitchStates.insert(HatSwitchStates::value_type(i, -1));
                    this->oldHatSwitchStates.insert(HatSwitchStates::value_type(i, -1));
                }
            }
        }

        // allocate data list

        UINT maxDataListLength = HidP_MaxDataListLength(HidP_Input, this->preparsedData);

        if (0 == maxDataListLength)
        {
            ErrorString("<RI.Hid> Failed to get maximum data list length.");
            goto error;
        }

        this->dataList.resize(maxDataListLength);

        // allocate joystick state

        JoystickState* newJoystickState = JoystickState::Create(this->id);
        if (NULL == (this->state = newJoystickState))
        {
            ErrorString("<RI.Hid> Out of memory.");
            goto error;
        }

        // select joystick type

        if (XInputDevices::IsXInputDevice(name))
        {
            DWORD userIndex;

            if (XInputDevices::GetUserIndex(userIndexBitset, userIndex))
            {
                newJoystickState->SetType(JoystickState::T_XINPUT_DEVICE, userIndex);
            }
        }

        // reopen device handle in asynchronous mode

        this->handle.Close();

        this->handle.Attach(CreateFileW(name, GENERIC_READ, (FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE), NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL));

        if (!this->handle.IsOpen())
        {
            ErrorString(Format("<RI.Hid> Failed to open asynchronous device handle:\r\n%s", WIN_LAST_ERROR_TEXT));
            goto error;
        }

        // start reading input report

        this->inputEvent.Attach(CreateEventW(NULL, FALSE, FALSE, NULL));

        if (!this->inputEvent.IsOpen())
        {
            ErrorString(Format("<RI.Hid> Failed to create input event for %s:\r\n%s", this->name.c_str(), WIN_LAST_ERROR_TEXT));
            goto error;
        }

        ZeroMemory(&this->inputOverlapped, sizeof(this->inputOverlapped));
        this->inputOverlapped.hEvent = this->inputEvent;

        if (!ReadFile(this->handle, &this->inputReport.front(), this->inputReport.size(), NULL, &this->inputOverlapped))
        {
            DWORD error = GetLastError();

            if (ERROR_IO_PENDING != error)
            {
                ErrorString(Format("<RI.Hid> Failed to start reading input report for %s:\r\n%s", this->name.c_str(), WIN_LAST_ERROR_TEXT));
                goto error;
            }
        }

        this->reading = true;

        // succeeded

        return true;

        // failed

    error:

        this->Close();

        return false;
    }

    void RawInput::HidDevice::Close(void)
    {
        // cancel io

        if (this->handle.IsOpen())
        {
            BOOL const result = CancelIo(this->handle);
            Assert(FALSE != result);
        }

        // not reading any more

        this->reading = false;

        // close input event handle

        this->inputEvent.Close();
        ZeroMemory(&this->inputOverlapped, sizeof(this->inputOverlapped));

        // clear states

        // Reset joystick and button values so they're not stuck with non-zero values
        this->ResetButtonAndJoystickValues();

        this->newButtonStates.clear();
        this->oldButtonStates.clear();

        this->newHatSwitchStates.clear();
        this->oldHatSwitchStates.clear();

        // free state

        this->state.Free();

        // clear name

        this->name.clear();

        // clear data map and list

        this->dataMap.clear();
        this->dataList.clear();

        // free preparsed data

        if (NULL != this->preparsedData)
        {
            HidD_FreePreparsedData(this->preparsedData);
            this->preparsedData = NULL;
        }

        // close handle

        this->handle.Close();

        // stuart@unity3d.com - refresh all the device state here to prevent ghost input devices
        // this fixes the win editor
#if UNITY_EDITOR
        XInputDevices::RefreshAllDevices();
#else
        // we poll based on this during the Process() method, fixes in-game
        this->latentConnectionPoll = true;
#endif
    }

    void RawInput::HidDevice::AddButton(USAGE usage, USHORT dataIndex, ULONG &buttonIndex)
    {
        Data data(&HidDevice::OnButton, buttonIndex++);
        this->dataMap.insert(DataMap::value_type(dataIndex, data));
    }

    void RawInput::HidDevice::AddValue(USAGE usagePage, USAGE usage, USHORT dataIndex, LONG minimum, LONG maximum, Values &values)
    {
        if (0x01 == usagePage)
        {
            switch (usage)
            {
                case 0x30: // X
                case 0x31: // Y
                case 0x32: // Z
                case 0x33: // Rx
                case 0x34: // Ry
                case 0x35: // Rz
                {
                    // it looks like most drivers doesn't bother setting corrent min/max range

                    minimum = 0x0000;
                    maximum &= 0xffff;

                    // add value

                    Value value(kValueTypeAxis, (usage - 0x30), dataIndex, minimum, maximum);
                    values.push_back(value);
                }
                break;

                case 0x36: // Slider
                case 0x37: // Dial
                case 0x38: // Wheel
                {
                    // only two sliders are supported

                    USHORT count = count_if(values.begin(), values.end(), Value::IsSlider);

                    if (count < 2)
                    {
                        // it looks like most drivers doesn't bother setting corrent min/max range

                        minimum = 0x0000;
                        maximum &= 0xffff;

                        // add value

                        Value value(kValueTypeSlider, count, dataIndex, minimum, maximum);
                        values.push_back(value);
                    }
                }
                break;

                case 0x39: // Hat switch
                {
                    // only four hat switches are supported

                    USHORT count = count_if(values.begin(), values.end(), Value::IsHatSwitch);

                    if (count < 4)
                    {
                        // make sure hat switch has expected number of directions

                        if (_countof(hatSwitchDirections) == (maximum - minimum + 1))
                        {
                            // add value

                            Value value(kValueTypeHatSwitch, count, dataIndex, minimum, maximum);
                            values.push_back(value);
                        }
                    }
                }
                break;
            }
        }
        else if (0x02 == usagePage)
        {
            switch (usage)
            {
                case 0xba: // Rudder
                {
                    // it looks like most drivers doesn't bother setting corrent min/max range

                    minimum = 0x0000;
                    maximum &= 0xffff;

                    // add value

                    Value value(kValueTypeAxis, 2, dataIndex, minimum, maximum);
                    values.push_back(value);
                }
                break;

                case 0xbb: // Throttle
                {
                    // only two sliders are supported

                    USHORT count = count_if(values.begin(), values.end(), Value::IsSlider);

                    if (count < 2)
                    {
                        // it looks like most drivers doesn't bother setting corrent min/max range

                        minimum = 0x0000;
                        maximum &= 0xffff;

                        // add value

                        Value value(kValueTypeSlider, count, dataIndex, minimum, maximum);
                        values.push_back(value);
                    }
                }
                break;
            }
        }
    }

    bool RawInput::HidDevice::Activate(bool active)
    {
        if (!active && this->GetConnected())
        {
            // reset button states

            for (ButtonStates::size_type i = 0; i < this->newButtonStates.size(); ++i)
            {
                this->newButtonStates[i] = false;
                this->oldButtonStates[i] = false;
            }

            // reset hat switch states

            for (HatSwitchStates::iterator it = this->newHatSwitchStates.begin(); it != this->newHatSwitchStates.end(); ++it)
            {
                it->second = -1;
            }

            for (HatSwitchStates::iterator it = this->oldHatSwitchStates.begin(); it != this->oldHatSwitchStates.end(); ++it)
            {
                it->second = -1;
            }
        }

        return true;
    }

    bool RawInput::HidDevice::OnInput(PCHAR report, UINT reportLength)
    {
        NTSTATUS status;

        // get report data

        ULONG dataLength = this->dataList.capacity();
        bool isXInputDevice = this->state->GetType() == JoystickState::T_XINPUT_DEVICE;

        // For XInput devices completely ignore HID

        if (!isXInputDevice && HIDP_STATUS_SUCCESS != (status = HidP_GetData(HidP_Input, &this->dataList.front(), &dataLength, this->preparsedData, report, reportLength)))
        {
            ErrorString(Format("<RI.Hid> Failed to get report data (0x%.8x).", status));
            return false;
        }

        // reset button states

        for (ButtonStates::size_type i = 0; i < this->newButtonStates.size(); ++i)
        {
            this->oldButtonStates[i] = this->newButtonStates[i];
            this->newButtonStates[i] = false;
        }

        // reset hat switch states

        for (HatSwitchStates::iterator nit = this->newHatSwitchStates.begin(); nit != this->newHatSwitchStates.end(); ++nit)
        {
            HatSwitchStates::iterator oit = this->oldHatSwitchStates.find(nit->first);

            if (this->oldHatSwitchStates.end() != oit)
            {
                oit->second = nit->second;
            }

            nit->second = -1;
        }


        // XInput device handling
        // Note: we must read the Left/Right trigger data from XInput (HID doesn't report it properly) but we cannot
        // definitively map the HID device to XInput's dwUserIndex; if multiple gamepads connected the UserIndex might not
        // be properly mapped to this HIDDevice. Therefore, we must read the entire input state from XInput and skip HIDReport.
        if (isXInputDevice)
        {
            this->UpdateStateFromXInput();
        }
        else
        {
            // handle all data from the report
            for (DataList::const_iterator data = this->dataList.begin(); data < (this->dataList.begin() + dataLength); ++data)
            {
                DataMap::const_iterator dit = this->dataMap.find(data->DataIndex);

                if (this->dataMap.end() != dit)
                {
                    dit->second.Invoke(this, *data);
                }
            }
        }

        // update input state if any of the buttons have been pressed or released

        for (ButtonStates::size_type i = 0; i < this->newButtonStates.size(); ++i)
        {
            bool state = this->newButtonStates[i];

            if (state != this->oldButtonStates[i])
            {
                this->state->AddButton(i, state);
            }
        }

        // update input state if hat switch state has changed

        for (HatSwitchStates::iterator nit = this->newHatSwitchStates.begin(); nit != this->newHatSwitchStates.end(); ++nit)
        {
            HatSwitchStates::iterator oit = this->oldHatSwitchStates.find(nit->first);

            if (this->oldHatSwitchStates.end() != oit)
            {
                if (nit->second != oit->second)
                {
                    Vector2f direction;

                    if ((nit->second >= 0) && (nit->second < _countof(hatSwitchDirections)))
                    {
                        direction = hatSwitchDirections[nit->second];
                    }
                    else
                    {
                        direction = Vector2f::zero;
                    }

                    this->state->SetAxis((nit->first + 0), direction.x);
                    this->state->SetAxis((nit->first + 1), direction.y);
                }
            }
        }

        // done

        return true;
    }

    bool RawInput::HidDevice::UpdateState(void)
    {
        // ignore if device is disconnected

        if (!this->GetConnected())
        {
            return true;
        }

        // For XInput devices, we only read from XInput APIs and ignore HID altogether
        //
        // NOTE: If multiple XInput devices are connected we cannot be sure the UserIndex is actually correct
        // for this device, because API doesn't allow mapping between hardware ID and UserIndex. This means,
        // when a HID event is triggered for this device, we might end up reading from the wrong UserIndex
        // (if it's incorrectly mapped); the only safe solution is update every XInput device every frame.

        if (this->state->GetType() == JoystickState::T_XINPUT_DEVICE)
        {
            return this->OnInput(&this->inputReport.front(), this->inputReport.size());
        }

        // handle all input reports for regular HID devices

        for (int i = 0; i < 1000; ++i)  // better of two evils - delayed input vs infinite loop
        {
            // check if input report has arrived

            DWORD read = 0;

            if (this->reading)
            {
                if (!GetOverlappedResult(this->handle, &this->inputOverlapped, &read, FALSE))
                {
                    DWORD error = GetLastError();

                    if (ERROR_IO_INCOMPLETE == error)
                    {
                        break;
                    }
                    else if (ERROR_DEVICE_NOT_CONNECTED != error)
                    {
                        ErrorString(Format("<RI.Hid> Failed to read input report:\r\n%s", WIN_LAST_ERROR_TEXT));
                    }
                }

                this->reading = false;
            }

            // handle input report

            if (read == this->inputReport.size())
            {
                this->OnInput(&this->inputReport.front(), this->inputReport.size());
            }

            // start reading new input report

            ZeroMemory(&this->inputOverlapped, sizeof(this->inputOverlapped));
            this->inputOverlapped.hEvent = this->inputEvent;

            if (!ReadFile(this->handle, &this->inputReport.front(), this->inputReport.size(), NULL, &this->inputOverlapped))
            {
                DWORD error = GetLastError();

                if (ERROR_IO_PENDING != error)
                {
                    if (ERROR_DEVICE_NOT_CONNECTED == error)
                    {
                        core::string const name = this->GetName();
                        this->Close();
                        WarningStringMsg("Joystick disconnected (\"%s\").", name.c_str());
                        return false;
                    }
                    else
                    {
                        ErrorString(Format("<RI.Hid> Failed to start reading input report:\r\n%s", WIN_LAST_ERROR_TEXT));
                        return false;
                    }
                }

                this->reading = true;

                break;
            }

            this->reading = true;
        }

        // done

        return true;
    }

    void RawInput::HidDevice::OnButton(const HIDP_DATA &data, ULONG index, LONG minimum, LONG maximum)
    {
        if (index < this->newButtonStates.size())
        {
            this->newButtonStates[index] = data.On;
        }
    }

    void RawInput::HidDevice::OnValue(const HIDP_DATA &data, ULONG index, LONG minimum, LONG maximum)
    {
        LONG range = (maximum - minimum);
        float value = ((((data.RawValue - minimum) * 2LL) - range) / static_cast<float>(range));
        this->state->SetAxis(index, value);
    }

    void RawInput::HidDevice::OnHatSwitch(const HIDP_DATA &data, ULONG index, LONG minimum, LONG maximum)
    {
        HatSwitchStates::iterator it = this->newHatSwitchStates.find(index);

        if (this->newHatSwitchStates.end() != it)
        {
            it->second = (data.RawValue - minimum);
        }
    }

    void RawInput::HidDevice::UpdateStateFromXInput()
    {
        DebugAssertMsg(this->state->GetType() == JoystickState::T_XINPUT_DEVICE, "HID device isn't an XInput controller");

        XINPUT_STATE inputState = { 0 };
        if (XInputDevices::GetInputState(this->state->GetTypeData(), &inputState) != ERROR_SUCCESS)
            return;

        // Set newButtonStates according to XInput button bit-flags
        WORD& buttons = inputState.Gamepad.wButtons;
        this->newButtonStates[0] = buttons & XINPUT_GAMEPAD_A;
        this->newButtonStates[1] = buttons & XINPUT_GAMEPAD_B;
        this->newButtonStates[2] = buttons & XINPUT_GAMEPAD_X;
        this->newButtonStates[3] = buttons & XINPUT_GAMEPAD_Y;
        this->newButtonStates[4] = buttons & XINPUT_GAMEPAD_LEFT_SHOULDER;
        this->newButtonStates[5] = buttons & XINPUT_GAMEPAD_RIGHT_SHOULDER;
        this->newButtonStates[6] = buttons & XINPUT_GAMEPAD_BACK;
        this->newButtonStates[7] = buttons & XINPUT_GAMEPAD_START;
        this->newButtonStates[8] = buttons & XINPUT_GAMEPAD_LEFT_THUMB;
        this->newButtonStates[9] = buttons & XINPUT_GAMEPAD_RIGHT_THUMB;

        // Convert Dpad button flags into HatSwitch direction
        // NOTE: Xbox gamepad only has single DPad control; simple select first item from newHatSwtichStates
        HatSwitchStates::iterator firstHat = this->newHatSwitchStates.begin();
        if (firstHat != this->newHatSwitchStates.end())
        {
            LONG& hatDirection = firstHat->second;
            if ((buttons & XINPUT_GAMEPAD_DPAD_UP) && (buttons & XINPUT_GAMEPAD_DPAD_RIGHT))
            {
                hatDirection = 1;
            }
            else if ((buttons & XINPUT_GAMEPAD_DPAD_RIGHT) && (buttons & XINPUT_GAMEPAD_DPAD_DOWN))
            {
                hatDirection = 3;
            }
            else if ((buttons & XINPUT_GAMEPAD_DPAD_DOWN) && (buttons & XINPUT_GAMEPAD_DPAD_LEFT))
            {
                hatDirection = 5;
            }
            else if ((buttons & XINPUT_GAMEPAD_DPAD_LEFT) && (buttons & XINPUT_GAMEPAD_DPAD_UP))
            {
                hatDirection = 7;
            }
            else if (buttons & XINPUT_GAMEPAD_DPAD_UP)
            {
                hatDirection = 0;
            }
            else if (buttons & XINPUT_GAMEPAD_DPAD_RIGHT)
            {
                hatDirection = 2;
            }
            else if (buttons & XINPUT_GAMEPAD_DPAD_DOWN)
            {
                hatDirection = 4;
            }
            else if (buttons & XINPUT_GAMEPAD_DPAD_LEFT)
            {
                hatDirection = 6;
            }
        }

        // Hard code mapping from Xbox thumbsticks/triggers to their respective Joystick Axis indeces
        // NOTE: Thumbstick range is signed 16-bits; Y-Axis is inverted in Unity's coordinate space
        this->state->SetAxis(0, static_cast<float>(inputState.Gamepad.sThumbLX) / 32767.0);
        this->state->SetAxis(1, -static_cast<float>(inputState.Gamepad.sThumbLY) / 32767.0);
        this->state->SetAxis(3, static_cast<float>(inputState.Gamepad.sThumbRX) / 32767.0);
        this->state->SetAxis(4, -static_cast<float>(inputState.Gamepad.sThumbRY) / 32767.0);

        // Combine Left/Right triggers to "Axis 3" for legacy support
        this->state->SetAxis(2, static_cast<float>(inputState.Gamepad.bRightTrigger - inputState.Gamepad.bLeftTrigger) / 255.0f);
        this->state->SetAxis(8, static_cast<float>(inputState.Gamepad.bLeftTrigger) / 255.0f);
        this->state->SetAxis(9, static_cast<float>(inputState.Gamepad.bRightTrigger) / 255.0f);
    }

    #pragma endregion

    #if !ENABLE_RAW_INPUT_FOR_HID

namespace
{
    inline GUID GetHidGuid()
    {
        GUID value;
        HidD_GetHidGuid(&value);
        return value;
    }
}

    GUID RawInput::hidGuid = GetHidGuid();

    #endif

    bool RawInput::IsSupportedHidDevice(USHORT usagePage, USHORT usage)
    {
        if (0x01 == usagePage)  // generic desktop controls
        {
            if ((0x04 == usage) ||  // joystick
                (0x05 == usage))    // game pad
            {
                return true;
            }
        }
        else if (0x02 == usagePage) // simulation controls
        {
            return true;
        }

        return false;
    }

    bool RawInput::IsKeyboardorDevice(USHORT usagePage, USHORT usage)
    {
        return (0x01 == usagePage &&        // generic desktop controls
            0x80 == usage) ||               // System Control
            (0x0c == usagePage &&           // Consumer
                0x01 == usage);             // Consumer Control
    }

    RawInput::RawInput(void) :
        lastDeviceId(0)
    {
        // dummy constructor
    }

    RawInput::~RawInput(void)
    {
        this->Close();
    }

    bool RawInput::Open(HWND window)
    {
        // invoke base class

        if (!__super::Open(window))
        {
            return false;
        }

        #if ENABLE_RAW_INPUT_FOR_HID

        typedef std::vector<RAWINPUTDEVICELIST, STL_ALLOCATOR_ALIGNED(kMemSTL, RAWINPUTDEVICELIST, sizeof(RAWINPUTDEVICELIST))> DeviceVector;
        DeviceVector devices;

        // get device count

        UINT deviceCount = 0;

        if (static_cast<UINT>(-1) == GetRawInputDeviceList(NULL, &deviceCount, sizeof(RAWINPUTDEVICELIST)))
        {
            ErrorString(Format("<RI> Failed to get device count:\r\n%s", WIN_LAST_ERROR_TEXT));
            goto error;
        }

        // initialize XInput

        XInputDevices::Initialize();

        // get devices

        for (UINT neededDeviceCount = deviceCount;;)
        {
            devices.resize(neededDeviceCount);

            if (static_cast<UINT>(-1) != (deviceCount = GetRawInputDeviceList(&devices.front(), &neededDeviceCount, sizeof(RAWINPUTDEVICELIST))))
            {
                break;
            }
            else if (ERROR_INSUFFICIENT_BUFFER != GetLastError())
            {
                ErrorString(Format("<RI> Failed to get devices:\r\n%s", WIN_LAST_ERROR_TEXT));
                goto error;
            }
        }

        // enumerate devices

        for (UINT i = 0; i < deviceCount; ++i)
        {
            RAWINPUTDEVICELIST const& device = devices[i];

            // filter out mice and keyboards

            if (RIM_TYPEHID == device.dwType)
            {
                // get device info

                RID_DEVICE_INFO deviceInfo;
                deviceInfo.cbSize = sizeof(deviceInfo);

                UINT deviceInfoSize = sizeof(deviceInfo);

                if (static_cast<UINT>(-1) != GetRawInputDeviceInfoW(device.hDevice, RIDI_DEVICEINFO, &deviceInfo, &deviceInfoSize))
                {
                    // we are only interested in joysticks and game pads

                    if (IsSupportedHidDevice(deviceInfo.hid.usUsagePage, deviceInfo.hid.usUsage))
                    {
                        // get device name

                        WCHAR deviceName[1024] = {};
                        UINT deviceNameSize = _countof(deviceName);

                        if (static_cast<UINT>(-1) != GetRawInputDeviceInfoW(device.hDevice, RIDI_DEVICENAME, deviceName, &deviceNameSize))
                        {
                            this->AddDevice(deviceName, false);
                        }
                        else
                        {
                            WarningString(Format("<RI> Failed to get device name:\r\n%s", WIN_LAST_ERROR_TEXT));
                        }
                    }
                }
                else
                {
                    WarningString(Format("<RI> Failed to get device info:\r\n%s", WIN_LAST_ERROR_TEXT));
                }
            }
        }

        #else

        // get devices

        HDEVINFO const deviceInfoSet = SetupDiGetClassDevsW(&hidGuid, NULL, NULL, (DIGCF_DEVICEINTERFACE | DIGCF_PRESENT));
        Assert(INVALID_HANDLE_VALUE != deviceInfoSet);

        if (INVALID_HANDLE_VALUE == deviceInfoSet)
        {
            ErrorString(Format("<RI> Failed to get devices:\r\n%s", WIN_LAST_ERROR_TEXT));
            goto error;
        }

        // enumerate devices

        for (DWORD index = 0;; ++index)
        {
            // get device

            SP_DEVICE_INTERFACE_DATA deviceInterfaceData;
            deviceInterfaceData.cbSize = sizeof(deviceInterfaceData);

            if (FALSE == SetupDiEnumDeviceInterfaces(deviceInfoSet, NULL, &hidGuid, index, &deviceInterfaceData))
            {
                DWORD const error = GetLastError();
                Assert(ERROR_NO_MORE_ITEMS == error);

                if (ERROR_NO_MORE_ITEMS != error)
                {
                    WarningString(Format("<RI> Failed to get device:\r\n%s", WIN_ERROR_TEXT(error)));
                }

                break;
            }

            // get device id

            DWORD const deviceInterfaceDetailDataSize = (offsetof(SP_DEVICE_INTERFACE_DETAIL_DATA_W, DevicePath) + (1024 * sizeof(WCHAR)));
            BYTE buffer[deviceInterfaceDetailDataSize];

            PSP_DEVICE_INTERFACE_DETAIL_DATA_W deviceInterfaceDetailData = reinterpret_cast<PSP_DEVICE_INTERFACE_DETAIL_DATA_W>(buffer);
            deviceInterfaceDetailData->cbSize = sizeof(*deviceInterfaceDetailData);

            BOOL const getDeviceInterfaceDetailResult = SetupDiGetDeviceInterfaceDetailW(deviceInfoSet, &deviceInterfaceData, deviceInterfaceDetailData, deviceInterfaceDetailDataSize, NULL, NULL);
            Assert(FALSE != getDeviceInterfaceDetailResult);

            if (FALSE != getDeviceInterfaceDetailResult)
            {
                this->AddDevice(deviceInterfaceDetailData->DevicePath, false);
            }
            else
            {
                WarningString(Format("<RI> Failed to get device id:\r\n%s", WIN_LAST_ERROR_TEXT));
            }
        }

        // destroy devices

        if (INVALID_HANDLE_VALUE != deviceInfoSet)
        {
            BOOL const destroyDeviceInfoSetResult = SetupDiDestroyDeviceInfoList(deviceInfoSet);
            Assert(FALSE != destroyDeviceInfoSetResult);
        }

        #endif

        // register devices

        RAWINPUTDEVICE rawInputDevices[] =
        {
            { 0x01, 0x02, NULL, window },    // mouse
        };

        if (!RegisterRawInputDevices(rawInputDevices, ARRAYSIZE(rawInputDevices), sizeof(RAWINPUTDEVICE)))
        {
            ErrorString(Format("<RI> Failed to register devices:\r\n%s", WIN_LAST_ERROR_TEXT));
            goto error;
        }

        // succeeded

        printf_console("<RI> Input initialized.\r\n");

        return true;

        // failed

    error:

        this->Close();

        return false;
    }

    void RawInput::Close(void)
    {
        // clear devices

        XInputDevices::Shutdown();
        this->joysticks.clear();
        this->devices.clear();

        // invoke base class

        __super::Close();
    }

    bool RawInput::CompareHidDevicePtr(HidDevicePtr const& left, HidDevicePtr const& right)
    {
        if ((NULL == left) || (NULL == right))
        {
            return (NULL == right);
        }

        return (left->GetId() <= right->GetId());
    }

    bool RawInput::GetJoystickNames(dynamic_array<core::string>& names)
    {
        if (!__super::GetJoystickNames(names))
        {
            return false;
        }

        int maxId = -1;

        for (Devices::const_iterator it = devices.begin(); it != devices.end(); ++it)
        {
            maxId = std::max(maxId, (*it)->GetId());
        }

        names.resize_initialized(maxId + 1);

        for (std::vector<HidDevicePtr>::const_iterator it = devices.begin(); it != devices.end(); ++it)
        {
            if ((*it)->GetConnected())
                names[(*it)->GetId()] = (*it)->GetName();
        }

        return true;
    }

    bool RawInput::Activate(bool active)
    {
        bool result = true;

        // if active invoke base class now

        if (active)
        {
            result &= __super::Activate(active);
        }

        // notify devices

        for (Devices::iterator it = this->devices.begin(); it != this->devices.end(); ++it)
        {
            result &= (*it)->Activate(active);
        }

        // if not active invoke base class now

        if (!active)
        {
            result &= __super::Activate(active);
        }

        // done

        return result;
    }

    bool RawInput::ToggleFullscreen(bool fullscreen, HWND window)
    {
        if (!__super::ToggleFullscreen(fullscreen, window))
        {
            return false;
        }

        RAWINPUTDEVICE devices[] =
        {
            { 0x01, 0x02, NULL, window },    // mouse
        };

        if (!RegisterRawInputDevices(devices, _countof(devices), sizeof(RAWINPUTDEVICE)))
        {
            ErrorString(Format("<RI> Failed to register devices:\r\n%s", WIN_LAST_ERROR_TEXT));
            return false;
        }

        return true;
    }

    bool RawInput::Process(bool discard)
    {
        if (!__super::Process(discard))
        {
            return false;
        }

        // stuart@unity3d.com - we poll recently disconnected xinput device state each frame to ensure
        // the OS/drivers registers it; if we don't do this we get ghost devices sticking around (fixes win standalone)
        for (JoystickStates::const_iterator state = this->joysticks.begin(); state != this->joysticks.end(); ++state)
        {
            if (state->second->GetType() == JoystickState::T_XINPUT_DEVICE)
            {
                XINPUT_STATE fakeState;
                for (Devices::iterator device = this->devices.begin(); device != this->devices.end(); ++device)
                {
                    if ((*device)->GetId() == state->second->GetId() && (*device)->GetLatentConnectionPoll() == true)
                    {
                        DWORD result = XInputDevices::GetInputState(state->second->GetTypeData(), &fakeState);

                        if (result == ERROR_DEVICE_NOT_CONNECTED)
                        {
                            (*device)->StopLatentConnectionPoll();
                        }
                    }
                }
            }
        }

        return true;
    }

    bool RawInput::UpdateState(void)
    {
        bool result = __super::UpdateState();

        for (Devices::iterator it = this->devices.begin(); it != this->devices.end(); ++it)
        {
            result &= (*it)->UpdateState();
        }

        return result;
    }

    LRESULT RawInput::OnInput(HWND window, UINT message, WPARAM wParam, LPARAM lParam)
    {
        // get input data

        BYTE staticBuffer[1024];
        AutoVectorPtr<BYTE> dynamicBuffer;

        PRAWINPUT input = reinterpret_cast<PRAWINPUT>(staticBuffer);
        UINT size = sizeof(staticBuffer);

        for (int i = 0; i < 1000; ++i)  // better of two evils - lost input vs infinite loop
        {
            if (-1 != GetRawInputData(reinterpret_cast<HRAWINPUT>(lParam), RID_INPUT, input, &size, sizeof(RAWINPUTHEADER)))
            {
                // only mouse events seems to be delivered when running in low integrity process

                if (RIM_TYPEMOUSE == input->header.dwType)
                {
                    this->OnMouse(input->header, input->data.mouse);
                }

                break;
            }
            else
            {
                DWORD error = GetLastError();

                if (ERROR_INSUFFICIENT_BUFFER != error)
                {
                    ErrorString(Format("<RI> Failed to get input data:\r\n%s", WIN_LAST_ERROR_TEXT));
                    break;
                }

                if (!dynamicBuffer.Allocate(size))
                {
                    ErrorString("<RI> Out of memory.");
                    break;
                }

                input = reinterpret_cast<PRAWINPUT>(dynamicBuffer.GetData());
            }
        }

        // invoke base class

        return __super::OnInput(window, message, wParam, lParam);
    }

    bool RawInput::OnMouse(const RAWINPUTHEADER &header, const RAWMOUSE &data)
    {
        // update delta

        if (MOUSE_MOVE_RELATIVE == data.usFlags)
        {
            this->mouse.AddDeltaX(data.lLastX);
            this->mouse.AddDeltaY(data.lLastY);
        }

        // update wheel

        if (data.usButtonFlags & RI_MOUSE_WHEEL)
        {
            this->mouse.AddDeltaW(static_cast<SHORT>(data.usButtonData));
        }

        #define RI_MOUSE_HORIZONTAL_WHEEL 0x0800
        if (data.usButtonFlags & RI_MOUSE_HORIZONTAL_WHEEL)
        {
            this->mouse.AddDeltaZ(static_cast<SHORT>(data.usButtonData));
        }
        // done

        return true;
    }

namespace
{
    template<size_t size>
    inline void FixDeviceName(LPCWSTR source, WCHAR(&destination)[size])
    {
        // hack. windows xp sometimes returns device name starting with \?? instead of \\?

        wcscpy_s(destination, source);

        if (0 == wcsncmp(destination, L"\\??", 3))
        {
            destination[1] = L'\\';
        }
    }
}

    LRESULT RawInput::OnDeviceChange(LPCWSTR name, bool add)
    {
        if (add)
        {
            this->AddDevice(name, true);
        }
        else
        {
            // fix device name

            WCHAR deviceName[1024];
            FixDeviceName(name, deviceName);

            // find device and close it

            for (Devices::iterator it = this->devices.begin(); it != this->devices.end(); ++it)
            {
                if (0 == _wcsicmp(deviceName, (*it)->GetRawName().c_str()))
                {
                    if ((*it)->GetConnected())
                    {
                        core::string const name = (*it)->GetName();
                        (*it)->Close();
                        WarningStringMsg("Joystick disconnected (\"%s\").", name.c_str());
                    }

                    break;
                }
            }
        }

        return TRUE;
    }

    bool RawInput::AddDevice(LPCWSTR name, bool reopen)
    {
        // fix device name

        WCHAR deviceName[1024];
        FixDeviceName(name, deviceName);

        // build used xinput device bitset

        DWORD userIndexBitset = 0;

        for (JoystickStates::const_iterator state = this->joysticks.begin(); state != this->joysticks.end(); ++state)
        {
            if (state->second->GetType() == JoystickState::T_XINPUT_DEVICE)
            {
                // ignore disconnected joysticks

                bool connected = true;

                for (Devices::const_iterator device = this->devices.begin(); device != this->devices.end(); ++device)
                {
                    if ((*device)->GetId() == state->second->GetId())
                    {
                        connected = (*device)->GetConnected();
                        break;
                    }
                }

                if (connected)
                {
                    DWORD const index = state->second->GetTypeData();
                    Assert(index < 4);

                    Assert(0 == (userIndexBitset & (1 << index)));
                    userIndexBitset |= (1 << index);
                }
            }
        }

        // find device and reopen it

        if (reopen)
        {
            for (Devices::iterator it = this->devices.begin(); it != this->devices.end(); ++it)
            {
                if (0 == _wcsicmp(deviceName, (*it)->GetRawName().c_str()))
                {
                    if (!(*it)->Open(deviceName, userIndexBitset))
                    {
                        return false;
                    }

                    this->joysticks[(*it)->GetId()] = (*it)->GetState();

                    LogStringMsg("Joystick reconnected (\"%s\").", (*it)->GetName().c_str());

                    return true;
                }
            }
        }

        // create device otherwise

        HidDevicePtr device = HidDevice::Create(this->lastDeviceId);

        if (NULL != device)
        {
            // open device

            if (device->Open(deviceName, userIndexBitset))
            {
                // store device

                this->devices.push_back(device);
                this->joysticks.insert(JoystickStates::value_type(device->GetId(), device->GetState()));

                // increment id for next device

                ++this->lastDeviceId;

                if (reopen)
                {
                    LogStringMsg("Joystick connected (\"%s\").", device->GetName().c_str());
                }
            }
        }
        else
        {
            WarningString("<RI> Out of memory.");
        }

        // done

        return true;
    }

    void RawInput::HidDevice::ResetButtonAndJoystickValues(void)
    {
        int buttonIndex = 0;
        for (auto& buttonState : this->newButtonStates)
        {
            buttonState = false;

            // In addition to clearing newbuttonStates we must also add a "button changed" event
            // so the new state value is applied to InputManager
            this->state->AddButton(buttonIndex++, false);
        }

        for (auto& hatState : this->newHatSwitchStates)
        {
            hatState.second = -1;
        }

        if (this->state != nullptr)
        {
            for (int i = 0; i < kMaxJoyStickAxis; i++)
            {
                this->state->SetAxis(i, 0);
            }
        }
    }
}
