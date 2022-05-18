//
// Created by VincentZhang on 5/17/2022.
//

#ifndef HUAHUOENGINE_RENDERINGCOMMANDBUFFERTYPES_H
#define HUAHUOENGINE_RENDERINGCOMMANDBUFFERTYPES_H

#include "GfxDevice/GfxDeviceTypes.h"
#include "Math/Color.h"

struct RenderCommandClearRT
{
    ColorRGBAf color;
    GfxClearFlags clearFlags;
    float depth;
    UInt32 stencil;
};

#endif //HUAHUOENGINE_RENDERINGCOMMANDBUFFERTYPES_H
