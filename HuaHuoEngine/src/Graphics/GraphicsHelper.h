//
// Created by VincentZhang on 5/26/2022.
//

#ifndef HUAHUOENGINE_GRAPHICSHELPER_H
#define HUAHUOENGINE_GRAPHICSHELPER_H

#include "GfxDevice/GfxDeviceObjects.h"
#include "GfxDevice/GfxDevice.h"

namespace GraphicsHelper{
    void ValidateMemoryless(GfxRenderTargetSetup& setup);

    inline void SetWorldViewAndProjection(GfxDevice& device, const Matrix4x4f* world, const Matrix4x4f* view, const Matrix4x4f* proj)
    {
        // The order of setting world, view and projection is important!
        // When you set V, VP is implicitly updated by the GfxDevice, and world is reset to identity.
        if (proj)
        {
            device.SetProjectionMatrix(*proj);
            // We have to explicitly update VP if we set P without V.
            if (!view)
                device.UpdateViewProjectionMatrix();
        }
        if (view)
            device.SetViewMatrix(*view);
        if (world)
            device.SetWorldMatrix(*world);
    }
}


#endif //HUAHUOENGINE_GRAPHICSHELPER_H
