//
// Created by VincentZhang on 5/24/2022.
//

#ifndef HUAHUOENGINE_GRAPHICSCAPS_H
#define HUAHUOENGINE_GRAPHICSCAPS_H
#include "GfxDevice/opengles/GraphicsCapsGLES.h"
#include "Utilities/EnumFlags.h"
#include "GfxDevice/GfxDeviceTypes.h"
#include "Graphics/Format.h"
#include "Math/ColorSpaceConversion.h"
#include <string>
#include <vector>


// Ability to handle non-power-of-two (NPOT) textures
enum NPOTCaps
{
    // "none" used to be 0; all platforms we support today have at least "restricted" NPOT caps
    kNPOTRestricted = 1, // available with restrictions (no mips, no wrap mode, compressed must be a multiple of block size)
    kNPOTFull = 2,       // available, no restrictions
};

// BIND_MANAGED_TYPE_NAME(NPOTCaps, UnityEngine_NPOTSupport);

enum SparseTextureCaps
{
    kSparseTextureNone = 0,
    kSparseTextureTier1,    // no support for reading/writing outside of mapped tiles; no packed mips (see DX11.2 tiled resources Tier 1)
    kSparseTextureTier2,    // min/max filtering, shader functions that return residency (see DX11.2 tiled resources Tier 2)
    kSparseTextureTier3,    // volume tiled texture support
};

enum GPUSkinningCaps
{
    kGPUSkinningCapsNone = 0,
    kGPUSkinningSupported = 1 << 0,
    kGPUSkinningTakesAnyVertexFormats = 1 << 2,
    kGPUSkinningUsesVertexDeclaration = 1 << 3,
    kGPUSkinningHasOnlyComputeSkinning = 1 << 4,
    kGPUSkinningFixedBoneCountCompute = 1 << 5,
    kGPUSkinningVariableBoneCountCompute = 1 << 6,
    kGPUSkinningAllBoneCountCompute = kGPUSkinningFixedBoneCountCompute | kGPUSkinningVariableBoneCountCompute,
    kGPUSkinningSupportsBlendShapes = 1 << 7, // Implies kGfxBufferCapsComputeBuffersAsVertexInput or kGfxBufferCapsCopyBuffer support
};
ENUM_FLAGS(GPUSkinningCaps);

enum VertexFormatCaps
{
    kVertexFormatCapsNone       = 0,
    kVertexFormatCapsX          = 1 << 0,
    kVertexFormatCapsXY         = 1 << 1,
    kVertexFormatCapsXYZ        = 1 << 2,
    kVertexFormatCapsXYZW       = 1 << 3,
    kVertexFormatCapsNot1D      = kVertexFormatCapsXY | kVertexFormatCapsXYZ | kVertexFormatCapsXYZW,
    kVertexFormatCapsNot3D      = kVertexFormatCapsX | kVertexFormatCapsXY | kVertexFormatCapsXYZW,
    kVertexFormatCapsAnyDim     = kVertexFormatCapsX | kVertexFormatCapsXY | kVertexFormatCapsXYZ | kVertexFormatCapsXYZW,
    kVertexFormatCapsEvenDim    = kVertexFormatCapsXY | kVertexFormatCapsXYZW,
};
ENUM_FLAGS(VertexFormatCaps);

enum GfxBufferCaps
{
    kGfxBufferCapsNone = 0,
    kGfxBufferCapsZeroStrideVertexStreams = 1 << 0,     // Support for zero stride vertex buffers, i.e. repeating the same value. On GL this is emulated and only works in combination with GetDefaultVertexBuffer().
    kGfxBufferCapsComputeBuffersAsVertexInput = 1 << 1, // Support for compute buffers as input to vertex shaders (unlike DX11)
    kGfxBufferCapsCopyBuffer = 1 << 2,                  // Support for GfxDevice::CopyBuffer(GfxBuffer*, GfxBuffer*)
};
ENUM_FLAGS(GfxBufferCaps);

enum FormatSupport
{
    kSupportNative, // The format is directly supported by the GPU
    kSupportCaveat, // We can fallback to a compatible format which may requires twiddling of the data before upload
};

enum VendorID
{
    kVendorDummyRef = 0x0000,
    kVendor3DLabs = 0x3d3d,
    kVendorMatrox = 0x102b,
    kVendorS3 = 0x5333,
    kVendorSIS = 0x1039,
    kVendorXGI = 0x18ca,
    kVendorIntel = 0x8086,
    kVendorATI = 0x1002,
    kVendorNVIDIA = 0x10de,
    kVendorTrident = 0x1023,
    kVendorImgTech_Legacy = 0x104a,
    kVendorVIAS3G = 0x1106,
    kVendor3dfx = 0x121a,
    kVendorParallels = 0x1ab8,
    kVendorMicrosoft = 0x1414,
    kVendorVMWare = 0x15ad,
    kVendorVBox = 0x80ee,
    kVendorAnka = 0x1d9e,
    kVendorQualcomm = 0x5143,
    kVendorQualcomm_WindowsRT = 0x4d4f4351,
    kVendorNVIDIA_WindowsRT = 0x4144564e,
    kVendorARM = 0x13b5,
    kVendorImgTech = 0x1010,
};

std::string GetVendorString(VendorID vendorID);


inline VertexFormatCaps VertexFormatCapsForDimension(UInt8 dim)
{
    DebugAssert(dim >= 1 && dim <= 4);
    return static_cast<VertexFormatCaps>(1 << (dim - 1));
}

inline bool VertexFormatSupportsDimension(VertexFormatCaps caps, UInt8 dim)
{
    return (caps & VertexFormatCapsForDimension(dim)) != 0;
}


// Returns true if the param is one of GL ES device levels.
inline bool IsGfxLevelES2(GfxDeviceLevelGL level)
{
    return level == kGfxLevelES2;
}

inline bool IsGfxLevelES3(GfxDeviceLevelGL level, GfxDeviceLevelGL minLevel = kGfxLevelES3First)
{
    DebugAssertMsg(minLevel >= kGfxLevelES3First && minLevel <= kGfxLevelES3Last, "OPENGL ERROR: 'minLevel' must be a ES3 level");
    return level >= minLevel && level <= kGfxLevelES3Last;
}

inline bool IsGfxLevelES(GfxDeviceLevelGL level)
{
    return level >= kGfxLevelESFirst && level <= kGfxLevelESLast;
}

inline bool IsGfxLevelCore(GfxDeviceLevelGL level, GfxDeviceLevelGL minLevel = kGfxLevelCoreFirst)
{
    DebugAssertMsg(minLevel >= kGfxLevelCoreFirst && minLevel <= kGfxLevelCoreLast, "OPENGL ERROR: 'minLevel' must be a core level");
    return level >= minLevel && level <= kGfxLevelCoreLast;
}

struct GraphicsCaps {
public:
    GraphicsCaps();

    // ---------- caps common for all devices
    std::string        rendererString; // graphics card name
    std::string        vendorString;   // graphics card vendor name
    std::string        driverVersionString; // (GL) version as reported by the driver
    std::string        fixedVersionString; // (GL) correct GL version appended in front by us
    std::string        driverLibraryString;    // Name of driver's DLL and version
    VendorID        vendorID;
    int             rendererID;

    ShaderRequirements shaderCaps; // Which of shader capabilities are supported

    GraphicsTier activeTier; // rough 'hardware tier' to separate different capability hardware for shader fallbacks. GfxDevice auto-detects, then user overridable

    float videoMemoryMB;        // Approx. amount of video memory in MB. Used to limit texture, render target and so on sizes to sane values.

    bool    usesOpenGLTextureCoords; // OpenGL: texture V coordinate is 0 at the bottom; 1 at the top. Otherwise: texture V coordinate is 0 at the top; 1 at the bottom.
    bool    usesReverseZ;            // D3D style : Z is inverted 1 at near plane, 0 at far plane.

    int     maxVSyncInterval;   // Max frames that device can wait for vertical blank
    int     maxAnisoLevel;
    int     maxTextureBinds;
    UInt32  maxConstantBufferSize;

    int     maxTextureSize;
    int     maxCubeMapSize;
    int     maxRenderTextureSize;   // usually maxTextureSize, except on some really old GPUs
    int     maxTextureArraySlices;

    int     maxMRTs;
    int     maxRandomWrites;
    int     maxComputeBufferInputsVertex;
    int     maxComputeBufferInputsFragment;
    int     maxComputeBufferInputsDomain;
    int     maxComputeBufferInputsHull;
    int     maxComputeBufferInputsGeometry;
    int     maxComputeBufferInputsCompute;

    int     maxComputeWorkGroupSize;
    int     maxComputeWorkGroupSizeX;
    int     maxComputeWorkGroupSizeY;
    int     maxComputeWorkGroupSizeZ;

    bool    hasAnisoFilter;     // has anisotropic filtering?
    bool    hasMipLevelBias;    // can apply mipmap bias in texture sampling state? (defaults to true)
    bool    hasMipMaxLevel;     // can specify max mip level

    bool    hasStereoscopic3D;

    bool    hasMultiSample;
    bool    hasMultiSampleAutoResolve;  // uses texture instead, and under-the-hood rb will be auto-resolved to it
    bool    hasMultiSampleDownScale;    // MSAA hardware automatically down scales from chip to memory so MSAA render textures don't need a resolve.
    bool    hasMultiSampleTexture2DArray;
    bool    hasMultiSampleTexture2DArrayAutoResolve; // uses texture 2d array instead, and under-the-hood rb will auto-resolved to it.

    bool    hasMemorylessRenderTexture;     // render surfaces that only use temporary memory (usually 'on-chip')
    bool    hasMemorylessRenderTextureMSAA; // supports resolve from a memoryless MSAA surface to a memory-backed resolve surface

    // Blending capabilities
    bool    hasSeparateMRTBlend;    // Can do different blend setups for each render target in MRT setup
    bool    hasBlendMinMax;         // Min/Max blending operations (defaults to true)
    bool    hasBlendLogicOps;   // kBlendOpLogical*
    bool    hasBlendAdvanced;    // GL_KHR_blend_equation_advanced (subset of GL_NV_blend_equation_advanced)
    bool    hasBlendAdvancedCoherent; // Coherent version (does not require blend barriers)

    bool    has32BitIndexBuffer;
    bool    has16BitReadPixel;

    bool    hasTimerQuery;

    bool    hasTexture2DMS; // has multisampled texture support
     FormatUsageFlags supportsFormatUsageBits[kGraphicsFormatCount];

    bool hasTextureWrapMirrorOnce; // is mirror once texture coordinate wrap mode supported?

    // On most platforms, use floating point render targets to store depth of point
    // light shadowmaps. However, on some others they either have issues, or aren't widely
    // supported; in which case fallback to encoding depth into RGBA channels.
    // Make sure UNITY_USE_RGBA_FOR_POINT_SHADOWS shader define matches this.
    bool useRGBAForPointShadows;

    // Preferred format the built-in UnityAttenuation texture
    // Defaults to kFormatNone, otherwise the the format is expected to be supported
    // for sampling with linear filtering and the channel should match UNITY_ATTEN_CHANNEL (HLSLSupport.cginc)
    GraphicsFormat attenuationFormat;

    bool hasNativeQuad;

    bool has3DTexture;

    bool hasCompressedTexture3D;

    bool supportsDepthCubeTexture;

    NPOTCaps    npot;

    bool hasSRGBReadWrite;          // Controls whether linear mode is supported within Unity
    bool hasWideColorReadWrite;     // Controls whether wide-gamut display modes is supported within Unity
    bool hasWideColorRequested;     // Controls whether wide-gamut display modes is requested to be used


    std::vector<GraphicsFormat> supportedStencilFormats; // The formats supported for viewing the stencil information in shaders. These are different per platform.

    bool hasNonFullscreenClear;     // Can do clears on non-full screen? (e.g. D3D11 can not)
    bool hasClearMRT;               // Supports GfxDevice::ClearMRT
    bool needsUpscalingShader;      // Requires blit shader to upscale on PresentFrame()

    bool hasRenderTo3D;             // We have render-to-volume functionality
    bool hasShadows;                // Has some way of doing shadows (either through native shadowmaps, or depth textures with manual comparison)
    bool hasRawShadowDepthSampling; // Allows for sampling raw depth from shadowmap (along with SetShadowSamplingMode()).

    bool hasRenderTargetArrayIndexFromAnyShader; // Can set output render target array index from any shader stage (not requiring geometry shader)
    bool hasNativeDepthTexture;     // Depth textures come from actual depth buffer
    bool hasStencilInDepthTexture;  // Has native depth texture AND stencil buffer of it can be used at the same time
    bool hasNativeShadowMap;        // Has RT formats that can do shadow comparison sampling
    bool hasTiledGPU;               // Uses tiled rendering (prefers to clear/discard render targets when possible, to save on memory traffic)
    bool hasHiddenSurfaceRemovalGPU;// Uses "deferred" pixel rendering (i.e. shades only frontmost opaque surfaces; no point in sorting them on the CPU)

    // are GfxRTLoadAction/GfxRTStoreAction [UnityEngine.Rendering.RenderBufferLoadAction/RenderBufferStoreAction] actually doing anything
    // this is important mostly for diagnostics, as we can be more lax when load/store actions are ignored
    // for example, you should not mix DontCare load action with non-fullscreen camera's viewport, but it will work if load actions are ignored
    bool usesLoadStoreActions;
    bool usesStoreAndResolveAction;

    bool hasDynamicUniformArrayIndexing;    // OpenGL ES2 specification says "it's not mandated". We're exposing this to provide an opportunity to fallback to e.g. vertex lighting in LW.

    bool hasDecentRGBMCompression;  // Widely available texture formats can compress RGBM fine (e.g. true for DXT5 etc, false for PVRTC)

    GPUSkinningCaps gpuSkinningCaps; // Which, if any, types of GPU skinning are supported

    VertexFormatCaps vertexFormatCaps[kVertexFormatCount];

    GfxBufferCaps bufferCaps;

    bool disableSubTextureUpload;                   // Can we do UploadTextureSubData2D?
    bool supportsSubTextureCompressedFormatUploads; // Can we use UploadTextureSubData2D on textures with a compressed format?
    bool warnRenderTargetUnresolves;                // Warn when doing RT un-resolves

    bool supportsAsyncCompute;          //Does the platform support async compute queues and has it been implemented in Unity on that platform
    bool supportsGpuRecorder;           //Does the platform support GPU Recorder API ( ie Recorder.gpuElapsedNanoseconds )
    bool supportsGPUFence;              //Had the platform implemented the GPU fence system
    bool supportsAsyncReadback;         //Does the platform support async readback and has it been implemented in Unity on that platform
#if ENABLE_TEXTURE_STREAMING
    bool supportsMipStreaming;          //Does the platform support mip map streaming (requires async read support)
#endif
    bool supportsUploadSurface;         //Does the platform implements the GfxDevice::UploadSurface API ?

    bool supportsRayTracing;            // The selected API and current hardware has ray tracing capabilities which are supported by Unity

    bool supportsDynamicResolution;     //Does the platform support Dynamic Resolution and has it been implemented in Unity on that platform

    bool hasRenderPass;                 // If true, has a custom (not the GfxDevice common) implementation of the RenderPass API.

    bool hasReadOnlyDepth;              // If true, the renderer supports binding the depth attachment as read-only, and simultaneously reading from it

    HDRDisplaySupportFlags hdrDisplaySupportFlags; // Support level for HDR Display output

    bool hasFragmentDensityMapAttachment;       //If true, the platform supports an additional fragment density map attachment, often for resolution-tuning

    // cross platform caps initialized in SharedCapsPostInitialize
    bool hasPrePassRenderLoop;
    bool hasDeferredRenderLoop;

    SinglePassStereo singlePassStereo;
    SparseTextureCaps sparseTextures;
    CopyTextureSupport copyTextureSupport;
    bool threadedResourceCreationSupport;
    bool directUploadGpuMemoryWriteOnly;    // If true, indicates GPU memory read is slow so we should avoid reading back (E.g. during the direct texture upload)
    bool conservativeRasterSupport;

    bool hasSetConstantBuffer;              // If true, Shader.SetConstantBuffer etc take effect.
    int  minConstantBufferOffsetAlignment;  // The minimum alignment of the offset given when binding a constant buffer. If this is 0, the renderer cannot bind a constant buffer with a non-zero offset.
    bool canCreateUninitializedTextures;

    bool UsesRGBMCompression() const { return maxTextureBinds >= 3 && hasDecentRGBMCompression; }

    bool SupportsShaderRequirements(ShaderRequirements requirements) const;

    std::string CheckGPUSupported() const;

    GraphicsFormat GetCompatibleFormat(const GraphicsFormat requestedFormat, const FormatUsage usage) const;

    // The backend implements GraphicsFormat, supporting all GPU formats
    bool hasGraphicsFormat;
    // Info for SRP batcher
    bool hasNoSeparateFragmentShaderStage;  // True when the current GfxDevice doesn't have separate shader stages.
    bool bindSlotContainsMoreInfo;          // In VK and Switch, bind slot contains more info than a slot number ( see VKBinding.h )
    bool srpBatcherSupported;               // True when the current GfxDevice supports SRP batcher.

    // ---- hardware/driver workarounds for all renderers

    bool disableSoftShadows;        // Soft shadows should work, but the driver is buggy on those shaders
    bool buggyDepthBlitWithSRGBEnabled; // depth blit with FRAMEBUFFER_SRGB enabled is buggy
    bool buggySRGBWritesOnLinearTextures; // does sRGB conversions on linear textures
    bool buggyDepthStencilClear;    // If this is true, color/depth/stencil is cleared by drawing a quad instead.
    // Vertex shaders aren't fully IEEE float, and forward objects in deferred rendering need slight
    // Z bias to make them stop Z-fighting with later actual forward rendering.
    bool needsZBiasInDeferredForForwardObjects;
    float skyboxProjectionEpsilonFactor; // Some drivers/GPUs have artifacts with default epsilon
    bool disableNativeGraphicsJobs;     // True when the current GPU driver is known to have issues with native graphics jobs support
    // ---- caps specific for renderers

    bool IsFormatSupported(const GraphicsFormat requestedFormat, const FormatUsage usage, const FormatSupport support = kSupportNative) const;

    GraphicsFormat FindUploadFormat(const GraphicsFormat requestedFormat, const FormatUsage usage) const;
    GraphicsFormat FindUploadFormat(const GraphicsFormat requestedFormat, const FormatUsage usage, bool alphaOptional) const;

    void InitDefaultFormat();

    // defaultFormats[kDefaultFormatLDR] changes with current color space
    // defaultFormatLDR[] always stores the default LDR format for linear and gamma modes.
    GraphicsFormat defaultFormatLDR[2]; // 2 == kColorSpaceCount

    GfxBufferTarget computeBufferTargetForGeometryBuffer; //Global common settings for vertexBuffer dedicated to compute shader bindings. SkinnedMesh & VFX are concerned by this target.
#if PLATFORM_SUPPORTS_OPENGL_UNIFIED
    GraphicsCapsGLES    gles;
#endif

    void ClearMemory();
    static void CleanupGraphicsCapsMemory();

    // Update the default LDR format to the new colour space
    void UpdateDefaultLDRFormat();
    void SetDefaultLDRFormat(GraphicsFormat format);

    // Set the Default HDR format after initialisation
    void UpdateDefaultHDRFormat();
    void SetDefaultHDRFormat(GraphicsFormat format);

    bool SupportsFormatUsagePixels32(GraphicsFormat format) const;
    bool SupportsFormatUsageGetPixel(GraphicsFormat format) const;
    bool SupportsFormatUsageSetPixel(GraphicsFormat format) const;
    bool SupportsFormatUsageReadback(GraphicsFormat format) const;

    // Each platform may use different graphics format for specific usages that are represented by DefaultFormat
    GraphicsFormat GetGraphicsFormat(DefaultFormat defaultFormat, ColorSpace colorSpace = kCurrentColorSpace) const;
private:
    GraphicsFormat defaultFormats[kDefaultFormatCount];
};

GraphicsCaps& GetGraphicsCaps();

#endif //HUAHUOENGINE_GRAPHICSCAPS_H
