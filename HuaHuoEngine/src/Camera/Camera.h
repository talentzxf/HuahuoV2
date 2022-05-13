//
// Created by VincentZhang on 5/13/2022.
//

#ifndef HUAHUOENGINE_CAMERA_H
#define HUAHUOENGINE_CAMERA_H
#include "GameCode/Behaviour.h"
#include "Math/Matrix4x4.h"


class Camera : public Behaviour{
//    REGISTER_CLASS_TRAITS(kTypeNoFlags);
    REGISTER_CLASS(Camera);
    DECLARE_OBJECT_SERIALIZE();
public:
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
public:
    void Render();

    void StandaloneRender(RenderFlag renderFlags, const std::string& replacementTag);
};


#endif //HUAHUOENGINE_CAMERA_H
