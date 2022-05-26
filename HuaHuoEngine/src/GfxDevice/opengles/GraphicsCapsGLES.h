//
// Created by VincentZhang on 5/24/2022.
//

#ifndef HUAHUOENGINE_GRAPHICSCAPSGLES_H
#define HUAHUOENGINE_GRAPHICSCAPSGLES_H


#include "GfxDevice/GfxDeviceTypes.h"

struct GraphicsCapsGLES {
    bool    isVideoCoreGpu;
    bool    isPvrGpu;
    bool    isMaliGpu;
    bool    isAdrenoGpu;
    bool    isTegraGpu;
    bool    isIntelGpu;
    bool    isNvidiaGpu;
    bool    isAMDGpu;
    bool    isAMDVegaGpu;
    bool    isVivanteGpu;
    bool    isES2Gpu;       // A GPU that supports only OpenGL ES 2.0

    // macOS High Sierra + Intel Iris 6100 has troubles with MSAA when certain features are switched on
    bool    buggyMSAA;

    int     driverGLESVersion;
    GfxDeviceLevelGL featureLevel;

    bool    hasFramebufferSRGBEnable;       // GL_ARB_framebuffer_sRGB || GL_EXT_sRGB_write_control
    bool    hasTextureSwizzle;                  // ES3 / GL3.3 / ARB_texture_swizzle / Not available on WebGL
    bool    hasTextureStorage;                  // GL 4.2 / ES 3.0 / GL_ARB_texture_storage / GL_EXT_texture_storage
    bool    hasTextureRG;                       // Not in GLES without EXT_texture_rg

    int     majorVersion;           // Major OpenGL version, eg OpenGL 4.2 major version is 4
    int     minorVersion;           // Minor OpenGL version, eg OpenGL 4.2 minor version is 2
};

extern GraphicsCapsGLES* g_GraphicsCapsGLES;

#endif //HUAHUOENGINE_GRAPHICSCAPSGLES_H
