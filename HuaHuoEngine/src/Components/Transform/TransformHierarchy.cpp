//
// Created by VincentZhang on 4/24/2022.
//

#include "TransformHierarchy.h"
#include "Memory/MemoryMacros.h"
#include "Memory/BatchAllocator.h"
#include "Math/TransformType.h"

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
        hierarchy->mainThreadOnlyTransformPointers = NEW_ARRAY(Transform*, transformCapacity);
        hierarchy->nextIndices = NEW_ARRAY(SInt32, transformCapacity);
        hierarchy->prevIndices = NEW_ARRAY(SInt32, transformCapacity);

        // ClearFenceWithoutSync(hierarchy->fence);

        hierarchy->transformCapacity = transformCapacity;
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

    static inline void CopyTransformMinimal(TransformHierarchy& srcHierarchy, UInt32 srcIndex, TransformHierarchy& dstHierarchy, UInt32 dstIndex)
    {
        dstHierarchy.localTransforms[dstIndex] = srcHierarchy.localTransforms[srcIndex];
        dstHierarchy.localTransformTypes[dstIndex] = srcHierarchy.localTransformTypes[srcIndex];
        dstHierarchy.deepChildCount[dstIndex] = srcHierarchy.deepChildCount[srcIndex];
        dstHierarchy.mainThreadOnlyTransformPointers[dstIndex] = srcHierarchy.mainThreadOnlyTransformPointers[srcIndex];

#if UNITY_EDITOR
        dstHierarchy.eulerHints[dstIndex] = srcHierarchy.eulerHints[srcIndex];
#endif
    }

    static inline void CopyTransformForCloning(TransformHierarchy& srcHierarchy, UInt32 srcIndex, TransformHierarchy& dstHierarchy, UInt32 dstIndex, TransformChangeSystemMask interestMask, TransformChangeSystemMask changeMask, UInt32 hierarchyInterestMask)
    {
        CopyTransformMinimal(srcHierarchy, srcIndex, dstHierarchy, dstIndex);
        TransformChangeSystemMask dstInterestMask = srcHierarchy.systemInterested[srcIndex] & interestMask;
        TransformChangeSystemMask dstChangedMask = dstInterestMask & (srcHierarchy.systemChanged[srcIndex] | changeMask);
        UInt32 dstHierarchyInterestMask = srcHierarchy.hierarchySystemInterested[srcIndex] & hierarchyInterestMask;
        dstHierarchy.systemChanged[dstIndex] = dstChangedMask;
        dstHierarchy.systemInterested[dstIndex] = dstInterestMask;
        dstHierarchy.combinedSystemChanged |= dstChangedMask;
        dstHierarchy.hierarchySystemInterested[dstIndex] = dstHierarchyInterestMask;
        dstHierarchy.combinedSystemInterest |= dstInterestMask;
    }

    static inline void CopyTransform(TransformHierarchy& srcHierarchy, UInt32 srcIndex, TransformHierarchy& dstHierarchy, UInt32 dstIndex, TransformChangeSystemMask changeMask)
    {
        CopyTransformMinimal(srcHierarchy, srcIndex, dstHierarchy, dstIndex);

        TransformChangeSystemMask dstInterestMask = srcHierarchy.systemInterested[srcIndex];
        TransformChangeSystemMask dstChangedMask = dstInterestMask & (srcHierarchy.systemChanged[srcIndex] | changeMask);
        dstHierarchy.systemChanged[dstIndex] = dstChangedMask;
        dstHierarchy.systemInterested[dstIndex] = dstInterestMask;
        dstHierarchy.combinedSystemChanged |= dstChangedMask;
        dstHierarchy.combinedSystemInterest |= dstInterestMask;

        dstHierarchy.hierarchySystemInterested[dstIndex] = srcHierarchy.hierarchySystemInterested[srcIndex];
    }

    void InsertTransformThreadAfter(TransformHierarchy& hierarchy, UInt32 index, UInt32 threadFirst, UInt32 threadLast)
    {
        SInt32 next = hierarchy.nextIndices[index];
        hierarchy.nextIndices[index] = threadFirst;
        hierarchy.prevIndices[threadFirst] = index;
        hierarchy.nextIndices[threadLast] = next;
        if (next != -1)
            hierarchy.prevIndices[next] = threadLast;
    }

    void CopyTransformSubhierarchy(TransformHierarchy& srcHierarchy, UInt32 srcIndex, TransformHierarchy& dstHierarchy, TransformChangeSystemMask interestMask, TransformChangeSystemMask changeMask, UInt32 hierarchyInterestMask, bool copyForCloning)
    {
        UInt32 count = GetDeepChildCount(srcHierarchy, srcIndex);

        AllocateTransformThread(dstHierarchy, 0, count - 1);

        SInt32 cur = srcIndex;
        for (UInt32 i = 0; i < count; i++)
        {
            if (copyForCloning)
                CopyTransformForCloning(srcHierarchy, cur, dstHierarchy, i, interestMask, changeMask, hierarchyInterestMask);
            else
                CopyTransform(srcHierarchy, cur, dstHierarchy, i, changeMask);

            cur = srcHierarchy.nextIndices[cur];
        }
    }

    void DetachTransformThread(TransformHierarchy& hierarchy, UInt32 threadFirst, UInt32 threadLast)
    {
        Assert(threadFirst > 0);
        Assert(threadLast > 0);

        SInt32 prev = hierarchy.prevIndices[threadFirst];
        SInt32 next = hierarchy.nextIndices[threadLast];
        hierarchy.prevIndices[threadFirst] = -1;
        hierarchy.nextIndices[prev] = next;
        hierarchy.nextIndices[threadLast] = -1;
        if (next != -1)
            hierarchy.prevIndices[next] = prev;
    }

    void FreeTransformThread(TransformHierarchy& hierarchy, UInt32 threadFirst, UInt32 threadLast)
    {
        Assert(threadFirst > 0);
        Assert(threadLast > 0);
        Assert(hierarchy.prevIndices[threadFirst] == -1);
        Assert(hierarchy.nextIndices[threadLast] == -1);

#if ENABLE_DEBUG_ASSERTIONS
        for (SInt32 cur = threadFirst; cur != -1; cur = hierarchy.nextIndices[cur])
            PoisonTransform(hierarchy, cur);
#endif

        SInt32 next = hierarchy.firstFreeIndex;
        hierarchy.firstFreeIndex = threadFirst;
        hierarchy.nextIndices[threadLast] = next;
        if (next != -1)
            hierarchy.prevIndices[next] = threadLast;
    }

    void UpdateDeepChildCountUpwards(TransformHierarchy& hierarchy, SInt32 index, SInt32 addedNodeCount)
    {
        while (index != -1)
        {
            Assert(0 < GetDeepChildCount(hierarchy, index) + addedNodeCount);
            hierarchy.deepChildCount[index] += addedNodeCount;
            index = hierarchy.parentIndices[index];
        }
    }

    void AddTransformSubhierarchy(TransformHierarchy& srcHierarchy, UInt32 srcIndex, TransformHierarchy& dstHierarchy, UInt32& dstFirst, UInt32& dstLast, TransformChangeSystemMask interestMask, TransformChangeSystemMask changeMask, UInt32 hierarchyInterestMask, bool copyForCloning)
    {
        UInt32 count = GetDeepChildCount(srcHierarchy, srcIndex);

        UInt32 first = dstHierarchy.firstFreeIndex;
        UInt32 last = first;

        if (copyForCloning)
            CopyTransformForCloning(srcHierarchy, srcIndex, dstHierarchy, last, interestMask, changeMask, hierarchyInterestMask);
        else
            CopyTransform(srcHierarchy, srcIndex, dstHierarchy, last, changeMask);

        SInt32 cur = srcHierarchy.nextIndices[srcIndex];
        for (UInt32 i = 1; i < count; i++)
        {
            last = dstHierarchy.nextIndices[last];

            if (copyForCloning)
                CopyTransformForCloning(srcHierarchy, cur, dstHierarchy, last, interestMask, changeMask, hierarchyInterestMask);
            else
                CopyTransform(srcHierarchy, cur, dstHierarchy, last, changeMask);

            cur = srcHierarchy.nextIndices[cur];
        }

        AllocateTransformThread(dstHierarchy, first, last);
        dstFirst = first;
        dstLast = last;
    }
}