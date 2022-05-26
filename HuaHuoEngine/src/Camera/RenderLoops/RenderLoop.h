//
// Created by VincentZhang on 5/26/2022.
//

#ifndef HUAHUOENGINE_RENDERLOOP_H
#define HUAHUOENGINE_RENDERLOOP_H
#include "GfxDevice/GfxDevice.h"

inline int GetRenderLoopDefaultDepthSlice(SinglePassStereo spsMode)
{
#if GFX_SUPPORTS_SINGLE_PASS_STEREO
    bool bindAllSlices = (spsMode == kSinglePassStereoInstancing) || (spsMode == kSinglePassStereoMultiview);
    return bindAllSlices ? kTextureArraySliceAll : 0;
#else
    return 0;
#endif
}
#endif //HUAHUOENGINE_RENDERLOOP_H
