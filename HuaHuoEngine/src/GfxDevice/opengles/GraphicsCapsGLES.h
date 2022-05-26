//
// Created by VincentZhang on 5/24/2022.
//

#ifndef HUAHUOENGINE_GRAPHICSCAPSGLES_H
#define HUAHUOENGINE_GRAPHICSCAPSGLES_H


#include "GfxDevice/GfxDeviceTypes.h"

struct GraphicsCapsGLES {
    GfxDeviceLevelGL featureLevel;

    bool    hasFramebufferSRGBEnable;       // GL_ARB_framebuffer_sRGB || GL_EXT_sRGB_write_control
    bool    hasTextureSwizzle;                  // ES3 / GL3.3 / ARB_texture_swizzle / Not available on WebGL
    bool    hasTextureStorage;                  // GL 4.2 / ES 3.0 / GL_ARB_texture_storage / GL_EXT_texture_storage
    bool    hasTextureRG;                       // Not in GLES without EXT_texture_rg
};

extern GraphicsCapsGLES* g_GraphicsCapsGLES;

#endif //HUAHUOENGINE_GRAPHICSCAPSGLES_H
