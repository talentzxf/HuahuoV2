//
// Created by VincentZhang on 5/26/2022.
//

#ifndef HUAHUOENGINE_FRAMEBUFFERGLES_H
#define HUAHUOENGINE_FRAMEBUFFERGLES_H

#include "Utilities/NonCopyable.h"
#include "ApiGLES.h"
#include "Math/Rect.h"
#include "HandleObjectGLES.h"
#include "Graphics/RenderSurface.h"
#include "GfxDevice/opengles/RenderSurfaceGLES.h"
#include "GfxDevice/GfxDeviceObjects.h"


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

    // Call after switching context to resolve invalid state
    // if outColor/outDepth pointers are not null it will output default attachments
    void ActiveContextChanged(RenderSurfaceBase** outColor, RenderSurfaceBase** outDepth);

    // setup backbuffer
    // this one should be called early: it will setup backbuffer/dummy render surfaces and this is needed for everything else
    // if pointers are not null it will output default attachments
    // The inFbo parameter must be filled with a valid FBO
    // (this is needed because on android and iOS the default framebuffer isn't the one GfxDevice should be primarily rendering into)
    void SetupDefaultFramebuffer(RenderSurfaceBase** outColor, RenderSurfaceBase** outDepth, gl::FramebufferHandle inFbo);

    // reestablish consistency in state tracking
    void InvalidateActiveFramebufferState();

    // Restore a valid state of current and pending framebuffer if one or both of them are in an invalid state.
    // This may cause a switch to the default framebuffer.
    void FallbackToValidFramebufferState();

    // Called once context is activated. Processes the list generated with calls to the above, and cleans up FBOs
    void ProcessInvalidatedRenderSurfaces();

    gl::FramebufferHandle GetDefaultFBO() const { return m_DefaultFBO; }

private:
    // Update the current viewport from the pending viewport: it checks if the OpenGL states need to be
    void ApplyViewport();

private:
    FramebufferSetup                m_CurrentFramebufferSetup;
    FramebufferSetup                m_PendingFramebufferSetup;
    // The flag to avoid redundant and to delay glViewport and glScissor calls
    bool                            m_RequiresFramebufferSetup;
    ApiGLES &                       m_Api;

    gl::FramebufferHandle           m_DefaultFBO; // Editor game view (and other places) may override the back buffer, and FBOs are per context.

    // some extra magic:
    // in gfx device we keep pointers to RS for backbuffer, and they can be queried and stored
    // so if we want to handle external backbuffer we need to tweak them directly (not change pointers)
    RenderSurfaceGLES               m_BackBufferColorSurface;
    RenderSurfaceGLES               m_BackBufferDepthSurface;

    // Default framebuffer
    GfxRenderTargetSetup            m_DefaultFramebuffer;

    // Current framebuffer
    GfxRenderTargetSetup            m_CurrentFramebuffer;

    // Pending framebuffer for delayed framebuffer setup
    GfxRenderTargetSetup            m_PendingFramebuffer;
};

namespace gles
{
    void FillRenderTargetSetup(GfxRenderTargetSetup *setup, RenderSurfaceBase *col, RenderSurfaceBase *depth);
}//namespace gles

#endif //HUAHUOENGINE_FRAMEBUFFERGLES_H
