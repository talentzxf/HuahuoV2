#include "FrameTiming.h"

FrameTimingManager::FrameTimingManager()
    : m_ValidCapturedFrames(0)
{}

UInt32 FrameTimingManager::GetLatestTimings(UInt32 numFrames, ScriptingArrayPtr timings)
{
    UInt32 framesToReturn = 0;

#if MAX_FRAME_TIMING_CAPTURE_FRAMES
    AssertMsg(m_ValidCapturedFrames <= GetMaxFrameCount(), "Valid Captured Frames exceeds total number of stored frames.");
    framesToReturn = std::min(numFrames, m_ValidCapturedFrames);
    for (UInt32 i = 0; i < framesToReturn; i++)
        Scripting::SetScriptingArrayElement(timings, i, m_CapturedFrames[i]);
#endif

    return framesToReturn;
}
