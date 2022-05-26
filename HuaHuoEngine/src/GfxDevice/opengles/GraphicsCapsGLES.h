//
// Created by VincentZhang on 5/24/2022.
//

#ifndef HUAHUOENGINE_GRAPHICSCAPSGLES_H
#define HUAHUOENGINE_GRAPHICSCAPSGLES_H


struct GraphicsCapsGLES {
    GfxDeviceLevelGL featureLevel;

    bool    hasFramebufferSRGBEnable;       // GL_ARB_framebuffer_sRGB || GL_EXT_sRGB_write_control
};


#endif //HUAHUOENGINE_GRAPHICSCAPSGLES_H
