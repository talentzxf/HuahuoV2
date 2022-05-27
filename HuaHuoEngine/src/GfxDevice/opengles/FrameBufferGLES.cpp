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

void GfxFramebufferGLES::ReleaseFramebuffer(RenderSurfaceBase* rs, GfxContextGLES *contexts)
{
    GLES_ASSERT(&m_Api, !HasAsAttachment(GLESRenderTargetSetup(m_DefaultFramebuffer), rs), "Cannot delete surface of default framebuffer");
//
//    if (m_IntermediateMSAAFBO != gl::FramebufferHandle::Default())
//    {
//        m_Api.DeleteFramebuffer(m_IntermediateMSAAFBO, GetDefaultFBO());
//        m_IntermediateMSAAFBO = gl::FramebufferHandle::Default();
//    }
//
//    // When deleting a rendersurface, we'll need to notify all other GfxFramebufferGLES
//    // instances in other contexts as well that a surface is no more (for both textures and rendersurfaces).
//    if (contexts)
//        contexts->AddRenderSurfaceToDeferredFBOInvalidateList((RenderSurfaceGLES*)rs);
//
//    // Also cancel any possible mip gens.
//    static_cast<GfxDeviceGLES &>(GetRealGfxDevice()).CancelPendingMipGen(rs);
//
//    // update cache
//    gl::FramebufferHandle curFB = m_Api.GetFramebufferBinding(gl::kDrawFramebuffer);
//
//    // When we release the current framebuffer, we don't rebind the current framebuffer and let the active FBO code figure it out
//    bool rebindCurrentFramebuffer = true;
//
//    for (FramebufferMap::iterator fbi = m_FramebufferMap.begin(), fbend = m_FramebufferMap.end(); fbi != fbend;)
//    {
//        if (::HasAsAttachment(fbi->first, rs))
//        {
//#if DEBUG_GLES_FRAMEBUFFER
//            printf_console("*** GLES FBO Delete %d %s\n", fbi->second, DescribeRT(GLESRenderTargetSetup(fbi->first)).c_str());
//#endif
//
//            // in order to avoid leaks when we destroy rb/tex that is still attached to some FBO
//            // we need to
//            // 1. attach 0 to all FBO points (buggy drivers)
//            // 2. delete fbo itself
//            m_Api.BindFramebuffer(gl::kDrawFramebuffer, fbi->second);
//            const GLenum target = GetGraphicsCaps().gles.framebufferTargetForBindingAttachments;
//            for (int i = 0, n = fbi->first.m_ColorCount; i < n; ++i)
//                GLES_CALL(&m_Api, glFramebufferTexture2D, target, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, 0, 0);
//
//            if (fbi->first.m_HasDepth)
//            {
//                GLES_CALL(&m_Api, glFramebufferRenderbuffer, target, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, 0);
//                GLES_CALL(&m_Api, glFramebufferRenderbuffer, target, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, 0);
//            }
//
//            // It turns out that we found out that we are trying to delete the currently bound framebuffer so we can't rebind it after deleting it
//            if (fbi->second == curFB)
//                rebindCurrentFramebuffer = false;
//
//            m_Api.DeleteFramebuffer(fbi->second, m_DefaultFBO);
//            m_FramebufferMap.erase(fbi++);
//        }
//        else
//        {
//            ++fbi;
//        }
//    }
//
//    if (rebindCurrentFramebuffer)
//        m_Api.BindFramebuffer(gl::kDrawFramebuffer, curFB);
//
//    m_CurrentFramebufferSetup.fbo = m_Api.GetFramebufferBinding(gl::kDrawFramebuffer); // DeleteFramebuffer may change the current binding if current is deleted
//
//    // update active FBOs
//
//    m_PendingFramebufferValid = !HasAsAttachment(GLESRenderTargetSetup(m_PendingFramebuffer), rs);
//    m_CurrentFramebufferValid = !HasAsAttachment(GLESRenderTargetSetup(m_CurrentFramebuffer), rs);
//    if (!m_PendingFramebufferValid && !PLATFORM_WEBGL) // With WebGL we might destroy the active render texture when we enter or leave fullscreen
//        ErrorString("RenderTexture warning: Destroying active render texture. Switching to main context.");
//
//    FallbackToValidFramebufferState();
//
//    AssertMsg(!InvalidateSurfacePtr(m_PendingFramebuffer, rs) && !InvalidateSurfacePtr(m_CurrentFramebuffer, rs),
//              "GfxFramebufferGLES: An active RenderTargetSetup has dangling pointers.");
//
//    // Effectively delete the rs object and delete the OpenGL renderbuffer of texture object it contains
//    gles::DestroyRenderSurface(&m_Api, reinterpret_cast<RenderSurfaceGLES*>(rs));
//
//    AssertMsg(m_Api.GetFramebufferBinding(gl::kDrawFramebuffer) == m_CurrentFramebufferSetup.fbo, "GfxFramebufferGLES: Inconsistent framebuffer setup");
}
