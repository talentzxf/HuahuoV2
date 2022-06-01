#if PLATFORM_WIN
#include "PlatformDependent/WinPlayer/WinMain.h"
#endif

#if PLATFORM_IOS || PLATFORM_TVOS
#include "PlatformDependent/iPhonePlayer/TrampolineInterface.h"
#endif

#if PLATFORM_ANDROID
#include "PlatformDependent/AndroidPlayer/Source/DisplayInfo.h"
#include "PlatformDependent/AndroidPlayer/Source/SwappyWrapper.h"
#endif

static int s_TargetFrameRate = -1;
int GetTargetFrameRate() { return s_TargetFrameRate; }
void SetTargetFrameRateDirect(int value) { s_TargetFrameRate = value; }

void SetTargetFrameRate(int target)
{
    SetTargetFrameRateDirect(target);
#if PLATFORM_ANDROID
    Swappy::UpdateSwapInterval();
#endif
#if PLATFORM_IOS || PLATFORM_TVOS
    UnityFramerateChangeCallback(GetTargetFrameRate());
#endif
}

//static float GetSyncedRefreshRate(int vSyncCount)
//{
//    // return target frame-rate when vsync is off
//    if (vSyncCount <= 0)
//        return GetTargetFrameRate();
//#if PLATFORM_ANDROID
//    const float defaultRefreshRate = DisplayInfo::GetDefaultDisplayInfo().refreshRate;
//#else
//    const float defaultRefreshRate = 60;
//#endif
//    int refreshRate = GetScreenManager().GetCurrentResolution().refreshRate;
//
//    // Can be a fraction of an odd refresh rate like 59 so we can't use int
//    return float(refreshRate > 0 ? refreshRate : defaultRefreshRate) / vSyncCount;
//}

//int GetWantedVSyncCount()
//{
//#if ENABLE_UNIT_TESTS_WITH_FAKES
//    __FAKEABLE_FUNCTION_OVERLOADED__(GetWantedVSyncCount, (), int());
//#endif
//    // Note: we treat batchmode as if vsync is off, since there is nothing we're rendering to
//    // and therefore nothing to sync. See case 982337.
//    if (IsBatchmode() || (GetIVRDevice() && GetIVRDevice()->GetDisableVSync()))
//        return 0;
//
//    #if !UNITY_EDITOR
//    // What the user has requested, no clamping to actual caps
//    QualitySettings* qualitySettings = GetQualitySettingsPtr();
//    if (qualitySettings)
//    {
//        int renderFrameInterval = 1;
//        Scripting::UnityEngine::Rendering::OnDemandRenderingProxy::GetRenderFrameInterval(&renderFrameInterval);
//        return qualitySettings->GetCurrent().vSyncCount / renderFrameInterval;
//    }
//    #endif
//    return 0;
//}

float GetActualTargetFrameRate()
{
    return 60.0f;
//    float framerate = GetSyncedRefreshRate(GetWantedVSyncCount());
//
//    if (framerate <= 0)
//#if PLATFORM_ANDROID
//        return 30;
//#else
//        return -1; // The target framerate is high as possible
//#endif
//
//    return framerate;
}
//
//int GetMaxSupportedVSyncCount()
//{
//    // In the editor there are multiple rendering contexts and vsync may not always work as expected.
//#if UNITY_EDITOR
//    return 0;
//#else
//    int vSync = GetGraphicsCaps().maxVSyncInterval;
//
//#if !PLATFORM_WINRT && GFX_SUPPORTS_D3D11 && !PLATFORM_XBOXONE
//    GfxDeviceRenderer renderer = GetGfxDevice().GetRenderer();
//
//    if (renderer == kGfxRendererD3D11)
//    {
//        if (gMinimized)
//        {
//            // D3D11 does not support intervals >0 when the application is minimized (case 784933)
//            vSync = 0;
//        }
//    }
//#endif
//
//    return vSync;
//#endif
//}
