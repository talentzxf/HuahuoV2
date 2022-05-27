//
// Created by VincentZhang on 5/26/2022.
//

#include "FrameBufferGLES.h"
#include "AssertGLES.h"

namespace gles
{
    namespace internal
    {
        void FillRenderTargetSetup(GfxRenderTargetSetup *setup, RenderSurfaceBase *col, RenderSurfaceBase *depth)
        {
            ::memset(setup, 0x00, sizeof(GfxRenderTargetSetup));
            setup->color[0] = col;
            setup->depth = depth;
            setup->colorCount = col ? 1 : 0;
            setup->colorLoadAction[0] = kGfxRTLoadActionLoad;
            setup->colorStoreAction[0] = kGfxRTStoreActionStore;
            setup->depthLoadAction = kGfxRTLoadActionLoad;
            setup->depthStoreAction = kGfxRTStoreActionStore;
            setup->cubemapFace = kCubeFaceUnknown;
            setup->mipLevel = 0;
            setup->flags = 0;
        }
    }
    void FillRenderTargetSetup(GfxRenderTargetSetup *setup, RenderSurfaceBase *col, RenderSurfaceBase *depth)
    {
        Assert(setup && col && depth);
        internal::FillRenderTargetSetup(setup, col, depth);
    }
} // namespace

GfxFramebufferGLES::GfxFramebufferGLES(ApiGLES & api, void* context)
        : m_FramebufferMap()
        , m_CurrentFramebufferSetup()
//        , m_CurrentFramebufferValid(false)
        , m_PendingFramebufferSetup()
//        , m_PendingFramebufferValid(false)
//        , m_PendingFramebufferColorLoadAction()
//        , m_PendingFramebufferDepthLoadAction(kGfxRTLoadActionLoad)
//        , m_CurrentFramebufferColorStoreAction()
//        , m_CurrentFramebufferDepthStoreAction(kGfxRTStoreActionStore)
        , m_RequiresFramebufferSetup(true)
//        , m_Context(context)
        , m_Api(api)
//        , m_BlitQuad()
//        , m_DiscardQuad()
        , m_DefaultFBO()
//        , m_IntermediateMSAAFBO()
#if USES_GLES_DEFAULT_FBO
, m_DefaultGLESFBOInited(false)
#endif
{
//    memset(&m_BackBufferColorSurface, 0, sizeof(RenderSurfaceGLES));
//    memset(&m_BackBufferDepthSurface, 0, sizeof(RenderSurfaceGLES));
    using gles::internal::FillRenderTargetSetup;
    FillRenderTargetSetup(&m_DefaultFramebuffer, &m_BackBufferColorSurface, &m_BackBufferDepthSurface);
    FillRenderTargetSetup(&m_CurrentFramebuffer, &m_BackBufferColorSurface, &m_BackBufferDepthSurface);
    FillRenderTargetSetup(&m_PendingFramebuffer, &m_BackBufferColorSurface, &m_BackBufferDepthSurface);
//
//    std::fill(m_CurrentFramebufferColorStoreAction.begin(), m_CurrentFramebufferColorStoreAction.end(), kGfxRTStoreActionStore);
//    std::fill(m_PendingFramebufferColorLoadAction.begin(), m_PendingFramebufferColorLoadAction.end(), kGfxRTLoadActionLoad);
}

void GfxFramebufferGLES::ActiveContextChanged(RenderSurfaceBase** outColor, RenderSurfaceBase** outDepth)
{
    this->SetupDefaultFramebuffer(outColor, outDepth, GetDefaultFBO());
    this->InvalidateActiveFramebufferState();   // force that framebuffer is rebound on next Prepare()
    this->FallbackToValidFramebufferState();    // make sure that current and pending framebuffer have valid rendertarget, may fall back to default framebuffer
    this->ProcessInvalidatedRenderSurfaces();
}

void GfxFramebufferGLES::SetupDefaultFramebuffer(RenderSurfaceBase** outColor, RenderSurfaceBase** outDepth, gl::FramebufferHandle inFbo)
{
    RenderSurfaceBase_InitColor(m_BackBufferColorSurface);
    m_BackBufferColorSurface.backBuffer = true;

    RenderSurfaceBase_InitDepth(m_BackBufferDepthSurface);
    m_BackBufferDepthSurface.backBuffer = true;

    // fill out and register default fbo
    gles::FillRenderTargetSetup(&m_DefaultFramebuffer, &m_BackBufferColorSurface, &m_BackBufferDepthSurface);

    GLESRenderTargetSetup setup(m_DefaultFramebuffer);
    // In case of non-0 FBO this may well already exist in the map, but do it just in case.
    m_FramebufferMap[setup] = inFbo;

    m_DefaultFBO = inFbo;
    this->UpdateDefaultFramebufferViewport();

    if (outColor)
        *outColor = &m_BackBufferColorSurface;
    if (outDepth)
        *outDepth = &m_BackBufferDepthSurface;
}

void GfxFramebufferGLES::UpdateDefaultFramebufferViewport()
{
#if PLATFORM_ANDROID || PLATFORM_WIN || PLATFORM_OSX || PLATFORM_LINUX || PLATFORM_WEBGL
    if (GetScreenManagerPtr())
    {
        // do this in a more proper way.. laterTM
        const Rectf window = GetScreenManager().GetRect();

        // TODO: take care about external FBO
        // for now this is android-only code, so no-external default fbo
        m_BackBufferColorSurface.width  = m_BackBufferDepthSurface.width    = window.width;
        m_BackBufferColorSurface.height = m_BackBufferDepthSurface.height   = window.height;
    }
#endif//PLATFORM_ANDROID || PLATFORM_WIN || PLATFORM_OSX || PLATFORM_LINUX || PLATFORM_WEBGL
}

void GfxFramebufferGLES::Invalidate()
{
    m_FramebufferMap.clear();
//
//#if USES_GLES_DEFAULT_FBO
//    m_DefaultGLESFBOInited = false;
//    this->EnsureDefaultFBOInited();
//#endif
//
//    gles::UninitializeBlitFramebuffer(m_BlitQuad);
//    gles::UninitializeDiscardQuad(m_DiscardQuad);
}

void GfxFramebufferGLES::ApplyViewport()
{
    if (m_CurrentFramebufferSetup.viewport != m_PendingFramebufferSetup.viewport)
    {
        m_CurrentFramebufferSetup.viewport = m_PendingFramebufferSetup.viewport;
        GLES_CALL(&m_Api, glViewport, m_CurrentFramebufferSetup.viewport.x, m_CurrentFramebufferSetup.viewport.y, m_CurrentFramebufferSetup.viewport.width, m_CurrentFramebufferSetup.viewport.height);
    }
}
void GfxFramebufferGLES::SetViewport(const RectInt& rect)
{
    m_PendingFramebufferSetup.viewport = rect;
    if (!m_RequiresFramebufferSetup)
        this->ApplyViewport();
}
