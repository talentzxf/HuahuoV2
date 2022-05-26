//
// Created by VincentZhang on 5/16/2022.
//

#include "GfxDeviceSetup.h"
#include "GfxDevice.h"
#include "opengles/GfxDeviceGLES.h"
#include "Memory/MemoryMacros.h"

#if GFX_SUPPORTS_OPENGL_UNIFIED
int gDefaultFBO = -1;
GfxDeviceLevelGL g_ForcedGLLevel = kGfxLevelUninitialized;
GfxDeviceLevelGL g_RequestedGLLevel = kGfxLevelCoreLast;
#endif

GfxDevice* InitializeGfxDevice(){

    GfxDeviceRenderer renderer = kGfxRendererOpenGLCore;

#if GFX_SUPPORTS_OPENGL_UNIFIED
    if (IsUnifiedGLRenderer(renderer))
    {
        extern GfxDevice* CreateGLESGfxDevice(GfxDeviceRenderer renderer);
        return CreateGLESGfxDevice(renderer);
    }
#endif
    return NULL;
}