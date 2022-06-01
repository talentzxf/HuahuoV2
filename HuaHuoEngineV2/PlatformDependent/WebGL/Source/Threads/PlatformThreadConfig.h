#pragma once

#include "Runtime/Threads/DefaultThreadConfig.h"

class PlatformThreadConfig : private DefaultThreadConfig
{
protected:
    // common API
    friend class ThreadConfig;

    static int GetJobSchedulerMaxThreads()
    {
        // limit max number of JobScheduler threads since each additional thread requires ~2MB
        return 5;
    }

    static int GetEnlightenWorkerMaxThreads()
    {
        return 1;
    }

    using DefaultThreadConfig::GetAudioFeederThreadNumber;
    using DefaultThreadConfig::GetAudioMixerThreadNumber;
    using DefaultThreadConfig::GetAudioNonBlockingThreadNumber;
    using DefaultThreadConfig::GetAudioStreamThreadNumber;
    using DefaultThreadConfig::GetAudioFileThreadNumber;
    using DefaultThreadConfig::GetAudioGeometryThreadNumber;
    using DefaultThreadConfig::GetGfxDeviceWorkerAffinity;
    using DefaultThreadConfig::GetGfxDeviceWorkerPriority;
    using DefaultThreadConfig::GetJobSchedulerStartProcessor;
    using DefaultThreadConfig::GetLoadingThreadWorkerAffinity;
    using DefaultThreadConfig::GetLoadingThreadWorkerNormalPriority;
    using DefaultThreadConfig::GetLoadingThreadWorkerHighPriority;
    using DefaultThreadConfig::GetMonoWorkerThreadAffinity;
    using DefaultThreadConfig::GetJobSchedulerMaxThreadsFromCommandLine;
};
