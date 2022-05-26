//
// Created by VincentZhang on 5/25/2022.
//

#ifndef HUAHUOENGINE_DEVICESTATEGLES_H
#define HUAHUOENGINE_DEVICESTATEGLES_H


#include "Math/Rect.h"
#include "ApiGLES.h"

struct DeviceStateGLES {
    ApiGLES*                        api;
    RectInt                         viewport;
};

extern DeviceStateGLES* g_DeviceStateGLES;

namespace gles
{
    // state reset
    void    Invalidate(const GfxContextGLES & context, DeviceStateGLES& state);
}
#endif //HUAHUOENGINE_DEVICESTATEGLES_H
