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