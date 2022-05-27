//
// Created by VincentZhang on 5/13/2022.
//

#ifndef HUAHUOENGINE_GFXDEVICE_H
#define HUAHUOENGINE_GFXDEVICE_H
#include "Modules/ExportModules.h"
#include "BaseClasses/BaseTypes.h"
#include "GfxDeviceTypes.h"
#include "Math/Vector2f.h"
#include "GfxDeviceAsyncCommand.h"
#include "Job/JobTypes.h"
#include "GfxDeviceObjects.h"
#include "Math/Rect.h"
#include "BuiltinShaderParams.h"
#include "TransformState.h"

class AutoGfxDeviceBeginEndFrame
{
public:
    AutoGfxDeviceBeginEndFrame();
    ~AutoGfxDeviceBeginEndFrame();

    void End();
    bool GetSuccess() const { return m_Success; }

private:
    bool m_Success;
    bool m_NeedsEndFrame;
};

// Struct to hold all state data that needs propagation to gfx job
// At the point where a graphics job is spawned this gets cached on the main thread
// and a ptr to the cached instance is passed to the job.
struct GfxContextData
{
    GfxContextData()
            : m_Dirty(false)
    {
    }

    // if this returns true we need to create a new cached instance of GfxContextData on the main thread
    // and move the old cached instance to recycle pool
    bool IsDirty() const
    {
        return m_Dirty ;//|| m_BuiltinParamValues.isDirty || m_TransformState.worldViewMatrixDirty;
    }

    // call this after creating a new cached instance on the main thread, it makes sure transform state is not stale
    // and resets the dirty flag
    void Update()
    {
//        m_TransformState.UpdateWorldViewMatrix(m_BuiltinParamValues);
//        m_BuiltinParamValues.isDirty = false;
        m_Dirty = false;
    }

    void SetInsideFrame(bool val) { m_InsideFrame = val; m_Dirty = true; }
    bool GetInsideFrame() const { return m_InsideFrame; }

    void SetActiveCubemapFace(CubemapFace val) { m_ActiveCubemapFace = val; m_Dirty = true; }
    CubemapFace GetActiveCubemapFace() const { return m_ActiveCubemapFace; }

    void SetActiveMipLevel(int val) { m_ActiveMipLevel = val; m_Dirty = true; }
    int GetActiveMipLevel() const { return m_ActiveMipLevel; }

    void SetActiveDepthSlice(int val) { m_ActiveDepthSlice = val; m_Dirty = true; }
    int GetActiveDepthSlice() const { return m_ActiveDepthSlice; }

//    void SetStereoActiveEye(StereoscopicEye val) { m_StereoActiveEye = val; m_Dirty = true; }
//    StereoscopicEye GetStereoActiveEye() const { return m_StereoActiveEye; }

//    void SetSinglePassStereo(SinglePassStereo val) { m_SinglePassStereo = val; m_Dirty = true; }
//    SinglePassStereo GetSinglePassStereo() const { return m_SinglePassStereo; }

    void SetInstanceCountMultiplier(int val) { m_InstanceCountMultiplier = val; m_Dirty = true; }
    int GetInstanceCountMultiplier() const
    {
        if (m_InstanceCountMultiplier == 0)
            return (m_SinglePassStereo == kSinglePassStereoInstancing ? 2 : 1);
        return m_InstanceCountMultiplier;
    }

    void SetSinglePassStereoEyeMask(TargetEyeMask val) { m_SinglePassStereoEyeMask = val; m_Dirty = true; }
    TargetEyeMask GetSinglePassStereoEyeMask() const { return m_SinglePassStereoEyeMask; }

    void SetInvertProjectionMatrix(bool val) { m_InvertProjectionMatrix = val; m_Dirty = true; }
    bool GetInvertProjectionMatrix() const { return m_InvertProjectionMatrix; }

    void SetUserBackfaceMode(bool val) { m_UserBackfaceMode = val; m_Dirty = true; }
    bool GetUserBackfaceMode() const { return m_UserBackfaceMode; }

    void SetForceCullMode(CullMode val) { m_ForceCullMode = val; m_Dirty = true; }
    CullMode GetForceCullMode() const { return m_ForceCullMode; }

    void SetGlobalDepthBias(float val) { m_GlobalDepthBias = val; m_Dirty = true; }
    float GetGlobalDepthBias() const { return m_GlobalDepthBias; }

    void SetGlobalSlopeDepthBias(float val) { m_GlobalSlopeDepthBias = val; m_Dirty = true; }
    float GetGlobalSlopeDepthBias() const { return m_GlobalSlopeDepthBias; }

    void SetActiveScaledHeight(UInt16 val) { m_ActiveScaledHeight = val; m_Dirty = true; }
    UInt16 GetActiveScaledHeight() const { return m_ActiveScaledHeight; }

#if UNITY_EDITOR || ENABLE_PLAYERCONNECTION
    void SetUsingAsyncDummyShader(bool val) { m_UsingAsyncDummyShader = val; m_Dirty = true; }
    bool GetUsingAsyncDummyShader() const { return m_UsingAsyncDummyShader; }
#endif // UNITY_EDITOR || ENABLE_PLAYERCONNECTION

    BuiltinShaderParamValues m_BuiltinParamValues;
    TransformState m_TransformState;

private:
    bool                m_InsideFrame;
    CubemapFace         m_ActiveCubemapFace;
    int                 m_ActiveMipLevel;
    int                 m_ActiveDepthSlice;
    StereoscopicEye     m_StereoActiveEye;
    SinglePassStereo    m_SinglePassStereo;
    TargetEyeMask       m_SinglePassStereoEyeMask;
    int                 m_InstanceCountMultiplier;
    bool                m_InvertProjectionMatrix;
    bool                m_UserBackfaceMode;
    // Override shader-specified cull mode with this
    // (kCullUnknown does not override)
    CullMode            m_ForceCullMode;
    float               m_GlobalDepthBias;
    float               m_GlobalSlopeDepthBias;
    UInt16              m_ActiveScaledHeight;
    Vector2f            m_StereoScale;
    Vector2f            m_StereoOffset;

#if UNITY_EDITOR || ENABLE_PLAYERCONNECTION
    bool                m_UsingAsyncDummyShader;
#endif // UNITY_EDITOR || ENABLE_PLAYERCONNECTION

    bool m_Dirty;
};



class GfxDevice {
public:
    enum RenderTargetFlags
    {
        kFlagDontRestoreColor   = (1 << 0),   // Xbox 360 specific: do not restore old contents to EDRAM
        kFlagDontRestoreDepth   = (1 << 1),   // Xbox 360 specific: do not restore old contents to EDRAM
        kFlagDontRestore        = kFlagDontRestoreColor | kFlagDontRestoreDepth,
        kFlagForceResolve       = (1 << 3),   // Xbox 360 specific: force a resolve to system RAM
        // currently this is used only in editor, so it is implemented only for editor gfx devices (gl/dx)
        kFlagForceSetRT         = (1 << 4),   // force SetRenderTargets call (bypass any caching involved and actually set RT)
        kFlagDontResolve        = (1 << 5),   // Skip AA resolve even when the previous RT was AA. Used when doing shenanigans with default backbuffers.
        kFlagReadOnlyDepth      = (1 << 6),   // Bind the depth surface as read-only (if supported by the gfxdevice; see GraphicsCaps::hasReadOnlyDepth)
        kFlagReadOnlyStencil    = (1 << 7),   // Bind the stencil surface as read-only (if supported by the gfxdevice; see GraphicsCaps::hasReadOnlyStencil)
    };

    // tracks if View/Proj matrices need to be reapplied to shader (i.e. after SetPass)
    enum ViewProjMatrixNeedApplyFlag
    {
        kMatricesToApplyFlagView    = 1 << 0,
        kMatricesToApplyFlagProj    = 1 << 1,
        kMatricesToApplyFlagNone    = 0
    };

    explicit GfxDevice(MemLabelRef label);
    virtual ~GfxDevice();

    GfxDevice(GfxDevice&) = delete;

    virtual void BeginFrame() = 0;
    virtual void EndFrame() = 0;
    // Check if device is in valid state. E.g. lost device on D3D9; in this case all rendering should be skipped.
    virtual bool    IsValidState() { return true; }

    inline bool     IsInsideFrame() const { return m_GfxContextData.GetInsideFrame(); }

    void ExecuteAsync(int count, GfxDeviceAsyncCommand::Func* func, GfxDeviceAsyncCommand::ArgScratch** scratches, const GfxDeviceAsyncCommand::Arg* arg, const JobFence& depends);
    MemLabelId GetMemoryLabel() { return m_MemoryLabel; }

    virtual void    SetViewport(const RectInt& rect) =0;
    virtual RectInt GetViewport() const =0;

    virtual void SetSRGBWrite(const bool) =0;
    virtual bool GetSRGBWrite() =0;

    virtual RenderSurfaceBase* AllocRenderSurface(bool colorSurface);
    // sizeof of appropriate platform struct
    virtual size_t              RenderSurfaceStructMemorySize(bool colorSurface) =0;

    virtual void DestroyRenderSurface(RenderSurfaceHandle& rs);
    virtual void                DeallocRenderSurface(RenderSurfaceBase* rs);

    // destroys platform-specific part of render surface
    virtual void DestroyRenderSurfacePlatform(RenderSurfaceBase* rs) =0;

    // TODO: we might need to extend it in the future, e.g. for multi-display
    virtual RenderSurfaceHandle GetBackBufferColorSurface()    { return m_BackBufferColor; }
    virtual RenderSurfaceHandle GetBackBufferDepthSurface()    { return m_BackBufferDepth; }

    virtual void    SetProjectionMatrix(const Matrix4x4f& matrix);

    // Calculate a device projection matrix given a Unity projection matrix, this needs to be virtual so that platforms can override if necessary.
    virtual void CalculateDeviceProjectionMatrix(Matrix4x4f& m, bool usesOpenGLTextureCoords, bool invertY) const;

    // VP matrix (kShaderMatViewProj) is updated when setting view matrix but not when setting projection.
    // Call UpdateViewProjectionMatrix() explicitly if you only change projection matrix.
    virtual void    UpdateViewProjectionMatrix();

    virtual void    SetViewMatrix(const Matrix4x4f& matrix);
    virtual void    SetWorldMatrix(const Matrix4x4f& matrix);

    const BuiltinShaderParamValues& GetBuiltinParamValues() const { return m_GfxContextData.m_BuiltinParamValues; }
    BuiltinShaderParamValues& GetBuiltinParamValues() { return m_GfxContextData.m_BuiltinParamValues; }

    virtual const Matrix4x4f& GetWorldViewMatrix() const;
    virtual const Matrix4x4f& GetWorldMatrix() const;
    virtual const Matrix4x4f& GetViewMatrix() const;
    virtual const Matrix4x4f& GetProjectionMatrix() const; // get projection matrix as passed from Unity (OpenGL projection conventions)
    virtual const Matrix4x4f& GetDeviceProjectionMatrix() const; // get projection matrix that will be actually used

    void SetInvertProjectionMatrix(bool val) { m_InvertProjectionMatrix = val; m_Dirty = true; }
    bool GetInvertProjectionMatrix() const { return m_InvertProjectionMatrix; }
protected:
    // Mutable state
    GfxContextData      m_GfxContextData;
    MemLabelId          m_MemoryLabel;

    // Immutable data
    GfxDeviceRenderer   m_Renderer;

    RenderSurfaceHandle m_BackBufferColor;
    RenderSurfaceHandle m_BackBufferDepth;

    ViewProjMatrixNeedApplyFlag m_ViewProjMatrixNeedApplyFlags;

private:
    bool                m_InvertProjectionMatrix;
    bool m_Dirty;
};

ENUM_FLAGS(GfxDevice::ViewProjMatrixNeedApplyFlag)

// GetGfxDevice() returns the graphics device that you should use in rendering code.
// It may be a client device which sends commands to a separate thread (to the real device),
// or it may be a native GfxDevice used to generate command buffers in a graphics job.
EXPORT_COREMODULE GfxDevice& GetGfxDevice();

// Is the graphics device initialized? (TODO: Rename)
bool                IsGfxDevice();
void                SetGfxDevice(GfxDevice* device);

inline AutoGfxDeviceBeginEndFrame::AutoGfxDeviceBeginEndFrame() :
        m_Success(true), m_NeedsEndFrame(false)
{
    GfxDevice& device = GetGfxDevice();
    if (!device.IsInsideFrame())
    {
        device.BeginFrame();
        m_Success = device.IsValidState();
        m_NeedsEndFrame = true;
    }
}

inline AutoGfxDeviceBeginEndFrame::~AutoGfxDeviceBeginEndFrame()
{
    End();
}

inline void AutoGfxDeviceBeginEndFrame::End()
{
    if (m_NeedsEndFrame)
        GetGfxDevice().EndFrame();
    m_NeedsEndFrame = false;
}

class GfxThreadableDevice : public GfxDevice
{
public:
    GfxThreadableDevice(MemLabelRef label) : GfxDevice(label) {}
    //! Called by the worker thread on thread startup
    virtual void    OnDeviceCreated(bool /*callingFromRenderThread*/) {}

//    virtual void    SetShadersMainThread(const ShaderLab::SubPrograms& programs, const ShaderPropertySheet* local, const ShaderPropertySheet* global)
//    {
//        ErrorString("Don't call SetShadersMainThread on threadable device! Use GraphicsHelper instead");
//    }

//    virtual void    SetShadersThreadable(GpuProgram* programs[kShaderTypeCount], const GpuProgramParameters* params[kShaderTypeCount], UInt8 const * const paramsBuffer[kShaderTypeCount]) {}
//
//    virtual void    SetGeometryRayTracingShaderMainThread(RayTracingProgramHandle& rpHandle, UInt32 geometryIndex, const ShaderLab::SubProgram* subProgram, const ShaderPropertySheet* localShaderProperties, const ShaderPropertySheet* globalShaderProperties)
//    {
//        ErrorString("Don't call SetGeometryRayTracingShaderMainThread on threadable device! Use GraphicsHelper instead");
//    }
//
//    virtual void    SetGeometryRayTracingShaderThreadable(RayTracingProgramHandle& rpHandle, UInt32 geometryIndex, GpuProgram* gpuProgram, const GpuProgramParameters* gpuProgramParameters, UInt8 const * const paramsBuffer) {}
//
//    // A shortcut for Client/worker threading in BeginBufferWrite(): It first attempts to call his directly (in the main thread),
//    // and if the buffer can be mapped (or is already mapped) without getting the render thread involved at all,
//    // this function returns the mapped pointer, or NULL if mapping is not possible from the main thread.
//    // In this case the GfxDeviceClient will fall back to old-style threaded buffer update.
//    virtual void*   BeginBufferWriteThreadSafe(GfxBuffer* buffer, size_t offset = 0, size_t size = 0) { return NULL; }
//
//    // If the previous call to BeginBufferWriteThreadSafe succeeded (returned non-NULL)
//    // If the function returns true user needs to do no more.
//    // If the function returns false there has to be some extra device work and EndBufferWrite will be called on the buffer
//    virtual bool    EndBufferWriteThreadSafe(GfxBuffer* buffer, size_t bytesWritten) { return false;  }
//
//    virtual bool IsCPUFencePassed(UInt32 fence) const
//    {
//        // Same test as WaitOnCPUFence to handle wrap-around cases
//        return SInt32(fence - m_CurrentCPUFence) <= 0;
//    }
};

GfxDevice&          GetUncheckedRealGfxDevice();

#endif //HUAHUOENGINE_GFXDEVICE_H
