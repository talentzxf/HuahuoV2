//
// Created by VincentZhang on 5/24/2022.
//

#ifndef HUAHUOENGINE_FORMAT_H
#define HUAHUOENGINE_FORMAT_H

#include "Utilities/EnumFlags.h"

// Each platform may use different graphics format for specific usages that are represented by DefaultFormat
enum DefaultFormat
{
    kDefaultFormatLDR, kDefaultFormatFirst = kDefaultFormatLDR,
    kDefaultFormatHDR,
    kDefaultFormatDepth,
    kDefaultFormatShadow,
    kDefaultFormatVideo, kDefaultFormatLast = kDefaultFormatVideo
};

// Keep in sync with GraphicsFormat in Runtime/Export/GraphicsEnums.cs
enum GraphicsFormat
{
    kFormatNone = 0, kFormatFirst = kFormatNone,

    // sRGB formats
    kFormatR8_SRGB,
    kFormatR8G8_SRGB,
    kFormatR8G8B8_SRGB,
    kFormatR8G8B8A8_SRGB,

    // 8 bit integer formats
    kFormatR8_UNorm,
    kFormatR8G8_UNorm,
    kFormatR8G8B8_UNorm,
    kFormatR8G8B8A8_UNorm,
    kFormatR8_SNorm,
    kFormatR8G8_SNorm,
    kFormatR8G8B8_SNorm,
    kFormatR8G8B8A8_SNorm,
    kFormatR8_UInt,
    kFormatR8G8_UInt,
    kFormatR8G8B8_UInt,
    kFormatR8G8B8A8_UInt,
    kFormatR8_SInt,
    kFormatR8G8_SInt,
    kFormatR8G8B8_SInt,
    kFormatR8G8B8A8_SInt,

    // 16 bit integer formats
    kFormatR16_UNorm,
    kFormatR16G16_UNorm,
    kFormatR16G16B16_UNorm,
    kFormatR16G16B16A16_UNorm,
    kFormatR16_SNorm,
    kFormatR16G16_SNorm,
    kFormatR16G16B16_SNorm,
    kFormatR16G16B16A16_SNorm,
    kFormatR16_UInt,
    kFormatR16G16_UInt,
    kFormatR16G16B16_UInt,
    kFormatR16G16B16A16_UInt,
    kFormatR16_SInt,
    kFormatR16G16_SInt,
    kFormatR16G16B16_SInt,
    kFormatR16G16B16A16_SInt,

    // 32 bit integer formats
    kFormatR32_UInt,
    kFormatR32G32_UInt,
    kFormatR32G32B32_UInt,
    kFormatR32G32B32A32_UInt,
    kFormatR32_SInt,
    kFormatR32G32_SInt,
    kFormatR32G32B32_SInt,
    kFormatR32G32B32A32_SInt,

    // HDR formats
    kFormatR16_SFloat,
    kFormatR16G16_SFloat,
    kFormatR16G16B16_SFloat,
    kFormatR16G16B16A16_SFloat,
    kFormatR32_SFloat,
    kFormatR32G32_SFloat,
    kFormatR32G32B32_SFloat,
    kFormatR32G32B32A32_SFloat,

    // Luminance and Alpha format
    kFormatL8_UNorm,
    kFormatA8_UNorm,
    kFormatA16_UNorm,

    // BGR formats
    kFormatB8G8R8_SRGB,
    kFormatB8G8R8A8_SRGB,
    kFormatB8G8R8_UNorm,
    kFormatB8G8R8A8_UNorm,
    kFormatB8G8R8_SNorm,
    kFormatB8G8R8A8_SNorm,
    kFormatB8G8R8_UInt,
    kFormatB8G8R8A8_UInt,
    kFormatB8G8R8_SInt,
    kFormatB8G8R8A8_SInt,

    // 16 bit packed formats
    kFormatR4G4B4A4_UNormPack16,
    kFormatB4G4R4A4_UNormPack16,
    kFormatR5G6B5_UNormPack16,
    kFormatB5G6R5_UNormPack16,
    kFormatR5G5B5A1_UNormPack16,
    kFormatB5G5R5A1_UNormPack16,
    kFormatA1R5G5B5_UNormPack16,

    // Packed formats
    kFormatE5B9G9R9_UFloatPack32,
    kFormatB10G11R11_UFloatPack32,

    kFormatA2B10G10R10_UNormPack32,
    kFormatA2B10G10R10_UIntPack32,
    kFormatA2B10G10R10_SIntPack32,
    kFormatA2R10G10B10_UNormPack32,
    kFormatA2R10G10B10_UIntPack32,
    kFormatA2R10G10B10_SIntPack32,
    kFormatA2R10G10B10_XRSRGBPack32,
    kFormatA2R10G10B10_XRUNormPack32,
    kFormatR10G10B10_XRSRGBPack32,
    kFormatR10G10B10_XRUNormPack32,
    kFormatA10R10G10B10_XRSRGBPack32,
    kFormatA10R10G10B10_XRUNormPack32,

    // ARGB formats... TextureFormat legacy
    kFormatA8R8G8B8_SRGB,
    kFormatA8R8G8B8_UNorm,
    kFormatA32R32G32B32_SFloat,

    // Depth Stencil for formats
    kFormatD16_UNorm,
    kFormatD24_UNorm,
    kFormatD24_UNorm_S8_UInt,
    kFormatD32_SFloat,
    kFormatD32_SFloat_S8_Uint,
    kFormatS8_Uint,

    // Compression formats
    kFormatRGBA_DXT1_SRGB, kFormatDXTCFirst = kFormatRGBA_DXT1_SRGB,
    kFormatRGBA_DXT1_UNorm,
    kFormatRGBA_DXT3_SRGB,
    kFormatRGBA_DXT3_UNorm,
    kFormatRGBA_DXT5_SRGB,
    kFormatRGBA_DXT5_UNorm, kFormatDXTCLast = kFormatRGBA_DXT5_UNorm,
    kFormatR_BC4_UNorm, kFormatRGTCFirst = kFormatR_BC4_UNorm,
    kFormatR_BC4_SNorm,
    kFormatRG_BC5_UNorm,
    kFormatRG_BC5_SNorm, kFormatRGTCLast = kFormatRG_BC5_SNorm,
    kFormatRGB_BC6H_UFloat, kFormatBPTCFirst = kFormatRGB_BC6H_UFloat,
    kFormatRGB_BC6H_SFloat,
    kFormatRGBA_BC7_SRGB,
    kFormatRGBA_BC7_UNorm, kFormatBPTCLast = kFormatRGBA_BC7_UNorm,

    kFormatRGB_PVRTC_2Bpp_SRGB, kFormatPVRTCFirst = kFormatRGB_PVRTC_2Bpp_SRGB,
    kFormatRGB_PVRTC_2Bpp_UNorm,
    kFormatRGB_PVRTC_4Bpp_SRGB,
    kFormatRGB_PVRTC_4Bpp_UNorm,
    kFormatRGBA_PVRTC_2Bpp_SRGB,
    kFormatRGBA_PVRTC_2Bpp_UNorm,
    kFormatRGBA_PVRTC_4Bpp_SRGB,
    kFormatRGBA_PVRTC_4Bpp_UNorm, kFormatPVRTCLast = kFormatRGBA_PVRTC_4Bpp_UNorm,

    kFormatRGB_ETC_UNorm, kFormatETCFirst = kFormatRGB_ETC_UNorm,
    kFormatRGB_ETC2_SRGB,
    kFormatRGB_ETC2_UNorm,
    kFormatRGB_A1_ETC2_SRGB,
    kFormatRGB_A1_ETC2_UNorm,
    kFormatRGBA_ETC2_SRGB,
    kFormatRGBA_ETC2_UNorm, kFormatETCLast = kFormatRGBA_ETC2_UNorm,

    kFormatR_EAC_UNorm, kFormatEACFirst = kFormatR_EAC_UNorm,
    kFormatR_EAC_SNorm,
    kFormatRG_EAC_UNorm,
    kFormatRG_EAC_SNorm, kFormatEACLast = kFormatRG_EAC_SNorm,

    kFormatRGBA_ASTC4X4_SRGB, kFormatASTCFirst = kFormatRGBA_ASTC4X4_SRGB,
    kFormatRGBA_ASTC4X4_UNorm,
    kFormatRGBA_ASTC5X5_SRGB,
    kFormatRGBA_ASTC5X5_UNorm,
    kFormatRGBA_ASTC6X6_SRGB,
    kFormatRGBA_ASTC6X6_UNorm,
    kFormatRGBA_ASTC8X8_SRGB,
    kFormatRGBA_ASTC8X8_UNorm,
    kFormatRGBA_ASTC10X10_SRGB,
    kFormatRGBA_ASTC10X10_UNorm,
    kFormatRGBA_ASTC12X12_SRGB,
    kFormatRGBA_ASTC12X12_UNorm, kFormatASTCLast = kFormatRGBA_ASTC12X12_UNorm,

    // Video formats
    kFormatYUV2,

    // Automatic formats, back-end decides
    kFormatDepthAuto,
    kFormatShadowAuto,
    kFormatVideoAuto,

    kFormatRGBA_ASTC4X4_UFloat, kFormatASTCHDRFirst = kFormatRGBA_ASTC4X4_UFloat,
    kFormatRGBA_ASTC5X5_UFloat,
    kFormatRGBA_ASTC6X6_UFloat,
    kFormatRGBA_ASTC8X8_UFloat,
    kFormatRGBA_ASTC10X10_UFloat,
    kFormatRGBA_ASTC12X12_UFloat, kFormatASTCHDRLast = kFormatRGBA_ASTC12X12_UFloat,

    kFormatLast = kFormatASTCHDRLast, // Remove?
};

enum
{
    kGraphicsFormatCount = kFormatLast - kFormatFirst + 1,
};

// Keep in sync with FormatUsage in Runtime/Export/Graphics/GraphicsEnums.cs
enum FormatUsage
{
    kUsageSample, kUsageFirst = kUsageSample,   // To sample textures.
    kUsageLinear,                               // To sample textures with a linear filter
    kUsageSparse,                               // To create sparse texture
    kUsageVertex,                               // Can be used as vertex attribute format
    kUsageRender,                               // To create and render to a rendertexture
    kUsageBlend,                                // To blend on a rendertexture.
    kUsageGetPixels,                            // To use GetPixel* C# APIs
    kUsageSetPixels,                            // To use SetPixel* C# APIs
    kUsageSetPixels32,                          // RGBA 32 bits format dedicated SetPixels32
    kUsageReadPixels,                           // Can be used as for texels readback
    kUsageLoadStore,                            // To perform resource load and store on a texture
    kUsageMSAA2x,                               // To create and render to a MSAA 2X rendertexture
    kUsageMSAA4x,                               // To create and render to a MSAA 4X rendertexture
    kUsageMSAA8x,                               // To create and render to a MSAA 8X rendertexture
    kUsageMSAA16x,                              // To create and render to a MSAA 16X rendertexture
    kUsageMSAA32x,                              // To create and render to a MSAA 32X rendertexture
    kUsageStencilSampling,                      // To create and render to the stencil element of a rendertexture.
    kUsageLast = kUsageStencilSampling
};

enum FormatUsageFlags
{
    kUsageNoneBit               = 0,
    kUsageSampleBit             = 1 << kUsageSample,
    kUsageLinearBit             = 1 << kUsageLinear,
    kUsageSparseBit             = 1 << kUsageSparse,
    kUsageVertexBit             = 1 << kUsageVertex,
    kUsageRenderBit             = 1 << kUsageRender,
    kUsageBlendBit              = 1 << kUsageBlend,
    kUsageGetPixelsBit          = 1 << kUsageGetPixels,
    kUsageSetPixelsBit          = 1 << kUsageSetPixels,
    kUsageSetPixels32Bit        = 1 << kUsageSetPixels32,
    kUsageReadPixelsBit         = 1 << kUsageReadPixels,
    kUsageLoadStoreBit          = 1 << kUsageLoadStore,
    kUsageMSAA2Bit              = 1 << kUsageMSAA2x,
    kUsageMSAA4Bit              = 1 << kUsageMSAA4x,
    kUsageMSAA8Bit              = 1 << kUsageMSAA8x,
    kUsageMSAA16Bit             = 1 << kUsageMSAA16x,
    kUsageMSAA32Bit             = 1 << kUsageMSAA32x,
    kUsageStencilSamplingBit    = 1 << kUsageStencilSampling,
    kFormatUsageMSAABits        = kUsageMSAA2Bit | kUsageMSAA4Bit | kUsageMSAA8Bit | kUsageMSAA16Bit | kUsageMSAA32Bit,
    kFormatUsageAllBits         = ~0
};
ENUM_FLAGS(FormatUsageFlags);

enum FormatPropertyFlags
{
    kFormatPropertyCompressedBit = (1 << 0),
    kFormatPropertyPackedBit = (1 << 1),
    kFormatPropertySRGBBit = (1 << 2),
    kFormatPropertyNormBit = (1 << 3),
    kFormatPropertyUnsignedBit = (1 << 4),
    kFormatPropertySignedBit = (1 << 5),
    kFormatPropertyIntegerBit = (1 << 6),
    kFormatPropertyIEEE754Bit = (1 << 7),
    kFormatPropertyDepthBit = (1 << 8),
    kFormatPropertyStencilBit = (1 << 9),
    kFormatPropertyBlockSizeIsMinTextureMipSizeBit = (1 << 10),         // Format which last mipmap size must be the block size. Mips smaller are not stored. This is NOT used to flag that a mip smaller than the block size is stored in one block.
    kFormatPropertyNoNeedMipMapsBit = (1 << 11),                        // There is no need of mipmaps support. Eg:kFormatYUV2
    kFormatPropertyAlphaTestBit = (1 << 12),                            // The format is has a 1 or 2 bits alpha channel
    kFormatPropertyXRBit = (1 << 13),
};
ENUM_FLAGS(FormatPropertyFlags);

enum FormatSwizzle
{
    kFormatSwizzleR, kFormatSwizzleChannelFirst = kFormatSwizzleR,
    kFormatSwizzleG,
    kFormatSwizzleB,
    kFormatSwizzleA, kFormatSwizzleChannelLast = kFormatSwizzleA,
    kFormatSwizzle0,
    kFormatSwizzle1, kFormatSwizzleMax = kFormatSwizzle1
};

struct FormatDesc;
const FormatDesc& GetDesc(GraphicsFormat format);

// Return a fully descriptive string for a specific GraphicsFormat
std::string GetFormatString(GraphicsFormat format);
UInt32 GetColorComponentCount(GraphicsFormat format);

UInt32 GetAlphaComponentCount(GraphicsFormat format);
// The format includes an alpha channel. Eg kFormatR8G8B8A8_SRGB
bool HasAlphaChannel(GraphicsFormat format);

bool IsCompressedFormat(GraphicsFormat format);

bool IsAlphaOnlyFormat(GraphicsFormat format);

// A format has a 1 or 2 bits alpha channel, enought for alpha testing
bool IsAlphaTestFormat(GraphicsFormat format);

// A format with a depth channel
bool IsDepthFormat(GraphicsFormat format);

// A format with a stencil channel
bool IsStencilFormat(GraphicsFormat format);

// A depth format with stencil channel
bool IsShadowFormat(GraphicsFormat format);

// Half, float, packed float
bool IsIEEE754Format(GraphicsFormat format);

bool IsFloatFormat(GraphicsFormat format);

bool IsHalfFormat(GraphicsFormat format);

bool IsUnsignedFormat(GraphicsFormat format);

bool IsSignedFormat(GraphicsFormat format);

bool IsNormFormat(GraphicsFormat format);

bool IsUNormFormat(GraphicsFormat format);

bool IsSNormFormat(GraphicsFormat format);

bool IsIntegerFormat(GraphicsFormat format);

bool IsUIntFormat(GraphicsFormat format);

bool IsSIntFormat(GraphicsFormat format);

bool IsXRFormat(GraphicsFormat format);

bool IsSRGBFormat(GraphicsFormat format);

UInt32 GetBlockSize(GraphicsFormat format);
#endif //HUAHUOENGINE_FORMAT_H
