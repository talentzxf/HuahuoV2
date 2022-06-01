#pragma once

#include "Internal/JobGroupID.h"

// JobFences are used to ensure a job has completed before any data is accessed.
// See SyncFence function
struct JobFence
{
    JobFence()
    {
    }

    JobFence(const JobGroupID& inGroupID) : groupID(inGroupID)
    {
    }

    void operator=(const JobFence& fence)
    {
        groupID = fence.groupID;
    }

    void operator=(const JobGroupID& inGroupID)
    {
        groupID = inGroupID;
    }

    JobGroupID groupID;
};

inline bool operator==(const JobFence& lhs, const JobFence& rhs) { return lhs.groupID == rhs.groupID; }
inline bool operator!=(const JobFence& lhs, const JobFence& rhs) { return lhs.groupID != rhs.groupID; }
inline bool operator<(const JobFence& lhs, const JobFence& rhs) { return lhs.groupID < rhs.groupID; }

// BIND_MANAGED_TYPE_NAME(JobFence, Unity_Jobs_JobHandle);
