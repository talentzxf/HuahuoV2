#include "UnityPrefix.h"
#include "ExternalProfiler.h"

#if EXTERNAL_PROFILER_ENABLE

typedef void (__cdecl* ProfilerFunc)(void);

struct ExternalProfilerLoader
{
    ExternalProfilerLoader()
        :   m_ProfilerDll(NULL)
        ,   m_ProfilerStart(NULL)
        ,   m_ProfilerStop(NULL)
        ,   m_InsideProfiling(false)
        ,   m_Message(NULL)
    {
        #if EXTERNAL_PROFILER_USE_CODE_ANALYST
        m_ProfilerDll = LoadLibraryA("CAProfAPI32.dll");
        if (m_ProfilerDll)
        {
            m_ProfilerStart = (ProfilerFunc)GetProcAddress(m_ProfilerDll, "CAProfResume");
            m_ProfilerStop = (ProfilerFunc)GetProcAddress(m_ProfilerDll, "CAProfPause");
            if (!m_ProfilerStart || !m_ProfilerStop)
                m_Message = "Profiler: could not load code analyst functions\n";
            else
                m_Message = "Profiler: all ok!\n";
        }
        else
        {
            m_Message = "Profiler: could not load code analyst dll\n";
        }
        #endif
    }

    ~ExternalProfilerLoader()
    {
        if (m_ProfilerDll)
            FreeLibrary(m_ProfilerDll);
        m_ProfilerStart = NULL;
        m_ProfilerStop = NULL;
    }

    HMODULE m_ProfilerDll;
    ProfilerFunc    m_ProfilerStop;
    ProfilerFunc    m_ProfilerStart;
    bool            m_InsideProfiling;
    const char*     m_Message;
};
ExternalProfilerLoader g_ExternalProfiler;


void ExternalProfilerStart()
{
    if (g_ExternalProfiler.m_Message)
    {
        printf_console(g_ExternalProfiler.m_Message);
        g_ExternalProfiler.m_Message = NULL;
    }
    ErrorIf(g_ExternalProfiler.m_InsideProfiling);
    g_ExternalProfiler.m_InsideProfiling = true;
    if (g_ExternalProfiler.m_ProfilerStart)
        g_ExternalProfiler.m_ProfilerStart();
}

void ExternalProfilerStop()
{
    ErrorIf(!g_ExternalProfiler.m_InsideProfiling);
    g_ExternalProfiler.m_InsideProfiling = false;
    if (g_ExternalProfiler.m_ProfilerStop)
        g_ExternalProfiler.m_ProfilerStop();
}

#endif // EXTERNAL_PROFILER_ENABLE
