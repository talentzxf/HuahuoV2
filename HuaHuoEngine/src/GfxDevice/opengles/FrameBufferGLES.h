//
// Created by VincentZhang on 5/26/2022.
//

#ifndef HUAHUOENGINE_FRAMEBUFFERGLES_H
#define HUAHUOENGINE_FRAMEBUFFERGLES_H

#include "Utilities/NonCopyable.h"
#include "ApiGLES.h"
#include "Math/Rect.h"
#include "HandleObjectGLES.h"


struct FramebufferSetup
{
    FramebufferSetup() : fbo() {}
    gl::FramebufferHandle   fbo;
    RectInt viewport;
    RectInt scissor;
};

class GfxFramebufferGLES : NonCopyable
{
public:
    GfxFramebufferGLES(ApiGLES & api, void* context);
public:
    enum Type
    {
        kBack, // Default framebuffer
        kObject // Framebuffer object
    };

    // Update the pending viewport. Update the current viewport only if internal states demands it
    void SetViewport(const RectInt& rect);

    // Clear the framebuffer object map, called when shutting down
    void Invalidate();

private:
    // Update the current viewport from the pending viewport: it checks if the OpenGL states need to be
    void ApplyViewport();

private:
    FramebufferSetup                m_CurrentFramebufferSetup;
    FramebufferSetup                m_PendingFramebufferSetup;
    // The flag to avoid redundant and to delay glViewport and glScissor calls
    bool                            m_RequiresFramebufferSetup;
    ApiGLES &                       m_Api;
};


#endif //HUAHUOENGINE_FRAMEBUFFERGLES_H
