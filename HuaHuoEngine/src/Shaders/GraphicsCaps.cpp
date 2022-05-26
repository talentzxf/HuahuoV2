//
// Created by VincentZhang on 5/24/2022.
//

#include "GraphicsCaps.h"
#include "Utilities/RegisterRuntimeInitializeAndCleanup.h"
#include "Memory/MemoryMacros.h"
#include "GraphicsCapsScriptBinding.h"
#include <cstring>
#include <vector>

static GraphicsCaps* gGraphicsCaps;
void StaticInitializeGraphicsCaps(void*){
    gGraphicsCaps = HUAHUO_NEW(GraphicsCaps, kMemGfxDevice);
}

void StaticCleanupGraphicsCaps(void*){

}

static RegisterRuntimeInitializeAndCleanup s_GraphicsCaps(StaticInitializeGraphicsCaps, StaticCleanupGraphicsCaps, 0);


GraphicsCaps& GetGraphicsCaps()
{
    // __FAKEABLE_FUNCTION__(GetGraphicsCaps, ());
    return *gGraphicsCaps;
}

GraphicsCaps::GraphicsCaps(){
    ::memset(this, 0x00, sizeof(GraphicsCaps));

    // Evil hack to make the memset above work with the constructors of the std::string
    //@TODO: Make this either be const char* or move the strings into a separate struct.
    new(&rendererString) std::string();
    new(&vendorString) std::string();
    new(&driverVersionString) std::string();
    new(&fixedVersionString) std::string();
    new(&driverLibraryString) std::string();
    new(&supportedStencilFormats) std::vector<GraphicsFormat>;//(kMemGfxDevice);
    vendorID = kVendorDummyRef;
    rendererID = 0;

    memset(&supportsFormatUsageBits, 0, sizeof(supportsFormatUsageBits));

    for (std::size_t i = 0; i < kGraphicsFormatCount; ++i)
        supportsFormatUsageBits[i] = kUsageLinearBit | kUsageLoadStoreBit | kUsageBlendBit;

    hasTextureWrapMirrorOnce = true;
    hasTexture2DMS = true;

    shaderCaps = kShaderRequireShaderModel20;
    videoMemoryMB = 16.0f;

    usesReverseZ = false;
    usesOpenGLTextureCoords = false;

    maxAnisoLevel = 1;
    maxTextureBinds = 8;
    maxConstantBufferSize = 0;
    hasMipLevelBias = true;
    hasMipMaxLevel = true;
    hasBlendMinMax = true;
    has32BitIndexBuffer = true;

    maxTextureSize = 256;
    maxCubeMapSize = 64;
    maxRenderTextureSize = 128;
    maxTextureArraySlices = 1;
    maxMRTs = 1;
    maxRandomWrites = 0;


    npot = kNPOTRestricted;
    hasNonFullscreenClear = true;

    hasDecentRGBMCompression = true;
    hasShadows = true;
    hasNativeShadowMap = true;
    hasRawShadowDepthSampling = true;

    sparseTextures = kSparseTextureNone;
    threadedResourceCreationSupport = false;
    directUploadGpuMemoryWriteOnly = true;   // Default to avoiding reads from GPU memory

    hasDynamicUniformArrayIndexing = true; // This is a ES2-only thing, so it's supported by default

    supportsAsyncCompute = false;
    supportsGpuRecorder = false;
    supportsGPUFence = false;
    supportsAsyncReadback = false;
    supportsDynamicResolution = false;
#if ENABLE_TEXTURE_STREAMING
    supportsMipStreaming = false;
#endif
    supportsUploadSurface = false;

    supportsRayTracing = false;

    // default to highest tier
    activeTier = kGraphicsTierMax;

    hasMultiSampleDownScale = false;

    skyboxProjectionEpsilonFactor = 1.0f;

    attenuationFormat = kFormatNone;
    supportsSubTextureCompressedFormatUploads = false; // opt-in for now

    hdrDisplaySupportFlags = kHDRDisplaySupportFlagsNone;
    hasCompressedTexture3D = true;
    canCreateUninitializedTextures = true;

//    if (HasARGV("force-rawbuffer-for-compute-geometrybuffer"))
//    {
//        computeBufferTargetForGeometryBuffer = kGfxBufferTargetRaw;
//    }
//    else
    {
        //Default mode of most platform, use the simplest shader in skinning with less ALU/Wait, D3D uses kGfxBufferTargetRaw
        computeBufferTargetForGeometryBuffer = kGfxBufferTargetStructured;
    }

    usesStoreAndResolveAction = true;
}

GraphicsFormat GraphicsCaps::FindUploadFormat(const GraphicsFormat requestedFormat, const FormatUsage usage) const
{
    if (requestedFormat == kFormatNone)
        return kFormatNone;

    const bool requestedFormatSupport = IsFormatSupported(requestedFormat, usage, kSupportNative);
    if (requestedFormatSupport)
        return requestedFormat;

    GraphicsFormat fallbackFormat = requestedFormat;
    while (true)
    {
        const GraphicsFormat newFallbackFormat = GetDesc(fallbackFormat).fallbackFormat;
        if (newFallbackFormat == requestedFormat || newFallbackFormat == fallbackFormat)
            return kFormatNone;

        const bool fallbackFormatSupport = IsFormatSupported(newFallbackFormat, usage, kSupportNative);
        if (fallbackFormatSupport)
            return newFallbackFormat;

        fallbackFormat = newFallbackFormat;
    }
}

GraphicsFormat GraphicsCaps::FindUploadFormat(const GraphicsFormat requestedFormat, const FormatUsage usage, bool alphaOptional) const
{
    GraphicsFormat uploadFormat = FindUploadFormat(requestedFormat, usage);
    if (!alphaOptional && !HasAlphaChannel(uploadFormat) && HasAlphaChannel(requestedFormat))
        uploadFormat = FindUploadFormat(GetDesc(uploadFormat).alphaFormat, usage);
    return uploadFormat;
}

bool GraphicsCaps::IsFormatSupported(const GraphicsFormat requestedFormat, const FormatUsage usage, const FormatSupport support) const
{
    if (requestedFormat == kFormatNone)
        return false;
    const bool hasNativeSupport = (supportsFormatUsageBits[requestedFormat] & (1 << usage)) != 0;
    if (support == kSupportNative || hasNativeSupport)
        return hasNativeSupport;
    if (support == kSupportCaveat)
        return FindUploadFormat(requestedFormat, usage) != kFormatNone;
    return false;
}

GraphicsFormat GraphicsCaps::GetCompatibleFormat(const GraphicsFormat requestedFormat, const FormatUsage usage) const
{
    if (ScriptingGraphicsCaps::IsFormatSupported(requestedFormat, usage))
        return requestedFormat;

    for (int i = 0; i < kGraphicsFormatCount && !IsCompressedFormat(requestedFormat); ++i)
    {
        const GraphicsFormat format = static_cast<GraphicsFormat>(i);
        if (!ScriptingGraphicsCaps::IsFormatSupported(format, usage))
            continue;

        if (GetColorComponentCount(format) != GetColorComponentCount(requestedFormat))
            continue;
        if (GetAlphaComponentCount(format) != GetAlphaComponentCount(requestedFormat))
            continue;
        if (IsDepthFormat(format) != IsDepthFormat(requestedFormat))
            continue;
        if (IsStencilFormat(format) != IsStencilFormat(requestedFormat))
            continue;
        if (IsUNormFormat(format) != IsUNormFormat(requestedFormat))
            continue;
        if (IsSNormFormat(format) != IsSNormFormat(requestedFormat))
            continue;
        if (IsUIntFormat(format) != IsUIntFormat(requestedFormat))
            continue;
        if (IsSIntFormat(format) != IsSIntFormat(requestedFormat))
            continue;
        if (IsIEEE754Format(format) != IsIEEE754Format(requestedFormat))
            continue;
        if (GetBlockSize(format) != GetBlockSize(requestedFormat))
            continue;
        if (IsAlphaTestFormat(format) != IsAlphaTestFormat(requestedFormat))
            continue;
        if (IsSRGBFormat(format) && !IsSRGBFormat(requestedFormat)) // We allow to fallback to linear format but not to sRGB format.
            continue;

        return format;
    }

    const GraphicsFormat compatibleFormat = FindUploadFormat(requestedFormat, usage);

    if (ScriptingGraphicsCaps::IsFormatSupported(compatibleFormat, usage))
        return compatibleFormat;
    else
        return kFormatNone;
}
