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

    BarrierTime                     barrierTimes[gl::kBarrierTypeCount]; // Record the times of last called barriers
    BarrierTime                     barrierTimeCounter; // Time counter for barrier resolving. Incremented on each write/barrier
    GLbitfield                      requiredBarriers; // Bitfield marking the required barriers before next draw/dispatch call
    GLbitfield                      requiredBarriersMask; // Mask for temporarily enabling only a subset of barriers
};

extern DeviceStateGLES* g_DeviceStateGLES;

namespace gles
{
    // state reset
    void    Invalidate(const GfxContextGLES & context, DeviceStateGLES& state);
}
#endif //HUAHUOENGINE_DEVICESTATEGLES_H
