//
// Created by VincentZhang on 5/26/2022.
//

#include "FrameBufferGLES.h"
#include "AssertGLES.h"


GfxFramebufferGLES::GfxFramebufferGLES(ApiGLES & api, void* context)
//        : m_FramebufferMap()
        : m_CurrentFramebufferSetup()
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
//        , m_DefaultFBO()
//        , m_IntermediateMSAAFBO()
#if USES_GLES_DEFAULT_FBO
, m_DefaultGLESFBOInited(false)
#endif
{
//    memset(&m_BackBufferColorSurface, 0, sizeof(RenderSurfaceGLES));
//    memset(&m_BackBufferDepthSurface, 0, sizeof(RenderSurfaceGLES));
//    using gles::internal::FillRenderTargetSetup;
//    FillRenderTargetSetup(&m_DefaultFramebuffer, &m_BackBufferColorSurface, &m_BackBufferDepthSurface);
//    FillRenderTargetSetup(&m_CurrentFramebuffer, &m_BackBufferColorSurface, &m_BackBufferDepthSurface);
//    FillRenderTargetSetup(&m_PendingFramebuffer, &m_BackBufferColorSurface, &m_BackBufferDepthSurface);
//
//    std::fill(m_CurrentFramebufferColorStoreAction.begin(), m_CurrentFramebufferColorStoreAction.end(), kGfxRTStoreActionStore);
//    std::fill(m_PendingFramebufferColorLoadAction.begin(), m_PendingFramebufferColorLoadAction.end(), kGfxRTLoadActionLoad);
}

void GfxFramebufferGLES::Invalidate()
{
//    m_FramebufferMap.clear();
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