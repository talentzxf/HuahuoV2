//
// Created by VincentZhang on 5/16/2022.
//

#include "GfxDeviceGLES.h"
#include "Shaders/GraphicsCaps.h"
#include "GfxDevice/TextureIdMap.h"
#include "AssertGLES.h"

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

void GfxDeviceGLES::UpdateSRGBWrite()
{
//    if (!GetGraphicsCaps().hasSRGBReadWrite)
//        return;
//
//    bool enable = m_sRGBWrite;
//
//    // Sometimes sRGB writes happen on linear textures when FRAMEBUFFER_SRGB is set,
//    // therefore we explicitly disable FRAMEBUFFER_SRGB for linear textures.
//    if (GetGraphicsCaps().buggySRGBWritesOnLinearTextures && m_State.renderTargetsAreLinear > 0)
//        enable = false;
//
//    if ((int)enable == m_State.actualSRGBWrite)
//        return;
//
//    if (GetGraphicsCaps().gles.hasFramebufferSRGBEnable)
//    {
//        if (enable)
//            this->m_Api.Enable(gl::kFramebufferSRGB);
//        else
//            this->m_Api.Disable(gl::kFramebufferSRGB);
//    }
//
//    m_State.actualSRGBWrite = enable;
}

void GfxDeviceGLES::SetSRGBWrite(bool enable)
{
    m_sRGBWrite = enable;

//    if (PLATFORM_WIN && enable && !GetGraphicsCaps().hasSRGBReadWrite && m_Context->GetFramebuffer().GetCurrentFramebufferName() == gl::FramebufferHandle::Default())
//        AssertMsg(0, "Your platform doesn't support linear rendering with OpenGL ES, switch to OpenGL core graphics API");

    this->UpdateSRGBWrite();
}

bool GfxDeviceGLES::GetSRGBWrite()
{
    return GetGraphicsCaps().hasSRGBReadWrite && GetGraphicsCaps().gles.hasFramebufferSRGBEnable && m_sRGBWrite;
}

extern GfxDeviceLevelGL g_RequestedGLLevel;

void GfxDeviceGLES::SetViewport(const RectInt& rect)
{
    m_State.viewport = rect;

#if GFX_SUPPORTS_SINGLE_PASS_STEREO
    if (m_GfxContextData.GetSinglePassStereo() != kSinglePassStereoSideBySide)
#endif
    {
        // Side-by-side stereo should not update the actual viewport
        SetViewportInternal(rect);
    }
}

void GfxDeviceGLES::SetViewportInternal(const RectInt& rect)
{
    m_Context->GetFramebuffer().SetViewport(rect);
}

RectInt GfxDeviceGLES::GetViewport() const
{
    return m_State.viewport;
}

void GfxDeviceGLES::ProcessPendingMipGens()
{
    // Generate mipmaps for all pending surfaces
    for (size_t i = 0; i < m_PendingMipGens.size(); i++)
    {
        const GLESTexture* texInfo = (GLESTexture*)TextureIdMap::QueryNativeTexture(m_PendingMipGens[i]->textureID);
        GLES_ASSERT(gGL, m_Api.translate.GetTextureTargetDimension(texInfo->target) == m_PendingMipGens[i]->dim, "Invalid pending mip gen dimension.");
        m_Api.GenerateMipmap(texInfo->texture, texInfo->target);
    }
    m_PendingMipGens.clear();
}

void GfxDeviceGLES::SetActiveContext(void* context)
{
#   if UNITY_DESKTOP
    GraphicsContextGL* glctx = static_cast<GraphicsContextGL*>(context);
    DebugAssert(glctx);
    gl::ContextHandle requestedContextHandle(glctx->GetContext());

    if (requestedContextHandle != gl::GetCurrentContext())
        ActivateGraphicsContextGL(*glctx, kGLContextSkipInvalidateState | kGLContextSkipUnbindObjects | kGLContextSkipFlush);
    DebugAssertFormatMsg(requestedContextHandle == gl::GetCurrentContext(), "The context (%p) must already be the active context (%p)", context, gl::GetCurrentContext().Get());
#   else
    gl::ContextHandle requestedContextHandle(context);
#   endif

    m_Context->MakeCurrent(m_Api, requestedContextHandle);

    // We also invalidate the API, because we need to re-bind all objects
    // so that their state reflects possible changes in other contexts.
    gles::Invalidate(*m_Context, m_State);
    this->UpdateSRGBWrite();

    ProcessPendingMipGens();

    m_Context->GetFramebuffer().ActiveContextChanged(&m_BackBufferColor.object, &m_BackBufferDepth.object);
}

// The content of this function should be in GfxDeviceGLES constructor
// but GfxDeviceGLES instances are created with UNITY_NEW_AS_ROOT which doesn't allow arguments passing.
bool GfxDeviceGLES::Init(GfxDeviceLevelGL deviceLevel)
{
    void* masterContextPointer = NULL;
    g_RequestedGLLevel = deviceLevel;

//#   if UNITY_DESKTOP
//    #       if PLATFORM_WIN
//    SetMasterContextClassName(L"WindowGLClassName");
//
//    GfxDeviceLevelGL CreateMasterGraphicsContext(GfxDeviceLevelGL level);
//    deviceLevel = CreateMasterGraphicsContext(deviceLevel);
//    if (deviceLevel == kGfxLevelUninitialized)
//        return false;
//#       endif
//
//    GraphicsContextHandle masterContext = GetMasterGraphicsContext();
//    if (!masterContext.IsValid())
//        return false;
//
//    SetMainGraphicsContext(masterContext);
//    ActivateGraphicsContext(masterContext, true, kGLContextSkipGfxDeviceMakeCurrent);
//
//    GraphicsContextGL* context = OBJECT_FROM_HANDLE(masterContext, GraphicsContextGL);
//    masterContextPointer = context;
//#   else
//    // Read in context version
//    ContextGLES::Create(contextLevelToGLESVersion(deviceLevel));
//    masterContextPointer = gl::ContextHandle::DummyMaster().Get();    // masterContextPointer should not be null when calling SetActiveContext
//#   endif
//
//    GLES_ASSERT(&m_Api, masterContextPointer, "No master context created");

    // Initialize context and states
    g_DeviceStateGLES = &m_State;

    if (IsGfxLevelES2(deviceLevel))
        m_Renderer = kGfxRendererOpenGLES20;
    else if (IsGfxLevelES(deviceLevel))
        m_Renderer = kGfxRendererOpenGLES3x;
    else if (IsGfxLevelCore(deviceLevel))
        m_Renderer = kGfxRendererOpenGLCore;
//    else
//        GLES_ASSERT(&m_Api, 0, "OPENGL ERROR: Invalid device level");

    m_Context = new GfxContextGLES;

    m_Api.Init(*m_Context, deviceLevel);
    gGL = m_State.api = &m_Api;

    this->SetActiveContext(masterContextPointer);

    m_Api.InitDebug();
    m_Api.debug.Log(Format("OPENGL LOG: GfxDeviceGLES::Init - CreateMasterGraphicsContext\n").c_str());

    printf_console("OPENGL LOG: Creating OpenGL%s%d.%d graphics device ; Context level %s ; Context handle %d\n",
                   IsGfxLevelES(deviceLevel) ? " ES " : " ",
                   GetGraphicsCaps().gles.majorVersion, GetGraphicsCaps().gles.minorVersion,
                   GetGfxDeviceLevelString(deviceLevel),
                   m_Api.GetContext().Get());

#if UNITY_APPLE_PVR
    printf_console("OPENGL LOG: OpenGLES%d is deprecated on this platform\n", GetGraphicsCaps().gles.majorVersion);
#endif

    m_FrameTimingManager = UNITY_NEW(FrameTimingManagerGLES, kMemGfxDevice)(*gGL);

    InitCommonState(m_State);
    InvalidateState();
    m_IsThreadable = true;

    m_GfxContextData.SetGlobalDepthBias(0.0f);
    m_GfxContextData.SetGlobalSlopeDepthBias(0.0f);

    m_GfxContextData.SetUserBackfaceMode(false);

    m_AtomicCounterBuffer = NULL;
    m_AtomicCounterSlots.resize_initialized(GetGraphicsCaps().gles.maxAtomicCounterBufferBindings, nullptr);

#if UNITY_ANDROID
    m_platformSettings.requiresEyeIndexArray = true;
#endif //UNITY_ANDROID
    m_SinglePassStereoSupport.InitSinglePassStereoSupport(this, this);

    CreateDefaultVertexBuffers();

    PluginsSetGraphicsDevice(NULL, m_Renderer, kGfxDeviceEventInitialize);

#if SUPPORT_MULTIPLE_DISPLAYS && PLATFORM_STANDALONE
    m_DisplayManager.Initialize();
#endif

    static_cast<FrameTimingManagerGLES*>(m_FrameTimingManager)->FrameStart();

    return true;
}