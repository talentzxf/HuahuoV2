//
// Created by VincentZhang on 5/24/2022.
//

#include "GraphicsCaps.h"
#include "Utilities/RegisterRuntimeInitializeAndCleanup.h"
#include "Memory/MemoryMacros.h"
#include "GraphicsCapsScriptBinding.h"
#include "Math/ColorSpaceConversion.h"
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


void GraphicsCaps::UpdateDefaultLDRFormat()
{
    const ColorSpace colorSpace = GetActiveColorSpace();

    AssertMsg(colorSpace != kUninitializedColorSpace, "Colorspace needs to be initialized before updating default formats");

    defaultFormats[kDefaultFormatLDR] = defaultFormatLDR[colorSpace];
}

void GraphicsCaps::SetDefaultLDRFormat(GraphicsFormat format)
{
    AssertMsg(IsValidFormat(format), "Invalid LDR Graphics Format %i specified", format);
    defaultFormats[kDefaultFormatLDR] = format;
}

GraphicsFormat GraphicsCaps::GetGraphicsFormat(DefaultFormat defaultFormat, ColorSpace colorSpace) const
{
    const GraphicsFormat format = (defaultFormat == kDefaultFormatLDR && colorSpace != kCurrentColorSpace)
                                  ? defaultFormatLDR[colorSpace]
                                  : defaultFormats[defaultFormat];

    AssertMsg(format != kFormatNone, "Default formats haven't been initialized");
    return format;
}

void GraphicsCaps::UpdateDefaultHDRFormat()
{
    if (/*GetGraphicsSettings().GetTierSettings().hdrMode == kHDRModeR11G11B10Float &&*/ IsFormatSupported(kFormatB10G11R11_UFloatPack32, kUsageRender))
        defaultFormats[kDefaultFormatHDR] = kFormatB10G11R11_UFloatPack32;
    else if (IsFormatSupported(kFormatR16G16B16A16_SFloat, kUsageRender))
        defaultFormats[kDefaultFormatHDR] = kFormatR16G16B16A16_SFloat;
    else if (IsFormatSupported(kFormatR32G32B32A32_SFloat, kUsageRender))
        defaultFormats[kDefaultFormatHDR] = kFormatR32G32B32A32_SFloat;
    else
        defaultFormats[kDefaultFormatHDR] = kFormatR8G8B8A8_UNorm; // We fallback to unorm if no HDR format is supported.
}

void GraphicsCaps::SetDefaultHDRFormat(GraphicsFormat format)
{
    AssertMsg(IsValidFormat(format), "Invalid HDR Graphics Format %i specified", format);
    defaultFormats[kDefaultFormatHDR] = format;
}

void GraphicsCaps::InitDefaultFormat()
{
    const ColorSpace colorSpace = GetActiveColorSpace();

    AssertMsg(colorSpace != kUninitializedColorSpace, "Colorspace needs to be initialized before initializing default formats");

    CompileTimeAssertArraySize(defaultFormatLDR, kColorSpaceCount);

    defaultFormatLDR[kLinearColorSpace] = kFormatR8G8B8A8_SRGB;
    defaultFormatLDR[kGammaColorSpace] = kFormatR8G8B8A8_UNorm;

    GraphicsFormat formatHDR = kFormatR8G8B8A8_UNorm; // We fallback to unorm if no HDR format is supported.
    if (/*GetGraphicsSettings().GetTierSettings().hdrMode == kHDRModeR11G11B10Float &&*/ IsFormatSupported(kFormatB10G11R11_UFloatPack32, kUsageRender))
        formatHDR = kFormatB10G11R11_UFloatPack32;
    else if (IsFormatSupported(kFormatR16G16B16A16_SFloat, kUsageRender))
        formatHDR = kFormatR16G16B16A16_SFloat;
    else if (IsFormatSupported(kFormatR32G32B32A32_SFloat, kUsageRender))
        formatHDR = kFormatR32G32B32A32_SFloat;

    GraphicsFormat const defaultDefaultFormats[kDefaultFormatCount] =
            {
                    defaultFormatLDR[colorSpace],   // kDefaultFormatLDR
                    formatHDR,                      // kDefaultFormatHDR
                    kFormatD24_UNorm_S8_UInt,       // kDefaultFormatDepth
                    kFormatD16_UNorm,               // kDefaultFormatShadow
                    kFormatYUV2,                    // kDefaultFormatVideo
            };

    memcpy(defaultFormats, defaultDefaultFormats, sizeof(defaultDefaultFormats));
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
