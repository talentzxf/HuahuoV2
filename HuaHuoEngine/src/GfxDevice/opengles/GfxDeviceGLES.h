//
// Created by VincentZhang on 5/16/2022.
//

#ifndef HUAHUOENGINE_GFXDEVICEGLES_H
#define HUAHUOENGINE_GFXDEVICEGLES_H
#include "GfxDevice/GfxDevice.h"

class GfxDeviceGLES : public GfxThreadableDevice{
public:
    GfxDeviceGLES();

    virtual void    BeginFrame();
    virtual void    EndFrame();
};


#endif //HUAHUOENGINE_GFXDEVICEGLES_H
