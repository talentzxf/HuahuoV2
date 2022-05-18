//
// Created by VincentZhang on 5/18/2022.
//

#ifndef HUAHUOENGINE_LIGHTMAPTYPES_H
#define HUAHUOENGINE_LIGHTMAPTYPES_H

#include "Configuration/IntegerDefinitions.h"

// keep in sync with GraphicsRenderers.bindings.cs
enum LightmapType
{
    kNoLightmap = -1,
    kStaticLightmap = 0,
    kDynamicLightmap,
    kLightmapTypeCount
};

// Object does not use lightmaps.
#define kLightmapIndexNotLightmapped 0xFFFF

struct LightmapIndices
{
    union
    {
        UInt32 combined;
        UInt16 indices[2];
    };

    UInt16& operator[](int index) { return indices[index]; }
    const UInt16& operator[](int index) const { return indices[index]; }

    bool operator<(const LightmapIndices& rhs) const { return combined < rhs.combined; }
    bool operator==(const LightmapIndices& rhs) const { return combined == rhs.combined; }
    bool operator!=(const LightmapIndices& rhs) const { return combined != rhs.combined; }

    void Reset() { indices[1] = kLightmapIndexNotLightmapped; indices[0] = kLightmapIndexNotLightmapped; }
};

#endif //HUAHUOENGINE_LIGHTMAPTYPES_H
