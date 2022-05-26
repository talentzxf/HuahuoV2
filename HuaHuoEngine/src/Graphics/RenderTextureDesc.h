//
// Created by VincentZhang on 5/26/2022.
//

#ifndef HUAHUOENGINE_RENDERTEXTUREDESC_H
#define HUAHUOENGINE_RENDERTEXTUREDESC_H

#include "Format.h"
#include "GfxDevice/GfxDeviceTypes.h"
#include "Utilities/Utility.h"

enum
{
    kScreenOffscreen = -1,
};

// Match RenderTextureCreationFlags on C# side
enum RenderTextureFlags
{
    kRTFlagNone = 0,
    kRTFlagMipMap = 1 << 0, //todo remove this and use base class mipmap value
    kRTFlagAutoGenerateMips = 1 << 1,
    kRTFlagSRGB = 1 << 2,
    kRTFlagEyeTexture = 1 << 3,
    kRTFlagRandomWrite = 1 << 4,
    kRTFlagCreatedFromScript = 1 << 5,
    kRTFlagSampleOnlyDepth = 1 << 6,
    kRTFlagAllowVerticalFlip = 1 << 7,
    kRTFlagNoResolvedColorSurface = 1 << 8,
    kRTFlagSampleMSDepth = 1 << 9,  /* when set we can configure the pipeline to allow reads frm the depth surface, even though it's MSAA (critical for checkerboard rendering) */
    kRTFlagDynamicallyScalable = 1 << 10,
    kRTFlagBindMS = 1 << 11, // The texture should be bound as MSAA texture
    kRTFlagsAssignTextureForDepth = 1 << 12,
    kRTFlagDisableCompatibleFormat = 1 << 13, // Unity will not automatically select a compatible format if the requested format is not supported.
};

ENUM_FLAGS(RenderTextureFlags);

struct RenderTextureDesc
{
    // Layout must match RenderTextureDescriptor on C# side.
    // It needs to be tightly packed for memory comparison and hashing to work.
    int width; ///< range {1, 20000}
    int height; ///< range {1, 20000}
    int antiAliasing; ///< enum { None = 1, 2xMSAA = 2, 4xMSAA = 4, 8xMSAA = 8 } Anti-aliasing
    int volumeDepth; ///< range {1, 20000}
    int mipCount; ///< range {-1, 16}
    GraphicsFormat colorFormat; /// enum { ARGB32 = 0, Depth = 1, ARGBHalf = 2, Shadowmap = 3, RGB565 = 4, ARGB4444 = 5, ARGB1555 = 6, ARGB2101010 = 8, ARGBFloat = 11, RGFloat = 12, RGHalf = 13, RFloat = 14, RHalf = 15, R8 = 16, ARGBInt = 17, RGInt = 18, RInt = 19 } Color buffer format
    GraphicsFormat stencilFormat; /// The desired format for reading the stencil as a texture if any. Available options for each platform defined in graphics caps with None and R8_UInt being most common.
    DepthBufferFormat depthFormat;  ///< enum { No depth buffer = 0, At least 16 bits depth (no stencil), At least 24 bits depth (with stencil) = 2 } Depth buffer format
    TextureDimension dimension;
    ShadowSamplingMode shadowSamplingMode;
//    VRTextureUsage vrUsage;
    RenderTextureFlags flags;
    RenderTextureMemoryless memoryless;


    RenderTextureDesc()
            : width(256)
            , height(256)
            , antiAliasing(1)
            , volumeDepth(1)
            , mipCount(-1)
            , colorFormat(kFormatR8G8B8A8_UNorm)
            , depthFormat(kDepthFormatMin24bits_Stencil)   // note this does not mirror the default in C# which is no depth buffer.
            , dimension(kTexDim2D)
            , shadowSamplingMode(kShadowSamplingNone)
            // , vrUsage(kVRTextureUsageNone)
            , memoryless(kMemorylessNone)
            , stencilFormat(kFormatNone)
    {
        flags = kRTFlagAutoGenerateMips | kRTFlagAllowVerticalFlip;
    }

    bool operator==(const RenderTextureDesc& other) const { return MemoryEquals(*this, other); }
    bool operator!=(const RenderTextureDesc& other) const { return !operator==(other); }
};
#endif //HUAHUOENGINE_RENDERTEXTUREDESC_H
