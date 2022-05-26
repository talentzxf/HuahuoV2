//
// Created by VincentZhang on 5/25/2022.
//

#ifndef HUAHUOENGINE_GFXCONTEXTGLES_H
#define HUAHUOENGINE_GFXCONTEXTGLES_H
#include "Utilities/NonCopyable.h"
#include "FrameBufferGLES.h"

class GfxContextGLES :NonCopyable {
public:
    class Instance : NonCopyable
    {
        friend class GfxContextGLES;

    public:
        Instance(ApiGLES & Api);
        ~Instance();

    public:
        GfxFramebufferGLES& GetFramebuffer();
        const GfxFramebufferGLES& GetFramebuffer() const;

        void Invalidate();

    private:
        GfxFramebufferGLES m_Framebuffer;
        gl::VertexArrayHandle m_DefaultVertexArrayName; // OpenGL ES 3.0 indirect draw and OpenGL core profile requires a default vertex array object that is not 0
    };

    // Access to the framebuffer manager and the default FBO of the current context
    GfxFramebufferGLES& GetFramebuffer();
    const GfxFramebufferGLES& GetFramebuffer() const;
public:
    // Access to the current context instance
    Instance& GetCurrent() const;

    // Make current the context instance associated with "context"
    Instance& MakeCurrent(ApiGLES & api, gl::ContextHandle contextHandle);

};


#endif //HUAHUOENGINE_GFXCONTEXTGLES_H
