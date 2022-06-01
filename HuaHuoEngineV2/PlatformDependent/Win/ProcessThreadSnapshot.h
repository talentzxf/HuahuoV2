#pragma once

#include "UnityPrefix.h"
#include "Runtime/Core/Containers/hash_map.h"
#include <windef.h>

//
// ProcessThreadsSnapshot captures the handles for all threads in the specified
// process. It also optionally suspends them. This allows us to operate on
// those thread handles in bulk, e.g. suspending and resuming all threads in
// the entire app. The crash handler uses this, for example, to suspend all
// threads in the current process in the case of a crash.
//
// Note: if using ProcessThreadsSnapshot on the current process, the current
// thread is *not* captured.
//
class ProcessThreadsSnapshot
{
public:
    typedef dynamic_array<HANDLE> HandleList;

    ProcessThreadsSnapshot(DWORD processID, DWORD rights, bool suspend, bool autoResume);
    ~ProcessThreadsSnapshot();

    void SuspendAll();
    void ResumeAll();

    bool Suspend(DWORD threadID);
    bool Resume(DWORD threadID);

    const dynamic_array<HANDLE>& GetThreads() const { return m_Threads; }

private:
    dynamic_array<HANDLE> m_Threads;
    core::hash_map<DWORD, HANDLE> m_ThreadMap;
    HANDLE m_hSnapshot;
    DWORD m_ProcessID;
    bool m_Suspended;
    bool m_AutoResume;
};
