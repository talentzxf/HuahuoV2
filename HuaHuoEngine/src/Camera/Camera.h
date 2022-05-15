//
// Created by VincentZhang on 5/13/2022.
//

#ifndef HUAHUOENGINE_CAMERA_H
#define HUAHUOENGINE_CAMERA_H
#include "GameCode/Behaviour.h"
#include "Math/Matrix4x4.h"
#include "Math/Vector2f.h"
#include "GfxDevice/GfxDeviceTypes.h"
#include "Math/Rect.h"
#include "Shaders/ShaderPassContext.h"
#include "CullingParameters.h"
#include "BaseClasses/BitField.h"

class HuaHuoScene;

class Camera : public Behaviour {
//    REGISTER_CLASS_TRAITS(kTypeNoFlags);
    REGISTER_CLASS(Camera);
    DECLARE_OBJECT_SERIALIZE();
public:
    Camera(/*MemLabelId label,*/ ObjectCreationMode mode);
    // ~Camera (); declared-by-macro

    // Match OpaqueSortMode on C# side
    enum OpaqueSortMode
    {
        kOpaqueSortDefault = 0, // bucketed front-to-back or no depth sorting, depending on the GPU
        kOpaqueSortFrontToBack = 1,
        kOpaqueSortNoDepthSort = 2,
    };

    // Match TransparencySortMode on C# side
    enum TransparencySortMode
    {
        kTransparencySortDefault = 0,
        kTransparencySortPerspective = 1,
        kTransparencySortOrthographic = 2,
        kTransparencySortCustomAxis = 3,
    };

    enum
    {
        kPreviewLayer = 31,
        kNumLayers = 32
    };

    enum ProjectionMatrixMode
    {
        kProjectionMatrixModeExplicit,
        kProjectionMatrixModeImplicit,
        kProjectionMatrixModePhysicalPropertiesBased,
    };

    enum GateFitMode
    {
        kGateFitNone,      // Fit the resolution gate to the film gate.
        kGateFitVertical,   // Fit the resolution gate vertically within the film gate.
        kGateFitHorizontal, // Fit the resolution gate horizontally within the film gate.
        kGateFitFill,       // Fit the resolution gate within the film gate.
        kGateFitOverscan   // Fit the film gate within the resolution gate.
    };

    enum FieldOfViewAxisMode
    {
        kVertical,
        kHorizontal
    };

    // used to save/restore the internal matrix state
    struct MatrixState
    {
        Matrix4x4f viewMatrix;
        Matrix4x4f projMatrix;
        Matrix4x4f skyboxProjMatrix;

        bool implicitViewMatrix;
        ProjectionMatrixMode projectionMatrixMode;
        bool implicitSkyboxProjMatrix;
    };

    enum RenderFlag
    {
        kRenderFlagNone,
        kRenderFlagStandalone = 1 << 0,
        kRenderFlagSinglePassStereo = 1 << 1,
        kRenderFlagSetRenderTarget = 1 << 2,
        kRenderFlagDontRestoreRenderState = 1 << 4,
        kRenderFlagSetRenderTargetFinal = 1 << 5,
        kRenderFlagExplicitShaderReplace = 1 << 6,
        kRenderFlagDontBlitTargetTexture = 1 << 7,
        kRenderFlagInstancingStereo = 1 << 8,
        kRenderFlagMultiviewStereo = 1 << 9,
        kRenderFlagUseExistingCameraStackRenderingState = 1 << 10,
    };

    enum StereoViewMatrixMode
    {
        // The stereo view matrices are computed by the camera
        kStereoViewMatrixModeImplicit,

        // The stereo view matrices have been set to arbitrary
        // values which requires a double cull
        kStereoViewMatrixModeExplicitUnsafeForSingleCull,

        // The stereo view matrices have been set outside
        // the camera, but close enough to the mono camera
        // that we can still assume a single cull is enough
        kStereoViewMatrixModeExplicitSafeForSingleCull,
    };
public:
    virtual void Reset() override;
    virtual void SmartReset() override;
    virtual void AwakeFromLoad(AwakeFromLoadMode awakeMode) override;

    Vector3f GetPosition() const;
    void UpdateVelocity();

    void Render();
    void StandaloneRender(RenderFlag renderFlags, const std::string& replacementTag);
    void CustomRenderWithPipeline(ShaderPassContext& passContext, RenderFlag renderFlags, PostProcessCullResults* postProcessCullResults = NULL, void* postProcessCullResultsData = NULL/*, ScriptingObjectPtr requests = NULL*/);

    float GetAspect() const;
    void SetAspect(float aspect);
    void ResetAspect();

    bool IsValidToRender() const;

    // The screen view port rect of the camera.
    // If the cameras normalized viewport rect is set to be the fullscreen, then this will always go from
    // 0, 0 to width, height.
    Rectf GetScreenViewportRect(bool adjustForDynamicScale = true) const { return GetCameraRect(true, adjustForDynamicScale); }
    RectInt GetScreenViewportRectInt(bool adjustForDynamicScale = true) const;
    // Similar to GetScreenViewportRect, except this can have non-zero origin even for fullscreen cameras.
    // This only ever happens in editor's game view when using forced aspect ratio or size.
    Rectf GetPhysicalViewportRect(bool adjustForDynamicScale = true) const { return GetCameraRect(false, adjustForDynamicScale); }

    void SetScreenViewportRect(const Rectf& pixelRect);

    bool GetStereoEnabled() const;

    static float FocalLengthToFieldOfView_Safe(float focalLength, const float sensorSize);
    static float FieldOfViewToFocalLength_Safe(float fov, float sensorSize);

    static float FocalLengthToFieldOfView(float focalLength, const float sensorSize);
    static float FieldOfViewToFocalLength(float fov, float sensorSize);

//protected:
//    // Behaviour stuff
//    virtual void AddToManager() override;
//    virtual void RemoveFromManager() override;
private:
    Rectf GetCameraRect(bool zeroOrigin, bool adjustForDynamicScale = true) const;
    void CalculateGateFitParams();
private:
    // When generating tooltips/ranges/enums for inspector with Doxygen, we want to attribute all these to Camera class and not to CopiableState.
    // For this the struct declaration is excluded with the cond command (unless HIDDEN_SYMBOLS is defined in ENABLED_SECTIONS).
    /// @cond HIDDEN_SYMBOLS
    //
    // Camera state that is copied in Camera::CopyFrom
    //
    // Rule of thumb: only values that can be set from script (either directly, or as a side-effect) should be part of camera state copy.
    //
    struct CopiableState
    {
        CopiableState();
        void Reset();
        /// @endcond

        Vector2f m_SensorSize;
        Vector2f m_LensShift;
        float m_FocalLength;
        GateFitMode m_GateFitMode;
        FieldOfViewAxisMode m_FOVAxisMode;

        mutable Matrix4x4f  m_WorldToCameraMatrix;
        mutable Matrix4x4f  m_ProjectionMatrix;
        mutable Matrix4x4f  m_WorldToClipMatrix;
        mutable Matrix4x4f  m_SkyboxProjectionMatrix;
        mutable float       m_FieldOfView;///< Vertical Field of view of the camera range { 0.00001, 179 }
        mutable float       m_FieldOfViewBeforeEnablingVRMode;

        mutable Matrix4x4f  m_StereoViewMatrices[kStereoscopicEyeCount];
        mutable Matrix4x4f  m_StereoProjectionMatrices[kStereoscopicEyeCount];
        mutable Matrix4x4f  m_StereoWorldToClipMatrices[kStereoscopicEyeCount];
        mutable Matrix4x4f  m_CullingMatrix;

//        PPtr<RenderTexture> m_TargetTexture; ///< The texture to render this camera into
//
//        RenderSurfaceHandle m_TargetColorBuffer[kMaxSupportedRenderTargets];
        int                 m_TargetColorBufferCount;
//        RenderSurfaceHandle m_TargetDepthBuffer;
        // we need to set active RenderTexture's for GfxDevice. While this is very unfortunate
        // some code paths use "active RT" to determine what to do
//        RenderTexture*      m_TargetBuffersOriginatedFrom[kMaxSupportedRenderTargets];

        int                 m_TargetDisplay;        ///< Target Display Index. Can be between 1 to 8. 0 is default display {0, 8}
        TargetEyeMask       m_TargetEye;
//        PPtr<Shader>        m_ReplacementShader;
        std::string            m_ReplacementTag;

        unsigned int        m_ClearFlags;///< enum { Skybox = 1, Solid Color = 2, Depth only = 3, Don't Clear = 4 }
//        ColorRGBAf          m_BackGroundColor;///< The color to which camera clears the screen
        Rectf               m_NormalizedViewPortRect;

        BitField            m_CullingMask;///< Which layers the camera renders
        BitField            m_EventMask;///< Which layers receive events

        float               m_Depth;///< A camera with a larger depth is drawn on top of a camera with a smaller depth range {-100, 100}
        Vector3f            m_Velocity;
        Vector3f            m_LastPosition;

        float               m_OrthographicSize;
        float               m_NearClip; ///< Near clipping plane
        float               m_FarClip;  ///< Far clipping plane
        int                 m_RenderingPath;///< enum { Use Graphics Settings = -1, Legacy Vertex Lit=0, Forward=1, Legacy Deferred (light prepass)=2, Deferred=3 } Rendering path to use.

        float               m_LayerCullDistances[kNumLayers];
        float               m_Aspect;
        OpaqueSortMode      m_OpaqueSortMode;
        TransparencySortMode m_TransparencySortMode;
        Vector3f            m_TransparencySortAxis;
        bool                m_ImplicitTransparencySortSettings;

        UInt32              m_DepthTextureMode;

        mutable bool        m_DirtyProjectionMatrix;
        mutable bool        m_DirtySkyboxProjectionMatrix;
        bool                m_ImplicitWorldToCameraMatrix;
        ProjectionMatrixMode m_ProjectionMatrixMode;
        mutable bool        m_ImplicitSkyboxProjectionMatrix;
        StereoViewMatrixMode m_StereoViewMatrixMode;
        bool                m_ImplicitStereoProjectionMatrices;
        bool                m_ImplicitCullingMatrix;
        bool                m_ImplicitAspect;
        bool                m_Orthographic;///< Is camera orthographic?
        bool                m_OcclusionCulling;
        bool                m_LayerCullSpherical;
        bool                m_AllowHDR;
        bool                m_UsingHDR;
        bool                m_AllowMSAA;
        bool                m_AllowDynamicResolution;
        bool                m_ForceIntoRT;
        bool                m_ClearStencilAfterLightingPass;
        float               m_StereoSeparation;
        float               m_StereoConvergence;
        int                 m_StereoFrameCounter;
        CameraType          m_CameraType;
#if UNITY_EDITOR
        bool                m_AnimateMaterials;
        float               m_AnimateMaterialsTime;
#endif

        UInt64              m_SceneCullingMaskOverride;

        HuaHuoScene          *m_Scene;

        // See DOXYGEN comment above
        /// @cond HIDDEN_SYMBOLS
    };
    /// @endcond

    CopiableState       m_State;

    // Transient state: not copied by CopyFrom!
    bool                    m_IsRendering;
    bool                    m_IsRenderingStereo;
    bool                    m_IsStandaloneCustomRendering;
    bool                    m_IsCulling;
    bool                    m_IsNonJitteredProjMatrixSet;
    // bool                    m_IsStereoNonJitteredProjMatrixCopied[kStereoscopicEyeCount];
    bool                    m_UseJitteredProjMatrixForTransparent;
    bool                    m_BuffersSetFromScripts;    // when true we've set the cameras m_TargetColorBuffer and m_TargetDepthBuffer from scripts so favour those values over the m_CurrentTargetTexture
    float                   m_gateFittedFOV;
    Vector2f                m_gateFittedLensShift;
};

ENUM_FLAGS(Camera::RenderFlag);

#endif //HUAHUOENGINE_CAMERA_H
