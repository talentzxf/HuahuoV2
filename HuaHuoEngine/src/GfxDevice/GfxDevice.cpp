//
// Created by VincentZhang on 5/13/2022.
//

#include "GfxDevice.h"
#include "Logging/LogAssert.h"
#include "Job/Jobs.h"
#include <cstdlib>
#include "Graphics/RenderSurface.h"
#include "Shaders/GraphicsCaps.h"

static GfxDevice* g_MainGfxDevice = NULL;

GfxDevice& GetGfxDevice()
{
//    __FAKEABLE_FUNCTION__(GetGfxDevice, ());
#if GFX_TLS_DEVICE
    Assert(g_ThreadedGfxDevice);
    return *g_ThreadedGfxDevice;
#elif ENABLE_MULTITHREADED_CODE
    AssertMsg(CurrentThread::IsMainThread(), "GetGfxDevice() should only be called from main thread");
#endif
    Assert(g_MainGfxDevice);
    return *g_MainGfxDevice;
}

bool IsGfxDevice()
{
    return g_MainGfxDevice != NULL;
}


GfxDevice::GfxDevice(MemLabelRef label) :
        m_MemoryLabel(label)
//        m_DynamicVBO(NULL),
//        m_AsyncJobFences(kMemTempJobAlloc),
#if UNITY_DEFER_GRAPHICS_JOBS_SCHEDULE
        m_AsyncJobFunctions(m_MemoryLabel),
    m_AsyncJobCommands(m_MemoryLabel),
    m_AsyncJobDepends(m_MemoryLabel),
#endif // #if UNITY_DEFER_GRAPHICS_JOBS_SCHEDULE
//        m_ParamsScratchBuffer(m_MemoryLabel),
//        m_ProceduralQuadIndexBuffer(NULL),
//        m_ProceduralQuadIndexBuffer32(NULL),
//        m_ProceduralQuadMax32(0),
//        m_PresentFrameCallbacks(label),
//        m_WaitForPresentSyncPoint(kGfxDeviceWaitForPresentBeginFrame),
//        m_CurrentCPUFence(0)
{
//    m_IsDeviceClient = false;
//    m_DrawImmediate = NULL;
//    m_BufferList = NULL;
    m_FrameTimingManager = NULL;

//    m_GraphicsJobsSyncPoint = kGfxDeviceGraphicsJobsSyncPointAfterScriptUpdate;
//
//    // Initialize the input FB names
//    m_InputFBNames[0].SetName("_UnityFBInput0");
//    m_InputFBNames[1].SetName("_UnityFBInput1");
//    m_InputFBNames[2].SetName("_UnityFBInput2");
//    m_InputFBNames[3].SetName("_UnityFBInput3");
//    m_InputFBNames[4].SetName("_UnityFBInput4");
//    m_InputFBNames[5].SetName("_UnityFBInput5");
//    m_InputFBNames[6].SetName("_UnityFBInput6");
//    m_InputFBNames[7].SetName("_UnityFBInput7");

#if SUPPORT_MULTIPLE_DISPLAYS
    m_DisplayTarget = 0;
#endif

//    OnCreate();
}

GfxDevice::~GfxDevice()
{
//    OnDelete();
}

void GfxDevice::UpdateViewProjectionMatrix()
{
    const Matrix4x4f& viewMat = m_GfxContextData.m_BuiltinParamValues.GetMatrixParam(kShaderMatView);
    const Matrix4x4f& projMat = m_GfxContextData.m_BuiltinParamValues.GetMatrixParam(kShaderMatProj);
    Matrix4x4f& viewProjMat = m_GfxContextData.m_BuiltinParamValues.GetWritableMatrixParam(kShaderMatViewProj);
    MultiplyMatrices4x4(&projMat, &viewMat, &viewProjMat);
}

void GfxDevice::SetViewMatrix(const Matrix4x4f& matrix)
{
    m_GfxContextData.m_TransformState.SetViewMatrix(matrix, m_GfxContextData.m_BuiltinParamValues);
    GfxDevice::UpdateViewProjectionMatrix();
    m_ViewProjMatrixNeedApplyFlags |= kMatricesToApplyFlagView;

#if GFX_SUPPORTS_SINGLE_PASS_STEREO
    if (GetCopyMonoTransformsToStereo())
    {
        SetStereoMatrix(kMonoOrStereoscopicEyeLeft, kShaderMatView, matrix);
        SetStereoMatrix(kMonoOrStereoscopicEyeRight, kShaderMatView, matrix);
    }
#endif
}

const Matrix4x4f& GfxDevice::GetWorldViewMatrix() const
{
    m_GfxContextData.m_TransformState.UpdateWorldViewMatrix(m_GfxContextData.m_BuiltinParamValues);
    return m_GfxContextData.m_TransformState.worldViewMatrix;
}

const Matrix4x4f& GfxDevice::GetWorldMatrix() const
{
    return m_GfxContextData.m_TransformState.worldMatrix;
}

const Matrix4x4f& GfxDevice::GetViewMatrix() const
{
    return m_GfxContextData.m_BuiltinParamValues.GetMatrixParam(kShaderMatView);
}

const Matrix4x4f& GfxDevice::GetProjectionMatrix() const
{
    return m_GfxContextData.m_TransformState.projectionMatrixOriginal;
}

const Matrix4x4f& GfxDevice::GetDeviceProjectionMatrix() const
{
    return m_GfxContextData.m_BuiltinParamValues.GetMatrixParam(kShaderMatProj);
}

void GfxDevice::SetWorldMatrix(const Matrix4x4f& matrix)
{
    m_GfxContextData.m_TransformState.SetWorldMatrix(matrix);
}

void GfxDevice::CalculateDeviceProjectionMatrix(Matrix4x4f& m, bool usesOpenGLTextureCoords, bool invertY) const
{
    bool revertZ = GetGraphicsCaps().usesReverseZ;

    if (usesOpenGLTextureCoords)
    {
        if (revertZ)
        {
            m.Get(2, 0) = -m.Get(2, 0);
            m.Get(2, 1) = -m.Get(2, 1);
            m.Get(2, 2) = -m.Get(2, 2);
            m.Get(2, 3) = -m.Get(2, 3);
        }

        return; // nothing else to do on OpenGL-like devices
    }

    // Otherwise, the matrix is OpenGL style, and we have to convert it to
    // D3D-like projection matrix

    if (invertY)
    {
        m.Get(1, 0) = -m.Get(1, 0);
        m.Get(1, 1) = -m.Get(1, 1);
        m.Get(1, 2) = -m.Get(1, 2);
        m.Get(1, 3) = -m.Get(1, 3);
    }

    // Now scale&bias to get Z range from -1..1 to 0..1 or 1..0
    // matrix = scaleBias * matrix
    //  1   0   0   0
    //  0   1   0   0
    //  0   0 0.5 0.5
    //  0   0   0   1
    m.Get(2, 0) = m.Get(2, 0) * (revertZ ? -0.5f : 0.5f) + m.Get(3, 0) * 0.5f;
    m.Get(2, 1) = m.Get(2, 1) * (revertZ ? -0.5f : 0.5f) + m.Get(3, 1) * 0.5f;
    m.Get(2, 2) = m.Get(2, 2) * (revertZ ? -0.5f : 0.5f) + m.Get(3, 2) * 0.5f;
    m.Get(2, 3) = m.Get(2, 3) * (revertZ ? -0.5f : 0.5f) + m.Get(3, 3) * 0.5f;
}

void GfxDevice::SetProjectionMatrix(const Matrix4x4f& matrix)
{
    Matrix4x4f& m = m_GfxContextData.m_BuiltinParamValues.GetWritableMatrixParam(kShaderMatProj);
    m = matrix;
    GetUncheckedRealGfxDevice().CalculateDeviceProjectionMatrix(m, GetGraphicsCaps().usesOpenGLTextureCoords, m_GfxContextData.GetInvertProjectionMatrix());
    m_GfxContextData.m_TransformState.SetProjectionMatrix(matrix);
    m_ViewProjMatrixNeedApplyFlags |= kMatricesToApplyFlagProj;

#if GFX_SUPPORTS_SINGLE_PASS_STEREO
    if (GetCopyMonoTransformsToStereo())
    {
        SetStereoMatrix(kMonoOrStereoscopicEyeLeft, kShaderMatProj, matrix);
        SetStereoMatrix(kMonoOrStereoscopicEyeRight, kShaderMatProj, matrix);
    }
#endif
}

void SetGfxDevice(GfxDevice* device)
{
#if GFX_TLS_DEVICE
    g_MainGfxDevice = device;
    g_ThreadedGfxDevice = device;
#else
    g_MainGfxDevice = device;
#endif
}

void GfxDevice::ExecuteAsync(int count, GfxDeviceAsyncCommand::Func* func, GfxDeviceAsyncCommand::ArgScratch** scratches, const GfxDeviceAsyncCommand::Arg* arg, const JobFence& depends)
{
#if ENABLE_EMULATED_JOB_ASSERT
    g_ThreadRunningEmulatedJob = true;
#endif

    SyncFenceNoClear(depends);

    for (int i = 0; i < count; i++)
    {
        GfxDeviceAsyncCommand::ArgScratch* scratch = scratches[i];
        scratch->device = this;
        func(scratch, arg);
        scratch->ThreadedCleanup();
    }

#if ENABLE_EMULATED_JOB_ASSERT
    g_ThreadRunningEmulatedJob = false;
#endif
}

RenderSurfaceBase* GfxDevice::AllocRenderSurface(bool colorSurface)
{
    const size_t memSz = RenderSurfaceStructMemorySize(colorSurface);
    RenderSurfaceBase* ret = (RenderSurfaceBase*)HUAHUO_MALLOC_ALIGNED(kMemGfxDevice, memSz, 16);
    ::memset(ret, 0x00, memSz);
    ret->samples = 1;
    ret->colorSurface = colorSurface;
    return ret;
}

void GfxDevice::DeallocRenderSurface(RenderSurfaceBase* rs)
{
#if DEBUGMODE
    const size_t memSz = RenderSurfaceStructMemorySize(rs->colorSurface);
    ::memset(rs, 0xFD, memSz);
#endif

    HUAHUO_FREE(kMemGfxDevice, rs);
}

void GfxDevice::DestroyRenderSurface(RenderSurfaceHandle& rsh)
{
    RenderSurfaceBase* rs = rsh.object;

    // lots of GfxDevice implementations had this if, so we keep it
    if (rs == 0)
        return;

    // we disallow destroying backbuffer through high level interface
    if (rs->backBuffer)
        return;

//    // TODO@MT: Waits for all async render jobs to complete before allowing the texture to be deleted.
//    GfxDeviceWaitForAllRenderJobsToComplete();

//    bool unused = (rs->flags & kSurfaceCreateNeverUsed) || (!rs->colorSurface && (rs->flags & kSurfaceCreateNoDepth));
//    if ((rs->flags & kSurfaceCreateDynamicScale) && !unused)
//    {
//        ScalableBufferManager::GetInstance().UnregisterRenderSurface(rs, false);
//    }

    DestroyRenderSurfacePlatform(rs);
    DeallocRenderSurface(rs);
    rsh.object = 0;
}

GfxDevice* GetUncheckedRealGfxDevicePointer()
{
#if ENABLE_MULTITHREADED_CODE
    return g_RealGfxDevice;
#else
    return g_MainGfxDevice;
#endif
}


GfxDevice& GetUncheckedRealGfxDevice()
{
    GfxDevice* ret = GetUncheckedRealGfxDevicePointer();
    DebugAssert(ret); // note that making reference out of NULL is UB!
    return *ret;
}