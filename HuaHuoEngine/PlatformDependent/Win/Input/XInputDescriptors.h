#pragma once

#include "Runtime/Serialize/SerializeUtility.h"

namespace win
{
// Shared between Win32 and UWP (Metro)
namespace shared
{
    struct XInputGamepadCapabilities
    {
        DEFINE_GET_TYPESTRING(XInputGamepadCapabilities);

        int buttons;
        int leftTrigger;
        int rightTrigger;
        int leftStickX;
        int leftStickY;
        int rightStickX;
        int rightStickY;

        template<typename TransferFunc>
        inline void Transfer(TransferFunc& transfer)
        {
            TRANSFER(buttons);
            TRANSFER(leftTrigger);
            TRANSFER(rightTrigger);
            TRANSFER(leftStickX);
            TRANSFER(leftStickY);
            TRANSFER(rightStickX);
            TRANSFER(rightStickY);
        }
    };

    struct XInputVibrationCapabilities
    {
        DEFINE_GET_TYPESTRING(XInputVibrationCapabilities);

        int leftMotor;
        int rightMotor;

        template<typename TransferFunc>
        inline void Transfer(TransferFunc& transfer)
        {
            TRANSFER(leftMotor);
            TRANSFER(rightMotor);
        }
    };

    struct XInputDeviceCapabilities
    {
        int userIndex;
        int type;
        int subType;
        int flags;
        XInputGamepadCapabilities gamepad;
        XInputVibrationCapabilities vibration;

        template<typename TransferFunc>
        inline void Transfer(TransferFunc& transfer)
        {
            TRANSFER(userIndex);
            TRANSFER(type);
            TRANSFER(subType);
            TRANSFER(flags);
            TRANSFER(gamepad);
            TRANSFER(vibration);
        }
    };

    enum { kXInputMaxControllers = 4 };
} // namespace shared
} // namespace win
