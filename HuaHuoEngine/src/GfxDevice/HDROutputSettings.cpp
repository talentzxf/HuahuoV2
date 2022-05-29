//#include "HDROutputSettings.h"
////#include "Runtime/Graphics/ScreenManager.h"
////#include "Runtime/Misc/PlayerSettings.h"
//#include "Shaders/GraphicsCaps.h"
////#include "Utilities/LogUtility.h"
////#include "Runtime/Scripting/ScriptingExportUtility.h"
//
//// Default value from the original implementation on desktop / XB1
//const float HDROutputSettings::DefaultPaperWhiteInNits = 160.0f;
//
//HDROutputSettings::HDROutputSettings()
//    : m_Active(false)
//    , m_Available(false)
//    , m_AutomaticToneMapping(true)
//    , m_DisplayColorGamut(kColorGamutSRGB)
//    , m_GraphicsFormat(kFormatNone)
//    , m_PaperWhiteInNits(DefaultPaperWhiteInNits)
//    , m_MaxFullFrameToneMapLuminance(-1)
//    , m_MaxToneMapLuminance(-1)
//    , m_MinToneMapLuminance(-1)
//    , m_HDRModeChangeRequested(false)
//    // We always start with HDR active if the display and project both support it
//    , m_DesiredHDRActive(true)
//{
//    ResetDisplayChromacity(m_DisplayColorGamut);
//}
//
//void HDROutputSettings::ResetDisplayChromacity(ColorGamut displayColorGamut)
//{
//    static const DisplayChromacities DisplayChromacityList[] =
//    {
//        {   0.64000f, 0.33000f,
//            0.30000f, 0.60000f,
//            0.15000f, 0.06000f,
//            0.31270f, 0.32900f
//        }, // Display Gamut Rec709
//        {   0.70800f, 0.29200f,
//            0.17000f, 0.79700f,
//            0.13100f, 0.04600f,
//            0.31270f, 0.32900f
//        } // Display Gamut Rec2020
//    };
//
//    // use Rec709 by default, precise values queried later from the display on supported platforms
//    m_DisplayChromacities = DisplayChromacityList[0];
//}
//
//void HDROutputSettings::RequestHDRModeChange(bool enabled)
//{
//    if (!HasFlag(GetGraphicsCaps().hdrDisplaySupportFlags, kHDRDisplaySupportFlagsRuntimeSwitchable))
//    {
//        ErrorString("Cannot switch in or out of HDR mode at runtime if the platform doesn't support it.");
//        return;
//    }
//
//    if (enabled != GetActive() && !GetHDRModeChangeRequested())
//    {
//        SetHDRModeChangeRequested(true);
//        SetDesiredHDRActive(enabled);
//
//        // The actual implementation deals with actual toggle value logic
//        GetScreenManager().RequestHDRModeChange(enabled);
//    }
//}
//
//void HDROutputSettings::ValidateLuminances()
//{
//    // Maximum brightness of Rec2020.
//    const int maxHDRBrightness = 10000;
//
//    // Set the maximum luminance values to -1 if the value read from the TV/display
//    // is higher than the maximum brightness achievable in Rec2020
//    // or if the value is not known.
//    if (m_MaxFullFrameToneMapLuminance > maxHDRBrightness || m_MaxFullFrameToneMapLuminance == 0)
//    {
//        m_MaxFullFrameToneMapLuminance = -1;
//    }
//
//    // If HDR display support is not really enabled, suppress validation messages
//    bool useHDRDisplay = GetPlayerSettingsPtr() && GetPlayerSettings().GetUseHDRDisplay();
//    if (useHDRDisplay && m_MaxFullFrameToneMapLuminance == -1)
//    {
//        LogRepeatingWarningString("Unable to determine Max Full Frame Tonemap Luminance from display.");
//    }
//
//    if (m_MaxToneMapLuminance > maxHDRBrightness || m_MaxToneMapLuminance == 0)
//    {
//        m_MaxToneMapLuminance = -1;
//    }
//
//    if (useHDRDisplay && m_MaxToneMapLuminance == -1)
//    {
//        LogRepeatingWarningString("Unable to determine Max Tonemap Luminance from display");
//    }
//
//    //On DX platforms we get zero back for min brightness if the min brightness is zero OR if an error occurred getting the values from the display
//    //Take a guess here as to which of these two scenarios occurred based on whether or not we managed to get a sensible value for the max luminance.
//    if (m_MinToneMapLuminance > maxHDRBrightness || (m_MinToneMapLuminance == 0 && m_MaxToneMapLuminance == -1))
//    {
//        m_MinToneMapLuminance = -1;
//    }
//
//    if (useHDRDisplay && m_MinToneMapLuminance == -1)
//    {
//        LogRepeatingWarningString("Unable to determine Min Tonemap Luminance from display");
//    }
//}
//
//bool HDROutputSettings::GetAvailable() const
//{
//    if (!HasAnyFlags(GetGraphicsCaps().hdrDisplaySupportFlags, kHDRDisplaySupportFlagsSupported))
//        return false;
//
//    return m_Available && GetPlayerSettingsPtr() && GetPlayerSettings().GetUseHDRDisplay();
//}
//
//bool HDROutputSettingsBindings::GetActive(int displayIndex, ScriptingExceptionPtr* exception)
//{
//    HDROutputSettings* settings = GetHDROutputSettingsThrowIfNull(displayIndex, exception);
//
//    if (settings != nullptr)
//        return settings->GetActive();
//
//    return false;
//}
//
//bool HDROutputSettingsBindings::GetAvailable(int displayIndex, ScriptingExceptionPtr* exception)
//{
//    HDROutputSettings* settings = GetHDROutputSettingsThrowIfNull(displayIndex, exception);
//
//    if (settings == nullptr)
//        return false;
//
//    return settings->GetAvailable();
//}
//
//bool HDROutputSettingsBindings::GetAutomaticHDRTonemapping(int displayIndex, ScriptingExceptionPtr* exception)
//{
//    HDROutputSettings* settings = GetHDROutputSettingsThrowIfNull(displayIndex, exception);
//
//    if (settings != nullptr)
//        return settings->GetAutomaticHDRTonemapping();
//
//    return false;
//}
//
//void HDROutputSettingsBindings::SetAutomaticHDRTonemapping(int displayIndex, bool enabled, ScriptingExceptionPtr* exception)
//{
//    if (enabled && !HasAnyFlags(GetGraphicsCaps().hdrDisplaySupportFlags, kHDRDisplaySupportFlagsAutomaticTonemapping))
//    {
//        ErrorString("Cannot enable automatic HDR tonemapping if the platform doesn't support it.");
//        return;
//    }
//
//    HDROutputSettings* settings = GetHDROutputSettingsThrowIfNotAvailable(displayIndex, exception);
//
//    if (settings != nullptr)
//        settings->SetAutomaticHDRTonemapping(enabled);
//}
//
//ColorGamut HDROutputSettingsBindings::GetDisplayColorGamut(int displayIndex, ScriptingExceptionPtr* exception)
//{
//    HDROutputSettings* settings = GetHDROutputSettingsThrowIfNotAvailable(displayIndex, exception);
//
//    if (settings != nullptr)
//        return settings->GetDisplayColorGamut();
//
//    return kColorGamutSRGB;
//}
//
//GraphicsFormat HDROutputSettingsBindings::GetGraphicsFormat(int displayIndex, ScriptingExceptionPtr* exception)
//{
//    HDROutputSettings* settings = GetHDROutputSettingsThrowIfNotAvailable(displayIndex, exception);
//
//    if (settings != nullptr)
//        return settings->GetGraphicsFormat();
//
//    return GraphicsFormat::kFormatNone;
//}
//
//float HDROutputSettingsBindings::GetPaperWhiteNits(int displayIndex, ScriptingExceptionPtr* exception)
//{
//    HDROutputSettings* settings = GetHDROutputSettingsThrowIfNotAvailable(displayIndex, exception);
//
//    if (settings != nullptr)
//        return settings->GetPaperWhiteInNits();
//
//    return HDROutputSettings::DefaultPaperWhiteInNits;
//}
//
//void HDROutputSettingsBindings::SetPaperWhiteNits(int displayIndex, float paperWhite, ScriptingExceptionPtr* exception)
//{
//    HDROutputSettings* settings = GetHDROutputSettingsThrowIfNotAvailable(displayIndex, exception);
//
//    if (settings != nullptr)
//        settings->SetPaperWhiteInNits(paperWhite);
//}
//
//int HDROutputSettingsBindings::GetMaxFullFrameToneMapLuminance(int displayIndex, ScriptingExceptionPtr* exception)
//{
//    HDROutputSettings* settings = GetHDROutputSettingsThrowIfNotAvailable(displayIndex, exception);
//
//    if (settings != nullptr)
//        return settings->GetMaxFullFrameToneMapLuminance();
//
//    return -1;
//}
//
//int HDROutputSettingsBindings::GetMaxToneMapLuminance(int displayIndex, ScriptingExceptionPtr* exception)
//{
//    HDROutputSettings* settings = GetHDROutputSettingsThrowIfNotAvailable(displayIndex, exception);
//
//    if (settings != nullptr)
//        return settings->GetMaxToneMapLuminance();
//
//    return -1;
//}
//
//int HDROutputSettingsBindings::GetMinToneMapLuminance(int displayIndex, ScriptingExceptionPtr* exception)
//{
//    HDROutputSettings* settings = GetHDROutputSettingsThrowIfNotAvailable(displayIndex, exception);
//
//    if (settings != nullptr)
//        return settings->GetMinToneMapLuminance();
//
//    return -1;
//}
//
//bool HDROutputSettingsBindings::GetHDRModeChangeRequested(int displayIndex, ScriptingExceptionPtr* exception)
//{
//    HDROutputSettings* settings = GetHDROutputSettingsThrowIfNotAvailable(displayIndex, exception);
//
//    if (settings != nullptr)
//        return settings->GetHDRModeChangeRequested();
//
//    return false;
//}
//
//void HDROutputSettingsBindings::RequestHDRModeChange(int displayIndex, bool enabled, ScriptingExceptionPtr* exception)
//{
//    HDROutputSettings* settings = GetHDROutputSettingsThrowIfNotAvailable(displayIndex, exception);
//
//    if (settings != nullptr)
//        settings->RequestHDRModeChange(enabled);
//}
//
//HDROutputSettings* HDROutputSettingsBindings::GetHDROutputSettingsThrowIfNull(int displayIndex, ScriptingExceptionPtr* exception)
//{
//    UNUSED(displayIndex);
//
//    HDROutputSettings* ret = GetScreenManager().GetHDROutputSettings();
//
//#if !UNITY_EDITOR
//    if (ret == nullptr)
//    {
//        if (exception)
//            *exception = Scripting::CreateInvalidOperationException("Cannot obtain the HDROutputSettings as HDR displays are not supported on this platform.");
//
//        return nullptr;
//    }
//#endif
//
//    return ret;
//}
//
//HDROutputSettings* HDROutputSettingsBindings::GetHDROutputSettingsThrowIfNotAvailable(int displayIndex, ScriptingExceptionPtr* exception)
//{
//    HDROutputSettings* ret = GetHDROutputSettingsThrowIfNull(displayIndex, exception);
//
//    if (ret == nullptr)
//        return nullptr;
//
//    if (!ret->GetAvailable())
//    {
//        if (exception)
//        {
//            if (!GetPlayerSettings().GetUseHDRDisplay())
//            {
//                *exception = Scripting::CreateInvalidOperationException("Cannot obtain information from an HDR display. HDR is not enabled in the player settings for this project.");
//            }
//            else
//            {
//                *exception = Scripting::CreateInvalidOperationException("Cannot obtain information from an HDR display, check that one is available via HDROutputSettings::available before trying to retrieve data from it.");
//            }
//        }
//    }
//
//    return ret;
//}
