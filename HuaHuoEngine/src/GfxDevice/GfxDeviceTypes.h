//
// Created by VincentZhang on 5/13/2022.
//

#ifndef HUAHUOENGINE_GFXDEVICETYPES_H
#define HUAHUOENGINE_GFXDEVICETYPES_H

#include "Utilities/EnumFlags.h"

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
#endif