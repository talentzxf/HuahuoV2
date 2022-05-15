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
//    ::memset(m_TargetColorBuffer, 0x00, sizeof(m_TargetColorBuffer));
//    ::memset(m_TargetBuffersOriginatedFrom, 0x00, sizeof(m_TargetBuffersOriginatedFrom));
//
//    m_TargetColorBuffer[0] = RenderSurfaceHandle();
//    m_TargetDepthBuffer = RenderSurfaceHandle();

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
//    m_TargetTexture = nullptr;
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

Camera::Camera(/*MemLabelId label,*/ ObjectCreationMode mode)
        :   Super(/*label,*/ mode)
//        ,   m_RenderEvents(label, kRenderEventCount)
//#if UNITY_EDITOR
//        ,   m_EditorCullResults(NULL)
//    ,   m_FilterMode(0)
//    ,   m_IsSceneCamera(false)
//    ,   m_StackState(NULL)
//    ,   m_LastDrawingMode(kEditorDrawModeCount)
//#endif
        ,   m_IsRendering(false)
        ,   m_IsRenderingStereo(false)
        ,   m_IsStandaloneCustomRendering(false)
        ,   m_IsCulling(false)
        ,   m_IsNonJitteredProjMatrixSet(false)
        ,   m_UseJitteredProjMatrixForTransparent(true)
        ,   m_BuffersSetFromScripts(false)
//        ,   m_CurrentTargetTexture(NULL)
//        ,   m_DepthTexture(NULL)
//        ,   m_DepthNormalsTexture(NULL)
//        ,   m_ODSWorldTexture(NULL)
//        ,   m_ODSWorldShader(NULL)
//        ,   m_VRIgnoreImplicitCameraUpdate(false)
//        ,   m_gateFittedFOV(60.0f)
//        ,   m_gateFittedLensShift(Vector2f::zero)
{
//    m_RenderLoop = CreateRenderLoop(*this);
//    m_ShadowCache = CreateShadowMapCache();
//
//    for (int eye = 0; eye < kStereoscopicEyeCount; ++eye)
//        m_IsStereoNonJitteredProjMatrixCopied[eye] = false;
//
//    {
//        ReadWriteSpinLock::AutoWriteLock autoLock(s_AllCameraLock);
//        s_AllCamera->push_back(this);
//    }
}

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

//    // Note: we setup current target texture to point to final RT, after we have done rendering. This is somewhat
//    // confusing, as the variable is supposed to only mean something while we're rendering. However looks like quite
//    // some code might dependent on it's value being valid even after rendering is finished (hard to say why,
//    // the code has existed since forever). Whenever Camera gets serious refactoring (so that is only stores "settings",
//    // and all rendering state is in some sort of "context"), would be nice to clean this stateful mess.
//    // Stereo rendering overrides m_CurrentTargetTexture, so we shouldn't restore it here.
//    if (!m_IsRenderingStereo)
//    {
//        m_CurrentTargetTexture = m_State.m_TargetTexture;
//    }

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
//    RenderTexture* target = camera.GetTargetTexture();
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