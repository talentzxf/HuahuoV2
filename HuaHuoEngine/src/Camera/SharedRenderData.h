//
// Created by VincentZhang on 5/18/2022.
//
#ifndef HUAHUOENGINE_SHAREDRENDERDATA_H
#define HUAHUOENGINE_SHAREDRENDERDATA_H

#include "Math/Vector4f.h"
#include "Math/Matrix4x4.h"
#include "Graphics/LightmapTypes.h"
#include "Graphics/RendererType.h"
#include "SharedRendererDataTypes.h"
#include "Geometry/AABB.h"
#include "Camera/RenderLoops/GlobalLayeringData.h"
#include "Graphics/StaticBatchInfo.h"

struct TransformInfo
{
    Matrix4x4f     worldMatrix;             // 64
    Matrix4x4f     prevWorldMatrix;         // 64
    AABB           worldAABB;               // 24
    AABB           localAABB;               // 24   used by LightManager and Shadows
    SInt32         motionVectorFrameIndex;  // 4
    TransformType  transformType;           // 1
    UInt16         lateLatchIndex;          // 2
};

struct SharedRendererData
{
    TransformInfo           m_TransformInfo;
    StaticBatchInfo         m_StaticBatchInfo;
    GlobalLayeringData      m_GlobalLayeringData;

    Vector4f                m_LightmapST[kLightmapTypeCount];       ///< Lightmap tiling and offset

    LightmapIndices         m_LightmapIndex;

    UInt32                  m_RendererType                  : kRendererTypeBitSize; /// enum RendererType
    UInt32                  m_CastShadows                   : kShadowCastingModeBitSize;  ///< enum { Off = 0, On = 1, Two Sided = 2, Shadows Only = 3 }
    UInt32                  m_ReceiveShadows                : 1;
    UInt32                  m_HasLastPositionStream         : 1;
    UInt32                  m_MotionVectors                 : kMotionVectorGenerationModeBitSize; ///< enum { Camera Motion Only = 0, Per Object Motion = 1, Force No Motion = 2 } Motion Vector generation mode.
    UInt32                  m_IsVisible                     : 1;
//    UInt32                  m_ReflectionProbeUsage          : kReflectionProbeBitSize;
    UInt32                  m_LightProbeUsage               : kLightProbeUsageBitSize;
    UInt32                  m_RayTracingMode                : kRayTracingModeBitSize;
    UInt32                  m_RayTraceProcedural            : 1;
    UInt32                  m_DynamicOccludee               : 1;
#if UNITY_EDITOR
    UInt32                  m_EditorVizRender               : kEditorVizRenderBitSize;
#endif

    UInt32                  m_RenderingLayerMask;

    // Currently only used by SRPs.
    // Used as an additional criterion for sorting after render queue.
    SInt32                   m_RendererPriority;

    SharedRendererData() {}

    SharedRendererData(RendererType type);
};

#endif //HUAHUOENGINE_SHAREDRENDERDATA_H
