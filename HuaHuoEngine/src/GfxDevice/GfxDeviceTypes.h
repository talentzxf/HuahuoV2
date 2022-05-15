//
// Created by VincentZhang on 5/13/2022.
//

#ifndef HUAHUOENGINE_GFXDEVICETYPES_H
#define HUAHUOENGINE_GFXDEVICETYPES_H

#include "Utilities/EnumFlags.h"

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
#endif