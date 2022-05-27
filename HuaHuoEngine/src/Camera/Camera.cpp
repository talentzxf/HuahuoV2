//
// Created by VincentZhang on 5/13/2022.
//

#include "Camera.h"
#include "Components/Transform/Transform.h"
#include "RendererScene.h"
#include "Input/TimeManager.h"
#include "GfxDevice/GfxDevice.h"
#include "RenderManager.h"
#include "CameraStack.h"
#include "Graphics/ScriptableRenderLoop/ScriptableRenderContext.h"
#include "CameraUtil.h"
#include "CullResults.h"
#include "SceneManager/HuaHuoScene.h"
#include "Graphics/RenderTexture.h"
#include "Camera/RenderLoops/RenderLoop.h"
#include "Graphics/GraphicsHelper.h"
#include "Shaders/ShaderImpl/ShaderUtilities.h"
#include "Shaders/ShaderKeyWords.h"

static SHADERPROP(CameraDepthTexture);
static SHADERPROP(CameraDepthNormalsTexture);
static SHADERPROP(CameraODSWorldTexture);
static SHADERPROP(LastCameraDepthTexture);
static SHADERPROP(LastCameraDepthNormalsTexture);
static SHADERPROP(Reflection);

Camera::CopiableState::CopiableState()
        :   m_FieldOfViewBeforeEnablingVRMode(0.0f)
        ,   m_TargetDisplay(0)
        ,   m_TargetEye(kTargetEyeMaskBoth)
        ,   m_DepthTextureMode(0)
        ,   m_DirtyProjectionMatrix(true)
        ,   m_DirtySkyboxProjectionMatrix(true)
        ,   m_ClearStencilAfterLightingPass(false)
        ,   m_FocalLength(50.0f)
        ,   m_SensorSize(36, 24)
        ,   m_LensShift(0, 0)
        ,   m_GateFitMode(kGateFitHorizontal)
        ,   m_FOVAxisMode(kVertical)
#if UNITY_EDITOR
,   m_AnimateMaterials(false)
    ,   m_AnimateMaterialsTime(0.0f)
#endif
{
    m_CullingMask.m_Bits = 0xFFFFFFFF;
    m_EventMask.m_Bits = 0xFFFFFFFF;
    m_SceneCullingMaskOverride = 0;

    for (int i = 0; i < 32; i++)
        m_LayerCullDistances[i] = 0;
    m_LayerCullSpherical = false;
    m_OpaqueSortMode = kOpaqueSortDefault;
    m_ProjectionMatrixMode = kProjectionMatrixModeImplicit;
    m_ImplicitWorldToCameraMatrix = true;
    m_ImplicitCullingMatrix = true;
    m_ImplicitAspect = true;
    m_ImplicitSkyboxProjectionMatrix = true;
    m_AllowHDR = true;
    m_UsingHDR = false;
    m_AllowMSAA = true;
    m_AllowDynamicResolution = false;
    m_ForceIntoRT = false;
    m_Aspect = 1.0F;
    m_CameraType = kCameraTypeGame;
//    m_TransparencySortMode = (TransparencySortMode)GetGraphicsSettings().GetTransparencySortMode();
//    m_TransparencySortAxis = GetGraphicsSettings().GetTransparencySortAxis();
    m_ImplicitTransparencySortSettings = true;

    m_Velocity = Vector3f::zero;
    m_LastPosition = Vector3f::zero;
    m_WorldToCameraMatrix = m_WorldToClipMatrix = m_ProjectionMatrix = m_SkyboxProjectionMatrix = m_CullingMatrix = Matrix4x4f::identity;
    m_OcclusionCulling = true;

    m_TargetColorBufferCount = 1;
    ::memset(m_TargetColorBuffer, 0x00, sizeof(m_TargetColorBuffer));
    ::memset(m_TargetBuffersOriginatedFrom, 0x00, sizeof(m_TargetBuffersOriginatedFrom));

    m_TargetColorBuffer[0] = RenderSurfaceHandle();
    m_TargetDepthBuffer = RenderSurfaceHandle();

    m_StereoSeparation = 0.022f;
    m_StereoConvergence = 10.0f;
    m_StereoFrameCounter = 0;

    m_StereoViewMatrixMode = kStereoViewMatrixModeImplicit;
    m_ImplicitStereoProjectionMatrices = true;
    for (int i = 0; i < kStereoscopicEyeCount; ++i)
    {
        m_StereoViewMatrices[i] = Matrix4x4f::identity;
        m_StereoProjectionMatrices[i] = Matrix4x4f::identity;
    }

    m_Orthographic = false;

    m_Scene = NULL;
}

void Camera::CopiableState::Reset()
{
    m_FocalLength = 50.0f;
    m_GateFitMode = kGateFitHorizontal;
    m_FOVAxisMode = kVertical;
    m_SensorSize = Vector2f(36, 24);
    m_LensShift = Vector2f(0, 0);
    m_NormalizedViewPortRect = Rectf(0, 0, 1, 1);
    m_ProjectionMatrixMode = kProjectionMatrixModeImplicit;
//    m_BackGroundColor = ColorRGBA32(49, 77, 121, 0);
    m_Depth = 0.0F;
    m_NearClip = 0.3F;
    m_FarClip = 1000.0F;
    m_RenderingPath = -1;
    m_Aspect = 1.0F;
    m_Orthographic = false;
    m_AllowHDR = true;
    m_AllowMSAA = true;
    m_ForceIntoRT = false;
    m_OpaqueSortMode = kOpaqueSortDefault;
//    m_TransparencySortMode = (TransparencySortMode)GetGraphicsSettings().GetTransparencySortMode();
//    m_TransparencySortAxis = GetGraphicsSettings().GetTransparencySortAxis();
    m_ImplicitTransparencySortSettings = true;
    m_CullingMask.m_Bits = 0xFFFFFFFF;
    m_TargetTexture = nullptr;
    m_AllowDynamicResolution = false;

    m_OrthographicSize = 5.0F;
    m_FieldOfView = 60.0F;
    m_FieldOfViewBeforeEnablingVRMode = 0.0f;
//    m_ClearFlags = kSkybox;
    m_DirtyProjectionMatrix = m_DirtySkyboxProjectionMatrix = true;
    m_TargetDisplay = 0;
    m_TargetEye = kTargetEyeMaskBoth;

    m_Scene = NULL;
}

// Returns true if this is a non-standard (e.g. off-center) projection.
static bool IsNonStandardProjection(const Matrix4x4f& mat)
{
    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 3; j++)
            if (i != j && mat.Get(i, j) != 0.0f)
                return true;
    return false;
}

//Camera::Camera(/*MemLabelId label,*/ ObjectCreationMode mode)
//        :   Super(/*label,*/ mode)
////        ,   m_RenderEvents(label, kRenderEventCount)
////#if UNITY_EDITOR
////        ,   m_EditorCullResults(NULL)
////    ,   m_FilterMode(0)
////    ,   m_IsSceneCamera(false)
////    ,   m_StackState(NULL)
////    ,   m_LastDrawingMode(kEditorDrawModeCount)
////#endif
//        ,   m_IsRendering(false)
//        ,   m_IsRenderingStereo(false)
//        ,   m_IsStandaloneCustomRendering(false)
//        ,   m_IsCulling(false)
//        ,   m_IsNonJitteredProjMatrixSet(false)
//        ,   m_UseJitteredProjMatrixForTransparent(true)
//        ,   m_BuffersSetFromScripts(false)
////        ,   m_CurrentTargetTexture(NULL)
////        ,   m_DepthTexture(NULL)
////        ,   m_DepthNormalsTexture(NULL)
////        ,   m_ODSWorldTexture(NULL)
////        ,   m_ODSWorldShader(NULL)
////        ,   m_VRIgnoreImplicitCameraUpdate(false)
////        ,   m_gateFittedFOV(60.0f)
////        ,   m_gateFittedLensShift(Vector2f::zero)
//{
////    m_RenderLoop = CreateRenderLoop(*this);
////    m_ShadowCache = CreateShadowMapCache();
////
////    for (int eye = 0; eye < kStereoscopicEyeCount; ++eye)
////        m_IsStereoNonJitteredProjMatrixCopied[eye] = false;
////
////    {
////        ReadWriteSpinLock::AutoWriteLock autoLock(s_AllCameraLock);
////        s_AllCamera->push_back(this);
////    }
//}

//void Camera::AddToManager()
//{
//    GetRenderManager().AddCamera(this);
//    if (m_State.m_ImplicitAspect)
//        ResetAspect();
//    m_State.m_LastPosition = GetComponent<Transform>().GetPosition();
//    m_State.m_Velocity.SetZero();
//    InitializePreviousViewProjectionMatrix();
//}
//
//void Camera::RemoveFromManager()
//{
//    // Clear IntermediateRenderers such that if the camera is disabled before rendering occurs we dont introduce a leak (case 819470)
//    GetIntermediateRendererManager().ClearIntermediateRenderers(GetInstanceID());
//    GetRenderManager().RemoveCamera(this);
//}

void Camera::AwakeFromLoad(AwakeFromLoadMode awakeMode)
{
    if ((awakeMode & kDidLoadFromDisk) == 0 && IsAddedToManager())
    {
        GetRenderManager().RemoveCamera(this);
        GetRenderManager().AddCamera(this);
    }

//#if UNITY_EDITOR
//    if (IsPreviewSceneObject() && IsAddedToManager())
//        GetRenderManager().RemoveCamera(this);
//#endif
//
//    // Normally MonoBehaviour::AddToManager()/RemoveFromManager() handles the image effect callbacks.
//    // However, if camera is instantiated/created from a script we need to add callbacks for
//    // possible components that have been enabled & added to manager before the camera.
//    if ((awakeMode & kInstantiateOrCreateFromCodeAwakeFromLoad))
//    {
//        GameObject& gameObject = GetGameObject();
//        for (int i = 0; i < gameObject.GetComponentCount(); i++)
//        {
//            if (gameObject.GetComponentTypeAtIndex(i)->IsDerivedFrom<MonoBehaviour>())
//            {
//                MonoBehaviour& behaviour = static_cast<MonoBehaviour&>(gameObject.GetComponentAtIndex(i));
//                if (behaviour.GetEnabled() && behaviour.IsAddedToManager())
//                    behaviour.AddImageEffectCallbacksToManagers();
//            }
//        }
//    }
//
//    if (GetIVRDevice() && GetStereoEnabled())
//        GetIVRDevice()->InsertCameraReferenceTransform(*this);
//
//    m_State.m_DirtyProjectionMatrix = m_State.m_DirtySkyboxProjectionMatrix = true;
//    if (m_State.m_ImplicitAspect)
//        ResetAspect();
}

void Camera::Reset()
{
    Super::Reset();

    m_State.Reset();
}

void Camera::SmartReset()
{
    Super::SmartReset();
    m_State.Reset();
}

Vector3f Camera::GetPosition() const
{
    return GetComponent<Transform>().GetPosition();
}

void Camera::Render() {
    GetRendererScene().BeginCameraRender();
    StandaloneRender(Camera::kRenderFlagSetRenderTarget, "");
    GetRendererScene().EndCameraRender();
}

void Camera::UpdateVelocity()
{
    Vector3f curPosition = GetPosition();
    m_State.m_Velocity = (curPosition - m_State.m_LastPosition) * GetInvDeltaTime();
    m_State.m_LastPosition = curPosition;
}

bool Camera::IsValidToRender() const
{
    if (m_State.m_NormalizedViewPortRect.IsEmpty())
        return false;
    if (m_State.m_NormalizedViewPortRect.x >= 1.0F || m_State.m_NormalizedViewPortRect.GetRight() <= 0.0F)
        return false;
    if (m_State.m_NormalizedViewPortRect.y >= 1.0F || m_State.m_NormalizedViewPortRect.GetBottom() <= 0.0F)
        return false;

    if (m_State.m_FarClip <= m_State.m_NearClip)
        return false;
    if (!m_State.m_Orthographic)
    {
        if (m_State.m_NearClip <= 0.0f)
            return false; // perspective camera needs positive near plane
        if (Abs(m_State.m_FieldOfView) < 1.0e-6f)
            return false; // field of view has to be non zero
    }
    else
    {
        if (Abs(m_State.m_OrthographicSize) < 1.0e-6f)
            return false; // orthographic size has to be non zero
    }
    return true;
}

RectInt Camera::GetScreenViewportRectInt(bool adjustForDynamicScale) const
{
    return RectfToRectInt(GetScreenViewportRect(adjustForDynamicScale));
}

void Camera::GetClipToWorldMatrix(Matrix4x4f& outMatrix) const
{
    Matrix4x4f::Invert_Full(GetWorldToClipMatrix(), outMatrix);
}

float Camera::GetStereoSeparation() const
{
//    if (GetStereoEnabled() && m_State.m_StereoViewMatrixMode != kStereoViewMatrixModeExplicitUnsafeForSingleCull)
//    {
//        return GetVRDeviceStereoSeparation();
//    }

    return m_State.m_StereoSeparation;
}

CameraRenderingParams Camera::ExtractCameraRenderingParams() const
{
    CameraRenderingParams params;
    params.matView = GetWorldToCameraMatrix();
    params.matProj = GetProjectionMatrix();
    params.worldPosition = GetCameraToWorldMatrix().GetPosition();
    params.stereoSeparation = GetStereoSeparation();
    return params;
}

void Camera::SetupRender(ShaderPassContext& passContext, RenderFlag renderFlags)
{
    SetupRender(passContext, ExtractCameraRenderingParams(), renderFlags);
}

bool Camera::ApplyRenderTexture()
{
    // while we could return const ref (and grab address and use uniformly)
    // we pass non const pointer to SetActive (because surfaces can be reset internally)
    // so create local handle copy if we draw to real texture
    RenderSurfaceHandle rtcolor = m_CurrentTargetTexture ? m_CurrentTargetTexture->GetColorSurfaceHandle() : RenderSurfaceHandle();

    RenderSurfaceHandle defaultColor[kMaxSupportedRenderTargets];
    CompileTimeAssert(sizeof(defaultColor) == sizeof(m_State.m_TargetColorBuffer), "defaultColor array size needs to be updated!");
    memcpy(defaultColor, m_State.m_TargetColorBuffer, sizeof(defaultColor));

    if (!defaultColor[0].IsValid())
        defaultColor[0] = GetGfxDevice().GetBackBufferColorSurface();
    RenderSurfaceHandle defaultDepth = m_State.m_TargetDepthBuffer;
    if (!defaultDepth.IsValid())
        defaultDepth = GetGfxDevice().GetBackBufferDepthSurface();

    RenderSurfaceHandle* color  = m_CurrentTargetTexture ? &rtcolor : defaultColor;
    RenderSurfaceHandle  depth  = m_CurrentTargetTexture ? m_CurrentTargetTexture->GetDepthSurfaceHandle() : defaultDepth;
    int                  count  = m_CurrentTargetTexture ? 1 : m_State.m_TargetColorBufferCount;
    RenderTexture**      rt     = m_CurrentTargetTexture ? &m_CurrentTargetTexture : m_State.m_TargetBuffersOriginatedFrom;

    // if we have configured a depth buffer in scripts be sure to favour it over the m_CurrentTargetTexture or defaultDepth
    if (m_BuffersSetFromScripts)
        depth = m_State.m_TargetDepthBuffer;

    // if we have set buffers from scripts we don't want calls to ApplyRenderTexture to changes that behaviour (e.g. at end of RenderForwardShadowMaps)
    if ((!m_CurrentTargetTexture) && (!m_BuffersSetFromScripts))
        m_CurrentTargetTexture = rt[0];


    int depthSlice = m_CurrentTargetTexture != NULL ? GetRenderLoopDefaultDepthSlice(GetSinglePassStereo()) : 0;
    RenderTexture::SetActive(count, color, depth, rt, 0, kCubeFaceUnknown, depthSlice, RenderTexture::kFlagDontSetViewport);

    bool backBuffer = color->IsValid() && color[0].object->backBuffer;
    return backBuffer;
}

SinglePassStereo Camera::GetSinglePassStereo() const
{
#if GFX_SUPPORTS_SINGLE_PASS_STEREO
    IVRDevice* vrDevice = GetIVRDevice();
    // We can render just once for both eyes if we can single cull and we support either instancing or multiview
    if (GetStereoEnabled() && GetStereoSingleCullEnabled() && vrDevice && vrDevice->IsSinglePassStereoAllowed())
    {
        return GraphicsHelper::GetSinglePassStereoForStereoRenderingPath(vrDevice->GetStereoRenderingPath());
    }
#endif
    return kSinglePassStereoNone;
}

Rectf Camera::GetRenderRectangle() const
{
    // when we setup camera thats how we determine m_CurrentTargetTexture:
    // 1. image filters customize if needed
    // 2. target RenderTexture otherwise
    // 3. we call Camera::ApplyRenderTexture where we handle case of targeting RenderBuffers
    // we want special processing ONLY in case of image filters tweaking it
    bool useRenderTargetSize = false;

    if (m_CurrentTargetTexture)
    {
        const bool renderToTargetRB = m_CurrentTargetTexture == m_State.m_TargetBuffersOriginatedFrom[0];
        const bool renderToTargetRT = m_CurrentTargetTexture == (RenderTexture*)m_State.m_TargetTexture;
        const bool isStereoTexture = false; //m_CurrentTargetTexture->GetVRUsage() != kVRTextureUsageNone;
        const bool isVRRenderingToNonFullScreenViewport = false; //GetIVRDevice() && GetIVRDevice()->IsCurrentlyStereoRenderTarget() && !GetIVRDevice()->IsViewportFullscreen();
        bool currentlyRenderingInStereo = isStereoTexture && m_IsRenderingStereo;
        bool pluginNeedsFullRTSize = false;

#if GFX_SUPPORTS_RENDERING_EXT_PLUGIN
        pluginNeedsFullRTSize = PluginsIssueRenderingExtQuery(kUnityRenderingExtQueryOverrideVRSinglePass, kGfxDeviceRenderingExtQueryMethodAny);
#endif
        if ((!currentlyRenderingInStereo || pluginNeedsFullRTSize) && !renderToTargetRB && !renderToTargetRT && !isVRRenderingToNonFullScreenViewport)
            useRenderTargetSize = true;
    }

    Rectf viewRect;
    if (useRenderTargetSize)
    {
        viewRect = Rectf(0.0f, 0.0f, m_CurrentTargetTexture->GetScaledWidth(), m_CurrentTargetTexture->GetScaledHeight());
    }
    else
    {
        viewRect = GetPhysicalViewportRect();
    }
    return viewRect;
}

void Camera::SetRenderTargetAndViewport()
{
    m_CurrentTargetTexture = EnsureRenderTextureIsCreated(m_CurrentTargetTexture);
    bool backBuffer = ApplyRenderTexture();

    Rectf viewport = backBuffer ? GetPhysicalViewportRect() : GetRenderRectangle();
    RectInt viewcoord = RectfToRectInt(viewport);
    GetGfxDevice().SetViewport(viewcoord);
}

void Camera::CalculateMatrixShaderProps(const Matrix4x4f& inView, Matrix4x4f& outWorldToCamera, Matrix4x4f& outCameraToWorld)
{
    // World to camera matrix
    outWorldToCamera.SetScale(Vector3f(1, 1, -1));
    outWorldToCamera *= inView;

    // Camera to world matrix
    InvertMatrix4x4_General3D(outWorldToCamera.GetPtr(), outCameraToWorld.GetPtr());
}

void Camera::SetCameraShaderProps(ShaderPassContext& passContext, const CameraRenderingParams& params)
{
    float overrideTime = -1.0f;
#   if UNITY_EDITOR
    if (m_State.m_AnimateMaterials)
        overrideTime = m_State.m_AnimateMaterialsTime;
#   endif // if UNITY_EDITOR
    ShaderLab::UpdateGlobalShaderProperties(overrideTime);

    GfxDevice& device = GetGfxDevice();
    BuiltinShaderParamValues& shaderParams = device.GetBuiltinParamValues();

    shaderParams.SetVectorParam(kShaderVecWorldSpaceCameraPos, Vector4f(params.worldPosition, 0.0f));

    Matrix4x4f worldToCamera;
    Matrix4x4f cameraToWorld;
    CalculateMatrixShaderProps(params.matView, worldToCamera, cameraToWorld);
    shaderParams.SetMatrixParam(kShaderMatWorldToCamera, worldToCamera);
    shaderParams.SetMatrixParam(kShaderMatCameraToWorld, cameraToWorld);

    // Get the matrix to use for cubemap reflections.
    // It's camera to world matrix; rotation only, and mirrored on Y.
    worldToCamera.SetPosition(Vector3f::zero);  // clear translation
    Matrix4x4f invertY;
    invertY.SetScale(Vector3f(1, -1, 1));
    Matrix4x4f reflMat;
    MultiplyMatrices4x4(&worldToCamera, &invertY, &reflMat);
    passContext.properties.SetMatrix(kSLPropReflection, reflMat);

    // Camera clipping planes
    SetClippingPlaneShaderProps();

    const float projNear = GetProjectionNear();
    const float projFar = GetProjectionFar();
    const float invNear = (projNear == 0.0f) ? 1.0f : 1.0f / projNear;
    const float invFar = (projFar == 0.0f) ? 1.0f : 1.0f / projFar;
    shaderParams.SetVectorParam(kShaderVecProjectionParams, Vector4f(device.GetInvertProjectionMatrix() ? -1.0f : 1.0f, projNear, projFar, invFar));

    Rectf view = GetScreenViewportRect();
    shaderParams.SetVectorParam(kShaderVecScreenParams, Vector4f(view.width, view.height, 1.0f + 1.0f / view.width, 1.0f + 1.0f / view.height));

    // From http://www.humus.name/temp/Linearize%20depth.txt
    // But as depth component textures on OpenGL always return in 0..1 range (as in D3D), we have to use
    // the same constants for both D3D and OpenGL here.
    double zc0, zc1;
    // OpenGL would be this:
    // zc0 = (1.0 - projFar / projNear) / 2.0;
    // zc1 = (1.0 + projFar / projNear) / 2.0;
    // D3D is this:
    zc0 = 1.0 - projFar * invNear;
    zc1 = projFar * invNear;

    Vector4f v = Vector4f(zc0, zc1, zc0 * invFar, zc1 * invFar);
    if (GetGraphicsCaps().usesReverseZ)
    {
        v.y += v.x;
        v.x = -v.x;
        v.w += v.z;
        v.z = -v.z;
    }
    shaderParams.SetVectorParam(kShaderVecZBufferParams, v);

    // Ortho params
    Vector4f orthoParams;
    const bool isPerspective = params.matProj.IsPerspective();
    orthoParams.x = m_State.m_OrthographicSize * m_State.m_Aspect;
    orthoParams.y = m_State.m_OrthographicSize;
    orthoParams.z = 0.0f;
    orthoParams.w = isPerspective ? 0.0f : 1.0f;
    shaderParams.SetVectorParam(kShaderVecOrthoParams, orthoParams);

    // Camera projection matrices
    Matrix4x4f invProjMatrix;
    InvertMatrix4x4_Full(params.matProj.GetPtr(), invProjMatrix.GetPtr());
    shaderParams.SetMatrixParam(kShaderMatCameraProjection, params.matProj);
    shaderParams.SetMatrixParam(kShaderMatCameraInvProjection, invProjMatrix);

#if GFX_SUPPORTS_SINGLE_PASS_STEREO
    // Set stereo matrices to make shaders with UNITY_SINGLE_PASS_STEREO enabled work in mono
    // View and projection are handled by the device
    device.SetStereoMatrix(kMonoOrStereoscopicEyeMono, kShaderMatCameraInvProjection, invProjMatrix);
    device.SetStereoMatrix(kMonoOrStereoscopicEyeMono, kShaderMatWorldToCamera, worldToCamera);
    device.SetStereoMatrix(kMonoOrStereoscopicEyeMono, kShaderMatCameraToWorld, cameraToWorld);
#endif

    if (passContext.keywords.IsEnabled(keywords::kStereoCubemapRenderOn))
    {
        float halfStereoSeparation = 0.5f * params.stereoSeparation;
        GfxDevice& device = GetGfxDevice();
        float eyeIndex = device.GetBuiltinParamValues().GetWritableVectorParam(kShaderVecStereoEyeIndex).x;

        if (eyeIndex == 0)
        {
            //left eye gets negative separation value
            halfStereoSeparation *= -1.0f;
        }
        v = Vector4f(halfStereoSeparation, 0, 0, 0);
        shaderParams.SetVectorParam(kShaderVecHalfStereoSeparation, v);
    }
}

void Camera::SetupRender(ShaderPassContext& passContext, const CameraRenderingParams& params, RenderFlag renderFlags)
{
    GfxDevice& device = GetGfxDevice();

//    SetActiveVRUsage();
//
//    // Cache whether we use HDR for rendering.
//    m_State.m_UsingHDR = CalculateUsingHDR();
//    if (m_State.m_UsingHDR)
//        passContext.keywords.Enable(keywords::kHDROn);
//    else
//        passContext.keywords.Disable(keywords::kHDROn);

//    // want linear lighting?
//    bool linearLighting = GetActiveColorSpace() == kLinearColorSpace;
//    device.SetSRGBWrite(linearLighting);

    if (renderFlags & kRenderFlagSetRenderTarget)
        SetRenderTargetAndViewport();

    GraphicsHelper::SetWorldViewAndProjection(device, NULL, &params.matView, &params.matProj);
    SetCameraShaderProps(passContext, params);

//    // Setup billboard rendering.
//    BillboardBatchManager::SetBillboardShaderProps(
//            passContext.keywords,
//            device.GetBuiltinParamValues(),
//            GetQualitySettings().GetCurrent().billboardsFaceCameraPosition,
//            params.matView,
//            params.worldPosition);


//    GetRenderBufferManager().GetTextures().SetActiveVRUsage(kVRTextureUsageNone);
}


void Camera::CustomRenderWithPipeline(ShaderPassContext& passContext, RenderFlag renderFlags, PostProcessCullResults* postProcessCullResults, void* postProcessCullResultsData/*, ScriptingObjectPtr requests*/){
    // If camera's viewport rect is empty or invalid or there's no visible nodes, do nothing.
    // If a render pipeline is set, that means the cull results will be updated later by the pipeline.
    if (!IsValidToRender())
        return;

    if (m_IsRendering)
    {
        WarningStringObject(Format("Attempting to render from camera \'%s\' that is current being used for rendering. Create a copy of the camera (Camera.CopyFrom) if you wish to do this.", GetGameObject().GetName()), this);
        return;
    }

    // Begin the frame here instead of in the player loop so we can don't need to wait for the previous frame to start culling.
    GfxDevice& device = GetGfxDevice();
    if (!device.IsInsideFrame())
        device.BeginFrame();

    m_IsRendering = true;

    // Calling code should setup current camera stack renering state, and current camera
    // PPtr<Camera> currentCam = GetCurrentCameraPtr();
    // AssertFormatMsg(currentCam == PPtr<Camera>(this), "Rendering camera '%s', but calling code does not set it up as current camera (current camera: '%s')", GetName(), currentCam.IsValid() ? currentCam->GetName() : "<null>");

//    // @TODO:  do we still need to issue these? -va
//    INVOKE_GLOBAL_CALLBACK(beforeCameraRender, *this);

    // as we sent PreRender events, users had one more chance to tweak camera setup - make sure it is still valid to render
    if (!IsValidToRender())
    {
        WarningStringObject(Format("After executing OnPreRender callback, camera \'%s\' is no longer valid to use for rendering.", GetGameObject().GetName()), this);
        return;
    }

    RenderManager::UpdateAllRenderers();

    std::vector<Camera*> cameras;
    cameras.push_back(this);

    ScriptableRenderContext renderContext;
    renderContext.ExtractAndExecuteRenderPipeline(cameras, postProcessCullResults, postProcessCullResultsData);

    // Note: we setup current target texture to point to final RT, after we have done rendering. This is somewhat
    // confusing, as the variable is supposed to only mean something while we're rendering. However looks like quite
    // some code might dependent on it's value being valid even after rendering is finished (hard to say why,
    // the code has existed since forever). Whenever Camera gets serious refactoring (so that is only stores "settings",
    // and all rendering state is in some sort of "context"), would be nice to clean this stateful mess.
    // Stereo rendering overrides m_CurrentTargetTexture, so we shouldn't restore it here.
    if (!m_IsRenderingStereo)
    {
        m_CurrentTargetTexture = m_State.m_TargetTexture;
    }

    m_IsRendering = false;

//    RenderNodeQueue nodeQueue(kMemTempJobAlloc);
//    InvokeRenderEventCB(kRenderEvent_AfterEverything, passContext, nodeQueue);
}

// No need to use fixed pipeline anymore
void Camera::StandaloneRender(RenderFlag renderFlags, const std::string& replacementTag){
    if (GetDisableRendering())
    {
        ErrorString("Camera rendering during OnValidate is not allowed.");
        return;
    }

    // StandaloneCull() & CustomRenderWithPipeline() are protected from recursion (they don't allow it), which means it will cull/render nothing - skip it altogether
    if (m_IsStandaloneCustomRendering)
        return;

    m_IsStandaloneCustomRendering = true;
    renderFlags |= kRenderFlagStandalone;

    ShaderPassContext& passContext = GetDefaultPassContext();

    //    RenderManager::UpdateAllRenderers();

//    if (GetCameraType() != kCameraTypePreview)
//    {
//        // Make sure the UI canvas get updated and emitted for this render.
//        // UI uses IntermediateRenderers so they will get cleared after the render is done.
//        PlayerUpdateCanvases();
//
//        // Emit the geometry (both world canvas and screen - space camera) for this camera.
//        INVOKE_GLOBAL_CALLBACK(emitCanvasDataForCamera, this);
//    }

//    CameraRenderOldState state;
//    if (!(renderFlags & kRenderFlagDontRestoreRenderState))
//        StoreRenderState(state, passContext);

    {
        AutoScopedCameraStackRenderingState scopedStackRenderState(*this);
        if (m_State.m_ImplicitAspect)
            ResetAspect();

        // We may need BeginFrame() if we're called from script outside rendering loop (case 464376)
        AutoGfxDeviceBeginEndFrame frame;
        if (!frame.GetSuccess())
        {
            m_IsStandaloneCustomRendering = false;
            return;
        }

        // Render this camera
        UpdateVelocity();

        CustomRenderWithPipeline(passContext, renderFlags, NULL, NULL/*, requests*/);

//        if (!(renderFlags & kRenderFlagDontRestoreRenderState))
//        {
//            RestoreRenderState(state, passContext);
//        }
    }

    m_IsStandaloneCustomRendering = false;
}

bool Camera::GetStereoEnabled() const
{
    // __FAKEABLE_METHOD__(Camera, GetStereoEnabled, ());
//    bool vrSupported = GetIVRDevice() && GetIVRDevice()->GetActive() && (m_State.m_TargetEye != kTargetEyeMaskNone) && GetIVRDevice()->GetShouldRenderFrame();
//    bool editModeStereo = !::IsWorldPlayingThisFrame() && GetCameraType() == kCameraTypeVR;
//    bool playModeStereo = !editModeStereo && (GetTargetTexture() == NULL || m_IsRenderingStereo);
//    return (GetScreenManager().IsStereoscopic() || vrSupported) && (editModeStereo || playModeStereo);

    return false;
}

static inline Rectf GetCameraTargetRect(const Camera& camera, bool zeroOrigin, bool isRenderingStereo, bool adjustForDynamicScale)
{
    RenderTexture* target = camera.GetTargetTexture();
//    if (target != NULL)
//    {
//        if (adjustForDynamicScale)
//            return Rectf(0, 0, target->GetScaledWidth(), target->GetScaledHeight());
//        else
//            return Rectf(0, 0, target->GetWidth(), target->GetHeight());
//    }
//
//    RenderSurfaceHandle colorTarget = camera.GetTargetColorBuffer();
//    if (colorTarget.IsValid() && !colorTarget.object->backBuffer)
//    {
//        Rectf rect(0, 0, colorTarget.object->width, colorTarget.object->height);
//
//        if (adjustForDynamicScale && (colorTarget.object->flags & kSurfaceCreateDynamicScale))
//        {
//            rect.width = ceil(rect.width * ScalableBufferManager::GetInstance().GetWidthScaleFactor());
//            rect.height = ceil(rect.height * ScalableBufferManager::GetInstance().GetHeightScaleFactor());
//        }
//
//        return rect;
//    }
//
//    if (isRenderingStereo)
//    {
//        IVRDevice* vrDevice = GetIVRDevice();
//        if ((vrDevice != NULL) && vrDevice->GetActive())
//        {
//            int width = vrDevice->GetEyeTextureWidth();
//            int height = vrDevice->GetEyeTextureHeight();
//            return Rectf(0, 0, width, height);
//        }
//    }
//
//#if PLATFORM_IOS || PLATFORM_TVOS || PLATFORM_ANDROID
//    // on ios display RS are marked as backbuffer, but have totally valid info.
//    // And due to some multithreaded scenarios we have to check deeper, as backbuffer might have been just initialized and doesn't yet have valid data set
//    if (colorTarget.IsValid() && colorTarget.object->textureID.IsValid())
//        return Rectf(0, 0, colorTarget.object->width, colorTarget.object->height);
//#endif
//
//#if SUPPORT_MULTIPLE_DISPLAYS
//    // If targeting an External/Secondary display get target coordinates.
//    UInt32 targetDisplay = camera.GetTargetDisplay();
//    const DisplayDevice* device = UnityDisplayManager_GetDisplayDeviceAt(targetDisplay);
//
//    if ((targetDisplay != 0) && device)
//        return Rectf(0, 0, device->width, device->height);
//#endif
//
//    Rectf rect = GetScreenManager().GetRect();
//
//#if UNITY_EDITOR
//    // In the editor, if we're trying to get rect of a regular camera (visible in hierarchy etc.),
//    // use game view size instead of "whatever editor window was processed last" size.
//    // Otherwise Camera.main.aspect would return aspect of inspector when repainting it, for example.
//    //
//    // Only do this for regular cameras however; keep hidden cameras (scene view, material preview etc.)
//    // using the old behavior.
//    GameObject* go = camera.GetGameObjectPtr();
//    if (go && (go->GetHideFlags() & Object::kHideInHierarchy) != Object::kHideInHierarchy)
//    {
//        // If the current guiview is a GameView then GetScreenManager().GetRect() is already set up correctly and
//        // we do not need to find first available game view to get a valid rect. Fix for case 517158
//        bool isCurrentGUIViewAGameView = GUIView::GetCurrent() != NULL && GUIView::GetCurrent()->IsGameView();
//        if (!isCurrentGUIViewAGameView)
//        {
//            Vector2f mainGameViewSize = GetScreenManager().GetMainPlayModeViewSize();
//            rect = Rectf(0.0f, 0.0f, mainGameViewSize.x, mainGameViewSize.y);
//        }
//    }
//#endif

//    if (zeroOrigin)
//        rect.x = rect.y = 0.0f;
//    return rect;
    return Rectf();
}

Rectf Camera::GetCameraRect(bool zeroOrigin, bool adjustForDynamicScale) const
{
    // Get the screen rect from either the target texture or the viewport we're inside
    Rectf screenRect = GetCameraTargetRect(*this, zeroOrigin, GetStereoEnabled(), adjustForDynamicScale);

    // Now figure out how large this camera is depending on the normalized viewRect.
    Rectf viewRect = m_State.m_NormalizedViewPortRect;

    viewRect.Scale(screenRect.width, screenRect.height);
    viewRect.Move(screenRect.x, screenRect.y);
    viewRect.Clamp(screenRect);

    return viewRect;
}

void Camera::ResetAspect()
{
    Rectf r = GetScreenViewportRect();
    if (r.Height() != 0)
        m_State.m_Aspect = r.Width() / r.Height();
    else
        m_State.m_Aspect = 1.0f;

    m_State.m_DirtyProjectionMatrix = true;
    m_State.m_DirtySkyboxProjectionMatrix = true;
    m_State.m_ImplicitAspect = true;

    if (m_State.m_ProjectionMatrixMode == kProjectionMatrixModePhysicalPropertiesBased)
        CalculateGateFitParams();
}

void Camera::CalculateGateFitParams()
{
    m_gateFittedLensShift = m_State.m_LensShift;

    const float ratio = m_State.m_Aspect * m_State.m_SensorSize.y / m_State.m_SensorSize.x;
    const GateFitMode gateFitMode = m_State.m_GateFitMode;

    if ((gateFitMode == kGateFitFill && ratio > 1.0f) || (gateFitMode == kGateFitOverscan && ratio < 1.0f) || gateFitMode == kGateFitHorizontal)
    {
        m_gateFittedFOV = FocalLengthToFieldOfView(m_State.m_FocalLength, m_State.m_SensorSize.x / m_State.m_Aspect);
        m_gateFittedLensShift.y *= ratio;
    }
    else
    {
        m_gateFittedLensShift.x *= 1.0f / ratio;
        m_gateFittedFOV = FocalLengthToFieldOfView(m_State.m_FocalLength, m_State.m_SensorSize.y);
    }
    m_State.m_DirtySkyboxProjectionMatrix = true;
}

float Camera::FieldOfViewToFocalLength_Safe(float fov, float sensorSize)
{
    if (fov == 0)
        return 0;
    return sensorSize * .5f / tanf(kDeg2Rad * fov * .5f);
}

float Camera::FocalLengthToFieldOfView_Safe(const float focalLength, const float sensorSize)
{
    if (focalLength == 0)
        return 0;
    return kRad2Deg * 2.0f * atanf(sensorSize * .5f / focalLength);
}

float Camera::FieldOfViewToFocalLength(float fov, float sensorSize)
{
    return sensorSize * .5f / tanf(kDeg2Rad * fov * .5f);
}

float Camera::FocalLengthToFieldOfView(const float focalLength, const float sensorSize)
{
    return kRad2Deg * 2.0f * atanf(sensorSize * .5f / focalLength);
}

IMPLEMENT_REGISTER_CLASS(Camera, 8);
IMPLEMENT_OBJECT_SERIALIZE(Camera);
INSTANTIATE_TEMPLATE_TRANSFER(Camera)

template<class TransferFunction>
void Camera::Transfer(TransferFunction& transfer)
{
    Super::Transfer(transfer);

    // Note: transfer code for version 1 was just removed. It was around Unity 1.2 times,
    // and now we're fine with losing project folder compatibility with that.
    transfer.SetVersion(2);

#define TRANSFER_STATE(x) transfer.Transfer (m_State.x, #x)
#define TRANSFER_STATE_WITH_FLAGS(x, f) transfer.Transfer (m_State.x, #x, f)

    TRANSFER_STATE(m_ClearFlags);
//    TRANSFER_STATE(m_BackGroundColor);

//    TRANSFER_ENUM_WITH_NAME(m_State.m_ProjectionMatrixMode, "m_projectionMatrixMode");
//    TRANSFER_ENUM_WITH_NAME(m_State.m_GateFitMode, "m_GateFitMode");
//    TRANSFER_EDITOR_ONLY_ENUM_WITH_NAME(m_State.m_FOVAxisMode, "m_FOVAxisMode");
    transfer.Align();
    TRANSFER_STATE(m_SensorSize);
    TRANSFER_STATE(m_LensShift);
    TRANSFER_STATE_WITH_FLAGS(m_FocalLength, kDontAnimate);

    TRANSFER_STATE(m_NormalizedViewPortRect);
    transfer.Transfer(m_State.m_NearClip, "near clip plane");
    transfer.Transfer(m_State.m_FarClip, "far clip plane");
    transfer.Transfer(m_State.m_FieldOfView, "field of view", kDontAnimate);
    transfer.Transfer(m_State.m_Orthographic, "orthographic");
    transfer.Align();
    transfer.Transfer(m_State.m_OrthographicSize, "orthographic size");

    TRANSFER_STATE(m_Depth);
    TRANSFER_STATE(m_CullingMask);
    TRANSFER_STATE_WITH_FLAGS(m_RenderingPath, kDontAnimate);

    TRANSFER_STATE(m_TargetTexture);
    TRANSFER_STATE_WITH_FLAGS(m_TargetDisplay, kDontAnimate);   // Aligned
//    TRANSFER_ENUM_WITH_NAME(m_State.m_TargetEye, "m_TargetEye");
    transfer.Transfer(m_State.m_AllowHDR, "m_HDR");
    TRANSFER_STATE(m_AllowMSAA);
    TRANSFER_STATE_WITH_FLAGS(m_AllowDynamicResolution, kDontAnimate);
    TRANSFER_STATE(m_ForceIntoRT);
    TRANSFER_STATE(m_OcclusionCulling);
    transfer.Align();
    TRANSFER_STATE(m_StereoConvergence);
    TRANSFER_STATE(m_StereoSeparation);

#undef TRANSFER_STATE
}


CameraStackRenderingState::CameraStackRenderingState()
        : /*m_TempInitialEyePair(),*/
         m_TargetType(kGenerated)
        , m_CurrentCamera(NULL)
        , m_FirstCamera(NULL)
        , m_LastCamera(NULL)
        , m_LastLeftEyeCamera(NULL)
        , m_LastRightEyeCamera(NULL)
        , m_Eye(kStereoscopicEyeDefault)
        , m_ForceUseRT(false)
        , m_ForceUseTempRT(false)
        , m_HDR(false)
        , m_HasDeferred(false)
        , m_MSAA(false)
        , m_DynamicResolution(false)
        , m_FirstStack(false)
        , m_HasCommandBuffers(false)
{
    // memset(m_TempBuffers, 0, sizeof(m_TempBuffers));
}

void CameraStackRenderingState::ReleaseResources()
{
//    RenderBufferManager& rbm = GetRenderBufferManager();
//    for (int i = 0; i < kBuiltinRTCount; ++i)
//    {
//        rbm.GetTextures().ReleaseTempBuffer(m_TempBuffers[i]);
//        m_TempBuffers[i] = NULL;
//    }
//
//    if (!m_TempInitialEyePair.IsEmpty())
//    {
//        m_TempInitialEyePair.ReleaseTemp();
//    }
//
//    //@TODO: track grab passes per camera stack too?
//    //ShaderLab::ClearGrabPassFrameState();
}

void CameraStackRenderingState::BeginRenderingOneCamera(Camera& cam)
{
//    AssertMsg(m_TempInitialEyePair.IsEmpty(), "Render Target RT is already created!");
//    m_FirstStack = true;
//    m_ForceUseRT = cam.HasAnyImageFilters() || cam.GetForceIntoRT();
//    m_ForceUseTempRT = cam.HasCommandBufferBasedImageFilters();
//
//    const TierGraphicsSettings& settings = GetGraphicsSettings().GetTierSettings();
//    m_HDR = cam.GetAllowHDR() && settings.useHDR;
//
//    RenderingPath path = cam.CalculateRenderingPath();
//    m_HasDeferred = path == kRenderPathPrePass || path == kRenderPathDeferred;
//    m_MSAA = m_HasDeferred ? false : (cam.GetAllowMSAA() && CalculateDesiredAntiAliasingForStack() > 1);
//    m_DynamicResolution = cam.GetAllowDynamicResolution();
//    m_HasCommandBuffers = (cam.GetCommandBufferCount() > 0);
//
//    GetTargetsFromCamera(cam, m_CameraTarget);
//
//    m_FirstCamera = &cam;
//    m_CurrentCamera = &cam;
//    m_LastCamera = &cam;
//
//    dynamic_array<PPtr<Camera> > cameras(kMemCamera);
//    cameras.push_back(&cam);
//    m_TargetType = CalculateCameraTargetType(cameras);
//
//    cam.SetCurrentTargetTexture(GetTargetTexture());
}

const Matrix4x4f& Camera::GetCullingMatrix() const
{
    if (m_State.m_ImplicitCullingMatrix)
    {
        m_State.m_CullingMatrix = GetWorldToClipMatrix();
    }

    return m_State.m_CullingMatrix;
}

void Camera::GetImplicitWorldToCameraMatrix(Matrix4x4f& outMatrix) const
{
    outMatrix.SetScale(Vector3f(1.0F, 1.0F, -1.0F));
    outMatrix *= GetComponent<Transform>().GetWorldToLocalMatrixNoScale();
}

const Matrix4x4f& Camera::GetWorldToCameraMatrix() const
{
    if (m_State.m_ImplicitWorldToCameraMatrix)
        GetImplicitWorldToCameraMatrix(m_State.m_WorldToCameraMatrix);
    return m_State.m_WorldToCameraMatrix;
}

const Matrix4x4f& Camera::GetWorldToClipMatrix() const
{
    MultiplyMatrices4x4(&GetProjectionMatrix(), &GetWorldToCameraMatrix(), &m_State.m_WorldToClipMatrix);
    return m_State.m_WorldToClipMatrix;
}


void Camera::SetNear(float n)
{
    if (m_State.m_NearClip != n)
    {
        SetDirty();
        m_State.m_NearClip = n;
    }
    m_State.m_DirtyProjectionMatrix = true;
    m_State.m_DirtySkyboxProjectionMatrix = true;
}

float Camera::GetAspect() const
{
    // __FAKEABLE_METHOD__(Camera, GetAspect, ());
    return m_State.m_Aspect;
}

RenderTexture *Camera::GetTargetTexture() const { return m_State.m_TargetTexture; }

Vector3f Camera::ScreenToWorldPoint(const Vector3f& v, MonoOrStereoscopicEye eye) const
{
    RectInt viewport = GetScreenViewportRectInt();

    Matrix4x4f camToWorld;
    Matrix4x4f clipToWorld;
//    if (eye < kMonoOrStereoscopicEyeMono)  // VZ: Won't care about VR devices for now.
//    {
//        Matrix4x4f::Invert_General3D(GetStereoViewMatrix(StereoscopicEye(eye)), camToWorld);
//        Matrix4x4f::Invert_Full(GetStereoWorldToClipMatrix(StereoscopicEye(eye)), clipToWorld);
//    }
//    else
    {
        GetClipToWorldMatrix(clipToWorld);
        camToWorld = GetCameraToWorldMatrix();
    }

    Vector3f out;
    if (!CameraUnProject(v, camToWorld, clipToWorld, viewport, out, GetTargetTexture() != NULL))
    {
        ErrorString(Format("Screen position out of view frustum (screen pos %f, %f, %f) (Camera rect %d %d %d %d)", v.x, v.y, v.z, viewport.x, viewport.y, viewport.width, viewport.height));
    }
    return out;
}

float Camera::GetNear() const
{
    // __FAKEABLE_METHOD__(Camera, GetNear, ());
    return m_State.m_NearClip;
}

void Camera::SetFar(float f)
{
    if (m_State.m_FarClip != f)
    {
        SetDirty();
        m_State.m_FarClip = f;
    }
    m_State.m_DirtyProjectionMatrix = true;
    m_State.m_DirtySkyboxProjectionMatrix = true;
}

float Camera::GetFar() const
{
    // __FAKEABLE_METHOD__(Camera, GetFar, ());
    return m_State.m_FarClip;
}

void Camera::SetCullingMask(UInt32 cullingMask)
{
    if (m_State.m_CullingMask.m_Bits != cullingMask)
    {
        m_State.m_CullingMask.m_Bits = cullingMask;
        SetDirty();
    }
}

float Camera::GetVerticalFieldOfView() const
{
    // __FAKEABLE_METHOD__(Camera, GetVerticalFieldOfView, ());
//    if (ShouldUseVRFieldOfView())
//    {
//        float vrDeviceFieldOfView = GetIVRDevice()->GetFieldOfView();
//        if (m_State.m_FieldOfView != vrDeviceFieldOfView)
//        {
//            m_State.m_FieldOfViewBeforeEnablingVRMode = m_State.m_FieldOfView;
//        }
//        m_State.m_FieldOfView = vrDeviceFieldOfView;
//    }
    return m_State.m_FieldOfView;
}

void Camera::CalculateProjectionMatrixFromPhysicalProperties(Matrix4x4f& out, float focalLength, const Vector2f& sensorSize, Vector2f lensShift, float nearClip, float farClip, float gateAspect, GateFitMode gateFitMode /*= kGateFitNone*/)
{
    float fov;
    const float ratio = gateAspect * sensorSize.y / sensorSize.x;

    if (ratio == 1.0f)
    {
        gateFitMode = kGateFitNone;
    }
    else if ((gateFitMode == kGateFitFill && ratio > 1.0f) || (gateFitMode == kGateFitOverscan && ratio < 1.0f))
        gateFitMode = kGateFitHorizontal;
    else if ((gateFitMode == kGateFitFill && ratio < 1.0f) || (gateFitMode == kGateFitOverscan && ratio > 1.0f))
        gateFitMode = kGateFitVertical;

    if (gateFitMode == kGateFitHorizontal)
    {
        fov = FocalLengthToFieldOfView(focalLength, sensorSize.x / gateAspect);
        lensShift.y *= ratio;
    }
    else if (gateFitMode == kGateFitVertical)
    {
        fov = FocalLengthToFieldOfView(focalLength, sensorSize.y);
        lensShift.x *= 1.0f / ratio;
    }
    else // gateFitMode == kGateFitNone
    {
        fov = FocalLengthToFieldOfView(focalLength, sensorSize.y);
        gateAspect = sensorSize.x / sensorSize.y;
    }

    out.SetPerspective(fov, gateAspect, nearClip, farClip);
    out.m_Data[8] = lensShift.x * 2;
    out.m_Data[9] = lensShift.y * 2;
}

// Returns true if this an "oblique clipping" projection, e.g. as used
// for water reflections to clip along the water surface.
bool CameraScripting::IsObliqueProjection(const Matrix4x4f& mat)
{
    // If third row's x or y aren't zero this is an oblique clipping
    // matrix. See "Oblique Near-Plane Clipping With Orthographic Camera"
    // for example http://aras-p.info/texts/obliqueortho.html
    if (mat.Get(2, 0) != 0)
        return true;
    if (mat.Get(2, 1) != 0)
        return true;
    return false;
}


void Camera::GetImplicitProjectionMatrix(float overrideNearPlane, Matrix4x4f& outMatrix) const
{
    if (!m_State.m_Orthographic)
        outMatrix.SetPerspective(GetGateFittedFieldOfView(), GetAspect(), overrideNearPlane, m_State.m_FarClip);
    else
        outMatrix.SetOrtho(-m_State.m_OrthographicSize * m_State.m_Aspect, m_State.m_OrthographicSize * m_State.m_Aspect, -m_State.m_OrthographicSize, m_State.m_OrthographicSize, overrideNearPlane, m_State.m_FarClip);
}

void Camera::GetImplicitProjectionMatrix(float overrideNearPlane, float overrideFarPlane, float fov, float aspect, Matrix4x4f& outMatrix) const
{
    if (!m_State.m_Orthographic)
        outMatrix.SetPerspective(fov, aspect, overrideNearPlane, overrideFarPlane);
    else
        outMatrix.SetOrtho(-m_State.m_OrthographicSize * m_State.m_Aspect, m_State.m_OrthographicSize * m_State.m_Aspect, -m_State.m_OrthographicSize, m_State.m_OrthographicSize, overrideNearPlane, overrideFarPlane);
}

const Matrix4x4f& Camera::GetProjectionMatrix() const
{
    if (m_State.m_DirtyProjectionMatrix)
    {
        if (m_State.m_ProjectionMatrixMode == kProjectionMatrixModeImplicit)
        {
            if (!m_State.m_Orthographic)
                m_State.m_ProjectionMatrix.SetPerspective(GetVerticalFieldOfView(), GetAspect(), m_State.m_NearClip, m_State.m_FarClip);
            else
                m_State.m_ProjectionMatrix.SetOrtho(-m_State.m_OrthographicSize * m_State.m_Aspect, m_State.m_OrthographicSize * m_State.m_Aspect, -m_State.m_OrthographicSize, m_State.m_OrthographicSize, m_State.m_NearClip, m_State.m_FarClip);
        }
        else if (m_State.m_ProjectionMatrixMode == kProjectionMatrixModePhysicalPropertiesBased)
        {
            CalculateProjectionMatrixFromPhysicalProperties(m_State.m_ProjectionMatrix, m_State.m_FocalLength, m_State.m_SensorSize, m_State.m_LensShift, m_State.m_NearClip, m_State.m_FarClip, m_State.m_Aspect, m_State.m_GateFitMode);
        }
        m_State.m_DirtyProjectionMatrix = false;
    }

    return m_State.m_ProjectionMatrix;
}

Matrix4x4f Camera::GetCameraToWorldMatrix() const
{
    Matrix4x4f m;
    Matrix4x4f::Invert_Full(GetWorldToCameraMatrix(), m);
    return m;
}

float Camera::GetProjectionNear() const
{
    if (m_State.m_ProjectionMatrixMode == kProjectionMatrixModeImplicit)
        return m_State.m_NearClip;

    const Matrix4x4f& proj = GetProjectionMatrix();
    if (IsNonStandardProjection(proj))
        return m_State.m_NearClip;

    Vector4f nearPlane = proj.GetRow(3) + proj.GetRow(2);
    Vector3f nearNormal(nearPlane.x, nearPlane.y, nearPlane.z);
    return -nearPlane.w / Magnitude(nearNormal);
}

float Camera::GetProjectionFar() const
{
    if (m_State.m_ProjectionMatrixMode == kProjectionMatrixModeImplicit)
        return m_State.m_FarClip;

    const Matrix4x4f& proj = GetProjectionMatrix();
    if (IsNonStandardProjection(proj))
        return m_State.m_FarClip;

    Vector4f farPlane = proj.GetRow(3) - proj.GetRow(2);
    Vector3f farNormal(farPlane.x, farPlane.y, farPlane.z);
    return farPlane.w / Magnitude(farNormal);
}

CameraProjectionCache::CameraProjectionCache(const Camera& cam, MonoOrStereoscopicEye eye)
{
    m_ViewPort = cam.GetScreenViewportRect();
    m_ViewPortInt = RectfToRectInt(m_ViewPort);

//    if (eye < kMonoOrStereoscopicEyeMono)
//    {
//        Matrix4x4f::Invert_General3D(cam.GetStereoViewMatrix(StereoscopicEye(eye)), m_CameraToWorldMatrix);
//        m_WorldToClipMatrix = cam.GetStereoWorldToClipMatrix(StereoscopicEye(eye));
//    }
//    else
    {
        m_CameraToWorldMatrix = cam.GetCameraToWorldMatrix();
        m_WorldToClipMatrix = cam.GetWorldToClipMatrix();
    }
    m_IsTargetTextureNull = cam.IsTargetTextureNull();
}

Vector3f CameraProjectionCache::WorldToScreenPoint(const Vector3f& v, bool* canProject) const
{
    RectInt viewport = GetScreenViewportRectInt();

    Vector3f out;
    bool ok = CameraProject(v, m_CameraToWorldMatrix, m_WorldToClipMatrix, viewport, out, !m_IsTargetTextureNull);
    if (canProject != NULL)
        *canProject = ok;
    return out;
}

Vector3f Camera::WorldToScreenPoint(const Vector3f& v, MonoOrStereoscopicEye eye, bool* canProject) const
{
    // CameraProjectionCache does not "cache" any data for reuse in this function, we're using it to not duplicate code
    CameraProjectionCache cache(*this, eye);
    return cache.WorldToScreenPoint(v, canProject);
}


Vector2f Camera::GetFrustumPlaneSizeAt(const float distance) const
{
    Vector2f planeSize;

    if (m_State.m_ProjectionMatrixMode == kProjectionMatrixModeExplicit)
    {
        if (!IsNonStandardProjection(m_State.m_ProjectionMatrix))
        {
            float cotangent = m_State.m_ProjectionMatrix.m_Data[5];
            float aspect = cotangent / m_State.m_ProjectionMatrix.m_Data[0];

            float fov = atan(1.0f / cotangent) * 2.0 * kRad2Deg;

            planeSize.y = 2.0f * distance * tan(Deg2Rad(fov * 0.5f));
            planeSize.x = planeSize.y * aspect;
        }
        else
        {
            Rectf screenRect = GetScreenViewportRect();
            Vector3f p0 = ScreenToWorldPoint(Vector3f(screenRect.x, screenRect.y, distance));
            Vector3f p1 = ScreenToWorldPoint(Vector3f(screenRect.x + screenRect.width, screenRect.y, distance));
            Vector3f p2 = ScreenToWorldPoint(Vector3f(screenRect.x, screenRect.y + screenRect.height, distance));
            planeSize.x = Magnitude(p0 - p1);
            planeSize.y = Magnitude(p0 - p2);
        }
        return planeSize;
    }

    if (GetOrthographic())
    {
        planeSize.y = GetOrthographicSize() * 2.0f;
        planeSize.x = planeSize.y * m_State.m_Aspect;
    }
    else
    {
        if (m_State.m_ProjectionMatrixMode == kProjectionMatrixModePhysicalPropertiesBased)
        {
            planeSize.y = 2.0f * distance * tan(Deg2Rad(m_gateFittedFOV * 0.5f));

            if (m_State.m_GateFitMode == kGateFitNone)
                planeSize.x = planeSize.y * (m_State.m_SensorSize.x / m_State.m_SensorSize.y);
            else
                planeSize.x = planeSize.y * m_State.m_Aspect;
        }
        else
        {
            planeSize.y = 2.0f * distance * tan(Deg2Rad(m_State.m_FieldOfView * 0.5f));
            planeSize.x = planeSize.y * m_State.m_Aspect;
        }
    }
    return planeSize;
}

float Camera::CalculateFarPlaneWorldSpaceLength() const
{
    if (m_State.m_ProjectionMatrixMode == kProjectionMatrixModeExplicit)
    {
        Rectf screenRect = GetScreenViewportRect();
        Vector3f p0 = ScreenToWorldPoint(Vector3f(screenRect.x, screenRect.y, m_State.m_FarClip));
        Vector3f p1 = ScreenToWorldPoint(Vector3f(screenRect.x + screenRect.width, screenRect.y, m_State.m_FarClip));
        return Magnitude(p0 - p1);
    }
    Vector2f camSize = GetFrustumPlaneSizeAt(m_State.m_FarClip);
    return camSize.x;
}


void Camera::CalculateFrustumPlanes(Plane frustum[kPlaneFrustumNum], const Matrix4x4f& overrideWorldToClip, float overrideFarPlane, float& outBaseFarDistance, bool implicitNearFar) const
{
    ExtractProjectionPlanes(overrideWorldToClip, frustum);

    Plane& nearPlane = frustum[kPlaneFrustumNear];
    Plane& farPlane = frustum[kPlaneFrustumFar];

    bool hasCustomCullingMatrix = !m_State.m_ImplicitCullingMatrix;
    bool isImplicit = IsImplicitWorldToCameraMatrix() || implicitNearFar;

    // If the user has specified a custom culling matrix, all culling
    // decisions must be based on that, and we cannot use
    // GetCameraToWorldMatrix(). Precision problems with the custom matrix
    // can still be problematic.

    if (!hasCustomCullingMatrix && isImplicit)
    {
        // Extracted near and far planes may be unsuitable for culling.
        // E.g. oblique near plane for water refraction busts both planes.
        // Also very large far/near ratio causes precision problems.
        // Instead we calculate the planes from our position/direction.

        Matrix4x4f cam2world = GetCameraToWorldMatrix();
        Vector3f eyePos = cam2world.GetPosition();
        Vector3f viewDir = -NormalizeSafe(cam2world.GetAxisZ());

        nearPlane.SetNormalAndPosition(viewDir, eyePos);
        nearPlane.distance -= m_State.m_NearClip;

        farPlane.SetNormalAndPosition(-viewDir, eyePos);
        outBaseFarDistance = farPlane.distance;
        farPlane.distance += overrideFarPlane;
    }
    else
        outBaseFarDistance = farPlane.distance - overrideFarPlane;
}

float Camera::GetGateFittedFieldOfView() const
{
    if (m_State.m_ProjectionMatrixMode == kProjectionMatrixModePhysicalPropertiesBased)
        return m_gateFittedFOV;
    else
        return m_State.m_FieldOfView;
}

void Camera::CalculateFarCullDistances(float* farCullDistances, float baseFarDistance) const
{
    // baseFarDistance is the distance of the far plane shifted to the camera position
    // This is so layer distances work properly even if the far distance is very large
    for (int i = 0; i < kNumLayers; i++)
    {
        if (m_State.m_LayerCullDistances[i])
            farCullDistances[i] = baseFarDistance + m_State.m_LayerCullDistances[i];
        else
            farCullDistances[i] = baseFarDistance + m_State.m_FarClip;
    }
}

HuaHuoScene *Camera::GetScene() const
{
    return m_State.m_Scene;
}

UInt64 Camera::GetSceneCullingMask() const
{
    if (m_State.m_SceneCullingMaskOverride != 0UL)
        return m_State.m_SceneCullingMaskOverride;

    if (GetScene())
        return GetScene()->GetSceneCullingMask();

    switch (GetCameraType())
    {
        case kCameraTypeSceneView:
            return kMainStageSceneViewObjects_SceneCullingMask;
        default:
            return kGameViewObjects_SceneCullingMask;
    }
}

void Camera::CalculateCullingParameters(CullingParameters& cullingParameters) const
{
    Plane frustum[kPlaneFrustumNum];
    float baseFarDistance;

    Matrix4x4f cullingMatrix = GetCullingMatrix();
    cullingParameters.cullingMatrix = cullingMatrix;
    //Check if standard render pipeline is used:
//    bool usingSRP = !GetGraphicsSettings().GetCurrentRenderPipeline().IsNull();
//#if UNITY_EDITOR
//    usingSRP &= GetRenderManager().GetUseScriptableRenderPipeline();
//#endif
//    Vector3f cullingPos = (GetStereoSingleCullEnabled() || usingSRP) ? GetCameraToWorldMatrix().GetPosition() : GetPosition();
    Vector3f cullingPos = GetCameraToWorldMatrix().GetPosition();
    cullingParameters.position = cullingPos;

    CalculateFrustumPlanes(frustum, cullingMatrix, m_State.m_FarClip, baseFarDistance, false);

    LODParameters lodParams;
    lodParams.cameraPosition = cullingPos;
    lodParams.fieldOfView = GetGateFittedFieldOfView();
    lodParams.isOrthographic = GetOrthographic();
    lodParams.orthoSize = m_State.m_OrthographicSize;
    lodParams.cameraPixelHeight = int(GetPhysicalViewportRect().height);
    CalculateCustomCullingParameters(cullingParameters, lodParams, GetCullingMask(), GetSceneCullingMask(), frustum, kPlaneFrustumNum);

    if (m_State.m_LayerCullSpherical)
    {
        std::copy(m_State.m_LayerCullDistances, m_State.m_LayerCullDistances + kNumLayers, cullingParameters.layerFarCullDistances);
        cullingParameters.layerCull = CullingParameters::kLayerCullSpherical;
    }
    else
    {
        CalculateFarCullDistances(cullingParameters.layerFarCullDistances, baseFarDistance);
        cullingParameters.layerCull = CullingParameters::kLayerCullPlanar;
    }
}