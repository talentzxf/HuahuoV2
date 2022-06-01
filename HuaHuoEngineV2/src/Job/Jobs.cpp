//
// Created by VincentZhang on 5/18/2022.
//

#include "Jobs.h"
#include "JobsDebugger.h"

void ClearFenceWithoutSync(JobFence& fence)
{
    fence.groupID.group = 0;
    fence.groupID.version = 0u;
}

void SyncFenceNoClear(const JobFence& fence)
{
    DebugDidSyncFence(fence);

//    GetJobQueue().WaitForJobGroupID(fence.groupID, JobQueue::kWorkStealAllJobs);
}