#pragma once

// #include "Runtime/Scripting/BindingsDefs.h"
#include "Configuration/IntegerDefinitions.h"

class ScriptingArrayPtr;

struct FrameTiming
{
    // Keep in sync with managed FrameTiming struct

    // CPU events
    UInt64 m_CPUTimePresentCalled;
    double m_CPUFrameTime;

    // GPU events
    UInt64 m_CPUTimeFrameComplete; //This is the time the GPU finishes rendering the frame and interrupts the CPU
    double m_GPUFrameTime;

    // Linked data
    float m_HeightScale;
    float m_WidthScale;
    UInt32 m_SyncInterval;
};
// BIND_MANAGED_TYPE_NAME(FrameTiming, UnityEngine_FrameTiming);

class FrameTimingManager
{
public:
    FrameTimingManager();

    virtual ~FrameTimingManager() {}

    virtual void CaptureFrameTimings() {}

    virtual void CaptureFrameTimings(UInt32 numFrames) {}

    // Fills the timings array with the timing info for the latest complete frames. Data is requested for
    // the last numFrames frames, while the return value indicates how many objects were actually placed
    // in the array. Element 0 is the latest complete frame, element 1 is the frame before, etc. Returns
    // 0 if not supported.
    UInt32 GetLatestTimings(UInt32 numFrames, ScriptingArrayPtr timings);

    // Number of vsyncs per second on the current platform, used to interpret timing results.
    // 0 if not supported.
    virtual float GetVSyncsPerSecond() { return 0.0f; }

    // Frequency of GPU timer on the current platform in ticks per second, used to interpret timing results.
    // 0 if not supported.
    virtual UInt64 GetGpuTimerFrequency() { return 0; }

    // Frequency of CPU timer on the current platform in ticks per second, used to interpret timing results.
    // 0 if not supported.
    virtual UInt64 GetCpuTimerFrequency() { return 0; }

    UInt32 GetCapturedFrameCount() { return m_ValidCapturedFrames; }

    // The maximum number of frames that you can get timing info for. 0 if not supported.
    UInt32 GetMaxFrameCount()
    {
#if MAX_FRAME_TIMING_CAPTURE_FRAMES
        return MAX_FRAME_TIMING_CAPTURE_FRAMES;
#else
        return 0;
#endif
    }

protected:
#if MAX_FRAME_TIMING_CAPTURE_FRAMES
    FrameTiming m_CapturedFrames[MAX_FRAME_TIMING_CAPTURE_FRAMES];
#endif
    UInt32 m_ValidCapturedFrames;
};
//BIND_MANAGED_TYPE_NAME(FrameTimingManager, UnityEngine_FrameTimingManager);
