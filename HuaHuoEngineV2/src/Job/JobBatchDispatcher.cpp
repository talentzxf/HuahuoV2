#include "JobBatchDispatcher.h"

#if ENABLE_JOB_SCHEDULER

inline JobGroupID JobBatchDispatcher::GetJobQueueDependency(const JobFence& dependency)
{
    if (dependency.groupID.group != NULL)
        return dependency.groupID;
    else
        return JobGroupID();
}

JobBatchDispatcher::JobBatchDispatcher(JobPriority priority, int jobsPerBatch)
    : m_Head(NULL)
    , m_Tail(NULL)
    , m_JobsPerBatch(jobsPerBatch)
    , m_JobCount(0)
{
    JobQueue& queue = GetJobQueue();

    Assert((priority & kHighJobPriority) == 0);

    if (m_JobsPerBatch == kBatchKickByJobThreadCount)
    {
        m_JobsPerBatch  = std::max<int>(queue.GetWorkerThreadCount(), 1);
    }
}

JobBatchDispatcher::~JobBatchDispatcher()
{
    if (m_Head && m_JobCount > 0)
        KickJobs();
}

void JobBatchDispatcher::ScheduleJobDependsInternal(JobFence& fence, JobFunc jobFunc, void* userData, const JobFence& dependsOn)
{
    SyncFence(fence);
    JobQueue& queue = GetJobQueue();

    JobGroup* group = queue.CreateJobBatch(jobFunc, userData, GetJobQueueDependency(dependsOn), m_Tail);
    HandleJobKickInternal(queue, fence, group, 1);

    DebugDidScheduleJob(fence, dependsOn);
}

void JobBatchDispatcher::ScheduleJobForEachInternal(JobFence& fence, JobForEachFunc jobFunc, void* userData, int iterationCount, JobFunc combineFunc, const JobFence& dependsOn)
{
    SyncFence(fence);
    JobQueue& queue = GetJobQueue();

    JobGroup* group = queue.CreateForEachJobBatch(jobFunc, userData, iterationCount, combineFunc, GetJobQueueDependency(dependsOn), m_Tail);
    HandleJobKickInternal(queue, fence, group, iterationCount);

    DebugDidScheduleJob(fence, dependsOn);
}

void JobBatchDispatcher::HandleJobKickInternal(JobQueue& queue, JobFence& fence, JobGroup* group, int jobCount)
{
    if (m_Head == NULL)
        m_Head = group;
    m_Tail = group;

    // We can create a group ID for each job externally like so. These are individual
    // fences for each job.

    fence.groupID = queue.GetJobGroupID(group);

    m_JobCount += jobCount;
    if (m_JobsPerBatch != kManualJobKick && m_JobCount >= m_JobsPerBatch)
    {
        KickJobs();
    }
}

void JobBatchDispatcher::KickJobs()
{
    if (NULL != m_Head && m_JobCount > 0)
    {
        JobQueue& queue = GetJobQueue();
        queue.ScheduleGroups((JobGroup*)m_Head, (JobGroup*)m_Tail);

        m_JobCount = 0;
        m_Head     = NULL;
        m_Tail     = NULL;
    }
}

#else // ENABLE_JOB_SCHEDULER


JobBatchDispatcher::JobBatchDispatcher(JobPriority priority, int jobsPerBatch)
{
}

JobBatchDispatcher::~JobBatchDispatcher()
{
}

void JobBatchDispatcher::ScheduleJobDependsInternal(JobFence& fence, JobFunc* jobFunc, void* userData, const JobFence& dependsOn)
{
    jobFunc(userData);
}

void JobBatchDispatcher::ScheduleJobForEachInternal(JobFence& fence, JobForEachFunc jobFunc, void* userData, int iterationCount, JobFunc combineFunc, const JobFence& dependsOn)
{
    for (int i = 0; i < iterationCount; ++i)
        jobFunc(userData, i);

    if (combineFunc)
        combineFunc(userData);
}

void JobBatchDispatcher::KickJobs()
{
}

#endif // ENABLE_JOB_SCHEDULER
