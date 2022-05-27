#pragma once
// #include "Runtime/Scripting/BindingsDefs.h"

enum ColorGamut
{
    kColorGamutSRGB = 0,
    kColorGamutRec709 = 1,
    kColorGamutRec2020 = 2,
    kColorGamutDisplayP3 = 3,
    kColorGamutHDR10 = 4,
    kColorGamutDolbyHDR = 5,
    kColorGamutCount = 6
};
// BIND_MANAGED_TYPE_NAME(ColorGamut, UnityEngine_ColorGamut);

ColorGamut GetActiveColorGamut();
/*UNITY_EXPORT*/ bool PlatformIsColorGamutSupported(ColorGamut gamut);
