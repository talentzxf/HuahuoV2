//
// Created by VincentZhang on 5/18/2022.
//

#ifndef HUAHUOENGINE_RENDERNODE_H
#define HUAHUOENGINE_RENDERNODE_H
#include "SharedRenderData.h"
#include "Shaders/ShaderPropertySheet.h"

class MaterialInfo;
enum BatchingFlags
{
    kBatchingFlagNone = 0,
    kBatchingFlagAllowBatching = 1 << 0,
    kBatchingFlagAllowInstancing = 1 << 1,              // INSTANCING_ON keyword is enabled only if this flag is set
    kBatchingFlagAllowProceduralInstancing = 1 << 2,    // PROCEDURAL_INSTANCING_ON keyword is enabled only if this flag is set

    kBatchingFlagUseInstancingForSingleRenderer = 1 << 3,   // instancing and procedural instancing is used for rendering unbatched one single renderer
};
ENUM_FLAGS(BatchingFlags)

enum { kInstancingBatchingFlagsBitCount = 2 };
inline UInt32 GetInstancingBatchingFlags(BatchingFlags flags)
{
    return (flags & (kBatchingFlagAllowInstancing | kBatchingFlagAllowProceduralInstancing)) >> 1;
}

struct CustomPropsAndHash
{
    const ShaderPropertySheet* ptr;
    UInt32 perMaterial  : 1;
    UInt32 hash         : 31;
    UInt32 layoutHash;
};

struct RenderNode {
    // properties needed for rendering
    SharedRendererData          rendererData;


    RenderNode()  {}

    void InitWithDefaultValues()
    {
        smallMeshIndex = 0;
        sameDistanceSortPriority = 0;
        sortingFudge = 0;
        lightProbeProxyVolHandle = -1;
        reflectionProbeHandle = -1;
        batchingFlags = kBatchingFlagNone;
        batchingKey = 0;
        lodMask = 0;
    }

    union
    {
        int                     layer;
        //@TODO-Later: For cleanliness move this out of here into rendererSpecificData
        int                     projectorROQueueIndex;
    };

    int                         materialCount;
    int                         smallMeshIndex;
    UInt16                      lodFade;
    UInt8                       lodFadeMode;
    UInt8                       lodMask;
    float                       sortingFudge;

    // custom properties
    union
    {
        CustomPropsAndHash      customProperties;
        CustomPropsAndHash*     perMaterialCustomProperties; // alias with CustomPropsAndHash::ptr, so that CustomPropsAndHash::perMaterial can still tell which union member is active.
    };

    // light probe proxy volume data
    SInt16                      lightProbeProxyVolHandle;

    UInt16                      sameDistanceSortPriority;

    // reflection probe data
    SInt16                      reflectionProbeHandle;
    SInt16                      reflectionProbeImportance;
    AABB                        reflectionProbeAnchor;

    // offsets for additional data
    MaterialInfo*               materialInfos;
    void*                       rendererSpecificData;

    BatchingFlags               batchingFlags;
    UInt32                      batchingKey;        // TODO: Get rid of this custom batching key, break batch in the executeBatchedCallback instead.

};


#endif //HUAHUOENGINE_RENDERNODE_H
