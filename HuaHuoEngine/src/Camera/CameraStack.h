//
// Created by VincentZhang on 5/14/2022.
//

#ifndef HUAHUOENGINE_CAMERASTACK_H
#define HUAHUOENGINE_CAMERASTACK_H

#include "BaseClasses/PPtr.h"
#include "GfxDevice/GfxDeviceTypes.h"

class Camera;
class CameraStack {

};

// State kept around while rendering a single stack of cameras.
// Things like whether we're in HDR rendering right now; or what kind of temporary
// built-in buffers we're using etc.
class CameraStackRenderingState
{
public:
    enum TargetType
    {
        kGenerated,
        kUserDefined,       // rendering to user defined RT
        kToScreen,          // currently rendering into final target (after all image fx)
        kStereo,                // Render to VR buffer
        kStereoDirectToEyeTexture ,         // render directly to eye textures.
        kStereoResolveToEyeTexture, // render to temp and resolve to eye texture.
    };

    CameraStackRenderingState();
    void SetCurrentCamera(Camera* cam);
    Camera* GetCurrentCamera() const;
    void BeginRenderingStack(const CameraStack& stack, const bool firstStack);
    void BeginRenderingOneCamera(Camera& cam);
    void ReleaseResources();

//    // currently we still allow to update camera's targetTexture/target-buffers from script while it is rendering.
//    // i agree this is not quite reasonable thing to do but this is possible right now so lets keep the possibility for some time
//    void UpdateCameraTargetTexture(RenderTexture* rt);
//
//    RenderTexture* GetOrCreateBuiltinRT(BuiltinRenderTextureType type, int width, int height, DepthBufferFormat depthFormat, GraphicsFormat colorFormat, UInt32 flags, VRTextureUsage vrUsage, int antiAliasing);
//    RenderTexture* GetBuiltinRT(BuiltinRenderTextureType type);
//    RenderTexture* GetTargetTexture();
//    TargetType GetTarget() const { return m_TargetType; }
//    RenderTexture* GetSrcTextureForImageFilters() const;
//    RenderTexture* GetDstTextureForImageFilters() const;
//    bool IsRenderingLastCamera() const;
//    bool IsRenderingFirstCamera() const;
//    bool ShouldResolveLastTarget() const;
//    RenderTexture* GetAfterFinalCameraTarget() const;
    void SetCurrentlyRenderingEye(StereoscopicEye eye);
    void EndStereoRendering();
    const Camera* GetFirstCamera() const { return m_FirstCamera; }

private:
    void SetupLastEyeCameras(const CameraStack & stack);
//    RenderTexture* GetImageEffectTexture(bool src) const;
//    TargetType CalculateStereoCameraTargetType(const std::vector<PPtr<Camera> >& cameras) const;
//    TargetType CalculateCameraTargetType(const std::vector<PPtr<Camera> >& cameras) const;
//    RenderTexture* GetStereoImageEffectTexture(bool src) const;
//    RenderTextureDesc GetCameraStackTempEyeTextureDesc();
//    RenderTextureDesc GetCameraStackTempTextureDesc();
//    void ValidateRenderViewportScale();
//    bool IsRenderingToScalableBuffer() const;
//
//    RenderTexture*  m_TempBuffers[kBuiltinRTCount];
//    StereoRenderTexture m_TempInitialEyePair;
    TargetType      m_TargetType;

    Camera* m_CurrentCamera;
    const Camera* m_FirstCamera;
    const Camera* m_LastCamera;
    const Camera* m_LastLeftEyeCamera;
    const Camera* m_LastRightEyeCamera;
    // CameraTargetsAndRect    m_CameraTarget;
    StereoscopicEye m_Eye;
    bool m_ForceUseRT;  // Force rendering into RT?
    bool m_ForceUseTempRT;  // Don't use Camera.targetTexture for m_ForceUseRT, always create separate temporary RenderTexture
    bool m_HDR;     // Does any camera in the whole stack want to use HDR rendering?
    bool m_HasDeferred;         // Does any camera in the whole stack need deferred?
    bool m_MSAA;                    // Does any camera have MSAA
    bool m_DynamicResolution; //Does any camera in the stack want Dynamic Resolution?
    bool m_FirstStack; // is this the first camera stack we are rendering?
    bool m_HasCommandBuffers;
};


// Helper class for single-shot rendering of one camera - embeds a stack rendering state,
// starts rendering it in the constructor; ends in destructor.
class AutoScopedCameraStackRenderingState
{
public:
    AutoScopedCameraStackRenderingState(Camera& camera);
    ~AutoScopedCameraStackRenderingState();
    CameraStackRenderingState& GetStackState() { return m_StackState; }

private:
    Camera* m_PrevCamera;
    CameraStackRenderingState* m_PrevCameraStackState;
    CameraStackRenderingState m_StackState;
};


#endif //HUAHUOENGINE_CAMERASTACK_H
