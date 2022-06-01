//
// Created by VincentZhang on 5/26/2022.
//

#ifndef HUAHUOENGINE_GRAPHICSCAPSSCRIPTBINDING_H
#define HUAHUOENGINE_GRAPHICSCAPSSCRIPTBINDING_H
#include <string>
#include "Graphics/Format.h"
#include "GfxDevice/GfxDeviceTypes.h"
#include "GraphicsCaps.h"
#include "Graphics/RenderTextureDesc.h"

namespace ScriptingGraphicsCaps
{
    int GetGraphicsMemorySize();
    std::string GetGraphicsDeviceName();
    std::string GetGraphicsDeviceVendor();
    int GetGraphicsDeviceID();
    int GetGraphicsDeviceVendorID();
    GfxDeviceRenderer GetGraphicsDeviceType();
    bool GetGraphicsUVStartsAtTop();
    std::string GetGraphicsDeviceVersion();
    int GetGraphicsShaderLevel();
    bool GetGraphicsMultiThreaded();
//    GfxThreadingMode GetRenderingThreadingMode();
    bool HasHiddenSurfaceRemovalOnGPU();
    bool HasDynamicUniformArrayIndexingInFragmentShaders();
    bool SupportsShadows();
    bool SupportsRawShadowDepthSampling();
    bool SupportsRenderToCubemap();
    bool Supports3DTextures();
    bool SupportsCompressed3DTextures();
    bool Supports2DArrayTextures();
    bool Supports3DRenderTextures();
    bool SupportsCubemapArrayTextures();
    CopyTextureSupport GetCopyTextureSupport();
    bool SupportsComputeShaders();
    bool SupportsGeometryShaders();
    bool SupportsTessellationShaders();
    bool SupportsRenderTargetArrayIndexFromVertexShader();
    bool SupportsInstancing();
    bool SupportsHardwareQuadTopology();
    bool Supports32bitsIndexBuffer();
    bool SupportsSparseTextures();
    int SupportedRenderTargetCount();
    int SupportedRandomWriteTargetCount();
    int MaxComputeBufferInputsVertex();
    int MaxComputeBufferInputsFragment();
    int MaxComputeBufferInputsGeometry();
    int MaxComputeBufferInputsHull();
    int MaxComputeBufferInputsDomain();
    int MaxComputeBufferInputsCompute();
    bool SupportsSeparatedRenderTargetsBlend();
    bool SupportsMultisampledTextures();
    bool SupportsMultisampled2DArrayTextures();
    bool SupportsMultisampleAutoResolve();
    bool SupportsTextureWrapMirrorOnce();
    bool UsesReversedZBuffer();
    bool HasRenderTexture(RenderTextureFormat format);
    bool SupportsBlendingOnRenderTextureFormat(RenderTextureFormat format);
    bool SupportsTextureFormat(TextureFormat format);
    bool SupportsVertexAttributeFormat(VertexFormat format, int dimension);
    bool IsFormatSupported(GraphicsFormat format, FormatUsage usage);
    GraphicsFormat GetCompatibleFormat(GraphicsFormat format, FormatUsage usage);
    GraphicsFormat GetGraphicsFormat(DefaultFormat format);
    NPOTCaps GetNPOTSupport();
    int GetMaxTextureSize();
    int GetMaxCubemapSize();
    int GetMaxRenderTextureSize();
    int GetMaxComputeWorkGroupSize();
    int GetMaxComputeWorkGroupSizeX();
    int GetMaxComputeWorkGroupSizeY();
    int GetMaxComputeWorkGroupSizeZ();
    bool SupportsAsyncCompute();
    bool SupportsGpuRecorder();
    bool SupportsGPUFence();
    bool SupportsAsyncGPUReadback();
    bool SupportsRayTracing();
    bool HasMipMaxLevel();
#if ENABLE_TEXTURE_STREAMING
    bool SupportsMipStreaming();
#endif
    bool SupportsSetConstantBuffer();
    int MinConstantBufferOffsetAlignment();
    bool UsesLoadStoreActions();
    HDRDisplaySupportFlags GetHDRDisplaySupportFlags();
    bool SupportsConservativeRaster();
    bool SupportsMultiview();
    int GetRenderTextureSupportedMSAASampleCount(const RenderTextureDesc& desc);
    bool SupportsStoreAndResolveAction();
}



#endif //HUAHUOENGINE_GRAPHICSCAPSSCRIPTBINDING_H
