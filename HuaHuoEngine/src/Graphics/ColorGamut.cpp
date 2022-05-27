//
// Created by VincentZhang on 5/26/2022.
//

#include "ColorGamut.h"
#include "Input/TimeManager.h"
#include <vector>

static ColorGamut gCurrentColorGamut;
static std::vector<int> gSupportedColorGamuts;

void InitColorGamuts()
{
    // gSupportedColorGamuts = GetPlayerSettings().GetColorGamuts();
    gCurrentColorGamut = gSupportedColorGamuts.empty() ? kColorGamutSRGB : static_cast<ColorGamut>(gSupportedColorGamuts[0]);
}

ColorGamut GetActiveColorGamutNonCached()
{
    return kColorGamutSRGB;

//    if (GetPlayerSettingsPtr() == NULL)
//        return kColorGamutSRGB;
//
//    ScreenManagerPlatform* screenManager = GetScreenManagerPtr();
//    if (screenManager)
//    {
//        HDROutputSettings* HDRSettings = screenManager->GetHDROutputSettings();
//        if (HDRSettings && HDRSettings->GetActive())
//        {
//            ColorGamut HDRGamut = HDRSettings->GetDisplayColorGamut();
//            //If the HDRSettings returned anything other than the default kColorGamutSRGB then we know what color gamut we are in for certain at this point
//            if (HDRGamut != kColorGamutSRGB)
//                return HDRGamut;
//        }
//    }
//
//    const dynamic_array<int>& gamuts = GetPlayerSettingsPtr()->GetColorGamuts();
//
//    for (int i = 0; i < gamuts.size(); ++i)
//    {
//        ColorGamut gamut = (ColorGamut)gamuts[i];
//        if (PlatformIsColorGamutSupported(gamut))
//            return gamut;
//    }
//    return kColorGamutSRGB;
}


static int gCurrentColorGamutFrame = -1;

ColorGamut GetActiveColorGamut()
{
    if (gSupportedColorGamuts.empty())
        InitColorGamuts();
    // This function is not only exposed to users, is used from within our
    // rendering system. We don't want its result to change mid-frame. This
    // might happen due to platform-dependent factors, so the function
    // calculates the preferred color gamut only the first time it's called each
    // frame.
    TimeManager* timeManager = GetTimeManagerPtr();
    int currentFrame = timeManager ? timeManager->GetRenderFrameCount() : -1;
    if (currentFrame != gCurrentColorGamutFrame)
    {
        gCurrentColorGamut = GetActiveColorGamutNonCached();
        gCurrentColorGamutFrame = currentFrame;
    }
    return gCurrentColorGamut;
}


