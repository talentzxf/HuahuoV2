#pragma once

#include "TransformHierarchyTypes.h"
#include "TransformAccess.h"
//#include "Runtime/Threads/ThreadChecks.h"
//#include "Runtime/Jobs/Jobs.h"

//#define ASSERT_TRANSFORM_HIERARCHY_SYNCED(hierarchy) DebugAssert(FenceHasBeenSynced((hierarchy).fence))
//#define ASSERT_TRANSFORM_ACCESS_SYNCED(access) ASSERT_TRANSFORM_HIERARCHY_SYNCED(*(access).hierarchy)

inline void SyncTransformHierarchy(TransformHierarchy& hierarchy)
{
//    DEBUG_ASSERT_RUNNING_ON_MAIN_THREAD;
//    SyncFence(hierarchy.fence);
}

inline void SyncTransformAccess(TransformAccessReadOnly access)
{
    SyncTransformHierarchy(*access.hierarchy);
}
