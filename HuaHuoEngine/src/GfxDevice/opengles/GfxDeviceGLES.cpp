//
// Created by VincentZhang on 5/16/2022.
//

#include "GfxDeviceGLES.h"

GfxDeviceGLES::GfxDeviceGLES(MemLabelRef label)
        : GfxThreadableDevice(label)
//        , m_Context(NULL)
//        , m_ProgramUniformTimeStamp(0)
//        , m_HintedInstancingArraySize(0)
//        , m_sRGBWrite(false)
{
}

GfxDeviceGLES::~GfxDeviceGLES()
{
//    AdrenoDrawIndirectWorkaround::Terminate();
//    // ContextGLES::Acquire make current the main context. At this point, only the master context remain, set it to main.
//#   if UNITY_DESKTOP
//    GraphicsContextHandle mainContext = GetMasterGraphicsContext();
//    if (mainContext.IsValid())
//        SetMainGraphicsContext(mainContext);
//    else
//        // If the main graphics context is not valid, don't try to re-establish it or else this will
//        // crash the process instead of providing valuable information to the user (which happens
//        // during startup, where there are checks done later on to let the user know that graphics
//        // setup failed).
//        return;
//#   endif
//
//    ContextGLES::Acquire(); //Acquire context for resource cleanup
//
//#   if UNITY_DESKTOP
//    // Acquire sets current context but does not update our state caching -> call MakeCurrent for that.
//    // Can't use ActivateGraphicsContext here as the client/worker has already been torn down.
//    GraphicsContextGL* context = OBJECT_FROM_HANDLE(GetMainGraphicsContext(), GraphicsContextGL);
//    gl::ContextHandle ctx(context->GetContext());
//    m_Context->MakeCurrent(m_Api, ctx);
//#   endif
//
//#if SUPPORT_MULTIPLE_DISPLAYS && PLATFORM_STANDALONE
//    m_DisplayManager.Cleanup();
//#endif
//
//    PluginsSetGraphicsDevice(NULL, m_Renderer, kGfxDeviceEventShutdown);
//
//    CleanupSharedBuffers();
//
//    m_State.constantBuffers.Clear();
//
//    ReleaseBufferManagerGLES();
//
//    m_VertDeclCache.Clear();
//
//    UNITY_DELETE(m_FrameTimingManager, kMemGfxDevice);
//
//    if (GetGraphicsCaps().gles.hasSamplerObject)
//    {
//        for (std::size_t i = 0; i < InlineSamplerType::kCount; ++i)
//            m_Api.DeleteSampler(m_State.inlineSamplers[i]);
//    }
//
//    delete m_Context;
//    m_Context = NULL;
//
//    ContextGLES::Destroy();
}


void GfxDeviceGLES::BeginFrame()
{
//#if PLATFORM_ANDROID
//    Profiler_RenderingStart();
//#endif
//
//    m_GfxContextData.SetInsideFrame(true);
//
//    // either after last present or before first frame:
//    // default framebuffer doesn't have any valid content, so no need to load it
//    m_Context->GetFramebuffer().ActivateDefaultFramebufferWithLoadActionDontCare();
//
//    static_cast<FrameTimingManagerGLES*>(m_FrameTimingManager)->FrameStartGPU();
}


void GfxDeviceGLES::EndFrame()
{
//    // Player guarantees that no RenderTexture is bound at this point.
//    m_Context->GetFramebuffer().TryInvalidateDepthStencilBuffer(true, GetGraphicsCaps().gles.buggyInvalidateFrameBuffer);
//
//    if (!GetGraphicsCaps().gles.advanceBufferManagerFrameAfterSwapBuffers)
//        GetBufferManagerGLES()->AdvanceFrame();
//
//    AdrenoTextureUploadWorkaround::EndFrame();
//
//    GLES_ASSERT(&m_Api, m_Api.Verify(), "Tracked states don't match with actual OpenGL states");
//
//    m_GfxContextData.SetInsideFrame(false);
//
//#if PLATFORM_ANDROID
//    Profiler_RenderingEnd();
//#endif
}