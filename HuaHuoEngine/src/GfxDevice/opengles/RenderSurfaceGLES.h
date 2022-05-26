//
// Created by VincentZhang on 5/24/2022.
//

#ifndef HUAHUOENGINE_RENDERSURFACEGLES_H
#define HUAHUOENGINE_RENDERSURFACEGLES_H
#include "GfxDevice/GfxDeviceTypes.h"
#include "Graphics/RenderSurface.h"

enum RenderSurfaceType
{
    kRenderSurfaceDepth,
    kRenderSurfaceColor,
};

struct RenderSurfaceGLES : RenderSurfaceBase
{
    // Reference count
    volatile int*       refCount;

    // Color, depth or depth+stencil buffer.
    GLuint              buffer;

    // Format of the surface.
    // In case of separate depth and stencil buffers this will still be a packed format.
    GraphicsFormat      format;

    // Optional, only used for separate stencil buffer when packed depth+stencil is not available.
    GLuint              stencilBuffer;

    // Comparison operator for convenience
    bool operator==(const RenderSurfaceGLES &b) const
    {
        // Backbuffer?
        if (buffer == 0 && textureID.m_ID == 0)
            return b.buffer == 0 && b.textureID.m_ID == 0;

        if (buffer != 0)
            return buffer == b.buffer;

        return textureID.m_ID == b.textureID.m_ID;
    }
};

#endif //HUAHUOENGINE_RENDERSURFACEGLES_H
