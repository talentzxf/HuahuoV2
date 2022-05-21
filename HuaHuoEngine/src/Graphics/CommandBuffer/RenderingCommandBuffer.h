//
// Created by VincentZhang on 5/17/2022.
//

#ifndef HUAHUOENGINE_RENDERINGCOMMANDBUFFER_H
#define HUAHUOENGINE_RENDERINGCOMMANDBUFFER_H
#include "GfxDevice/GfxDeviceTypes.h"
#include "Math/Color.h"
#include "Containers/GrowableBuffer.h"
#include "Core/SharedObject.h"

enum RenderCommandType
{
    kRenderCommand_DrawRenderer = 0,
    kRenderCommand_DrawMesh,
    kRenderCommand_DrawProcedural,
    kRenderCommand_DrawProceduralIndexed,
    kRenderCommand_DrawProceduralIndirect,
    kRenderCommand_DrawProceduralIndexedIndirect,
    kRenderCommand_DrawMeshInstanced,
    kRenderCommand_DrawMeshInstancedProcedural,
    kRenderCommand_DrawMeshInstancedIndirect,
    kRenderCommand_DrawOcclusionMesh,
    kRenderCommand_SetComputeValueParam,
    kRenderCommand_SetComputeTextureParam,
    kRenderCommand_SetComputeBufferParam,
    kRenderCommand_SetComputeConstantBufferParam,
    kRenderCommand_SetComputeBufferData,
    kRenderCommand_SetComputeBufferCounterValue,
    kRenderCommand_DispatchCompute,
#if GFX_SUPPORTS_RAY_TRACING
    kRenderCommand_BuildRayTracingAccelerationStructure,
    kRenderCommand_SetRayTracingTextureParam,
    kRenderCommand_SetRayTracingAccelerationStructure,
    kRenderCommand_SetRayTracingBufferParam,
    kRenderCommand_SetRayTracingConstantBufferParam,
    kRenderCommand_SetRayTracingValueParam,
    kRenderCommand_SetRayTracingShaderPass,
    kRenderCommand_DispatchRays,
#endif
    kRenderCommand_CopyCounterValue,
    kRenderCommand_SetRT,
    kRenderCommand_SetRTBuffers,
    kRenderCommand_SetRandomWriteTargetTexture,
    kRenderCommand_SetRandomWriteTargetBuffer,
    kRenderCommand_ClearRandomWriteTargets,
    kRenderCommand_SetViewport,
    kRenderCommand_EnableScissor,
    kRenderCommand_DisableScissor,
    kRenderCommand_CopyTexture,
    kRenderCommand_ConvertTexture,
    kRenderCommand_BlitRT,
    kRenderCommand_ClearRT,
    kRenderCommand_GetTempRT,
    kRenderCommand_ReleaseTempRT,
    kRenderCommand_SetSinglePassStereo,
    kRenderCommand_SetGlobalFloat,
    kRenderCommand_SetGlobalVector,
    kRenderCommand_SetGlobalMatrix,
    kRenderCommand_SetGlobalTexture,
    kRenderCommand_SetGlobalBuffer,
    kRenderCommand_SetGlobalFloatArray,
    kRenderCommand_SetGlobalVectorArray,
    kRenderCommand_SetGlobalMatrixArray,
    kRenderCommand_SetShaderKeyword,
    kRenderCommand_SetViewProjectionMatrices,
    kRenderCommand_SetGlobalDepthBias,
    kRenderCommand_SetShadowSamplingMode,
    kRenderCommand_IssuePluginEvent,
    kRenderCommand_BeginSample,
    kRenderCommand_EndSample,
    kRenderCommand_CreateGPUFence,
    kRenderCommand_WaitOnGPUFence,
    kRenderCommand_GenerateMips,
    kRenderCommand_ResolveAntiAliasedSurface,
#if GFX_SUPPORTS_RENDERING_EXT_PLUGIN
    kRenderCommand_IssuePluginEventAndData,
    kRenderCommand_IssuePluginCustomBlit,
    kRenderCommand_IssuePluginTextureUpdate,
#endif
    kRenderCommand_SetInvertCulling,
    kRenderCommand_RequestBufferAsyncReadbackNative,
    kRenderCommand_WaitAllAsyncReadbackRequests,
    kRenderCommand_RequestBufferAsyncReadback,
    kRenderCommand_RequestTextureAsyncReadback,
    kRenderCommand_SetGlobalConstantBuffer,
    kRenderCommand_IncrementUpdateCount,
    kRenderCommand_SetInstanceMultiplier,
#if UNITY_EDITOR
    kRenderCommand_SetAsyncCompilation,
    kRenderCommand_RestoreAsyncCompilation,
#endif
    kRenderCommand_ProcessVTFeedback,
    kRenderCommand_SwitchIntoFastMemory,
    kRenderCommand_SwitchOutOfFastMemory,
    kRenderCommandCount
};

class RenderingCommandBuffer : public ThreadSharedObject<RenderingCommandBuffer>, public NonCopyable {
public:
    enum
    {
        kFlagsNone = 0,
        kFlagsKeepState = 1,
    };
public:
    explicit RenderingCommandBuffer(MemLabelRef label);
    RenderingCommandBuffer(MemLabelRef label, const RenderingCommandBuffer& other);
    ~RenderingCommandBuffer();

    void AddClearRenderTarget(GfxClearFlags clearFlags, const ColorRGBAf& color, float depth, UInt32 stencil);
    void Release();

private:
    GrowableBuffer          m_Buffer;
};


#endif //HUAHUOENGINE_RENDERINGCOMMANDBUFFER_H
