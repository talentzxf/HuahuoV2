#include "UnityPrefix.h"
#include "ProcessThreadSnapshot.h"

#include <windows.h>
#include <TlHelp32.h>

ProcessThreadsSnapshot::ProcessThreadsSnapshot(DWORD processID, DWORD rights, bool suspend, bool autoResume)
    : m_hSnapshot(CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, (processID == GetCurrentProcessId() ? 0 : processID)))
    , m_AutoResume(autoResume)
    , m_Suspended(false)
    , m_ProcessID(processID)
{
    if (m_hSnapshot == INVALID_HANDLE_VALUE)
        return;

    bool isCurrentProcess = (processID == 0 || processID == GetCurrentProcessId());

    // Open handles to all the threads in this process
    THREADENTRY32 threadEntry;
    threadEntry.dwSize = sizeof(threadEntry);

    size_t minSize = FIELD_OFFSET(THREADENTRY32, th32OwnerProcessID) + sizeof(threadEntry.th32OwnerProcessID);

    if (Thread32First(m_hSnapshot, &threadEntry))
    {
        do
        {
            if (threadEntry.dwSize >= minSize &&
                threadEntry.th32OwnerProcessID == processID &&
                (!isCurrentProcess || (isCurrentProcess && threadEntry.th32ThreadID != GetCurrentThreadId())))
            {
                HANDLE hThread = OpenThread(THREAD_SUSPEND_RESUME | rights, false, threadEntry.th32ThreadID);
                if (hThread != nullptr)
                {
                    m_Threads.push_back(hThread);
                    m_ThreadMap[threadEntry.th32ThreadID] = hThread;
                }
            }
            threadEntry.dwSize = sizeof(threadEntry);
        }
        while (Thread32Next(m_hSnapshot, &threadEntry));
    }

    if (suspend)
        SuspendAll();
}

ProcessThreadsSnapshot::~ProcessThreadsSnapshot()
{
    if (m_hSnapshot != INVALID_HANDLE_VALUE)
    {
        if (m_Suspended && m_AutoResume)
            ResumeAll();

        // Close handles to all the threads
        for (dynamic_array<HANDLE>::const_iterator i = m_Threads.begin();
             i != m_Threads.end(); ++i)
        {
            CloseHandle(*i);
        }

        CloseHandle(m_hSnapshot);
    }
}

void ProcessThreadsSnapshot::SuspendAll()
{
    if (m_hSnapshot != INVALID_HANDLE_VALUE && !m_Suspended)
    {
        for (dynamic_array<HANDLE>::const_iterator i = m_Threads.begin();
             i != m_Threads.end(); ++i)
        {
            m_Suspended |= (SuspendThread(*i) != -1);
        }
    }
}

void ProcessThreadsSnapshot::ResumeAll()
{
    if (m_hSnapshot != INVALID_HANDLE_VALUE && m_Suspended)
    {
        for (dynamic_array<HANDLE>::const_iterator i = m_Threads.begin();
             i != m_Threads.end(); ++i)
        {
            m_Suspended |= !(ResumeThread(*i) != -1);
        }
        m_AutoResume = m_Suspended;
    }
}

bool ProcessThreadsSnapshot::Suspend(DWORD threadID)
{
    core::hash_map<DWORD, HANDLE>::const_iterator i = m_ThreadMap.find(threadID);
    if (i == m_ThreadMap.end())
        return false;
    return SuspendThread(i->second) != -1;
}

bool ProcessThreadsSnapshot::Resume(DWORD threadID)
{
    core::hash_map<DWORD, HANDLE>::const_iterator i = m_ThreadMap.find(threadID);
    if (i == m_ThreadMap.end())
        return false;
    return ResumeThread(i->second) != -1;
}
