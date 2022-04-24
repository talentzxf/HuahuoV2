//
// Created by VincentZhang on 4/24/2022.
//

#ifndef PERSISTENTMANAGER_TRANSFORMHIERARCHYTYPES_H
#define PERSISTENTMANAGER_TRANSFORMHIERARCHYTYPES_H

#include "baselib/include/IntegerDefinitions.h"

class Transform;
namespace math
{
    struct trsX;
}

struct TransformHierarchy
{
    // JobFence        fence;

    UInt32          transformCapacity;
    SInt32          firstFreeIndex;

    math::trsX*     localTransforms;
    // For a given transform index. Returns the index of the parent. (the root (index=0) always returns -1)
    SInt32*         parentIndices;

    // The number of children this transform has directly and indirectly including itself.
    // A single transform with no children returns 1.
    // A free transform returns 0. Not strictly necessary, but will help find bugs.
    UInt32*         deepChildCount;

    // This may only be accessed on the main thread.
    Transform**     mainThreadOnlyTransformPointers;

    SInt32          changeDispatchIndex;
    TransformChangeSystemMask combinedSystemChanged;

    // Stores the bitmask if the transform has changed for a system.
    TransformChangeSystemMask* systemChanged;
    // Stores the bitmask for which system is interested in listening to changes on this transform.
    TransformChangeSystemMask* systemInterested;

    TransformChangeSystemMask combinedSystemInterest;

    // Stores the bitmask for which system is interested in listening to hierarchy changes involving this transform.
    UInt32*         hierarchySystemInterested;

    UInt8*          localTransformTypes;

    SInt32*         nextIndices;
    SInt32*         prevIndices;

#if UNITY_EDITOR
    math::float3*    eulerHints;
#endif
    // MemLabelId      memLabel;
};
#endif //PERSISTENTMANAGER_TRANSFORMHIERARCHYTYPES_H
