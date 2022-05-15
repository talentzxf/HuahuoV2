#define ENABLE_PROFILER

using System;
using UnityEngine.Profiling;

struct ProfilerBlock : IDisposable
{
    public ProfilerBlock(string name)
    {
        Profiler.BeginSample(name);
    }

    public void Dispose()
    {
        Profiler.EndSample();
    }
}
