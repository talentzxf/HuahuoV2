//
// Created by VincentZhang on 5/26/2022.
//

#include "GraphicsCapsScriptBinding.h"

namespace ScriptingGraphcsCaps
{
    bool IsFormatSupported(GraphicsFormat format, FormatUsage usage)
    {
        if (format == kFormatNone)
            return false;

        GraphicsCaps& caps = GetGraphicsCaps();

        const bool isUsageSupported = caps.IsFormatSupported(format, usage, kSupportNative);
        const bool isUsageSampleSupported = caps.IsFormatSupported(format, kUsageSample, kSupportNative) && GetTextureFormat(format) != kTexFormatNone;
        const bool isUsageRenderSupported = caps.IsFormatSupported(format, kUsageRender, kSupportNative) && ((GetRenderTextureFormat(format) != kRTFormatCount) || caps.hasGraphicsFormat);

        switch (usage) // Resolve usage dependence on other usage
        {
            case kUsageSample:
            case kUsageLinear:
            case kUsageSparse:
                return isUsageSupported && isUsageSampleSupported;
            case kUsageRender:
            case kUsageBlend:
            case kUsageLoadStore:
            case kUsageMSAA2x:
            case kUsageMSAA4x:
            case kUsageMSAA8x:
            case kUsageMSAA16x:
            case kUsageMSAA32x:
                return isUsageSupported && isUsageRenderSupported;
            default:
                return isUsageSupported;
        }
    }
}