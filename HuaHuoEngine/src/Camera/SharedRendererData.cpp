//
// Created by VincentZhang on 5/22/2022.
//
#include "SharedRendererData.h"

SharedRendererData::SharedRendererData(RendererType type)
        : m_RendererType(type)
        , m_CastShadows(kShadowCastingOn)
        , m_ReceiveShadows(true)
        , m_HasLastPositionStream(false)
        , m_MotionVectors(kMotionVectorObject)
        , m_IsVisible(false)
        // , m_ReflectionProbeUsage(kReflectionProbeOff)
        , m_LightProbeUsage(kLightProbeUsageOff)
        , m_RayTracingMode(kRayTracingModeOff)
        , m_RayTraceProcedural(false)
        , m_DynamicOccludee(1)
        , m_RendererPriority(0)
        // , m_RenderingLayerMask(GetGraphicsSettings().GetDefaultRenderingLayerMask())
#if UNITY_EDITOR
, m_EditorVizRender(kEditorVizRenderDefault)
#endif
{
    m_LightmapIndex.Reset();
    for (int i = 0; i < kLightmapTypeCount; i++)
        m_LightmapST[i] = kDefaultLightmapST;
}
