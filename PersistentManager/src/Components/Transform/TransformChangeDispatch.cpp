//
// Created by VincentZhang on 4/24/2022.
//

#include "TransformChangeDispatch.h"
#include "TransformHierarchy.h"
#include "Memory/MemoryMacros.h"
#include "Utilities/RegisterRuntimeInitializeAndCleanup.h"

TransformInternal::ChangeMaskCache TransformInternal::g_ChangeMaskCache;
TransformHierarchyChangeDispatch* gTransformHierarchyChangeDispatch = NULL;

static RegisterRuntimeInitializeAndCleanup s_TransformHierarchyChangeDispatchInitialize(TransformHierarchyChangeDispatch::InitializeClass, TransformHierarchyChangeDispatch::CleanupClass, -2);

TransformHierarchyChangeDispatch::TransformHierarchyChangeDispatch()
        : m_AllRegisteredSystemsMask(0)
        , m_PermanentInterestSystemsMask(0)
{
}

TransformHierarchyChangeDispatch::~TransformHierarchyChangeDispatch()
{
}

void TransformHierarchyChangeDispatch::InitializeClass(void*)
{
    gTransformHierarchyChangeDispatch = NEW(TransformHierarchyChangeDispatch);
}

void TransformHierarchyChangeDispatch::CleanupClass(void*)
{
    DELETE(gTransformHierarchyChangeDispatch);
}

void TransformHierarchyChangeDispatch::DispatchSelfAndAllChildren(TransformAccess transform, InterestType interestType)
{
    // DEBUG_ASSERT_RUNNING_ON_MAIN_THREAD;
    ASSERT_TRANSFORM_ACCESS(transform);
    // ASSERT_TRANSFORM_ACCESS_SYNCED(transform);

    TransformHierarchy& hierarchy = *transform.hierarchy;
    SInt32 index = transform.index;
    UInt32 count = GetDeepChildCount(hierarchy, index);

    TransformAccess* dispatchTransforms = NULL;
    unsigned dispatchTransformCount = 0;
    // ALLOC_TEMP_AUTO(dispatchTransforms, count);
    dispatchTransforms = ALLOC_ARRAY(TransformAccess, count);

    for (int systemIndex = 0; systemIndex < kMaxSupportedSystems; systemIndex++)
    {
        if ((interestType & m_Systems[systemIndex].interestType) == 0)
            continue;

        UInt32 mask = 1U << systemIndex;
        SInt32 cur = index;
        for (UInt32 i = 0; i < count; i++)
        {
            if ((hierarchy.hierarchySystemInterested[cur] & mask) != 0)
            {
                dispatchTransforms[dispatchTransformCount].hierarchy = &hierarchy;
                dispatchTransforms[dispatchTransformCount].index = cur;
                dispatchTransformCount++;
            }
            cur = hierarchy.nextIndices[cur];
        }

        if (dispatchTransformCount > 0)
        {
            m_Systems[systemIndex].callback(dispatchTransforms, dispatchTransformCount);
            dispatchTransformCount = 0;
        }
    }
}