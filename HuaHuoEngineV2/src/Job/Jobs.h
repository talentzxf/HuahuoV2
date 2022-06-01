//
// Created by VincentZhang on 5/18/2022.
//

#ifndef HUAHUOENGINE_JOBS_H
#define HUAHUOENGINE_JOBS_H

#include "JobTypes.h"
#include "Utilities/EnumFlags.h"

/// By default a job will be placed on the queue. The jobs will be executed in order.
/// When a job group (e.g. ScheduleJobForEach) is popped from the queue, it will be placed on the high priority stack.
/// All jobs that are on the stack will be processed first. When the stack is empty, more items will be taken from the queue to the stack.
///
// Implementation note: This enum must be kept in sync with JobQueue.h
enum JobPriority
{
    // Places a job on the queue. In order execution. This is the recommended approach.
    kNormalJobPriority = 0,

    /// Skip ahead of the queue and execute the job directly on the stack.
    /// Don't over-use otherwise it defeats the purpose. Only use it if you have lots of normal jobs
    /// and few jobs that you know you will schedule late but wait on immediately.
    kHighJobPriority = 1 << 0,
};
ENUM_FLAGS(JobPriority);

struct Job;
typedef void JobFunc (void* userData);
typedef void JobForEachFunc (void* userData, unsigned index);

// This version does not clear fence.groupID. Thus multiple jobs may call SyncFenceNoClear against the same JobFence..
void SyncFenceNoClear(const JobFence& fence);

// Sometimes fenced data is known to not be accessed after a certain point.
// It is not necessary to wait for it, and we will avoid the assert in the JobFence destructor.

// For example See SkinMeshInfo.writePoseMatricesFence
// SkinMeshInfo is passed via the render thread command queue to another thread
// The SkinMeshInfo on the main thread can safely be destructed without waiting.
// Because SkinMeshInfo on the render thread will now own the data which is written to from a job.
// We can safely call ClearFenceWithoutSync on the main thread and SyncFence is invoked by code on the render thread which will
// wait for the data to job to complete before the Matrix4x4 array of poses is destroyed or used.
void ClearFenceWithoutSync(JobFence& fence);

#endif //HUAHUOENGINE_JOBS_H
