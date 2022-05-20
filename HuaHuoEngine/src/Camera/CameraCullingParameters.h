//
// Created by VincentZhang on 5/19/2022.
//

#ifndef HUAHUOENGINE_CAMERACULLINGPARAMETERS_H
#define HUAHUOENGINE_CAMERACULLINGPARAMETERS_H

// Keep in sync with CullingParameter.cs CullFlags
enum CullingOptions
{
    kCullFlagNone                           = 0,
    kCullFlagForceEvenIfCameraIsNotActive   = 1 << 0,
    kCullFlagOcclusionCull                  = 1 << 1,
    kCullFlagNeedsLighting                  = 1 << 2,
    kCullFlagNeedsReflectionProbes          = 1 << 3,
    kCullFlagStereo                         = 1 << 4,
    kCullFlagDisablePerObjectCulling        = 1 << 5,
    kCullFlagShadowCasters                  = 1 << 6,
};

ENUM_FLAGS(CullingOptions)

#endif //HUAHUOENGINE_CAMERACULLINGPARAMETERS_H
