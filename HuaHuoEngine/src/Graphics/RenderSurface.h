//
// Created by VincentZhang on 5/24/2022.
//

#ifndef HUAHUOENGINE_RENDERSURFACE_H
#define HUAHUOENGINE_RENDERSURFACE_H


#include "Configuration/IntegerDefinitions.h"
#include "GfxDevice/GfxDeviceTypes.h"
#include "Format.h"

int CalculateMipMapCount3D(int width, int height, int depth); // Image.cpp

struct RenderSurfaceBase {
    TextureID           textureID;
    UInt16              width;
    UInt16              height;
    UInt16              scaledWidth;
    UInt16              scaledHeight;
    UInt16              depth;
    UInt8               samples;
    UInt8               mipCount;
    SurfaceCreateFlags  flags;
    TextureDimension    dim;
    GraphicsFormat      graphicsFormat;
    UInt8               loadAction;     // GfxRTLoadAction
    UInt8               storeAction;    // GfxRTStoreAction
    bool                colorSurface;
    bool                backBuffer;
    bool                clientSurface;
    // HDROutputSettings* hdrSettings;
    RenderSurfaceBase * resolveSurface; // If storeaction is Resolve or StoreAndResolve, resolve the AA surface into this surface. Currently only used by renderpasses.
};


#endif //HUAHUOENGINE_RENDERSURFACE_H
