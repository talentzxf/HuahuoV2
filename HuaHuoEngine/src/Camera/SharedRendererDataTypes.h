#pragma once

enum ShadowCastingMode
{
    kShadowCastingOff,          // No shadows from this object
    kShadowCastingOn,           // Regular shadows (backface culled as specified in shader)
    kShadowCastingTwoSided,     // Two-sided shadows (backface culling always off, even if shader doesn't say it)
    kShadowCastingShadowsOnly,  // Only rendered into the shadowmaps; the object not rendered by normal cameras otherwise
    kShadowCastingModeCount,

    kShadowCastingModeBitSize = 2
};

enum MotionVectorGenerationMode
{
    kMotionVectorCamera = 0,    // Use camera motion for motion vectors
    kMotionVectorObject,        // Use a per object motion vector pass for this object
    kMotionVectorForceNoMotion, // Force no motion for this object (0 into motion buffer)
    kMotionVectorHybridBatch,   // Force motion vector usage for that specific Hybrid renderer batch
    kMotionVectorModeCount,

    kMotionVectorGenerationModeBitSize = 2
};

enum LightProbeUsage
{
    kLightProbeUsageOff = 0,        // Light probing is off
    kLightProbeUsageBlendProbes,    // SH is interpolated from tetrahedrons
    kLightProbeUsageUseProxyVolume, // SH is taken from light probe proxy volume
    kLightProbeUsageExplicitIndex,  // SH is taken from an explicitly indexed probe
    kLightProbeUsageCustomProvided, // SH coefficients are provided by the user via e.g. MaterialPropertyBlock

    kLightProbeUsageModeCount,
    kLightProbeUsageBitSize = 3
};

enum RayTracingMode
{
    kRayTracingModeOff = 0,                     // Won't be added to RayTracingAccelerationStructures
    kRayTracingModeStatic,                      // Doesn't change the Transform(static Renderer)
    kRayTracingModeDynamicTransform,            // The Transform changes will be tracked
    kRayTracingModeDynamicGeometry,             // The geometry can be animated(e.g. SkinnedMeshRenderer) or procedural geometry

    kRayTracingModeBitSize = 2
};

#if UNITY_EDITOR

enum EditorVizRender
{
    kEditorVizRenderDefault = 0,    // Renders everything
    kEditorVizRenderSkip,           // Skips this render node completely in the editor visualization mode.
    kEditorVizRenderFirstMaterial,  // Only render the first material in the editor visualization mode.

    kEditorVizRenderBitSize = 2
};

#endif
