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
#include <map>


// A struct to act as a map key for FBO map. We can't really use GfxRenderTargetSetup because those surfaces may come and go.
struct GLESRenderTargetSetup
{
public:
    explicit GLESRenderTargetSetup(const GfxRenderTargetSetup &attach)
    {
        ::memset(this, 0, sizeof(GLESRenderTargetSetup));

        // Special case for dummy surfaces
        if (attach.colorCount == 1 && (attach.color[0]->flags & kSurfaceCreateNeverUsed))
        {
            m_ColorCount = 0;
        }
        else
        {
            m_ColorCount = attach.colorCount;
            for (unsigned i = 0; i < m_ColorCount; i++)
            {
                m_ColorTexIDs[i] = attach.color[i]->textureID;
                m_ColorRBIDs[i] = ((RenderSurfaceGLES*)attach.color[i])->buffer;
            }
        }

        // Special case for dummy surfaces
        if (attach.depth && (attach.depth->flags & kSurfaceCreateNeverUsed))
        {
            m_HasDepth = false;
        }
        else
        {
            m_HasDepth = attach.depth != NULL;
            if (m_HasDepth)
            {
                m_DepthTexID = attach.depth->textureID;
                m_DepthRBID = ((RenderSurfaceGLES*)attach.depth)->buffer;
                m_DepthStencilID = ((RenderSurfaceGLES*)attach.depth)->stencilBuffer;
            }
        }

        m_MipLevel = attach.mipLevel;
        m_DepthSlice = attach.depthSlice;
        m_CubemapFace = attach.cubemapFace;
    }

    GLESRenderTargetSetup(const GLESRenderTargetSetup &src)
    {
        ::memcpy(this, &src, sizeof(GLESRenderTargetSetup));
    }

    bool operator<(const GLESRenderTargetSetup &b) const
    {
#define CMP_MEMBER(a) if(a != b.a) return a < b.a
        CMP_MEMBER(m_ColorCount);
        for (unsigned i = 0; i < m_ColorCount; i++)
        {
            CMP_MEMBER(m_ColorTexIDs[i]);
            CMP_MEMBER(m_ColorRBIDs[i]);
        }
        CMP_MEMBER(m_HasDepth);
        if (m_HasDepth)
        {
            CMP_MEMBER(m_DepthRBID);
            CMP_MEMBER(m_DepthTexID);
            CMP_MEMBER(m_DepthStencilID);
        }
        CMP_MEMBER(m_MipLevel);
        CMP_MEMBER(m_DepthSlice);
        CMP_MEMBER(m_CubemapFace);
#undef CMP_MEMBER

        return false;
    }

    bool operator==(const GLESRenderTargetSetup &b) const
    {
#define CMP_MEMBER(a) if(a != b.a) return false
        CMP_MEMBER(m_ColorCount);
        for (unsigned i = 0; i < m_ColorCount; i++)
        {
            CMP_MEMBER(m_ColorTexIDs[i]);
            CMP_MEMBER(m_ColorRBIDs[i]);
        }
        CMP_MEMBER(m_HasDepth);
        if (m_HasDepth)
        {
            CMP_MEMBER(m_DepthRBID);
            CMP_MEMBER(m_DepthTexID);
            CMP_MEMBER(m_DepthStencilID);
        }
        CMP_MEMBER(m_MipLevel);
        CMP_MEMBER(m_DepthSlice);
        CMP_MEMBER(m_CubemapFace);
#undef CMP_MEMBER

        return true;
    }

    // If both texid and rbid are 0, then it's the backbuffer.
    unsigned            m_ColorCount;
    TextureID           m_ColorTexIDs[kMaxSupportedRenderTargets];
    GLuint              m_ColorRBIDs[kMaxSupportedRenderTargets];

    TextureID           m_DepthTexID;
    GLuint              m_DepthRBID;
    GLuint              m_DepthStencilID;

    UInt32              m_MipLevel;
    int                 m_DepthSlice;
    CubemapFace         m_CubemapFace;

    bool                m_HasDepth;

    // Create a GLESRenderTargetSetup with system default (0) color and depth. (Note that on iOS and some other platforms this is meaningless)
    static GLESRenderTargetSetup GetZeroSetup()
    {
        GLESRenderTargetSetup res;
        memset(&res, 0, sizeof(GLESRenderTargetSetup));
        res.m_ColorCount = 1;
        res.m_HasDepth = true;
        res.m_CubemapFace = kCubeFaceUnknown;
        return res;
    }

private:
    GLESRenderTargetSetup() {}
};


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

    // will update backbuffer from window extents: it is needed in cases where we switch gl context but still drawing to default fbo
    void UpdateDefaultFramebufferViewport();

    // handles deletion of RenderSurface:
    // updates cache to kill related fbos and handles deletion of currently active fbo
    void ReleaseFramebuffer(RenderSurfaceBase* rs, GfxContextGLES *contexts);

private:
    // Update the current viewport from the pending viewport: it checks if the OpenGL states need to be
    void ApplyViewport();

private:
    // we dont have an explicit link RT->FBO, because we started out with shared FBO + reattach
    // on the other hand on tiled GPUs FBO is more then just attachments (tiler setup etc)
    // so we really want to have FBO per RT
    // the easiest way would be to have a map hidden in gles code
    typedef std::map<GLESRenderTargetSetup, gl::FramebufferHandle> FramebufferMap;
    FramebufferMap                  m_FramebufferMap;

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
