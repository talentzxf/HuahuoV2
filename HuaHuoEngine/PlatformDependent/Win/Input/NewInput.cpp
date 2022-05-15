// Windows input device I/O for the new input system.
// - Converts Windows input messages to input events
// - Collects input from all HIDs and allows sending output to them
// - Mouse and keyboard are picked up through Raw Input
// - Pen input is picked up through WM_POINTER (Win8+)
// - Touch input is picked up through WM_TOUCH (Win7+)
// - Everything else is picked up through HID
// - Suppresses XInput HIDs and picks them up through XInput

#include "UnityPrefix.h"

#include <windows.h>
#include <windowsx.h>
#include <setupapi.h>
#include <Cfgmgr32.h>

#include "NewInput.h"
#if UNITY_EDITOR
#include "Editor/Src/Application/Application.h"
#endif
#include "Runtime/Core/Callbacks/GlobalCallbacks.h"
#include "Runtime/Input/InputManager.h"
#include "Runtime/Input/TimeManager.h"
#include "Runtime/Graphics/ScreenManager.h"
#include "Runtime/Misc/Player.h"
#include "Runtime/Testing/Faking.h"
#include "Runtime/Threads/Thread.h"
#include "Runtime/Threads/Semaphore.h"
#include "Runtime/Utilities/dynamic_array.h"
#include "Runtime/Utilities/RuntimeStatic.h"
#include "Runtime/Profiler/TimeHelper.h"
#include "Modules/Input/InputSystem.h"
#include "Modules/Input/Private/InputInternal.h"
#include "PlatformDependent/Win/Handle.h"
#include "PlatformDependent/Win/WinUnicode.h"
#include "RawInputHid.h"
#include "WinTouch.h"

#if UNITY_EDITOR
// For EditorWindow coordinate space conversion.
#include "Editor/Platform/Interface/GUIView.h"
#include "Editor/Platform/Interface/RepaintController.h"
#include "Editor/Platform/Interface/HighDpi.h"
#endif

#include "xinput.h"
#include "PlatformDependent/Win/Input/XInputDescriptors.h"

namespace win
{
    //IME dialogs are centered vertically, but we want to create a small offset to make sure the cursor aligns to the corner of the Dialog.  This is 30 pixels
#ifndef kIMETextMenuOffset
#define kIMETextMenuOffset 30
#endif

// Defines missing for player builds.
#ifndef SM_MOUSEHORIZONTALWHEELPRESENT
#define SM_MOUSEHORIZONTALWHEELPRESENT 91
#endif
#ifndef SM_MAXIMUMTOUCHES
#define SM_MAXIMUMTOUCHES 95
#endif
#ifndef NID_INTEGRATED_TOUCH
#define NID_INTEGRATED_TOUCH 0x01
#endif
#ifndef NID_EXTERNAL_TOUCH
#define NID_EXTERNAL_TOUCH 0x02
#endif

    static double g_MessageTimeToRealtime = 0;
    static DWORD g_StartTickCount = 0;

    static void InitWindowsEventTime()
    {
        g_StartTickCount = GetTickCount();
        g_MessageTimeToRealtime = (double)g_StartTickCount / Baselib_MillisecondsPerSecond - GetTimeSinceStartup();
    }

    double GetCurrentEventTimeInUnityTime()
    {
        DWORD messageTime = GetMessageTime();
        // TickCount time wrapped around (happens all 48 days of uptime. Reinit time)
        if (g_StartTickCount > messageTime)
            InitWindowsEventTime();
        return (double)messageTime / Baselib_MillisecondsPerSecond - g_MessageTimeToRealtime;
    }

    #pragma region Windows 8 Pointer API
    // For pen input, we use APIs only available from Windows 8 onward. Look them up dynamically
    // so we can still run on older systems.
#ifndef WM_POINTERDOWN
#define WM_POINTERDOWN                   0x0246
#endif
#ifndef WM_POINTERUP
#define WM_POINTERUP                     0x0247
#endif
#ifndef WM_POINTERENTER
#define WM_POINTERENTER                  0x0249
#endif
#ifndef WM_POINTERLEAVE
#define WM_POINTERLEAVE                  0x024A
#endif
#ifndef WM_POINTERUPDATE
#define WM_POINTERUPDATE                 0x0245
#endif
#ifndef WM_POINTERDEVICEINRANGE
#define WM_POINTERDEVICEINRANGE          0X239
#endif
#ifndef WM_POINTERDEVICEOUTOFRANGE
#define WM_POINTERDEVICEOUTOFRANGE       0X23A
#endif
#ifndef WM_POINTERCAPTURECHANGED
#define WM_POINTERCAPTURECHANGED         0x024C
#endif
    enum INPUT_MESSAGE_DEVICE_TYPE
    {
        IMDT_UNAVAIABLE = 0,
        IMDT_KEYBOARD = 1,
        IMDT_MOUSE = 2,
        IMDT_TOUCH = 4,
        IMDT_PEN = 8,
        IMDT_TOUCHPAD = 16,
    };
    enum INPUT_MESSAGE_ORIGIN_ID
    {
        IMO_UNAVAILABLE = 0,
        IMO_HARDWARE = 1,
        IMO_INJECTED = 2,
        IMO_SYSTEM = 4,
    };
    struct INPUT_MESSAGE_SOURCE
    {
        INPUT_MESSAGE_DEVICE_TYPE deviceType;
        INPUT_MESSAGE_ORIGIN_ID originId;
    };
    enum POINTER_INPUT_TYPE
    {
        PT_POINTER   = 0x00000001,
        PT_TOUCH     = 0x00000002,
        PT_PEN       = 0x00000003,
        PT_MOUSE     = 0x00000004,
        PT_TOUCHPAD  = 0x00000005
    };
    enum POINTER_BUTTON_CHANGE_TYPE
    {
        POINTER_CHANGE_NONE,
        POINTER_CHANGE_FIRSTBUTTON_DOWN,
        POINTER_CHANGE_FIRSTBUTTON_UP,
        POINTER_CHANGE_SECONDBUTTON_DOWN,
        POINTER_CHANGE_SECONDBUTTON_UP,
        POINTER_CHANGE_THIRDBUTTON_DOWN,
        POINTER_CHANGE_THIRDBUTTON_UP,
        POINTER_CHANGE_FOURTHBUTTON_DOWN,
        POINTER_CHANGE_FOURTHBUTTON_UP,
        POINTER_CHANGE_FIFTHBUTTON_DOWN,
        POINTER_CHANGE_FIFTHBUTTON_UP,
    };
    enum POINTER_FLAG
    {
        POINTER_FLAG_NEW = 1,
        POINTER_FLAG_INRANGE = 2,
        POINTER_FLAG_INCONTACT = 4,
        POINTER_FLAG_FIRSTBUTTON = 8,
        POINTER_FLAG_SECONDBUTTON = 16,
        POINTER_FLAG_THIRDBUTTON = 32,
        POINTER_FLAG_FOURTHBUTTON = 64,
        POINTER_FLAG_FIFTHBUTTON = 128,
        POINTER_FLAG_PRIMARY = 256,
        POINTER_FLAG_CONFIDENCE = 512,
        POINTER_FLAG_CANCELLED = 1024,
        POINTER_FLAG_DOWN = 2048,
        POINTER_FLAG_UPDATE = 4096,
        POINTER_FLAG_UP = 8192,
    };
    enum POINTER_DEVICE_TYPE
    {
        POINTER_DEVICE_TYPE_INTEGRATED_PEN  = 0x00000001,
        POINTER_DEVICE_TYPE_EXTERNAL_PEN    = 0x00000002,
        POINTER_DEVICE_TYPE_TOUCH           = 0x00000003,
        POINTER_DEVICE_TYPE_TOUCH_PAD       = 0x00000004,
        POINTER_DEVICE_TYPE_MAX             = 0xFFFFFFFF
    };
    struct POINTER_DEVICE_INFO
    {
        DWORD               displayOrientation;
        HANDLE              device;
        POINTER_DEVICE_TYPE pointerDeviceType;
        HMONITOR            monitor;
        ULONG               startingCursorId;
        USHORT              maxActiveContacts;
        WCHAR               productString[520];
    };
    struct POINTER_INFO
    {
        POINTER_INPUT_TYPE         pointerType;
        UINT32                     pointerId;
        UINT32                     frameId;
        DWORD                      pointerFlags;
        HANDLE                     sourceDevice;
        HWND                       hwndTarget;
        POINT                      ptPixelLocation;
        POINT                      ptHimetricLocation;
        POINT                      ptPixelLocationRaw;
        POINT                      ptHimetricLocationRaw;
        DWORD                      dwTime;
        UINT32                     historyCount;
        INT32                      inputData;
        DWORD                      dwKeyStates;
        UINT64                     PerformanceCount;
        POINTER_BUTTON_CHANGE_TYPE ButtonChangeType;
    };
    enum PEN_FLAGS
    {
        PEN_FLAG_BARREL = 1,
        PEN_FLAG_INVERTED = 2,
        PEN_FLAG_ERASER = 4,
    };
    struct POINTER_PEN_INFO
    {
        POINTER_INFO pointerInfo;
        DWORD        penFlags;
        DWORD        penMask;
        UINT32       pressure;
        UINT32       rotation;
        INT32        tiltX;
        INT32        tiltY;
    };
    typedef BOOL (WINAPI *GetCurrentInputMessageSourceProc)(INPUT_MESSAGE_SOURCE*);
    typedef BOOL (WINAPI *GetPointerPenInfoProc)(UINT32, POINTER_PEN_INFO*);
    typedef BOOL (WINAPI *GetPointerDevicesProc)(UINT*, POINTER_DEVICE_INFO*);
    static GetCurrentInputMessageSourceProc Win32GetCurrentInputMessageSource;
    static GetPointerPenInfoProc Win32GetPointerPenInfo;
    static GetPointerDevicesProc Win32GetPointerDevices;
    static bool InitializePointerInputAPI()
    {
        HMODULE user32dll = GetModuleHandle("User32.dll");

        Win32GetCurrentInputMessageSource = (GetCurrentInputMessageSourceProc)GetProcAddress(user32dll, "GetCurrentInputMessageSource");
        Win32GetPointerPenInfo = (GetPointerPenInfoProc)GetProcAddress(user32dll, "GetPointerPenInfo");
        Win32GetPointerDevices = (GetPointerDevicesProc)GetProcAddress(user32dll, "GetPointerDevices");

        return Win32GetCurrentInputMessageSource && Win32GetPointerPenInfo && Win32GetPointerDevices;
    }

    static bool GetPenPointerDeviceInfo(POINTER_DEVICE_INFO& outInfo)
    {
        if (!Win32GetPointerDevices)
            return false;

        UINT32 totalDevices;
        Win32GetPointerDevices(&totalDevices, NULL);

        POINTER_DEVICE_INFO* devices;
        ALLOC_TEMP_AUTO(devices, totalDevices);

        if (!Win32GetPointerDevices(&totalDevices, devices))
            return false;

        for (int i = 0; i < totalDevices; ++i)
            if (devices[i].pointerDeviceType == POINTER_DEVICE_TYPE_EXTERNAL_PEN || devices[i].pointerDeviceType == POINTER_DEVICE_TYPE_INTEGRATED_PEN)
            {
                memcpy(&outInfo, &devices[i], sizeof(outInfo));
                return true;
            }

        return false;
    }

    #pragma endregion

    #pragma region XInput

    // XInput support. Polled on background thread owned by Modules/Input.
    // To not constantly poll XInput devices that don't exist, we enlist the help
    // of the HID thread which recognizes and filters out XInput HIDs and tells us
    // when the device setup has changed.`
    //
    // We could support XInput devices through HID as well but the XInput HID driver
    // is made unusable due to the fact that the left and right trigger are combined
    // into a single axis. We do fall back to it, though, if we can't load the XInput
    // DLL. In that case, XInput controllers will surface as generic HIDs on the managed
    // side and probably get picked up by the HID fallbacks (won't be super useful but
    // maybe better than just making them not appear at all).
    struct XInputDevice : InputDeviceCallbacks
    {
        static XInputDevice s_XInputDevices[win::shared::kXInputMaxControllers];

        typedef DWORD (WINAPI* XInputGetStateFunc)(DWORD dwUserIndex, XINPUT_STATE* pState);
        typedef DWORD (WINAPI* XInputSetStateFunc)(DWORD dwUserIndex, XINPUT_VIBRATION* pState);
        typedef DWORD (WINAPI* XInputGetCapabilitiesFunc)(DWORD dwUserIndex, DWORD dwFlags, XINPUT_CAPABILITIES* pState);

        static HINSTANCE s_XInputDLL;
        static XInputGetStateFunc s_XInputGetStateFn;
        static XInputSetStateFunc s_XInputSetStateFn;
        static XInputGetCapabilitiesFunc s_XInputGetCapabilitiesFn;

        static void LoadXInputLibrary()
        {
            if (s_XInputDLL)
                return;

            s_XInputDLL = LoadLibrary("xinput1_4.dll");
            if (!s_XInputDLL)
            {
                printf_console("XInput1_4.dll not found. Trying XInput1_3.dll instead...\n");

                s_XInputDLL = LoadLibrary("xinput1_3.dll");
                if (!s_XInputDLL)
                {
                    printf_console("XInput1_3.dll not found. Trying XInput9_1_0.dll instead...\n");

                    s_XInputDLL = LoadLibrary("xinput9_1_0.dll");
                    if (!s_XInputDLL)
                    {
                        printf_console("XInput9_1_0.dll not found either. XInput-based controllers will use HID and probably not work as expected.\n");
                        return;
                    }
                }
            }

            // XInputGetState() is mandatory.
            s_XInputGetStateFn = (XInputGetStateFunc)GetProcAddress(s_XInputDLL, "XInputGetState");
            if (!s_XInputGetStateFn)
            {
                printf_console("XInputGetState not found in the XInput DLL. Switching back to HID. XInput devices will probably not work as expected.\n");
                UnloadXInputLibrary();
                return;
            }

            // Remaining functions are optional.
            s_XInputSetStateFn = (XInputSetStateFunc)GetProcAddress(s_XInputDLL, "XInputSetState");
            s_XInputGetCapabilitiesFn = (XInputGetCapabilitiesFunc)GetProcAddress(s_XInputDLL, "XInputGetCapabilities");
        }

        static void UnloadXInputLibrary()
        {
            if (s_XInputDLL)
            {
                FreeLibrary(s_XInputDLL);

                s_XInputDLL = NULL;
                s_XInputGetStateFn = NULL;
                s_XInputSetStateFn = NULL;
                s_XInputGetCapabilitiesFn = NULL;

                memset(s_XInputDevices, 0, sizeof(s_XInputDevices));
            }
        }

        static bool IsXInputDevice(const core::wstring& devicePath)
        {
            return (wcsstr(devicePath.c_str(), L"IG_") || wcsstr(devicePath.c_str(), L"ig_"));
        }

        static void CheckXInputDeviceConnectsAndDisconnects()
        {
            if (!s_XInputDLL)
                return;

            for (int i = 0; i < win::shared::kXInputMaxControllers; ++i)
            {
                XINPUT_STATE state;
                XInputDevice& device = s_XInputDevices[i];

                DWORD result = s_XInputGetStateFn(i, &state);
                if (result == ERROR_DEVICE_NOT_CONNECTED)
                {
                    if (device.deviceId != kInvalidInputDeviceId)
                    {
                        // Unplugged.
                        ReportInputDeviceRemoved(device.deviceId);
                        device.deviceId = kInvalidInputDeviceId;
                    }
                }
                else if (result == ERROR_SUCCESS && device.deviceId == kInvalidInputDeviceId)
                {
                    InputDeviceDescriptorWithCapabilities<win::shared::XInputDeviceCapabilities> descriptor;
                    memset(&descriptor.capabilities, 0, sizeof(descriptor.capabilities));

                    // XInput is not giving us any information about the *actual* device that is connected
                    // so we're not reporting any product and manufacturer information.

                    descriptor.interfaceName = "XInput";
                    descriptor.capabilities.userIndex = i;

                    // Query capabilities.
                    if (s_XInputGetCapabilitiesFn)
                    {
                        XINPUT_CAPABILITIES caps;
                        if (s_XInputGetCapabilitiesFn(i, 0, &caps) == ERROR_SUCCESS)
                        {
                            descriptor.capabilities.type = caps.Type;
                            descriptor.capabilities.subType = caps.SubType;
                            descriptor.capabilities.flags = caps.Flags;
                            descriptor.capabilities.gamepad.buttons = caps.Gamepad.wButtons;
                            descriptor.capabilities.gamepad.leftTrigger = caps.Gamepad.bLeftTrigger;
                            descriptor.capabilities.gamepad.rightTrigger = caps.Gamepad.bRightTrigger;
                            descriptor.capabilities.gamepad.leftStickX = caps.Gamepad.sThumbLX;
                            descriptor.capabilities.gamepad.leftStickY = caps.Gamepad.sThumbLY;
                            descriptor.capabilities.gamepad.rightStickX = caps.Gamepad.sThumbRX;
                            descriptor.capabilities.gamepad.rightStickY = caps.Gamepad.sThumbRY;
                            descriptor.capabilities.vibration.leftMotor = caps.Vibration.wLeftMotorSpeed;
                            descriptor.capabilities.vibration.rightMotor = caps.Vibration.wRightMotorSpeed;
                        }
                    }

                    // Report.
                    device.deviceId = ReportNewInputDevice(descriptor, &device);
                    device.userIndex = i;
                }
            }
        }

        int userIndex; // Device index used by XInput DLL.
        DWORD lastPacketNumber;
        InputDeviceID deviceId; // ID the controller is registered with under Unity's input system.

        XInputDevice()
            : userIndex(-1)
            , lastPacketNumber(0)
            , deviceId(kInvalidInputDeviceId)
        {}

        virtual bool ShouldPoll() { return true; }
        virtual void Poll()
        {
            XINPUT_STATE state;
            if (s_XInputGetStateFn(userIndex, &state) != ERROR_SUCCESS)
                return;

            // Ignore if controller has not changed.
            if (lastPacketNumber == state.dwPacketNumber)
                return;

            // Send event.
            StateInputEventData<XINPUT_GAMEPAD> event;
            event.type = kInputEventState;
            event.deviceId = deviceId;
            event.time = GetInputEventTimeNow();
            event.sizeInBytes = sizeof(event);
            event.stateFormat = 'XINP';
            memcpy(&event.stateData, &state.Gamepad, sizeof(XINPUT_GAMEPAD));
            QueueInputEvent(event);

            lastPacketNumber = state.dwPacketNumber;
        }

        virtual SInt64 IOCTL(int code, void* buffer, int bufferSize)
        {
            switch (code)
            {
                case kIOCTLQueryRunInBackground:
                {
                    if (bufferSize != sizeof(bool))
                        return kIOCTLFailure;

                    bool* canRunInBackground = static_cast<bool*>(buffer);
                    *canRunInBackground = true;

                    return kIOCTLSuccess;
                }

                case kIOCTLRequestSyncDevice:
                {
                    lastPacketNumber = 0;
                    Poll();
                    return kIOCTLSuccess;
                }
                case kIOCTLDualMotorRumble:
                {
                    if (bufferSize < sizeof(IOCTLDualMotorRumble))
                        return kIOCTLFailure;
                    IOCTLDualMotorRumble* rumble = reinterpret_cast<IOCTLDualMotorRumble*>(buffer);
                    XINPUT_VIBRATION vibration;
                    vibration.wLeftMotorSpeed = 65535 * rumble->lowFrequencyMotorSpeed;
                    vibration.wRightMotorSpeed = 65535 * rumble->highFrequencyMotorSpeed;
                    if (s_XInputSetStateFn(userIndex, &vibration) != ERROR_SUCCESS)
                        return kIOCTLFailure;
                    return kIOCTLSuccess;
                }
            }

            return kIOCTLFailure;
        }
    };

    XInputDevice XInputDevice::s_XInputDevices[win::shared::kXInputMaxControllers];
    HINSTANCE XInputDevice::s_XInputDLL;
    XInputDevice::XInputGetStateFunc XInputDevice::s_XInputGetStateFn;
    XInputDevice::XInputSetStateFunc XInputDevice::s_XInputSetStateFn;
    XInputDevice::XInputGetCapabilitiesFunc XInputDevice::s_XInputGetCapabilitiesFn;

    #pragma endregion

    #pragma region HID

    // This here is some ugly stuff. Windows neither has the means to provide us with raw HID report descriptors(**)
    // nor does it make element offsets available through the HID parser API or otherwise makes it possible to compute
    // those offsets (there's no way to find original element ordering as present on the device so we can't compute
    // offsets as they depend on elements being in the correct order and also on *all* elements being made visible to
    // us). However, we absolutely need the data as the C# code has to be able to place controls in memory and have them
    // read out data from input reports.
    //
    // So... we resort to hacks. What we do here is abuse the API that allows *creating* reports. By writing into
    // blank reports, we can find out where the HID class driver placed data in memory and thus infer where elements
    // are in memory.
    //
    // Thank you Microsoft.
    //
    // NOTE: This path is only relevant for when we actually *have* to look at the HID descriptor to make sense of a device.
    //       For devices we know and directly support, we already know the memory layout and do not need the descriptor. With
    //       those devices, there simply won't be an IOCTL to request the descriptor.
    //
    // (**) The HID class driver sits on top of the stack and prevents access to the HID mini driver which has the report
    //      descriptor. We can get to the USB hub and issue Get_Descriptor requests through it directly on the USB device
    //      but IOCTL_USB_GET_DESCRIPTOR_FROM_NODE_CONNECTION force-sets certain fields that we need to make a HID report
    //      descriptor request. We can get the HID class descriptor, no problem, but not the HID report descriptor. Genius
    //      design really.
    //
    //      On OSX, getting the HID report descriptor is one dictionary lookup.

    static int HACK_FindElementOffset(char* inputReport, int inputReportLength, bool hasReportId)
    {
        int i = 0;

        // HID reports only have an ID byte if the HID report descriptor has report IDs
        // on its elements.
        if (hasReportId)
            ++i;

        // Find the "first" (least significant) bit set within inputReport to calculate the bit-offset
        // for both single bit controls and multi-byte value controls
        for (; i < inputReportLength; ++i)
        {
            char value = inputReport[i];
            if (!value)
                continue;

            int bit = 0;
            while ((value & 0x1) == 0)
            {
                ++bit;
                value >>= 1;
            }

            return (i * 8) + bit;
        }

        return -1;
    }

    static HIDReportType ConvertReportType(HIDP_REPORT_TYPE reportType)
    {
        switch (reportType)
        {
            case HidP_Input: return kHIDInputReport;
            case HidP_Output: return kHIDOutputReport;
            case HidP_Feature: return kHIDFeatureReport;
        }
        return kHIDUnknownReport;
    }

    // If the lower bound is *not* negative, then the upper bound isn't either. However,
    // Windows seems to always perform a sign extension of the 16bit value coming from the HID
    // descriptor to a 32bit signed value. We detect this here and undo the sign extension.
    // (You can observe this with Xbox 360-compatible controllers which otherwise appear to have
    // min=0 and max=-1 when really the controller wants to tell us it has min=0 and max=65535).
    static void RepairWindowsMinMaxLimitsSignExtension(HIDP_VALUE_CAPS& caps)
    {
        if (caps.PhysicalMin >= 0 && caps.PhysicalMax < 0)
            caps.PhysicalMax = static_cast<ULONG>(caps.PhysicalMax & 0xffff);
        if (caps.LogicalMin >= 0 && caps.LogicalMax < 0)
            caps.LogicalMax = static_cast<ULONG>(caps.LogicalMax & 0xffff);
    }

    static void HACK_FindAndAddButtonElement(HIDP_BUTTON_CAPS& button, USAGE usage, HIDP_REPORT_TYPE reportType, char* reportBuffer, int reportLength, PHIDP_PREPARSED_DATA preparsedData, HIDDeviceDescriptor& descriptor)
    {
        memset(reportBuffer, 0, reportLength);

        ULONG numUsages = 1;
        if (HidP_SetUsages(reportType, button.UsagePage, button.LinkCollection, &usage, &numUsages,
            preparsedData, reportBuffer, reportLength) != HIDP_STATUS_SUCCESS)
            return;

        int offset = HACK_FindElementOffset(reportBuffer, reportLength, button.ReportID != 0);
        if (offset == -1)
            return;

        HIDElementDescriptor& element = descriptor.elements.emplace_back();

        element.usage = usage;
        element.usagePage = button.UsagePage;
        element.reportId = button.ReportID;
        element.reportType = ConvertReportType(reportType);
        element.reportCount = 1;
        element.reportOffsetInBits = offset;
        element.reportSizeInBits = 1;
        element.flags = (HIDElementFlags)button.BitField;
    }

    static void HACK_FindAndAddValueElement(HIDP_VALUE_CAPS& value, USAGE usage, HIDP_REPORT_TYPE reportType, char* reportBuffer, int reportLength, PHIDP_PREPARSED_DATA preparsedData, HIDDeviceDescriptor& descriptor)
    {
        for (int i = 0; i < value.ReportCount; ++i)
        {
            memset(reportBuffer, 0, reportLength);

            if (HidP_SetUsageValue(reportType, value.UsagePage, value.LinkCollection, usage, 0xFFFFFFFF,
                preparsedData, reportBuffer, reportLength) != HIDP_STATUS_SUCCESS)
                continue;

            // We don't care about the actual width of the value here. All we care about is it's starting bit position
            int offset = HACK_FindElementOffset(reportBuffer, reportLength, value.ReportID != 0);
            if (offset == -1)
                continue;

            RepairWindowsMinMaxLimitsSignExtension(value);

            HIDElementDescriptor& element = descriptor.elements.emplace_back();

            element.usage = usage;
            element.usagePage = value.UsagePage;
            element.reportId = value.ReportID;
            element.reportType = ConvertReportType(reportType);
            element.reportCount = 1;
            element.reportOffsetInBits = offset;
            element.reportSizeInBits = value.BitSize;
            element.unit = value.Units;
            element.unitExponent = value.UnitsExp;
            element.logicalMin = value.LogicalMin;
            element.logicalMax = value.LogicalMax;
            element.physicalMin = value.PhysicalMin;
            element.physicalMax = value.PhysicalMax;
            element.flags = (HIDElementFlags)value.BitField;
        }
    }

    static void HACK_AddReportElements(HIDP_REPORT_TYPE reportType, int reportLength, int numButtonCaps, int numValueCaps, PHIDP_PREPARSED_DATA preparsedData, HIDDeviceDescriptor& descriptor)
    {
        PHIDP_BUTTON_CAPS buttons;
        PHIDP_VALUE_CAPS values;
        ALLOC_TEMP_AUTO(buttons, numButtonCaps);
        ALLOC_TEMP_AUTO(values, numValueCaps);
        USHORT numButtons = numButtonCaps;
        USHORT numValues = numValueCaps;

        // Read buttons and values.
        if (HidP_GetButtonCaps(reportType, buttons, &numButtons, preparsedData) != HIDP_STATUS_SUCCESS)
            return;
        if (HidP_GetValueCaps(reportType, values, &numValues, preparsedData) != HIDP_STATUS_SUCCESS)
            return;

        // Go through elements and attempt to find their offsets.
        char* reportBuffer;
        ALLOC_TEMP_AUTO(reportBuffer, reportLength);
        for (int i = 0; i < numButtons; ++i)
        {
            HIDP_BUTTON_CAPS& button = buttons[i];
            if (button.IsRange)
            {
                for (USAGE usage = button.Range.UsageMin; usage <= button.Range.UsageMax; ++usage)
                    HACK_FindAndAddButtonElement(button, usage, reportType, reportBuffer, reportLength, preparsedData, descriptor);
            }
            else
            {
                HACK_FindAndAddButtonElement(button, button.NotRange.Usage, reportType, reportBuffer, reportLength, preparsedData, descriptor);
            }
        }
        for (int i = 0; i < numValues; ++i)
        {
            HIDP_VALUE_CAPS& value = values[i];
            if (value.IsRange)
            {
                for (USAGE usage = value.Range.UsageMin; usage <= value.Range.UsageMax; ++usage)
                    HACK_FindAndAddValueElement(value, usage, reportType, reportBuffer, reportLength, preparsedData, descriptor);
            }
            else
            {
                HACK_FindAndAddValueElement(value, value.NotRange.Usage, reportType, reportBuffer, reportLength, preparsedData, descriptor);
            }
        }
    }

    static bool HackTogetherHIDDeviceDescriptor(HIDDeviceDescriptor& descriptor, HANDLE fileHandle, PHIDP_PREPARSED_DATA preparsedData)
    {
        // Read attributes.
        HIDD_ATTRIBUTES attributes;
        if (!HidD_GetAttributes(fileHandle, &attributes))
            return false;

        descriptor.productId = attributes.ProductID;
        descriptor.vendorId = attributes.VendorID;

        // Read capabilities.
        HIDP_CAPS caps;
        if (HidP_GetCaps(preparsedData, &caps) != HIDP_STATUS_SUCCESS)
            return false;

        descriptor.usage = caps.Usage;
        descriptor.usagePage = caps.UsagePage;
        descriptor.inputReportSize = caps.InputReportByteLength;
        descriptor.outputReportSize = caps.OutputReportByteLength;
        descriptor.featureReportSize = caps.FeatureReportByteLength;

        // Add report elements.
        HACK_AddReportElements(HidP_Input, caps.InputReportByteLength, caps.NumberInputButtonCaps, caps.NumberInputValueCaps, preparsedData, descriptor);
        HACK_AddReportElements(HidP_Output, caps.OutputReportByteLength, caps.NumberOutputButtonCaps, caps.NumberOutputValueCaps, preparsedData, descriptor);
        HACK_AddReportElements(HidP_Feature, caps.FeatureReportByteLength, caps.NumberFeatureButtonCaps, caps.NumberFeatureValueCaps, preparsedData, descriptor);

        return true;
    }

    // State exclusive to the HID input thread.
    struct HIDInputThreadState
    {
        // Record we keep for every HID we discover via setupapi.
        struct HIDDevice : public InputDeviceCallbacks
        {
            int deviceId; // ID in the new input system.
            core::wstring devicePath;
            DEVINST deviceInstance;
            HANDLE fileHandle;
            PHIDP_PREPARSED_DATA preparsedData;
            HANDLE inputEvent;
            HANDLE outputEvent;
            OVERLAPPED inputOverlapped;
            OVERLAPPED outputOverlapped;
            int inputReportSize;
            dynamic_array<CHAR> inputBuffer;
            dynamic_array<CHAR> outputBuffer;

            HIDDevice()
                : deviceId(kInvalidInputDeviceId)
                , devicePath(kMemInput)
                , deviceInstance(0)
                , fileHandle(INVALID_HANDLE_VALUE)
                , preparsedData(NULL)
                , inputEvent(INVALID_HANDLE_VALUE)
                , outputEvent(INVALID_HANDLE_VALUE)
                , inputBuffer(kMemInput)
            {
                memset(&inputOverlapped, 0, sizeof(inputOverlapped));
                inputOverlapped.hEvent = INVALID_HANDLE_VALUE;

                memset(&outputOverlapped, 0, sizeof(outputOverlapped));
                outputOverlapped.hEvent = INVALID_HANDLE_VALUE;
            }

            ~HIDDevice()
            {
                ReleaseConnectionState();
            }

            bool PrepareForIO(HIDP_CAPS* capsPtr = 0);
            void CheckInputReport();
            bool StartReadingNextInputReport();
            void SendInputReportAsEvent();
            void ReleaseConnectionState();

            core::string ReadProductString();
            core::string ReadManufacturerString();
            core::string ReadSerialNumberString();

            virtual SInt64 IOCTL(int code, void* buffer, int size);

            int ReadHIDReportDescriptor(void* buffer, int size);
            bool WriteOutputReport(void* buffer, int size);

            void* GetInputStateBuffer()
            {
                return GetInputStateEvent().GetStateData();
            }

            StateInputEventBase& GetInputStateEvent()
            {
                return *reinterpret_cast<StateInputEventBase*>(inputBuffer.data());
            }
        };

        ////REVIEW: use dictionary rather than array?
        // Pointer to HIDDevices rather than embedded struct because we don't want the address
        // of the OVERLAPPED structs to change.
        dynamic_array<HIDDevice*> devices;

        // The HID input thread will use WaitForMultipleObjects on the handles in this array. The first handle
        // is always an event fired by the main thread either if the device setup in the system has changed
        // or if the main thread wants the HID input thread to exit. The remaining handles are for overlapped I/O
        // operations that are in progress.
        dynamic_array<HANDLE> waitHandles;
        dynamic_array<HIDDevice*> devicesForWaitHandles; // Map entry in 'waitHandles' to device.

        // These are accessed from the main thread.
        Thread thread;
        HANDLE checkDeviceListOrExit;
        volatile bool suspend;
        Semaphore wakeUpFromSuspend;

        HIDInputThreadState();
        ~HIDInputThreadState();

        void CheckForDeviceAddsAndRemovals();

        void OpenDevice(HIDDevice& device); // First-time opening of device. Happens once only on any device.
        void CloseDevice(HIDDevice& device); // Closing device happens when device is unplugged or when shutting down.
    };

    static RuntimeStatic<HIDInputThreadState> g_HIDInputThreadState(kMemInput);

    HIDInputThreadState::HIDInputThreadState()
        : devices(kMemInput)
        , waitHandles(kMemInput)
        , devicesForWaitHandles(kMemInput)
        , checkDeviceListOrExit(INVALID_HANDLE_VALUE)
        , suspend(false)
    {
        waitHandles.resize_uninitialized(1);
        devicesForWaitHandles.resize_uninitialized(1);

        // Event #0 is used to tell the HID thread about WM_DEVICECHANGE messages and to get it
        // to break out of WaitForMultipleObjects. We set the event initially to be signaled so that
        // the HID thread immediately does one pass of device discovery.
        checkDeviceListOrExit = CreateEvent(NULL, TRUE, TRUE, NULL);
        waitHandles[0] = checkDeviceListOrExit;
        devicesForWaitHandles[0] = NULL; // First entry is not associated with device.
    }

    HIDInputThreadState::~HIDInputThreadState()
    {
        CloseHandle(waitHandles[0]);
        for (int i = 0; i < devices.size(); ++i)
            UNITY_DELETE(devices[i], kMemInput);
    }

    bool HIDInputThreadState::HIDDevice::PrepareForIO(HIDP_CAPS* capsPtr)
    {
        AssertMsg(fileHandle == INVALID_HANDLE_VALUE, "Device file must be closed at this point!");
        HIDP_CAPS capsData;
        HIDP_CAPS& caps = capsPtr ? *capsPtr : capsData;

        // Open device file.
        fileHandle = SYSTEM_CALL(CreateFileW, (devicePath.c_str(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL));
        if (fileHandle == INVALID_HANDLE_VALUE)
            return false;

        // Fetch preparsed data.
        if (!SYSTEM_CALL(HidD_GetPreparsedData, (fileHandle, &preparsedData)))
        {
            CloseHandle(fileHandle);
            fileHandle = INVALID_HANDLE_VALUE;
            return false;
        }

        // Get caps.
        if (SYSTEM_CALL(HidP_GetCaps, (preparsedData, &caps)) != HIDP_STATUS_SUCCESS)
        {
            CloseHandle(fileHandle);
            fileHandle = INVALID_HANDLE_VALUE;
            return false;
        }

        // Set up buffers.
        inputReportSize = caps.InputReportByteLength;
        if (inputReportSize > 0)
        {
            // Set up state events.
            int stateEventSize = inputReportSize + sizeof(StateInputEventBase);
            inputBuffer.resize_uninitialized(stateEventSize);
            StateInputEventBase& stateEvent = GetInputStateEvent();
            stateEvent.sizeInBytes = stateEventSize;
            stateEvent.type = kInputEventState;
            stateEvent.stateFormat = kInputHIDState;

            // Get async input going.
            // NOTE: Use manual reset event as recommended by MSDN. See documentation for OVERLAPPED structure.
            inputEvent = SYSTEM_CALL(CreateEventW, (NULL, TRUE, FALSE, NULL));
            StartReadingNextInputReport();
        }

        if (caps.OutputReportByteLength > 0)
        {
            outputBuffer.resize_uninitialized(caps.OutputReportByteLength);
            outputEvent = SYSTEM_CALL(CreateEventW, (NULL, TRUE, FALSE, NULL));
        }

        return true;
    }

    void HIDInputThreadState::HIDDevice::CheckInputReport()
    {
        // Devices will usually generated input reports at regular intervals using interrupt transfers.
        // The HID driver will accumulate a certain number of reports but if we don't pick up reports
        // fast enough, newer reports will overwrite older reports and we'll loose input. So it's vital
        // to issue reads fast enough and check results fast enough to keep up with devices.

        DWORD bytesRead = 0; // Unused.
        if (SYSTEM_CALL(GetOverlappedResult, (fileHandle, &inputOverlapped, &bytesRead, FALSE)))
        {
            SendInputReportAsEvent();
        }
        else
        {
            DWORD error = GetLastError();
            if (error == ERROR_IO_INCOMPLETE)
            {
                return;
            }
        }

        // We cap our checks to avoid any driver that can produce reports faster than we can read them.
        static const int s_MaxReportReadsPerFrame = 5;
        for (int i = 0; i < s_MaxReportReadsPerFrame; i++)
        {
            if (StartReadingNextInputReport())
            {
                SendInputReportAsEvent();
            }
            else
            {
                break;
            }
        }
    }

    bool HIDInputThreadState::HIDDevice::StartReadingNextInputReport()
    {
        if (inputReportSize == 0)
            return true;

        inputOverlapped.hEvent = inputEvent;
        ResetEvent(inputEvent);

        void* buffer = GetInputStateEvent().GetStateData();
        return SYSTEM_CALL(ReadFile, (fileHandle, buffer, inputReportSize, NULL, &inputOverlapped));
    }

    void HIDInputThreadState::HIDDevice::SendInputReportAsEvent()
    {
        StateInputEventBase& stateEvent = GetInputStateEvent();
        stateEvent.time = GetInputEventTimeNow();
        QueueInputEvent(stateEvent);
    }

    void HIDInputThreadState::HIDDevice::ReleaseConnectionState()
    {
        if (preparsedData)
            SYSTEM_CALL(HidD_FreePreparsedData, (preparsedData));
        if (fileHandle != INVALID_HANDLE_VALUE)
            SYSTEM_CALL(CloseHandle, (fileHandle));
        if (inputEvent != INVALID_HANDLE_VALUE)
            SYSTEM_CALL(CloseHandle, (inputEvent));
        if (outputEvent != INVALID_HANDLE_VALUE)
            SYSTEM_CALL(CloseHandle, (outputEvent));

        fileHandle = INVALID_HANDLE_VALUE;
        inputEvent = INVALID_HANDLE_VALUE;
        outputEvent = INVALID_HANDLE_VALUE;
        deviceInstance = 0;
        inputBuffer.clear_dealloc();
        outputBuffer.clear_dealloc();
        preparsedData = NULL;
    }

    core::string HIDInputThreadState::HIDDevice::ReadProductString()
    {
        WCHAR wideString[1024];
        if (!SYSTEM_CALL(HidD_GetProductString, (fileHandle, wideString, sizeof(wideString))))
            return core::string();

        return WideToUtf8(wideString);
    }

    core::string HIDInputThreadState::HIDDevice::ReadManufacturerString()
    {
        WCHAR wideString[1024];
        if (!SYSTEM_CALL(HidD_GetManufacturerString, (fileHandle, wideString, sizeof(wideString))))
            return core::string();

        return WideToUtf8(wideString);
    }

    core::string HIDInputThreadState::HIDDevice::ReadSerialNumberString()
    {
        ////FIXME: not getting a serial number for Xbox One controller which clearly has one (looking at data in USBlyzer)
        WCHAR wideString[1024];
        if (!SYSTEM_CALL(HidD_GetSerialNumberString, (fileHandle, wideString, sizeof(wideString))))
            return core::string();

        return WideToUtf8(wideString);
    }

    SInt64 HIDInputThreadState::HIDDevice::IOCTL(int code, void* buffer, int size)
    {
        switch (code)
        {
            case kIOCTLHIDWriteOutputReport:
            {
                if (!WriteOutputReport(buffer, size))
                    return kIOCTLFailure;
                return kIOCTLSuccess;
            }

            // On Windows, we can't get raw descriptors. Serve preparsed descriptors.
            case kIOCTLGetParsedHIDReportDescriptor:
            {
                HIDDeviceDescriptor descriptor;
                if (!HackTogetherHIDDeviceDescriptor(descriptor, fileHandle, preparsedData))
                    return kIOCTLFailure;

                core::string descriptorString;
                JSONUtility::SerializeToJSON(descriptor, descriptorString);

                int requiredBufferSize = descriptorString.size();
                if (size < requiredBufferSize)
                    return -requiredBufferSize;

                memcpy(buffer, descriptorString.c_str(), requiredBufferSize);
                return requiredBufferSize;
            }

            case kIOCTLQueryRunInBackground:
            {
                if (size != sizeof(bool))
                    return kIOCTLFailure;

                bool* canRunInBackground = static_cast<bool*>(buffer);
                *canRunInBackground = false;

                return kIOCTLSuccess;
            }

            case kIOCTLRequestSyncDevice:
            {
                SendInputReportAsEvent();
                return kIOCTLSuccess;
            }
        }

        return kIOCTLFailure;
    }

    bool HIDInputThreadState::HIDDevice::WriteOutputReport(void* data, int size)
    {
        // Fail if device doesn't support output.
        if (!outputBuffer.size())
            return false;

        ////REVIEW: why are we doing this? only because we're allocating only a single outputBuffer?
        // Fail if there's still a write in progress.
        if (outputOverlapped.hEvent != INVALID_HANDLE_VALUE)
        {
            DWORD waitResult = WaitForSingleObject(outputOverlapped.hEvent, 0);
            if (waitResult != WAIT_OBJECT_0)
                return false;
        }

        // Initialize report.
        // NOTE: We're not using HidP_InitializeReportForID as it does nothing useful for us. Laying out the report
        //       correctly is entirely left to the caller and we're only making sure here that the caller got the
        //       right size.
        PCHAR report = outputBuffer.data();
        ULONG reportLength = outputBuffer.size();
        if (reportLength != size)
            return false;

        memcpy(report, data, size);

        // Write report.
        memset(&outputOverlapped, 0, sizeof(outputOverlapped));
        outputOverlapped.hEvent = outputEvent;
        if (!WriteFile(fileHandle, report, reportLength, NULL, &outputOverlapped))
        {
            DWORD error = GetLastError();

            // These two errors mean that the write is in progress or enqueued, respectively.
            // They are not true errors, but just report that the write is not yet completed.
            if (error == ERROR_IO_INCOMPLETE || error == ERROR_IO_PENDING)
            {
                return true;
            }

            // Clear last read state.
            outputOverlapped.hEvent = outputEvent;
            ResetEvent(outputEvent);

            return false;
        }

        return true;
    }

    void HIDInputThreadState::OpenDevice(HIDDevice& device)
    {
        HIDP_CAPS caps;
        if (!device.PrepareForIO(&caps))
            return;

        // We only want to add devices that have any input events we want to wait for.
        if (device.inputEvent != INVALID_HANDLE_VALUE)
        {
            waitHandles.push_back(device.inputEvent);
            devicesForWaitHandles.push_back(&device);
        }

        core::string product = device.ReadProductString();
        core::string manufacturer = device.ReadManufacturerString();
        core::string serialNumber = device.ReadSerialNumberString();

        // Create descriptor.
        HIDInputDeviceDescriptor descriptor;
        descriptor.interfaceName = "HID";
        descriptor.manufacturer = manufacturer;
        descriptor.product = product;
        descriptor.serial = serialNumber;
        descriptor.capabilities.usage = caps.Usage;
        descriptor.capabilities.usagePage = caps.UsagePage;
        descriptor.capabilities.inputReportSize = caps.InputReportByteLength;
        descriptor.capabilities.outputReportSize = caps.OutputReportByteLength;
        descriptor.capabilities.featureReportSize = caps.FeatureReportByteLength;

        HIDD_ATTRIBUTES attributes;
        memset(&attributes, 0, sizeof(attributes));
        attributes.Size = sizeof(attributes);

        HidD_GetAttributes(device.fileHandle, &attributes);
        descriptor.capabilities.vendorId = attributes.VendorID;
        descriptor.capabilities.productId = attributes.ProductID;
        descriptor.version = Format("%i", attributes.VersionNumber);

        // NOTE: We do not use HIDP_BUTTON_CAPS and HIDP_VALUE_CAPS as elements returned from the Windows HID
        //       parser do not come with an offset and do not preserve element ordering as it appears on the device.
        //       Instead, we rely solely on the binary HID descriptors coming straight from the device itself and
        //       perform our own HID parsing in C# (where necessary; for devices that we know, we don't need to do
        //       anything with the descriptors).

        // Report it.
        // Note that we have to get to this point, i.e. manage to actually open the device,
        // in order for us to report it. If we can't access the device, we won't report it
        // to through the API.
        device.deviceId = ReportNewInputDevice(descriptor, &device);
        if (device.inputReportSize > 0)
            device.GetInputStateEvent().deviceId = device.deviceId;
    }

    void HIDInputThreadState::CloseDevice(HIDDevice& device)
    {
        if (device.deviceId != kInvalidInputDeviceId)
            ReportInputDeviceRemoved(device.deviceId);

        // Remove from I/O wait list.
        for (int i = 0; i < devicesForWaitHandles.size(); ++i)
            if (devicesForWaitHandles[i] == &device)
            {
                devicesForWaitHandles.erase(devicesForWaitHandles.begin() + i);
                waitHandles.erase(waitHandles.begin() + i);
            }

        device.ReleaseConnectionState();
    }

    static core::wstring GetDevicePathFromInterfaceData(HDEVINFO deviceList, SP_DEVICE_INTERFACE_DATA& deviceInterfaceData)
    {
        // Query size of device path.
        DWORD sizeofInterfaceDetailData;
        SetupDiGetDeviceInterfaceDetailW(deviceList, &deviceInterfaceData, NULL, 0, &sizeofInterfaceDetailData, NULL);

        // Allocate buffer for device path.
        SP_DEVICE_INTERFACE_DETAIL_DATA_W* interfaceDetailData;
        MALLOC_TEMP_AUTO(interfaceDetailData, sizeofInterfaceDetailData);
        interfaceDetailData->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA_W); // Does not reflect size of variable-size portion.

        // Read device path.
        if (!SetupDiGetDeviceInterfaceDetailW(deviceList, &deviceInterfaceData, interfaceDetailData, sizeofInterfaceDetailData, NULL, NULL))
        {
            printf_console("Failed to retrieve device Path! (%i)\n", GetLastError());
            return core::wstring();
        }

        return core::wstring(interfaceDetailData->DevicePath, kMemInput);
    }

    void HIDInputThreadState::CheckForDeviceAddsAndRemovals()
    {
        GUID hidGUID;
        HidD_GetHidGuid(&hidGUID);

        // Prepare to list devices.
        HDEVINFO deviceList = SetupDiGetClassDevs(&hidGUID, NULL, NULL, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);
        if (deviceList == INVALID_HANDLE_VALUE)
        {
            printf_console("Could not enumerate input devices with SetupDiGetClassDevs! (%i)\n", GetLastError());
            return;
        }

        // Put all devices in unknown state.
        for (int i = 0; i < devices.size(); ++i)
            devices[i]->deviceInstance = 0;

        // Compare the list of devices to what we have stored.
        for (DWORD memberIndex = 0;; ++memberIndex)
        {
            SP_DEVINFO_DATA deviceInfo;
            SP_DEVICE_INTERFACE_DATA deviceInterfaceData;

            memset(&deviceInterfaceData, 0, sizeof(deviceInterfaceData));
            memset(&deviceInfo, 0, sizeof(deviceInfo));

            deviceInterfaceData.cbSize = sizeof(deviceInterfaceData);
            deviceInfo.cbSize = sizeof(deviceInfo);

            if (!SetupDiEnumDeviceInterfaces(deviceList, NULL, &hidGUID, memberIndex, &deviceInterfaceData))
                break;

            // Get device info.
            SetupDiEnumDeviceInfo(deviceList, memberIndex, &deviceInfo);

            // Get device path.
            core::wstring devicePath = GetDevicePathFromInterfaceData(deviceList, deviceInterfaceData);
            if (devicePath.empty())
                continue;

            // Is this a device we already have opened?
            bool isKnownDevice = false;
            for (int i = 0; i < devices.size(); ++i)
            {
                HIDDevice& device = *devices[i];
                if (device.devicePath == devicePath)
                {
                    device.deviceInstance = deviceInfo.DevInst;
                    isKnownDevice = true;
                    break;
                }
            }

            if (isKnownDevice)
            {
                // Yes, device we've already seen. Skip.
                continue;
            }

            // Skip if it's an XInputDevice and we managed to load the XInput DLL.
            if (XInputDevice::s_XInputDLL && XInputDevice::IsXInputDevice(devicePath))
                continue;

            // No, new device. Open it.
            HIDDevice& device = *UNITY_NEW(HIDDevice, kMemInput);
            devices.push_back(&device);
            device.devicePath = devicePath;
            device.deviceInstance = deviceInfo.DevInst;
            OpenDevice(device);
        }

        // Release device set.
        SetupDiDestroyDeviceInfoList(deviceList);

        // Any device that we couldn't correlate to a reported device from the system
        // we now know to have been disconnected.
        for (int i = 0; i < devices.size();)
        {
            HIDDevice* device = devices[i];
            if (!device->deviceInstance)
            {
                CloseDevice(*device);
                UNITY_DELETE(device, kMemInput);
                devices.erase(devices.begin() + i);
            }
            else
            {
                ++i;
            }
        }

        // Refresh XInput devices.
        //
        // We don't and cannot keep track of the correlation of HID and XInput devices (there's no way to tell
        // which XInput HID corresponds to whic XInput user index) so we may end up repeatedly checking for XInput
        // device changes here even though nothing has changed. However, we perform device discovery only when the
        // device setup in the system has actually changed so that should be fine. The important thing is that we
        // don't just blindly to XInputGetState every frame on four controllers even if most or all of them are not
        // connected.
        XInputDevice::CheckXInputDeviceConnectsAndDisconnects();
    }

    static void* HIDInputThread(void*)
    {
        HIDInputThreadState* state = g_HIDInputThreadState.EnsureInitialized();
        while (!state->thread.IsQuitSignaled())
        {
            if (state->suspend)
            {
                // Suspend thread while app is in background.
                state->wakeUpFromSuspend.WaitForSignal();
            }

            ////TODO: deal with MAXIMUM_WAIT_OBJECTS restriction
            ////  (simply put groups of additional devices on their own thread but keep device discovery on first HID thread)

            HANDLE* waitHandles = state->waitHandles.data();
            int numWaitHandles = state->waitHandles.size();

            ////TODO: Probably makes sense to have a timeout of something like 100ms and if it expires, cancel all pending IOs and restart reading of input reports
            ////  Either that or have a solution that times out async reads individually.

            DWORD result = WaitForMultipleObjects(numWaitHandles, waitHandles, FALSE, INFINITE);
            if (result == WAIT_OBJECT_0)
            {
                ResetEvent(waitHandles[0]);
                if (state->thread.IsQuitSignaled())
                    break;
                else
                {
                    // We got a WM_DEVICECHANGE on the main thread. Go sync our list of devices.
                    state->CheckForDeviceAddsAndRemovals();
                }
            }
            else if (result > WAIT_OBJECT_0 && result < (WAIT_OBJECT_0 + numWaitHandles))
            {
                ////REVIEW: is it a better strategy to just blindly check all devices for input any time any single one signals to be ready?
                HIDInputThreadState::HIDDevice* device = state->devicesForWaitHandles[result - WAIT_OBJECT_0];
                device->CheckInputReport();
            }
        }

        return NULL;
    }

    #pragma endregion

    SInt64 NewInput::Keyboard::IOCTL(int code, void* buffer, int bufferSize)
    {
        switch (code)
        {
            // Read name of current keyboard layout.
            case kIOCTLGetKeyboardLayout:
            {
                // While it's called GetKeyboardLayoutName(), what the function really does is just return
                // a hex version of the identifier returned by GetKeyboardLayout(). Returning an actual name
                // seems to require trawling through the registry. Given that we at least return *some*
                // means of identification for the current keyboard layout, we'll leave it at that.
                WCHAR nameBuffer[KL_NAMELENGTH + 1];
                if (!GetKeyboardLayoutNameW(nameBuffer))
                    return kIOCTLFailure;

                IOCTLGetKeyboardLayout* ioctl = reinterpret_cast<IOCTLGetKeyboardLayout*>(buffer);
                size_t nameLength = string_traits<WCHAR*>::get_size(nameBuffer);
                if (ioctl->GetStringBufferSizeInCharacters(bufferSize) < nameLength)
                    return kIOCTLFailure;

                ioctl->stringLength = nameLength;
                memcpy(ioctl->stringBuffer, nameBuffer, nameLength * sizeof(WCHAR));
                return kIOCTLSuccess;
            }

            // Read information about a specific key.
            case kIOCTLGetKeyInfo:
            {
                if (bufferSize < sizeof(IOCTLGetKeyInfo))
                    return kIOCTLFailure;

                // Read and translate key code.
                KeyboardInputState::KeyCode keyCode = *(KeyboardInputState::KeyCode*)buffer;
                if (keyCode >= KeyboardInputState::Count)
                    return kIOCTLFailure;

                int scanCode = NewInput::GetKeyboardMapping().GetScanCode(keyCode);
                if (!scanCode)
                    return kIOCTLFailure;

                IOCTLGetKeyInfo* ioctl = reinterpret_cast<IOCTLGetKeyInfo*>(buffer);
                ioctl->platformKeyCode = scanCode;

                // Translate.
                int stringLength = GetKeyNameTextW(scanCode << 16, (LPWSTR)ioctl->stringBuffer, ioctl->GetStringBufferSizeInCharacters(bufferSize));
                if (stringLength == 0)
                    return kIOCTLFailure;

                ioctl->stringLength = stringLength;

                return kIOCTLSuccess;
            }

            case kIOCTLQueryRunInBackground:
            {
                if (bufferSize != sizeof(bool))
                    return kIOCTLFailure;

                bool* canRunInBackground = static_cast<bool*>(buffer);
                *canRunInBackground = true;

                return kIOCTLSuccess;
            }

            case kIOCTLRequestResetDevice:
            {
                memset(&state.stateData, 0, sizeof(state.stateData));
                state.time = GetCurrentEventTimeInUnityTime();
                QueueInputEvent(state);

                return kIOCTLSuccess;
            }
            case kIOCTLSetIMEMode:
            {
                if (bufferSize < sizeof(char))
                    return kIOCTLFailure;

                char value = *static_cast<char*>(buffer);
                GetInputManager().SetTextFieldInput(value != 0);
                return kIOCTLSuccess;
            }
            case kIOCTLSetIMECursorPosition:
            {
                if (bufferSize < (sizeof(Vector2f)))
                    return kIOCTLFailure;

                GetInputManager().SetTextFieldCursorPos(*(Vector2f*)buffer);
                return kIOCTLSuccess;
            }
        }
        return kIOCTLFailure;
    }

    static Vector2f ScreenCoordinatesToPlayerDisplayCoordinates(float x, float y)
    {
        Vector2f position(x, y);

#if UNITY_EDITOR
        Rectf guiRect, cameraRect;
        Vector2f targetSize;
        bool hasFocus;

        ////FIXME: GetGameView() returns essentially a random view when no game view is focused instead of the last focused game view
        GUIView* gameView = GetGameView();
        RepaintController::GetScreenParamsFromGameView(gameView, false, false, &hasFocus, &guiRect, &cameraRect, &targetSize);

        if (gameView)
        {
            HWND gameWindow = gameView->GetWindowHandle();

            // Unfortunately, Windows makes us lose sub-pixel precision here.
            POINT pointInWindow = { (LONG)x, (LONG)y };
            if (ScreenToClient(gameWindow, &pointInWindow))
            {
                position.x = pointInWindow.x;
                position.y = pointInWindow.y;
            }

            position = DPIScaleHelper(gameWindow).FromNative(position);
            position.x -= guiRect.x;
            position.y = guiRect.height - position.y + guiRect.y;
            position.x *= targetSize.x / guiRect.width;
            position.y *= targetSize.y / guiRect.height;
        }
        else
        {
            // There is no game view so we don't really have a reference coordinate space.
            // We pretend the game view is at (0,0).
        }

#else
        const ScreenManagerWin& screenManager = GetScreenManager();

        POINT pointInWindow = { (LONG)x, (LONG)y };
        if (ScreenToClient(screenManager.GetWindow(), &pointInWindow))
        {
            position.x = pointInWindow.x;
            position.y = pointInWindow.y;
        }

        const RectInt& rect = screenManager.GetRepositionRect();
        const Vector2f& scale = screenManager.GetCoordinateScale();
        position.x = RoundfToInt((position.x - rect.x) * scale.x);
        position.y = RoundfToInt((position.y - rect.y) * scale.y);
        position.y = (screenManager.GetHeight() - position.y - 1);
#endif

        return position;
    }

    static POINT PlayerDisplayCoordinatesToScreenCoordinates(const Vector2f& position)
    {
        POINT result;
#if UNITY_EDITOR
        Rectf guiRect, cameraRect;
        Vector2f targetSize;
        bool hasFocus;

        GUIView* gameView = GetGameView();
        RepaintController::GetScreenParamsFromGameView(gameView, false, false, &hasFocus, &guiRect, &cameraRect, &targetSize);

        // Transform back into screen space.
        if (gameView)
        {
            HWND gameWindow = gameView->GetWindowHandle();

            // Unapply camera rect repositioning and scaling.
            Vector2f newPosition = position;
            newPosition.x /= targetSize.x / guiRect.width;
            newPosition.y /= targetSize.y / guiRect.height;
            newPosition.x += guiRect.x;
            newPosition.y = -(newPosition.y - guiRect.y - guiRect.height);
            newPosition = DPIScaleHelper(gameWindow).ToNative(newPosition);

            // Transform.
            result = { (LONG)newPosition.x, (LONG)newPosition.y };
            ClientToScreen(gameWindow, &result);
        }
        else
        {
            result = { (LONG)position.x, (LONG)position.y };
        }
#else
        const ScreenManagerWin& screenManager = GetScreenManager();
        const RectInt& rect = screenManager.GetRepositionRect();
        const Vector2f& scale = screenManager.GetCoordinateScale();

        float x = position.x;
        float y = screenManager.GetHeight() - position.y - 1;

        x /= scale.x;
        y /= scale.y;
        x += rect.x;
        y += rect.y;

        result = { (LONG)position.x, (LONG)position.y };
        ClientToScreen(screenManager.GetWindow(), &result);
#endif
        return result;
    }

    SInt64 NewInput::Mouse::IOCTL(int code, void* buffer, int size)
    {
        switch (code)
        {
#if UNITY_EDITOR
            // Convert pointer coordinates from game view space into window space of the
            // EditorWindow that is *currently* in a script callback. If we aren't, don't
            // do anything.
            case kIOCTLGetEditorWindowCoordinates:
            {
                if (size < sizeof(Vector2f))
                    return kIOCTLFailure;

                GUIView* currentView = GUIView::GetCurrent();
                if (!currentView || currentView->IsGameView())
                    return kIOCTLFailure;

                Vector2f& position = *static_cast<Vector2f*>(buffer);

                // Transform back into screen space.
                POINT point = PlayerDisplayCoordinatesToScreenCoordinates(position);

                // Transform position to EditorWindow space.
                HWND editorWindow = currentView->GetWindowHandle();
                ScreenToClient(editorWindow, &point);
                position.x = point.x;
                position.y = point.y;

                // Done.
                return sizeof(Vector2f);
            }
#endif

            case kIOCTLWarpMouse:
            {
                if (size < sizeof(Vector2f))
                    return kIOCTLFailure;
                const Vector2f& position = *static_cast<Vector2f*>(buffer);
                POINT screenSpacePosition = PlayerDisplayCoordinatesToScreenCoordinates(position);
                if (!::SetCursorPos(screenSpacePosition.x, screenSpacePosition.y))
                    return kIOCTLFailure;
                return kIOCTLSuccess;
            }

            ////TODO: this will have to be shared with pen and touch
            case kIOCTLDeviceDimensions:
            {
                if (size < sizeof(IOCTLDeviceDimensions))
                    return kIOCTLFailure;

                IOCTLDeviceDimensions& ioctl = *static_cast<IOCTLDeviceDimensions*>(buffer);

#if UNITY_EDITOR
                GUIView* currentView = GUIView::GetCurrent();
                if (!currentView)
                    return kIOCTLFailure;
                if (currentView->IsGameView())
                    ioctl.dimensions = currentView->GetGameViewRect().GetSize();
                else
                    ioctl.dimensions = currentView->GetPosition().GetSize();
#else
                ioctl.dimensions = GetScreenManager().GetRect().GetSize();
#endif
                return kIOCTLSuccess;
            }

            case kIOCTLQueryRunInBackground:
            {
                if (size != sizeof(bool))
                    return kIOCTLFailure;

                bool* canRunInBackground = static_cast<bool*>(buffer);
                *canRunInBackground = true;

                return kIOCTLSuccess;
            }

            case kIOCTLRequestResetDevice:
            {
                memset(&state.stateData, 0, sizeof(state.stateData));
                state.time = GetCurrentEventTimeInUnityTime();
                QueueInputEvent(state);

                return kIOCTLSuccess;
            }
        }
        return kIOCTLFailure;
    }

    SInt64 NewInput::Pen::IOCTL(int code, void* buffer, int size)
    {
        switch (code)
        {
            case kIOCTLRequestResetDevice:
            {
                memset(&state.stateData, 0, sizeof(state.stateData));
                state.time = GetCurrentEventTimeInUnityTime();
                QueueInputEvent(state);

                return kIOCTLSuccess;
            }
        }
        return kIOCTLFailure;
    }

    // Initializze static KeyboardMapping variable
    RuntimeStatic<win::shared::KeyboardMapping> NewInput::s_KeyboardMapping(kMemInput);

    NewInput::NewInput()
        : m_TouchscreenDeviceId(kInvalidInputDeviceId)
        , m_MouseButtonsSwapped(false)
    {
    }

    void NewInput::InitializeKeyboard()
    {
        // Always report a keyboard on Windows.

        InputDeviceDescriptor descriptor;
        descriptor.type = "Keyboard";
        descriptor.interfaceName = "RawInput";

        m_Keyboard.state.deviceId = ReportNewInputDevice(descriptor, &m_Keyboard);
        m_Keyboard.state.sizeInBytes = sizeof(m_Keyboard.state);
        m_Keyboard.state.type = kInputEventState;
        m_Keyboard.state.stateFormat = kInputKeyboardState;
    }

    void NewInput::InitializeMouse()
    {
        // Always report a mouse on Windows.

        MouseDeviceDescriptor descriptor;
        descriptor.type = "Mouse";
        descriptor.interfaceName = "RawInput";
        ////TODO: fill out details for mouse (# buttons and stuff)

        m_Mouse.state.deviceId = ReportNewInputDevice(descriptor, &m_Mouse);
        m_Mouse.state.sizeInBytes = sizeof(m_Mouse.state);
        m_Mouse.state.type = kInputEventState;
        m_Mouse.state.stateFormat = kInputMouseState;
        m_LastClickTime = -1;
        m_LastClickFlags = 0;
    }

    void NewInput::InitializeTouch()
    {
        // Check if system is touch-enabled.
        ////REVIEW: this seems to test tru even on systems that don't have touch support
        int digitizers = GetSystemMetrics(SM_DIGITIZER);
        if (!(digitizers & (NID_INTEGRATED_TOUCH | NID_EXTERNAL_TOUCH)))
            return;

        TouchDeviceDescriptor descriptor;
        descriptor.type = "Touchscreen";
        descriptor.interfaceName = "Win32";
        descriptor.maxTouches = GetSystemMetrics(SM_MAXIMUMTOUCHES);

        ////TODO: support for multiple touch devices

        m_TouchscreenDeviceId = ReportNewInputDevice(descriptor);
    }

    void NewInput::InitializePen()
    {
        // See if the new pointer input API is present on the system.
        if (!win::InitializePointerInputAPI())
            return;

        // Look for a pen pointer devices.
        win::POINTER_DEVICE_INFO penDeviceInfo;
        if (!win::GetPenPointerDeviceInfo(penDeviceInfo))
            return;

        ////TODO: support multiple pens

        StylusDeviceDescriptor descriptor;
        descriptor.type = "Pen";
        descriptor.interfaceName = "Win32";
        ConvertWideToUTF8String(penDeviceInfo.productString, descriptor.product);
        descriptor.pressureLevels = 1024;
        ////TODO: find out actual pen device capabilities (somehow...)

        m_Pen.state.deviceId = ReportNewInputDevice(descriptor, &m_Pen);
        m_Pen.state.sizeInBytes = sizeof(m_Pen.state);
        m_Pen.state.type = kInputEventState;
        m_Pen.state.stateFormat = kInputPenState;
    }

    void NewInput::RegisterRawInput(HWND window)
    {
        const int deviceCount = 2;
        RAWINPUTDEVICE devices[deviceCount];

        RAWINPUTDEVICE &mouse = devices[0];
        mouse.usUsagePage = HID_USAGE_PAGE_GENERIC;
        mouse.usUsage = HID_USAGE_GENERIC_MOUSE;
        mouse.dwFlags = NULL;
        mouse.hwndTarget = window;

        RAWINPUTDEVICE &keyboard = devices[1];
        keyboard.usUsagePage = HID_USAGE_PAGE_GENERIC;
        keyboard.usUsage = HID_USAGE_GENERIC_KEYBOARD;
        keyboard.dwFlags = NULL;
        keyboard.hwndTarget = window;

        if (!RegisterRawInputDevices(devices, deviceCount, sizeof(RAWINPUTDEVICE)))
            printf_console("Could not register mouse and keyboard for Raw Input!\n");
    }

    bool NewInput::Open(HWND window)
    {
        InitializeNewInputSystem();

        InitializeKeyboard();
        InitializeMouse();
        InitializeTouch();
        InitializePen();

        XInputDevice::LoadXInputLibrary();

        SetBlockBackgroundThreadsFromFeedingInputEvents(false);
        g_HIDInputThreadState.EnsureInitialized();
        g_HIDInputThreadState->thread.SetName("HIDInput");
        g_HIDInputThreadState->thread.Run(&HIDInputThread, NULL);

        InitWindowsEventTime();

        RegisterRawInput(window);

        return true;
    }

    void NewInput::Close(void)
    {
        // Make sure our HID input thread is not caught in a loop trying to put something
        // in an already full background event queue.
        SetBlockBackgroundThreadsFromFeedingInputEvents(true);

        g_HIDInputThreadState.EnsureInitialized();
        g_HIDInputThreadState->thread.SignalQuit();
        g_HIDInputThreadState->suspend = false;
        g_HIDInputThreadState->wakeUpFromSuspend.Signal();
        SetEvent(g_HIDInputThreadState->checkDeviceListOrExit);
        g_HIDInputThreadState->thread.WaitForExit();

        ShutdownNewInputSystem();

        XInputDevice::UnloadXInputLibrary();
    }

    bool NewInput::Activate(bool active)
    {
        HIDInputThreadState* inputThreadState = g_HIDInputThreadState.EnsureInitialized();
        if (active)
        {
            // Check if the mouse buttons are swapped during activation
            m_MouseButtonsSwapped = static_cast<bool>(GetSystemMetrics(SM_SWAPBUTTON));

            inputThreadState->suspend = false;
            inputThreadState->wakeUpFromSuspend.Signal();
        }
        else
        {
            inputThreadState->wakeUpFromSuspend.ResetAndReleaseWaitingThreads();
            inputThreadState->suspend = true;
        }
        return true;
    }

    bool NewInput::ToggleFullscreen(bool fullscreen, HWND window)
    {
        RegisterRawInput(window);
        return true;
    }

    void NewInput::SetKeyboardIMEIsSelected(bool isIMESelected)
    {
        bool wasIMESelected = m_Keyboard.state.stateData.GetKey(KeyboardInputState::KeyCode::IMESelected);

        // If we've got a state change, update and notify managed.
        if (isIMESelected != wasIMESelected)
        {
            m_Keyboard.state.stateData.SetKey(KeyboardInputState::KeyCode::IMESelected, isIMESelected);
            m_Keyboard.state.time = GetCurrentEventTimeInUnityTime();
            QueueInputEvent(m_Keyboard.state);
        }
    }

    LRESULT NewInput::OnMessage(HWND window, UINT message, WPARAM wParam, LPARAM lParam, BOOL& handled)
    {
        // The old input system is setup to handle mouse events even when not in focus
        // We need to manually exclude those until that system is removed
#if UNITY_EDITOR
        bool isFocused = GetApplication().IsFocused();
#else
        bool isFocused = IsPlayerFocused();
#endif

        // Mouse moves and button presses are handled via Raw Input, so we disregard the normal
        // mouse-related messages except for driving client window functionality

        switch (message)
        {
            case WM_INPUT:
                OnInput(window, message, wParam, lParam, isFocused);
                break;

            // We'd really want UTF-32 input which WM_UNICHAR gives but apparently that is sent
            // only to "ANSI Windows"....
            case WM_CHAR:
                if (isFocused)
                    OnText(window, message, wParam, lParam);
                break;

            case WM_TOUCH:
                if (isFocused)
                    OnTouch(window, message, wParam, lParam);
                break;

            // Stylus input (Win8+ only). While the pen isn't touching the surface, we only receive
            // position updates while the stylus is over our focused window (this includes individual
            // EditorWindows).
            case WM_POINTERUPDATE:
            case WM_POINTERDOWN:
            case WM_POINTERUP:
                OnPointer(window, message, wParam, lParam);
                break;

            case WM_LBUTTONDOWN:
            case WM_RBUTTONDOWN:
            case WM_MBUTTONDOWN:
            case WM_XBUTTONDOWN:
                if (isFocused)
                    SetMouseCapture(window);
                break;

            case WM_LBUTTONUP:
            case WM_RBUTTONUP:
            case WM_MBUTTONUP:
            case WM_XBUTTONUP:
                if (isFocused)
                    ReleaseMouseCapture();
                break;

            case WM_CAPTURECHANGED:
                ResetMouseCapture();
                break;

            // GetInput.cpp is doing the RegisterDeviceNotification call that we depend on for this
            // to work (see InputInitWindow).
            case WM_DEVICECHANGE:
                SetEvent(g_HIDInputThreadState->checkDeviceListOrExit);
                break;

            case WM_INPUTLANGCHANGE:
                ReportInputDeviceConfigurationChanged(m_Keyboard.state.deviceId);
                break;

            case WM_SETTINGCHANGE:
                // Check if the mouse buttons have swapped if notified that system settings have changed
                m_MouseButtonsSwapped = static_cast<bool>(GetSystemMetrics(SM_SWAPBUTTON));
                break;

            case WM_MOUSEMOVE:
                // Need to notify ScreenManager the mouse cursor is inside the app Window so Cursors are updated properly
                // Must also call TrackMouseEvent to ensure we receive a WM_MOUSELEAVE message
                if (!GetScreenManager().GetCursorInsideWindow())
                {
                    TRACKMOUSEEVENT tme = { sizeof(TRACKMOUSEEVENT) };
                    tme.dwFlags = TME_LEAVE;
                    tme.hwndTrack = window;
                    TrackMouseEvent(&tme);

                    GetScreenManager().SetCursorInsideWindow(true);
                }
                break;

            case WM_MOUSELEAVE:
            {
                #if !UNITY_EDITOR
                GetScreenManager().SetCursorInsideWindow(false);
                #endif
            }
            break;
        }

        return 0;
    }

    void NewInput::OnTouch(HWND window, UINT message, WPARAM wParam, LPARAM lParam)
    {
        TOUCHINPUT buffer[16];
        TOUCHINPUT* touches = buffer;

        UINT touchCount = LOWORD(wParam);
        if (touchCount > TouchscreenInputState::kMaxTouches)
            touchCount = TouchscreenInputState::kMaxTouches;

        HTOUCHINPUT touchHandle = reinterpret_cast<HTOUCHINPUT>(lParam);
        const double time = GetCurrentEventTimeInUnityTime();

        if (WinGetUserTouchInputInfo(touchHandle, touchCount, touches, sizeof(TOUCHINPUT)))
        {
            for (int i = 0; i < touchCount; ++i)
            {
                TOUCHINPUT& touch = touches[i];
                ////TODO: process time field from touch
                StateInputEventData<TouchInputState> touchEvent(m_TouchscreenDeviceId, time, kInputTouchState);
                TouchInputState& touchState = touchEvent.stateData;

                if (touch.dwFlags & TOUCHEVENTF_MOVE)
                    touchState.phase = TouchInputState::kMoved;
                else if (touch.dwFlags & TOUCHEVENTF_UP)
                    touchState.phase = TouchInputState::kEnded;
                else if (touch.dwFlags & TOUCHEVENTF_DOWN)
                    touchState.phase = TouchInputState::kBegan;
                else
                    continue; // Ignore palm touches.

                touchState.touchId = touch.dwID;

                Vector2f pos = ScreenCoordinatesToPlayerDisplayCoordinates(touch.x / 100.f, touch.y / 100.f);
                touchState.positionX = pos.x;
                touchState.positionY = pos.y;
                touchState.deltaX = 0;
                touchState.deltaY = 0;

                if (touch.dwMask & TOUCHINPUTMASKF_CONTACTAREA)
                {
                    touchState.radiusX = float(touch.cxContact) / 100.f;
                    touchState.radiusY = float(touch.cyContact) / 100.f;
                }

                QueueInputEvent(touchEvent);
            }
        }

        WinCloseUserTouchInputHandle(touchHandle);
    }

    void NewInput::OnInput(HWND window, UINT message, WPARAM wParam, LPARAM lParam, bool isFocused)
    {
        if (wParam != RIM_INPUTSINK && !isFocused)
            return;

        char buffer[sizeof(RAWINPUT)]; // We only expect RIM_TYPEMOUSE events so start with a fixed size buffer.

        HRAWINPUT rawInputHandle = reinterpret_cast<HRAWINPUT>(lParam);
        char* rawInputBuffer = buffer;
        UINT sizeofRawInputBuffer = sizeof(buffer);

        UINT result = GetRawInputData(rawInputHandle, RID_INPUT, rawInputBuffer, &sizeofRawInputBuffer, sizeof(RAWINPUTHEADER));
        if (result == -1 && GetLastError() == ERROR_INSUFFICIENT_BUFFER)
        {
            // Need a larger buffer. Shouldn't happen given we only expect mouse data but just in case.
            if (GetRawInputData(rawInputHandle, RID_INPUT, NULL, &sizeofRawInputBuffer, sizeof(RAWINPUTHEADER)) != 0)
                return;
            rawInputBuffer = static_cast<char*>(alloca(sizeofRawInputBuffer));
            result = GetRawInputData(rawInputHandle, RID_INPUT, rawInputBuffer, &sizeofRawInputBuffer, sizeof(RAWINPUTHEADER));
        }

        if (result == -1)
            return;

        RAWINPUT& rawInput = *reinterpret_cast<RAWINPUT*>(rawInputBuffer);
        if (rawInput.header.dwType == RIM_TYPEMOUSE)
        {
            OnMouse(rawInput.data.mouse);
        }
        else if (rawInput.header.dwType == RIM_TYPEKEYBOARD)
        {
            OnKey(rawInput.data.keyboard);
        }
    }

    void NewInput::OnText(HWND window, UINT message, WPARAM wParam, LPARAM lParam)
    {
        WCHAR character = (WCHAR)wParam;
        if (character == UNICODE_NOCHAR)
            return;

        QueueTextInputEvent(kInputEventText, m_Keyboard.state.deviceId, GetCurrentEventTimeInUnityTime(), character);
    }

    void NewInput::OnPointer(HWND window, UINT message, WPARAM wParam, LPARAM lParam)
    {
        if (!win::Win32GetCurrentInputMessageSource)
            return;

        win::INPUT_MESSAGE_SOURCE messageSource;
        if (!win::Win32GetCurrentInputMessageSource(&messageSource))
            return;

        // Pen input. Touch and mouse go through different APIs so this should be the only
        // thing that we need to process here.
        if (messageSource.deviceType == IMDT_PEN)
        {
            if (m_Pen.state.deviceId == kInvalidInputDeviceId)
                return;

            m_Pen.state.time = GetCurrentEventTimeInUnityTime(); ////TODO: process time from penInfo

            SHORT x = (lParam & 0x0000ffff);
            SHORT y = (lParam & 0xffff0000) >> 16;

            Vector2f pos = ScreenCoordinatesToPlayerDisplayCoordinates(x, y);
            m_Pen.state.stateData.positionX = pos.x;
            m_Pen.state.stateData.positionY = pos.y;

            win::POINTER_PEN_INFO penInfo;
            UInt32 windowsPointerId = wParam & 0xffff;
            const bool penInfoValid = win::Win32GetPointerPenInfo(windowsPointerId, &penInfo);
            if (penInfoValid)
            {
                m_Pen.state.stateData.pressure = penInfo.pressure / 1024.0f;
                m_Pen.state.stateData.twist = penInfo.rotation / 359.0f;
                m_Pen.state.stateData.tiltX = penInfo.tiltX / 90.0f;
                m_Pen.state.stateData.tiltY = penInfo.tiltY / 90.0f;

                pos = ScreenCoordinatesToPlayerDisplayCoordinates(penInfo.pointerInfo.ptPixelLocation.x, penInfo.pointerInfo.ptPixelLocation.y);
                m_Pen.state.stateData.positionX = pos.x;
                m_Pen.state.stateData.positionY = pos.y;

                // Documentation on these APIs is poor. What does PEN_FLAG_INVERTED mean?
                // If it means the pen is flipped, how is it different from PEN_FLAG_ERASER?

                // Windows (apparently) assigns the following meaning to pen buttons:
                // - First: pen tip *or* eraser (check PEN_FLAG_ERASER)
                // - Second: pen barrel button

                m_Pen.state.stateData.SetButton(PenInputState::kEraserButton, penInfo.penFlags & PEN_FLAG_ERASER);
                if (!(penInfo.penFlags & PEN_FLAG_ERASER))
                {
                    if (penInfo.pointerInfo.ButtonChangeType == POINTER_CHANGE_FIRSTBUTTON_DOWN)
                        m_Pen.state.stateData.SetButton(PenInputState::kTipButton, true);
                    else if (penInfo.pointerInfo.ButtonChangeType == POINTER_CHANGE_FIRSTBUTTON_UP)
                        m_Pen.state.stateData.SetButton(PenInputState::kTipButton, false);
                }

                if (penInfo.pointerInfo.ButtonChangeType == POINTER_CHANGE_SECONDBUTTON_DOWN)
                    m_Pen.state.stateData.SetButton(PenInputState::kFirstBarrelButton, true);
                else if (penInfo.pointerInfo.ButtonChangeType == POINTER_CHANGE_SECONDBUTTON_UP)
                    m_Pen.state.stateData.SetButton(PenInputState::kFirstBarrelButton, false);
                else if (penInfo.pointerInfo.ButtonChangeType == POINTER_CHANGE_THIRDBUTTON_DOWN)
                    m_Pen.state.stateData.SetButton(PenInputState::kSecondBarrelButton, true);
                else if (penInfo.pointerInfo.ButtonChangeType == POINTER_CHANGE_THIRDBUTTON_UP)
                    m_Pen.state.stateData.SetButton(PenInputState::kSecondBarrelButton, false);
                else if (penInfo.pointerInfo.ButtonChangeType == POINTER_CHANGE_FOURTHBUTTON_DOWN)
                    m_Pen.state.stateData.SetButton(PenInputState::kThirdBarrelButton, true);
                else if (penInfo.pointerInfo.ButtonChangeType == POINTER_CHANGE_FOURTHBUTTON_UP)
                    m_Pen.state.stateData.SetButton(PenInputState::kThirdBarrelButton, false);
                else if (penInfo.pointerInfo.ButtonChangeType == POINTER_CHANGE_FIFTHBUTTON_DOWN)
                    m_Pen.state.stateData.SetButton(PenInputState::kFourthBarrelButton, true);
                else if (penInfo.pointerInfo.ButtonChangeType == POINTER_CHANGE_FIFTHBUTTON_UP)
                    m_Pen.state.stateData.SetButton(PenInputState::kFourthBarrelButton, false);


                m_Pen.state.stateData.SetButton(PenInputState::kInRangeButton, penInfo.pointerInfo.pointerFlags & POINTER_FLAG_INRANGE);
            }

            QueueInputEvent(m_Pen.state);
        }
    }

    void NewInput::OnMouse(const RAWMOUSE &data)
    {
        // Stylus and touch input will not come down this API. When we're here, we're dealing with mouse input only.

        m_Mouse.state.time = GetCurrentEventTimeInUnityTime();

        const DWORD messagePos = GetMessagePos();

        int x = GET_X_LPARAM(messagePos);
        int y = GET_Y_LPARAM(messagePos);

        Vector2f pos = ScreenCoordinatesToPlayerDisplayCoordinates(x, y);

        if (data.usFlags == MOUSE_MOVE_RELATIVE)
        {
            m_Mouse.state.stateData.deltaX = data.lLastX;
            m_Mouse.state.stateData.deltaY = -data.lLastY;
        }
        else
        {
            // Absolute mouse position, we need to calculate the delta from the previous position.
            // This will happen when using Remote Desktop (RDP) as the mouse position will always be absolute when sent over RDP. (case 1224897)
            m_Mouse.state.stateData.deltaX = pos.x - m_Mouse.state.stateData.positionX;
            m_Mouse.state.stateData.deltaY = pos.y - m_Mouse.state.stateData.positionY;
        }

        m_Mouse.state.stateData.positionX = pos.x;
        m_Mouse.state.stateData.positionY = pos.y;

        ////TODO: support for multiple displays

        if (data.usButtonFlags & RI_MOUSE_LEFT_BUTTON_DOWN)
            m_Mouse.state.stateData.buttons |= m_MouseButtonsSwapped ? MouseInputState::kRightButton : MouseInputState::kLeftButton;
        if (data.usButtonFlags & RI_MOUSE_LEFT_BUTTON_UP)
            m_Mouse.state.stateData.buttons &= m_MouseButtonsSwapped ? ~MouseInputState::kRightButton : ~MouseInputState::kLeftButton;
        if (data.usButtonFlags & RI_MOUSE_RIGHT_BUTTON_DOWN)
            m_Mouse.state.stateData.buttons |= m_MouseButtonsSwapped ? MouseInputState::kLeftButton : MouseInputState::kRightButton;
        if (data.usButtonFlags & RI_MOUSE_RIGHT_BUTTON_UP)
            m_Mouse.state.stateData.buttons &= m_MouseButtonsSwapped ? ~MouseInputState::kLeftButton : ~MouseInputState::kRightButton;
        if (data.usButtonFlags & RI_MOUSE_MIDDLE_BUTTON_DOWN)
            m_Mouse.state.stateData.buttons |= MouseInputState::kMiddleButton;
        if (data.usButtonFlags & RI_MOUSE_MIDDLE_BUTTON_UP)
            m_Mouse.state.stateData.buttons &= ~MouseInputState::kMiddleButton;
        if (data.usButtonFlags & RI_MOUSE_BUTTON_4_DOWN)
            m_Mouse.state.stateData.buttons |= MouseInputState::kBackButton;
        if (data.usButtonFlags & RI_MOUSE_BUTTON_4_UP)
            m_Mouse.state.stateData.buttons &= ~MouseInputState::kBackButton;
        if (data.usButtonFlags & RI_MOUSE_BUTTON_5_DOWN)
            m_Mouse.state.stateData.buttons |= MouseInputState::kForwardButton;
        if (data.usButtonFlags & RI_MOUSE_BUTTON_5_UP)
            m_Mouse.state.stateData.buttons &= ~MouseInputState::kForwardButton;


        // Vertical wheel.
        if (data.usButtonFlags & RI_MOUSE_WHEEL)
            m_Mouse.state.stateData.scrollY = (SHORT)data.usButtonData;

        // Horizontal wheel.
        if (data.usButtonFlags & RI_MOUSE_HWHEEL)
            m_Mouse.state.stateData.scrollX = (SHORT)data.usButtonData;


        if (data.usButtonFlags & (RI_MOUSE_LEFT_BUTTON_DOWN | RI_MOUSE_RIGHT_BUTTON_DOWN | RI_MOUSE_MIDDLE_BUTTON_DOWN | RI_MOUSE_BUTTON_4_DOWN | RI_MOUSE_BUTTON_5_DOWN))
        {
            if (m_LastClickFlags == data.usButtonFlags && m_Mouse.state.time - m_LastClickTime <= GetDoubleClickTime() * 0.001)
                m_Mouse.state.stateData.clickCount++;
            else
                m_Mouse.state.stateData.clickCount = 1;
            m_LastClickTime = m_Mouse.state.time;
            m_LastClickFlags = data.usButtonFlags;
        }
        QueueInputEvent(m_Mouse.state);

        // The scroll value comes from the button data on Windows, but we do not
        // pass it up as a button.  Due to this, we need to make sure that if that button
        // flag has not been triggered,
        // clear out the state or stale scroll values are sent on every mouse event.

        m_Mouse.state.stateData.scrollX = 0;
        m_Mouse.state.stateData.scrollY = 0;
        m_Mouse.state.stateData.deltaX = 0;
        m_Mouse.state.stateData.deltaY = 0;
    }

    void NewInput::OnKey(const RAWKEYBOARD &data)
    {
        // RAWINPUT splits the Pause key's long make code between two individual messages
        // We'll handle Pause using the 1st event and ignore the second message
        static int lastScanCode = 0;
        if (lastScanCode == 0x1D && data.MakeCode == 0x45)
        {
            lastScanCode = data.MakeCode;
            return;
        }
        lastScanCode = data.MakeCode;

        // NOTE: This will get key repeats as well (raw keyboard input on Windows isn't all that raw).

        const bool keyPressed = (data.Flags & RI_KEY_BREAK) == 0;
        const bool alternate = (data.Flags & RI_KEY_E0) != 0;

        const KeyboardInputState::KeyCode key = s_KeyboardMapping->GetKeyCode(data.MakeCode, alternate, data.VKey);
        if (key != KeyboardInputState::None)
        {
            // Ignore key repeats.
            // NOTE: You'll still see repeats on the text events, of course.
            if (keyPressed && m_Keyboard.state.stateData.GetKey(key))
                return;

            m_Keyboard.state.stateData.SetKey(key, keyPressed);
            m_Keyboard.state.time = GetCurrentEventTimeInUnityTime();
            QueueInputEvent(m_Keyboard.state);
        }
    }
} // namespace win
