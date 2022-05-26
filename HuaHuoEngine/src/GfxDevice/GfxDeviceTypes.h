//
// Created by VincentZhang on 5/13/2022.
//

#ifndef HUAHUOENGINE_GFXDEVICETYPES_H
#define HUAHUOENGINE_GFXDEVICETYPES_H

#include "Utilities/EnumFlags.h"
#include "Graphics/Format.h"


// This file is intended for commonly used, lightweight graphics related types.
// Please avoid adding any complex types or including other files, except GfxDeviceConfigure.h.
// Thanks for your cooperation!

// Never change the enum values!
// They are used in low level native plugin interface, and exposed to scripts.
// Match GraphicsDeviceType scripting enum!
enum GfxDeviceRenderer
{
    //kGfxRendererOpenGLLegacy = 0, // OpenGL2.x, removed
    //kGfxRendererD3D9 = 1, // D3D9, removed
    kGfxRendererD3D11 = 2,
    //kGfxRendererGCM = 3, // PS3, removed
    kGfxRendererNull = 4,
    //kGfxRendererXenon = 6, // Xbox360, removed
    //kGfxRendererOpenGLES = 7, // GLES1.1, removed
    kGfxRendererOpenGLES20 = 8,
    //kGfxRendererMolehill = 9, // Flash Stage3D, removed
    //kGfxRendererOpenGLES20Desktop = 10, // NativeClient, removed
    kGfxRendererOpenGLES3x = 11, // OpenGL ES 3.0 and later
    //kGfxRendererGXM = 12, // Vita removed
    kGfxRendererPS4 = 13,
    kGfxRendererXboxOne = 14,
    //kGfxRendererPSM = 15, // Playstation Mobile, removed
    kGfxRendererMetal = 16,
    kGfxRendererOpenGLCore = 17, // OpenGL 3.x/4.x
    kGfxRendererD3D12 = 18,
    //kGfxRendererN3DS = 19,
    //kGfxRendererWiiU = 20, // Wii U GX2 API
    kGfxRendererVulkan = 21,
    kGfxRendererSwitch = 22, // Nintendo Switch NVN API
    kGfxRendererXboxOneD3D12 = 23,
    kGfxRendererGameCoreXboxOne = 24, // GameCore XboxOne
    kGfxRendererGameCoreScarlett = 25, // GameCore Scarlett
    kGfxRendererPS5 = 26,
    kGfxRendererPS5NGGC = 27,

    kGfxRendererCount = 28
};

enum GfxPrimitiveType
{
    kPrimitiveInvalid = -1,

    kPrimitiveTriangles = 0, kPrimitiveTypeFirst = kPrimitiveTriangles,
    kPrimitiveTriangleStrip,
    kPrimitiveQuads,
    kPrimitiveLines,
    kPrimitiveLineStrip,
    kPrimitivePoints, kPrimitiveTypeLast = kPrimitivePoints,

    kPrimitiveForce32BitInt = 0x7fffffff // force 32 bit enum size
};

enum
{
    // Maximum number of simultaneous texture resources per shader stage that we support across the board. Actual
    // number supported by the GPU/API might be lower, see GraphicsCaps.maxTextureBinds.
    kMaxSupportedTextureUnits = 64,

    kMaxSRPBatcherInstancedVars = 32,

    // Maximum number of samplers we can support per shader stage. Actual number might be lower,
    // or a platform might not have textures separate from samplers at all (e.g. GLES).
    kMaxSupportedSamplers = 32,

    kMaxSupportedVertexLights = 8,
    kMaxSupportedTextureCoords = 8,

    kMaxSupportedRenderTargets = 8,
    kMaxSupportedConstantBuffers = 16,
    kMaxSupportedComputeResources = 32,
    kMaxSupportedMSAASamples = 32,
};

enum MonoOrStereoscopicEye
{
    // Keep left/right index in sync with StereoscopicEye
    kMonoOrStereoscopicEyeLeft,
    kMonoOrStereoscopicEyeRight,
    kMonoOrStereoscopicEyeMono,
    kMonoOrStereoscopicEyeCount
};

enum StereoscopicEye
{
    kStereoscopicEyeDefault = 0,
    kStereoscopicEyeLeft = 0,
    kStereoscopicEyeRight = 1,
    kStereoscopicEyeCount = 2,
};


enum SinglePassStereo
{
    kSinglePassStereoNone = 0,
    kSinglePassStereoSideBySide,
    kSinglePassStereoInstancing,
    kSinglePassStereoMultiview,
};

enum CubemapFace
{
    kCubeFaceUnknown = -1,
    kCubeFacePX = 0,
    kCubeFaceNX,
    kCubeFacePY,
    kCubeFaceNY,
    kCubeFacePZ,
    kCubeFaceNZ,
};

enum TargetEyeMask
{
    kTargetEyeMaskNone = 0,
    kTargetEyeMaskLeft = 1 << kStereoscopicEyeLeft,
    kTargetEyeMaskRight = 1 << kStereoscopicEyeRight,
    kTargetEyeMaskBoth = kTargetEyeMaskLeft | kTargetEyeMaskRight
};
ENUM_FLAGS(TargetEyeMask);

enum CullMode
{
    kCullUnknown = -1,
    kCullOff = 0,
    kCullFront,
    kCullBack,
    kCullCount
};

enum GfxClearFlags
{
    kGfxClearNone = 0,
    kGfxClearColor = (1 << 0),
    kGfxClearDepth = (1 << 1),
    kGfxClearStencil = (1 << 2),
    kGfxClearDepthStencil = kGfxClearDepth | kGfxClearStencil,
    kGfxClearAll = kGfxClearColor | kGfxClearDepth | kGfxClearStencil,
};
ENUM_FLAGS(GfxClearFlags);

#if GFX_USE_POINTER_IN_RESOURCE_IDS
// A platform stores pointers to underlying resource implementation directly; needs to be pointer size.
typedef intptr_t GfxResourceIDType;
#else
// A resource ID is just an integer into some lookup structure.
typedef UInt32 GfxResourceIDType;
#endif

enum SurfaceCreateFlags
{
    kSurfaceCreateFlagNone = 0,
    // unused (1<<0),
    kSurfaceCreateMipmap = (1 << 1),
    kSurfaceCreateSRGB = (1 << 2),
    kSurfaceCreateShadowmap = (1 << 3),
    kSurfaceCreateRandomWrite = (1 << 4),
    kSurfaceCreateSampleOnly = (1 << 5),
    kSurfaceCreateNeverUsed = (1 << 6),
    kSurfaceCreateAutoGenMips = (1 << 7),
    kSurfaceCreateDynamicScale = (1 << 8),
    kSurfaceCreateVRUsage = (1 << 9), // Mark the surface for VR
    kSurfaceCreateWithVRDevice = (1 << 10), // Have the VRDevice create this surface
    kSurfaceRenderTextureAsBackBuffer = (1 << 11), // pretends to be backbuffer, but is really a RenderTexture (e.g. game view). Need to flip projection upside down when rendering into it just like with RTs.
    kSurfaceCreateNoDepth = (1 << 12), // for depth surfaces: does not really need the depth buffer (e.g. depth bits are set to 0)
    kSurfaceCreateMemoryless = (1 << 13), // Create render texture without system memory
    kSurfaceCreateBindMS = (1 << 14), // Do not resolve if multisampled, bind as TextureMS/samplerMS
    kSurfaceCreateNotFlipped = (1 << 15),
    kSurfaceCreateStencilTexture = (1 << 16),
    kSurfaceCreatePlatformDefined0 = (1 << 30),
    kSurfaceCreatePlatformDefined1 = (1 << 31),
};
ENUM_FLAGS(SurfaceCreateFlags);

struct TextureID
{
    explicit TextureID() : m_ID(0) {}
    explicit TextureID(GfxResourceIDType i) : m_ID(i) {}
    bool IsValid() const { return m_ID != 0; }
    bool operator==(const TextureID& o) const { return m_ID == o.m_ID; }
    bool operator!=(const TextureID& o) const { return m_ID != o.m_ID; }
    bool operator<(const TextureID& o) const { return m_ID < o.m_ID; }
    GfxResourceIDType m_ID;
};

enum TextureDimension
{
    kTexDimUnknown = -1, // unknown
    kTexDimNone = 0, // no texture
    kTexDimAny, // special value that indicates "any" texture type can be used here; used very rarely in shader property metadata
    kTexDim2D, kTexDimFirst = kTexDim2D,
    kTexDim3D,
    kTexDimCUBE,
    kTexDim2DArray,
    kTexDimCubeArray,
    kTexDimLast = kTexDimCubeArray,

    kTexDimCount, // keep this last!
    kTexDimForce32Bit = 0x7fffffff
};


// "Required feature" for some particular shader. Features a bit flags.
// Shader writers can either use "target shader model" (e.g. "DX SM4.0" via "#pragma target 4.0")
// or a set of features they need (e.g. "MRT + texture arrays" via "#pragma require mrt 2darray").
//
// The "shader model" concept does not map cleanly across various platforms (e.g. what is shader model
// of a platform that has compute shaders, but no geometry shaders?), so the set of features
// approach is more flexible.
//
// NOTE: When adding new requirement flags, make sure to update relevant shader directive parsing & macro
// setup places: GetShaderRequirementsFromFeatureString(), AddShaderTargetRequirementsMacrosImpl().
// kShaderCompPlatformMinSpecFeatures and kShaderCompPlatformFeatures tables might need an update too.
// Keep in sync with C# ShaderRequirements
// When changing these try not to change the order/remove caps without an upgrade plan for the shaders themselves
// as we've had bugs loading older shaders when caps bits have been changed/removed causing shaders built on older
// versions to fail.
enum ShaderRequirements
{
    kShaderRequireNothing = 0,
    kShaderRequireBaseShaders = (1 << 0),       // Basic "can have shaders" (SM2.0 level) capability
    kShaderRequireInterpolators10 = (1 << 1),   // 10 interpolators/varyings
    kShaderRequireInterpolators32 = (1 << 2),   // 32 interpolators/varyings
    kShaderRequireMRT4 = (1 << 3),              // Multiple render targets, at least 4 (ability for fragment shader to output up to 4 colors)
    kShaderRequireMRT8 = (1 << 4),              // Multiple render targets, at least 8 (ability for fragment shader to output up to 8 colors)
    kShaderRequireDerivatives = (1 << 5),       // Derivative (ddx/ddy) instructions in the fragment shader
    kShaderRequireSampleLOD = (1 << 6),         // Ability to sample textures in fragment shader with explicit LOD level
    kShaderRequireFragCoord = (1 << 7),         // Pixel position (VPOS/SV_Position/gl_FragCoord) input in fragment shader
    kShaderRequireFragClipDepth = (1 << 8),     // Pixel depth (SV_Position.zw/gl_FragCoord.zw) input in fragment shader
    kShaderRequireInterpolators15Integers = (1 << 9), // Integers + Interpolators15. We bundle them together since extremely unlikely a GPU/API will ever exist that only has part of that.
    kShaderRequire2DArray = (1 << 10),          // 2DArray textures
    kShaderRequireInstancing = (1 << 11),       // SV_InstanceID shader input
    kShaderRequireGeometry = (1 << 12),         // Geometry shaders
    kShaderRequireCubeArray = (1 << 13),        // Cubemap arrays
    kShaderRequireCompute = (1 << 14),          // Compute shaders
    kShaderRequireRandomWrite = (1 << 15),      // Random-write textures (UAVs) from shader stages
    kShaderRequireTessHW = (1 << 16),           // Tessellator hardware, i.e. Metal style
    kShaderRequireTessellation = (1 << 17),     // Tessellation shaders, i.e. DX11 style (hull/domain shader stages)
    kShaderRequireSparseTex = (1 << 18),        // Sparse textures with sampling instructions that return residency info
    kShaderRequireFramebufferFetch = (1 << 19), // Framebuffer fetch (ability to have in+out fragment shader color params)
    kShaderRequireMSAATex = (1 << 20),          // Access to MSAA'd textures in shaders (e.g. HLSL Texture2DMS)
    kShaderRequireSetRTArrayIndexFromAnyShader = (1 << 21),          // Must support setting the render target array index from any shader and not just the geometry shader.

    // possible future flags once we get functionality exposed:
    // kShaderRequireResourceArray -- arrays of freely indexable resources (DX12/Vulkan/Metal-iOS style)

    // NOTE: if/once this gets to more than 32 bit, need to change ENUM_FLAGS typedef to use 64 bit underlying type somehow


    // Requirements grouped into rough "shader model" sets, that match the shader compilation
    // "#pragma target" syntax.

    kShaderRequireShaderModel20 = kShaderRequireBaseShaders,
    // Note that DX11 FL9.3 (which this is modeled at) also supports four render targets,
    // but we don't explicitly pull that in, since many people use "#pragma target 2.5" just to enable
    // longer shaders/derivatives, without explicitly needing MRT. And many GLES2.0 platforms do not support
    // MRTs. So yes this is a bit confusing, but oh well.
    kShaderRequireShaderModel25_93 = kShaderRequireShaderModel20 | kShaderRequireDerivatives,
    // Does not pull in MRT flag either, see above.
    kShaderRequireShaderModel30 = kShaderRequireShaderModel25_93 | kShaderRequireInterpolators10 | kShaderRequireSampleLOD | kShaderRequireFragCoord,

    kShaderRequireShaderModel35_ES3 = kShaderRequireShaderModel30 | kShaderRequireInterpolators15Integers | kShaderRequireMRT4 | kShaderRequire2DArray | kShaderRequireInstancing | kShaderRequireFragClipDepth,

    // Note: SM4.0 and up on DX11/PC does guarantee MRT8 and 32 varyings support; however on mobile GLES3.1/Metal/Vulkan
    // only guarantee MRT4 and 15 varyings. So let's not pull them in, to make life easier for people who just
    // write "#pragma target 5.0" in some shader (really only requiring compute or tessellation), and expect that to work
    // e.g. on Android GLES3.1+AEP (has both compute & tessellation, but only 4MRT).
    //
    // Similar for CubeArray requirement; don't make it be required in SM4.6/5.0.
    // Similar for MSAATex requirement; don't make it be required in SM4.0 (only starting with shader models that guarantee Compute).
    kShaderRequireShaderModel40 = kShaderRequireShaderModel35_ES3 | kShaderRequireGeometry,
    kShaderRequireShaderModel50 = kShaderRequireShaderModel40 | kShaderRequireCompute | kShaderRequireRandomWrite | kShaderRequireTessHW | kShaderRequireTessellation | kShaderRequireMSAATex,
    kShaderRequireShaderModel50_Metal = kShaderRequireShaderModel50 & ~kShaderRequireGeometry,

    kShaderRequireShaderModel40_PC = kShaderRequireShaderModel40 | kShaderRequireInterpolators32 | kShaderRequireMRT8,
    kShaderRequireShaderModel41_PC = kShaderRequireShaderModel40_PC | kShaderRequireCubeArray | kShaderRequireMSAATex,
    kShaderRequireShaderModel50_PC = kShaderRequireShaderModel41_PC | kShaderRequireShaderModel50,

    // "strange" shader model sets that aren't strictly increasing supersets of previous ones
    kShaderRequireShaderModel45_ES31 = kShaderRequireShaderModel35_ES3 | kShaderRequireCompute | kShaderRequireRandomWrite | kShaderRequireMSAATex, // "4.5": GLES3.1 / MobileMetal (or DX10 SM5 without geometry, tessellation, cube arrays)
    kShaderRequireShaderModel46_GL41 = kShaderRequireShaderModel40 | kShaderRequireTessHW | kShaderRequireTessellation | kShaderRequireMSAATex, // "4.6": DX10 SM4 + tessellation (but without compute)
};
ENUM_FLAGS(ShaderRequirements);

// keep up to date with kGfxHardwareTierTags in ShaderParserUtilities.cpp
// If we ever change this, please update ShaderWriter.cpp@IsValidTierForPlatform()
enum GraphicsTier
{
    kGraphicsTier1 = 0,
    kGraphicsTier2,
    kGraphicsTier3,

    kGraphicsTierCount, // keep this last!
    kGraphicsTierMax = kGraphicsTierCount - 1
};

enum VertexFormat
{
    kVertexFormatFloat,
    kVertexFormatFloat16,
    kVertexFormatUNorm8,
    kVertexFormatSNorm8,
    kVertexFormatUNorm16,
    kVertexFormatSNorm16,
    kVertexFormatUInt8,
    kVertexFormatSInt8,
    kVertexFormatUInt16,
    kVertexFormatSInt16,
    kVertexFormatUInt32,
    kVertexFormatSInt32,
    kVertexFormatFirstInteger = kVertexFormatUInt8,
    kVertexFormatLastInteger = kVertexFormatSInt32,
    kVertexFormatCount
};

enum HDRDisplaySupportFlags
{
    kHDRDisplaySupportFlagsNone = 0,
    kHDRDisplaySupportFlagsSupported = 1 << 0,
    kHDRDisplaySupportFlagsRuntimeSwitchable = 1 << 1,
    kHDRDisplaySupportFlagsAutomaticTonemapping = 1 << 2
};
ENUM_FLAGS(HDRDisplaySupportFlags)

// Match CopyTextureSupport on C# side
// Bitmask indicating support for various Graphics.CopyTexture cases
enum CopyTextureSupport
{
    kCopyTextureSupportNone = 0,
    kCopyTextureSupportBasic = (1 << 0), // basic functionality
    kCopyTextureSupport3D = (1 << 1), // can copy 3D textures
    kCopyTextureSupportDifferentTypes = (1 << 2), // can copy different types (e.g. cubemap face -> Texture2D)
    kCopyTextureSupportTextureToRT = (1 << 3), // can copy Texture -> RenderTexture
    kCopyTextureSupportRTToTexture = (1 << 4), // can copy RenderTexture -> Texture
};
ENUM_FLAGS(CopyTextureSupport);

// Buffer target binding bit flags (e.g. will it be used as vertex buffer, etc.).
// Some graphics APIs might support more than one target at once, or they don't care about it.
// See GfxDevice::CreateBuffer, GfxBuffer, GfxBufferDesc.
enum GfxBufferTarget
{
    kGfxBufferTargetNone                            = 0,
    kGfxBufferTargetVertex                          = (1 << 0),
    kGfxBufferTargetIndex                           = (1 << 1),
    kGfxBufferTargetCopySrc                         = (1 << 2),
    kGfxBufferTargetCopyDest                        = (1 << 3),
    kGfxBufferTargetStructured                      = (1 << 4),
    kGfxBufferTargetRaw                             = (1 << 5),
    kGfxBufferTargetAppend                          = (1 << 6),
    kGfxBufferTargetCounter                         = (1 << 7),
    kGfxBufferTargetIndirectArgs                    = (1 << 8),
    kGfxBufferTargetUniform                         = (1 << 9),
    kGfxBufferTargetConstant                        = kGfxBufferTargetUniform, // Alias for kGfxBufferTargetUniform for naming consistency
    kGfxBufferTargetRayTracingAccelerationStructure = (1 << 10),
    kGfxBufferTargetRayTracingShaderTable           = (1 << 11),
    kGfxBufferTargetComputeNeeded                   = kGfxBufferTargetStructured | kGfxBufferTargetRaw | kGfxBufferTargetAppend | kGfxBufferTargetCounter | kGfxBufferTargetIndirectArgs | kGfxBufferTargetRayTracingAccelerationStructure,
};
ENUM_FLAGS(GfxBufferTarget);

#if GFX_SUPPORTS_OPENGL_UNIFIED

// These enums have 2 usages: for context creation telling what kind of context and which version it should be,
// and for emulation levels to clamp features to match certain GL (/ES) version
// For normal (non-emulated) usage both values can be set to kGfxLevelMax(ES/Desktop)
enum GfxDeviceLevelGL
{
    kGfxLevelUninitialized = 0, // Initial value, should never be used. Marks that caps have not been initialized yet.
    kGfxLevelES2, kGfxLevelFirst = kGfxLevelES2, kGfxLevelESFirst = kGfxLevelES2, kGfxLevelES2First = kGfxLevelES2, kGfxLevelES2Last = kGfxLevelES2,
    kGfxLevelES3, kGfxLevelES3First = kGfxLevelES3,
    kGfxLevelES31,
    kGfxLevelES31AEP,
    kGfxLevelES32, kGfxLevelESLast = kGfxLevelES32, kGfxLevelES3Last = kGfxLevelES32,
    kGfxLevelCore32, kGfxLevelCoreFirst = kGfxLevelCore32,
    kGfxLevelCore33,
    kGfxLevelCore40,
    kGfxLevelCore41,
    kGfxLevelCore42,
    kGfxLevelCore43,
    kGfxLevelCore44,
    kGfxLevelCore45, kGfxLevelLast = kGfxLevelCore45, kGfxLevelCoreLast = kGfxLevelCore45
};

enum
{
    kGfxLevelCount = kGfxLevelLast - kGfxLevelFirst + 1
};

#endif//GFX_SUPPORTS_OPENGL_UNIFIED

enum
{
    kTexDimActiveCount = kTexDimLast - kTexDimFirst + 1
};

// Returns true if the renderer is using the unified GL backend
inline bool IsUnifiedGLRenderer(GfxDeviceRenderer renderer)
{
    return (renderer == kGfxRendererOpenGLES20) || (renderer == kGfxRendererOpenGLES3x) || (renderer == kGfxRendererOpenGLCore);
}

// on some platforms/graphics-api we need to keep track of vertex-component to shader-channel bindings
//   see SHADER_PROGRAM_NEEDS_VERTEX_INPUT_BINDINGS
// when writing shaders to AssetBundle we will save this information, using actual VertexComponent enum values
// hence if you change the order (e.g. add component in the middle) you need to make sure we can handle loading of old data
//   see comment in FillSubProgramVertexInputBindingInfo in Runtime/Shaders/SerializedShader.cpp

//@TODO: it might be that majority of these are not needed anymore, or at all
enum VertexComponent
{
    kVertexCompNone = -1,
    kVertexCompVertex,
    kVertexCompNormal,
    kVertexCompTangent,
    kVertexCompColor,
    kVertexCompTexCoord,
    kVertexCompTexCoord0, kVertexCompTexCoord1, kVertexCompTexCoord2, kVertexCompTexCoord3,
    kVertexCompTexCoord4, kVertexCompTexCoord5, kVertexCompTexCoord6, kVertexCompTexCoord7,
    kVertexCompAttrib0, kVertexCompAttrib1, kVertexCompAttrib2, kVertexCompAttrib3,
    kVertexCompAttrib4, kVertexCompAttrib5, kVertexCompAttrib6, kVertexCompAttrib7,
    kVertexCompAttrib8, kVertexCompAttrib9, kVertexCompAttrib10, kVertexCompAttrib11,
    kVertexCompAttrib12, kVertexCompAttrib13, kVertexCompAttrib14, kVertexCompAttrib15,
    kVertexCompBlendWeights,
    kVertexCompBlendIndices,
    kVertexCompCount // keep this last!
};

enum GfxRTLoadAction
{
    kGfxRTLoadActionLoad = 0,
    kGfxRTLoadActionClear = 1,
    kGfxRTLoadActionDontCare = 2,

    kGfxRTLoadActionCount
};
enum GfxRTStoreAction
{
    kGfxRTStoreActionStore = 0,
    kGfxRTStoreActionResolve = 1,
    kGfxRTStoreActionStoreAndResolve = 2,
    kGfxRTStoreActionDontCare = 3,

    kGfxRTStoreActionCount
};

// Deprecated
enum DepthBufferFormat
{
    kDepthFormatNone = 0,   // no depth buffer (d3d11 graphics caps depends on this being the first enum value)
    kDepthFormatMin16bits_NoStencil,    // at least 16 bits depth buffer, no stencil
    kDepthFormatMin24bits_Stencil,      // at least 24 bits depth buffer, with a stencil
    kDepthFormatCount       // keep this last!
};

// Convert a DepthBufferFormat into a GraphicsFormat enum. This function should always return a valid value.
GraphicsFormat GetGraphicsFormat(DepthBufferFormat format);

// Convert a GraphicsFormat enum into a DepthBufferFormat. Return kDepthFormatNone if there is no equivalent DepthBufferFormat format.
inline DepthBufferFormat GetDepthBufferFormat(GraphicsFormat format)
{
    if (format == kFormatD24_UNorm_S8_UInt || format == kFormatD32_SFloat_S8_Uint || format == kFormatDepthAuto)
        return kDepthFormatMin24bits_Stencil;
    else if (format == kFormatD16_UNorm || format == kFormatShadowAuto)
        return kDepthFormatMin16bits_NoStencil;
    else
        return kDepthFormatNone;
}

// Match ShadowSamplingMode on C# side
enum ShadowSamplingMode
{
    kShadowSamplingCompareDepths = 0, // compare the depth to a reference value
    kShadowSamplingRawDepth = 1, // override the comparison sampler with a regular sampler
    kShadowSamplingNone = 2 // for non-shadowmaps, shadow sampling mode n/a; not mirrored in c#
};

enum RenderTextureMemoryless
{
    kMemorylessNone = 0,
    kMemorylessColor = (1 << 0),
    kMemorylessDepth = (1 << 1),
    kMemorylessMSAA = (1 << 2),
};
ENUM_FLAGS(RenderTextureMemoryless);

// Deprecated
enum RenderTextureFormat
{
    kRTFormatARGB32 = 0, kRTFormatFirst = kRTFormatARGB32, // ARGB, 8 bit/channel
    kRTFormatDepth,         // whatever is for "depth texture"; on most platforms this ends up being "depth buffer format"
    kRTFormatARGBHalf,      // ARGB, 16 bit floating point/channel
    kRTFormatShadowMap,     // whatever is "native" (with built-in comparisons) shadow map format
    kRTFormatRGB565,
    kRTFormatARGB4444,
    kRTFormatARGB1555,
    kRTFormatDefault,
    kRTFormatA2R10G10B10,
    kRTFormatDefaultHDR,
    kRTFormatARGB64,
    kRTFormatARGBFloat,
    kRTFormatRGFloat,
    kRTFormatRGHalf,
    kRTFormatRFloat,
    kRTFormatRHalf,
    kRTFormatR8,
    kRTFormatARGBInt,
    kRTFormatRGInt,
    kRTFormatRInt,
    kRTFormatBGRA32,
    kRTFormatVideo,         // whatever is "native" for playing streaming video
    kRTFormatR11G11B10Float,
    kRTFormatRG32,
    kRTFormatRGBAUShort,    // RGBA 16 bits unsigned integer
    kRTFormatRG16,          // RG, 8 bit/channel
    kRTFormatBGRA10_XR,
    kRTFormatBGR10_XR,
    kRTFormatR16,
    kRTFormatCount          // keep this last!
};

// Generally, the order of channels in uncompressed format in the name is like they are in memory (e.g. RGBA32 is R,G,B,A bytes in memory).
// A major exception is for 16 bits formats where no convention is followed so each format requires individual comment inspection
// Deprecated
enum TextureFormat
{
    kTexFormatUnknown = -1,
    kTexFormatNone = 0,

    kTexFormatAlpha8 = 1,           // In memory: A8U
    kTexFormatARGB4444 = 2,         // In memory: A4U,R4U,G4U,B4U; 0xBGRA if viewed as 16 bit word on little-endian, equivalent to VK_FORMAT_R4G4B4A4_UNORM_PACK16, Pixel layout depends on endianness, A4;R4;G4;B4 going from high to low bits.
    kTexFormatRGB24 = 3,            // In memory: R8U,G8U,B8U
    kTexFormatRGBA32 = 4,           // In memory: R8U,G8U,B8U,A8U; 0xAABBGGRR if viewed as 32 bit word on little-endian. Generally preferred for 32 bit uncompressed data.
    kTexFormatARGB32 = 5,           // In memory: A8U,R8U,G8U,B8U; 0xBBGGRRAA if viewed as 32 bit word on little-endian
    kTexFormatARGBFloat = 6,        // only for internal use at runtime
    kTexFormatRGB565 = 7,           // In memory: R5U,G6U,B5U; 0xBGR if viewed as 16 bit word on little-endian, equivalent to VK_FORMAT_R5G6B5_UNORM_PACK16, Pixel layout depends on endianness, R5;G6;B5 going from high to low bits
    kTexFormatBGR24 = 8,            // In memory: B8U,G8U,R8U
    kTexFormatR16 = 9,              // In memory: R16U

    // DXT/S3TC compression
    kTexFormatDXT1 = 10,            // aka BC1
    kTexFormatDXT3 = 11,            // aka BC2
    kTexFormatDXT5 = 12,            // aka BC3

    kTexFormatRGBA4444 = 13,        // In memory: A4U,R4U,G4U,B4U; 0xARGB if viewed as 16 bit word on little-endian, Pixel layout depends on endianness, R4;G4;B4;A4 going from high to low bits

    kTexFormatBGRA32    = 14,       // In memory: B8U,G8U,R8U,A8U; 0xAARRGGBB if viewed as 32 bit word on little-endian. Used by some WebCam implementations.

    // float/half texture formats
    kTexFormatRHalf = 15,           // In memory: R16F
    kTexFormatRGHalf = 16,          // In memory: R16F,G16F
    kTexFormatRGBAHalf = 17,        // In memory: R16F,G16F,B16F,A16F
    kTexFormatRFloat = 18,          // In memory: R32F
    kTexFormatRGFloat = 19,         // In memory: R32F,G32F
    kTexFormatRGBAFloat = 20,       // In memory: R32F,G32F,B32F,A32F

    kTexFormatYUY2 = 21,            // YUV format, can be used for video streams.

    // Three partial-precision floating-point numbers encoded into a single 32-bit value all sharing the same
    // 5-bit exponent (variant of s10e5, which is sign bit, 10-bit mantissa, and 5-bit biased(15) exponent).
    // There is no sign bit, and there is a shared 5-bit biased(15) exponent and a 9-bit mantissa for each channel.
    kTexFormatRGB9e5Float = 22,

    kTexFormatRGBFloat  = 23,       // Editor only format (used for saving HDR)

    // DX10/DX11 (aka BPTC/RGTC) compressed formats
    kTexFormatBC6H  = 24,           // RGB HDR compressed format, unsigned.
    kTexFormatBC7   = 25,           // HQ RGB(A) compressed format.
    kTexFormatBC4   = 26,           // One-component compressed format, 0..1 range.
    kTexFormatBC5   = 27,           // Two-component compressed format, 0..1 range.

    // Crunch compression
    kTexFormatDXT1Crunched = 28,    // DXT1 Crunched
    kTexFormatDXT5Crunched = 29,    // DXT5 Crunched

    // PowerVR / iOS PVRTC compression
    kTexFormatPVRTC_RGB2 = 30,
    kTexFormatPVRTC_RGBA2 = 31,
    kTexFormatPVRTC_RGB4 = 32,
    kTexFormatPVRTC_RGBA4 = 33,

    // OpenGL ES 2.0 ETC
    kTexFormatETC_RGB4 = 34,

    // EAC and ETC2 compressed formats, in OpenGL ES 3.0
    kTexFormatEAC_R = 41,
    kTexFormatEAC_R_SIGNED = 42,
    kTexFormatEAC_RG = 43,
    kTexFormatEAC_RG_SIGNED = 44,
    kTexFormatETC2_RGB = 45,
    kTexFormatETC2_RGBA1 = 46,
    kTexFormatETC2_RGBA8 = 47,

    // ASTC. The RGB and RGBA formats are internally identical.
    // before we had kTexFormatASTC_RGB_NxN and kTexFormatASTC_RGBA_NxN, thats why we have hole here
    kTexFormatASTC_4x4 = 48,
    kTexFormatASTC_5x5 = 49,
    kTexFormatASTC_6x6 = 50,
    kTexFormatASTC_8x8 = 51,
    kTexFormatASTC_10x10 = 52,
    kTexFormatASTC_12x12 = 53,
    // [54..59] were taken by kTexFormatASTC_RGBA_NxN

    // Nintendo 3DS
    kTexFormatETC_RGB4_3DS = 60,
    kTexFormatETC_RGBA8_3DS = 61,

    kTexFormatRG16 = 62,
    kTexFormatR8 = 63,

    // Crunch compression for ETC format
    kTexFormatETC_RGB4Crunched = 64,
    kTexFormatETC2_RGBA8Crunched = 65,

    kTexFormatASTC_HDR_4x4 = 66,
    kTexFormatASTC_HDR_5x5 = 67,
    kTexFormatASTC_HDR_6x6 = 68,
    kTexFormatASTC_HDR_8x8 = 69,
    kTexFormatASTC_HDR_10x10 = 70,
    kTexFormatASTC_HDR_12x12 = 71,

    // 16-bit raw integer formats
    kTexFormatRG32 = 72,
    kTexFormatRGB48 = 73,
    kTexFormatRGBA64 = 74,

    kTexFormatTotalCount,
};

enum
{
    kPrimitiveTypeCount = kPrimitiveTypeLast - kPrimitiveTypeFirst + 1
};

enum CompareFunction
{
    kFuncUnknown = -1,
    kFuncDisabled = 0, kFuncFirst = kFuncDisabled,
    kFuncNever,
    kFuncLess,
    kFuncEqual,
    kFuncLEqual,
    kFuncGreater,
    kFuncNotEqual,
    kFuncGEqual,
    kFuncAlways,
    kFuncCount
};

enum StencilOp
{
    kStencilOpKeep = 0, kStencilOpFirst = kStencilOpKeep,
    kStencilOpZero,
    kStencilOpReplace,
    kStencilOpIncrSat,
    kStencilOpDecrSat,
    kStencilOpInvert,
    kStencilOpIncrWrap,
    kStencilOpDecrWrap,
    kStencilOpCount
};

enum ColorWriteMask
{
    kColorWriteA = 1,
    kColorWriteB = 2,
    kColorWriteG = 4,
    kColorWriteR = 8,
    kColorWriteAll = (kColorWriteR | kColorWriteG | kColorWriteB | kColorWriteA)
};

enum BlendMode
{
    kBlendZero = 0, kBlendFirst = kBlendZero,
    kBlendOne,
    kBlendDstColor,
    kBlendSrcColor,
    kBlendOneMinusDstColor,
    kBlendSrcAlpha,
    kBlendOneMinusSrcColor,
    kBlendDstAlpha,
    kBlendOneMinusDstAlpha,
    kBlendSrcAlphaSaturate,
    kBlendOneMinusSrcAlpha,
    kBlendCount
};

enum BlendOp
{
    kBlendOpFirst = 0,
    kBlendOpAdd = kBlendOpFirst,
    kBlendOpSub,
    kBlendOpRevSub,
    kBlendOpMin,
    kBlendOpMax,
    kBlendOpLogicalClear, kBlendOpLogicalFirst = kBlendOpLogicalClear,
    kBlendOpLogicalSet,
    kBlendOpLogicalCopy,
    kBlendOpLogicalCopyInverted,
    kBlendOpLogicalNoop,
    kBlendOpLogicalInvert,
    kBlendOpLogicalAnd,
    kBlendOpLogicalNand,
    kBlendOpLogicalOr,
    kBlendOpLogicalNor,
    kBlendOpLogicalXor,
    kBlendOpLogicalEquiv,
    kBlendOpLogicalAndReverse,
    kBlendOpLogicalAndInverted,
    kBlendOpLogicalOrReverse,
    kBlendOpLogicalOrInverted, kBlendOpLogicalLast = kBlendOpLogicalOrInverted,
    kBlendOpMultiply, kBlendOpAdvancedFirst = kBlendOpMultiply,
    kBlendOpScreen,
    kBlendOpOverlay,
    kBlendOpDarken,
    kBlendOpLighten,
    kBlendOpColorDodge,
    kBlendOpColorBurn,
    kBlendOpHardLight,
    kBlendOpSoftLight,
    kBlendOpDifference,
    kBlendOpExclusion,
    kBlendOpHSLHue,
    kBlendOpHSLSaturation,
    kBlendOpHSLColor,
    kBlendOpHSLLuminosity, kBlendOpAdvancedLast = kBlendOpHSLLuminosity,
    kBlendOpCount,
};

enum TextureFilterMode
{
    kTexFilterInvalid = -1,
    kTexFilterNearest = 0,
    kTexFilterBilinear,
    kTexFilterTrilinear,
    kTexFilterCount // keep this last!
};

enum TextureWrapMode
{
    kTexWrapInvalid = -1,
    kTexWrapRepeat = 0,
    kTexWrapClamp,
    kTexWrapMirror,
    kTexWrapMirrorOnce,
    kTexWrapCount // keep this last!
};

struct FormatDesc
{
    UInt8 blockSize;
    UInt8 blockX;
    UInt8 blockY;
    UInt8 blockZ;
    FormatSwizzle swizzleR;
    FormatSwizzle swizzleG;
    FormatSwizzle swizzleB;
    FormatSwizzle swizzleA;
    GraphicsFormat fallbackFormat; // If this format it not supported by the platform, this is the alternative format. Eg if kFormatRGB_ETC_UNorm is not suppored use kFormatR8G8B8_UNorm
    GraphicsFormat alphaFormat; // Equivalent GraphicsFormat but with an alpha channel
    GraphicsFormat linearFormat; // Equivalent GraphicsFormat but as linear format
    GraphicsFormat srgbFormat; // Equivalent GraphicsFormat but as sRGB encode and decode if available
    TextureFormat textureFormat; // Equivalent TextureFormat
    RenderTextureFormat renderTextureFormat; // Equivalent RenderTextureFormat
    UInt8 colorComponents;
    UInt8 alphaComponents;
    const char* name;
    int flags; // a combination of GraphicsFormatFlag
};

// Convert a GraphicsFormat enum into a TextureFormat. Return kTexFormatNone if there is no equivalent TextureFormat format.
TextureFormat GetTextureFormat(GraphicsFormat format);

// Convert a GraphicsFormat enum into a TextureFormat. Return kRTFormatCount if there is no equivalent RenderTextureFormat format.
RenderTextureFormat GetRenderTextureFormat(GraphicsFormat format);
#endif