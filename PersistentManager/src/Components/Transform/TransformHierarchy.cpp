//
// Created by VincentZhang on 4/24/2022.
//

#include "TransformHierarchy.h"
#include "Memory/MemoryMacros.h"
#include "Memory/BatchAllocator.h"
#include "TransformType.h"

namespace TransformInternal {

    TransformType CalculateTransformType(TransformAccess transformAccess)
    {
        using namespace math;

        float3 epsilon = float3(0.0001f);

        float3 scale = transformAccess.hierarchy->localTransforms[transformAccess.index].s.xyz;

        UInt32 oddNegativeScale = chgsign(chgsign(chgsign(1.0f, scale.x), scale.y), scale.z) < 0.0f;

        // If there are any scale components different or if the scale is odd negative, then it's non-uniform
        UInt32 nonUniformScale = math::any(math::abs(scale - scale.yzx) > epsilon);
        nonUniformScale |= oddNegativeScale;

        // The scale is uniform if it's not identity scale, but also if it's non-uniform then it can't be uniform.
        UInt32 uniformScale = math::all(math::abs(scale - float3(1.0f)) > epsilon);
        uniformScale &= !nonUniformScale;

        UInt32 transformType = kUniformScaleTransform | kNonUniformScaleTransform | kOddNegativeScaleTransform;

        // Check internal state (We use multiply later so value really must be 0 or 1)
        DebugAssert(oddNegativeScale == 0 || oddNegativeScale == 1);
        DebugAssert(nonUniformScale == 0 || nonUniformScale == 1);
        DebugAssert(uniformScale == 0 || uniformScale == 1);
        DebugAssert((!oddNegativeScale) == 0 || (!oddNegativeScale) == 1);
        DebugAssert((!nonUniformScale) == 0 || (!nonUniformScale) == 1);
        DebugAssert((!uniformScale) == 0 || (!uniformScale) == 1);

        // clear the odd negative scale bit
        transformType &= ~(!oddNegativeScale * kOddNegativeScaleTransform);
        // clear the non unfiform scale bit
        transformType &= ~(!nonUniformScale * kNonUniformScaleTransform);
        // clear the unfiform scale bit
        transformType &= ~(!uniformScale * kUniformScaleTransform);

        return (TransformType)transformType;
    }

    void AllocateTransformThread(TransformHierarchy &hierarchy, UInt32 threadFirst, UInt32 threadLast) {
        Assert(hierarchy.firstFreeIndex == threadFirst);
        hierarchy.firstFreeIndex = hierarchy.nextIndices[threadLast];
        if (hierarchy.firstFreeIndex != -1)
            hierarchy.prevIndices[hierarchy.firstFreeIndex] = -1;
        hierarchy.nextIndices[threadLast] = -1;
    }

    void OnScaleChangedCalculateTransformType(TransformAccess transformAccess)
    {
        transformAccess.hierarchy->localTransformTypes[transformAccess.index] = CalculateTransformType(transformAccess);
    }

    // This codepath must be specialized / different from the normal / updating data code path.
    // It assumes that this is the first time any of the values are being initialized.
    // Thus comparing against previous values is invalid.
    // Particularly OnScaleChangedCalculateTransformType can't depend on a comparison against the previous value like SetLocalT does.
    void InitLocalTRS(TransformAccess transformAccess, const math::float3& t, const math::float4& r, const math::float3& s)
    {
        using namespace math;

        trsX& trs = TransformInternal::GetLocalTRSWritable(transformAccess);

        trs.t = t;
        trs.q = r;
        trs.s = s;

        OnScaleChangedCalculateTransformType(transformAccess);
    }

    TransformHierarchy *CreateTransformHierarchy(UInt32 transformCapacity) {
//    BatchAllocator batch;
//
//    TransformHierarchy* hierarchy = NULL;
//    batch.AllocateRoot(hierarchy, 1);
//    batch.AllocateField(hierarchy->localTransforms, transformCapacity);
//    batch.AllocateField(hierarchy->parentIndices, transformCapacity);
//    batch.AllocateField(hierarchy->deepChildCount, transformCapacity);
//    batch.AllocateField(hierarchy->mainThreadOnlyTransformPointers, transformCapacity);
//    batch.AllocateField(hierarchy->localTransformTypes, transformCapacity);
//    batch.AllocateField(hierarchy->systemChanged, transformCapacity);
//    batch.AllocateField(hierarchy->systemInterested, transformCapacity);
//    batch.AllocateField(hierarchy->hierarchySystemInterested, transformCapacity);
//
//#if UNITY_EDITOR
//    batch.AllocateField(hierarchy->eulerHints, transformCapacity);
//#endif
//
//    batch.AllocateField(hierarchy->nextIndices, transformCapacity);
//    batch.AllocateField(hierarchy->prevIndices, transformCapacity);
//
//    batch.Commit(label);

        TransformHierarchy *hierarchy = NEW(TransformHierarchy);
        hierarchy->localTransforms = NEW_ARRAY(math::trsX, transformCapacity);
        hierarchy->parentIndices = NEW_ARRAY(SInt32, transformCapacity);
        hierarchy->deepChildCount = NEW_ARRAY(UInt32, transformCapacity);
        hierarchy->localTransformTypes = NEW_ARRAY(UInt8, transformCapacity);
        hierarchy->systemChanged = NEW_ARRAY(TransformChangeSystemMask, transformCapacity);
        hierarchy->systemInterested = NEW_ARRAY(TransformChangeSystemMask, transformCapacity);
        hierarchy->hierarchySystemInterested = NEW_ARRAY(UInt32, transformCapacity);

        // ClearFenceWithoutSync(hierarchy->fence);

#if ENABLE_DEBUG_ASSERTIONS
        for (int i = 0; i < transformCapacity; i++)
                PoisonTransform(*hierarchy, i);
#endif

        for (int i = 0; i < transformCapacity; i++) {
            hierarchy->prevIndices[i] = i - 1;
            hierarchy->nextIndices[i] = i + 1;
        }
        hierarchy->prevIndices[0] = -1;
        hierarchy->nextIndices[transformCapacity - 1] = -1;
        hierarchy->firstFreeIndex = 0;

        hierarchy->changeDispatchIndex = -1;
        hierarchy->combinedSystemChanged = TransformChangeSystemMask(0);
        hierarchy->combinedSystemInterest = TransformChangeSystemMask(0);

        return hierarchy;
    }

    void DestroyTransformHierarchy(TransformHierarchy *hierarchy) {
//    if (hierarchy != NULL)
//    {
//        SyncTransformHierarchy(*hierarchy);
//        GetTransformChangeDispatch().RemoveTransformHierarchy(*hierarchy);
//        BatchAllocator::DeallocateRoot(hierarchy->memLabel, hierarchy);
//    }
    }
}