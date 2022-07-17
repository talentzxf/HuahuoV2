#include <cmath>
#include "TimeManager.h"
#include "BaseClasses/IsPlaying.h"
#include "Utilities/Utility.h"
#include "Math/FloatConversion.h"
#include "TargetFrameRate.h"
#include "Serialize/SerializeUtility.h"
#include "BaseClasses/ManagerContext.h"

#if PLATFORM_ANDROID
#include "PlatformDependent/AndroidPlayer/Source/DisplayInfo.h"
#endif

#define DEBUG_TIME_MANAGER 0 && DEBUGMODE

#if DEBUG_TIME_MANAGER
#define DEBUG_MSG(...) printf_console(__VA_ARGS__)
#else
#define DEBUG_MSG(...) {}
#endif


const float kMaximumDeltaTime = 1.0F / 3.0F;
const float kStartupDeltaTime = 0.02F;
const float kNewDeltaTimeWeight = 0.2F; // for smoothing
const float kMaximumParticleDeltaTime = 0.03F;

float CalcInvDeltaTime(float dt)
{
    if (dt > kMinimumDeltaTime)
        return 1.0F / dt;
    else
        return 1.0F;
}

TimeManager::TimeHolder::TimeHolder()
    :   m_CurFrameTime(0)
    ,   m_LastFrameTime(0)
    ,   m_DeltaTime(0)
    ,   m_SmoothDeltaTime(0)
    ,   m_SmoothingWeight(0)
    ,   m_InvDeltaTime(0)
{
}

TimeManager::TimeManager(MemLabelId label, ObjectCreationMode mode)
    :   Super(label, mode)
    ,   m_CullFrameCount(0)
{
    m_SetTimeManually = false;
    m_UseFixedTimeStep = false;

    m_FixedTime.m_DeltaTime = kStartupDeltaTime;

    m_LastSyncEnd = 0;
    memset(m_FrameSyncEnds, 0, sizeof(m_FrameSyncEnds));

    ResetTime(IsWorldPlaying());
}

void TimeManager::Reset()
{
    Super::Reset();

    m_FixedTime.m_DeltaTime = kStartupDeltaTime;
    m_MaximumTimestep = kMaximumDeltaTime;
    m_TimeScale = 1.0f;
    m_MaximumParticleTimestep = kMaximumParticleDeltaTime;
}

// void TimeManager::ThreadedCleanup() {}

inline void CalcSmoothDeltaTime(TimeManager::TimeHolder& time)
{
    // If existing weight is zero, don't take existing value into account
    time.m_SmoothingWeight *= (1.0F - kNewDeltaTimeWeight);
    time.m_SmoothingWeight += kNewDeltaTimeWeight;
    // As confidence in smoothed value increases the divisor goes towards 1
    float normalized = kNewDeltaTimeWeight / time.m_SmoothingWeight;
    time.m_SmoothDeltaTime = Lerp(time.m_SmoothDeltaTime, time.m_DeltaTime, normalized);
}

void TimeManager::SetPause(bool pause)
{
    m_FirstFrameAfterPause = true;
}

// Depending on OS, platform, presence of a VR Device, and the settings for vsync and Application.targetFrameRate,
// we perform the synchronization in different ways:
//
// * NoSync: Do not sync, run as fast as possible. Used if no sync is desired.
//
// * SyncOutsideOfPlayerLoop: Sync happens in whatever is calling the player loop, typically some OS scheduler outside
// of Unity code. Used if UNITY_GFX_USE_PLATFORM_VSYNC is defined.
//
// * SyncInUpdateTime: Sync happens in the player loop, when UpdateTime is called. Used if we don't support hardware
// sync for the desired frame rate.
//
// * SyncInWaitForPresent: Sync happens in GfxDeviceClient::WaitForPendingPresent, where we wait for the gfx device
// thread to have performed the present. Used if gfx device supports hardware sync for the desired frame rate.

TimeManager::SyncBehaviour TimeManager::GetSyncBehaviour() const
{
    return SyncOutsideOfPlayerLoop;
//#if UNITY_GFX_USE_PLATFORM_VSYNC
//    return SyncOutsideOfPlayerLoop;
//#else
//    IVRDevice* vrDevice = GetIVRDevice();
//    if (vrDevice && vrDevice->GetActive())
//        return vrDevice->GetDisableVSync() ? NoSync : SyncInUpdateTime;
//
//    const int vSyncWanted = GetWantedVSyncCount();
//    if (vSyncWanted > 0)
//    {
//        if (vSyncWanted <= GetMaxSupportedVSyncCount())
//            return SyncInWaitForPresent;
//        else
//            return SyncInUpdateTime;
//    }
//
//    if (GetActualTargetFrameRate() > 0)
//        return SyncInUpdateTime;
//
//    return NoSync;
//#endif
}

//PROFILER_INFORMATION(gFramerateSync, "WaitForTargetFPS", kProfilerVSync);
//PROFILER_INFORMATION(gTimeManagerUpdate, "TimeManager.Update", kProfilerPlayerLoop);

void TimeManager::StartSyncFrame()
{
    m_LastSyncEnd = GetTimeSinceStartup();
    m_FrameSyncEnds[m_FrameCount % kNumFramesToKeepForVSyncTimes] = m_LastSyncEnd;
//#ifdef PLATFORM_HAS_CUSTOM_PLAYER_LOOP_PROFILE_MARKERS
//    // The begin and end for gFramerateSync happen in different functions. If PlayerLoop markers are moved,
//    // the EndMarker for gFramerateSync may not get triggered before the start of the next PlayerLoop marker.
//    // Ignoring end marker here, as EndSyncFrame will have the marker.
//#else
//    PROFILER_END(gFramerateSync);
//#endif
}

#if PLATFORM_ANDROID
int s_LastVsyncCounter = 0;

void WaitVSync(int toFrame);
double GetVSyncTime(int frameRate, bool firstFrameAfterReset);
int GetVSyncCounter();

// Android has custom sync code. This is only used in cases when we don't use GPU vsync, but are syncing at every n-th vsync.
// This is the case when either:
// - A VR device is used
// - Quality settings want to sync in "every other vsync"
// - Vsync is off, and 60 is dividable by Application.targetFrameRate
// Wether we actually *need* a custom Android code path specifically for these cases, and if this visible improves vsync,
// I find hard to tell. I decided to leave this in, but Android team should do testing on if this is actually needed and
// remove it otherwise.
bool AndroidSync()
{
    const float AndroidDisplayRefreshRate = DisplayInfo::GetDefaultDisplayInfo().refreshRate;
    float framerate = GetActualTargetFrameRate();
    if (framerate > 0 && framerate <= AndroidDisplayRefreshRate && GetVSyncCounter() > 0)
    {
        if (fabs(remainder(AndroidDisplayRefreshRate, framerate)) < 0.01f)
        {
            WaitVSync(s_LastVsyncCounter + (AndroidDisplayRefreshRate / framerate + 0.5));
            s_LastVsyncCounter = GetVSyncCounter();
            return true;
        }
    }
    return false;
}

#endif

double TimeManager::GetTargetFrameTime() const
{
    SyncBehaviour syncBehaviour = GetSyncBehaviour();
    switch (syncBehaviour)
    {
        case NoSync:
            // If we don't have any frame rate target, we want to run as fast possible, setting a goal frame time of zero.
            return 0.0;

        case SyncInUpdateTime:
            // If we are syncing manually to a specific frame rate, then the inverse of that is our target frame time.
            return 1.0 / (double)GetActualTargetFrameRate();

        case SyncOutsideOfPlayerLoop:
        case SyncInWaitForPresent:
            // If we are syncing using the gfx hardware, then we don't have a known frame duration. Vsync rates cannot be reliably
            // and consistently determined across all platforms. Also, it can change at runtime when eg moving a window to another
            // screen.
            // So the best we can do is look at previous frame times, and take the shortest duration between two frames as our goal.
            // This should get us close to the actual vysnc time.
            //
            // This is only used for the purpose of scheduling tasks like GC during the available "free" time between frames. Even
            // if the guess at how much available time we have may not always be perfect, results should be better then using any
            // arbitrary amount of time to spend on these tasks.
        {
            const double inf = std::numeric_limits<double>::infinity();
            double frameTime = inf;
            int currentFrameIndex = m_FrameCount % kNumFramesToKeepForVSyncTimes;
            for (int i = 0; i < kNumFramesToKeepForVSyncTimes - 1; i++)
            {
                if (currentFrameIndex != i + 1)
                {
                    double thisFrameTime = m_FrameSyncEnds[i + 1] - m_FrameSyncEnds[i];
                    frameTime = std::min(frameTime, thisFrameTime);
                }
            }
            return frameTime;
        }
    }
    AssertString("Unknown enum value for GetSyncBehaviour");
    return 0.0;
}

void TimeManager::EndSyncFrame(SyncBehaviour behavior)
{
//#ifdef PLATFORM_HAS_CUSTOM_PLAYER_LOOP_PROFILE_MARKERS
//
//    // The begin and end for gFramerateSync happen in different functions. If PlayerLoop markers are moved,
//    // the EndMarker for gFramerateSync may not get triggered before the start of the next PlayerLoop marker.
//    // So putting this function in a separate profiler marker
//    PROFILER_AUTO(gFramerateSync);
//#else
//    PROFILER_BEGIN(gFramerateSync);
//#endif
    double frameTime = GetTargetFrameTime();

    // wait for enough time to pass for the requested framerate
    if (frameTime > 0)
    {
        double time = GetTimeSinceStartup();
        double timeToWait = frameTime - (time - m_LastSyncEnd);

//        // ***
//        // This place is where we can use available time for GC or other tasks!
//        // ***
//
//        if (GarbageCollector::GetIncrementalEnabled())
//        {
//            // If we have some time to spare, we can spend it garbage collecting.
//            // But lets keep a safety margin, in case the GC takes longer then anticipated,
//            // to make sure we don't miss vsync.
//            const double kTimeRemainderNotToTouchByGC = 0.001;
//            // But always allow at least 1 ms for GC if it needs it, to avoid needing a full collection eventually.
//            const double kMinGCTimeSlice = 0.001;
//
//            double gcTime = timeToWait - kTimeRemainderNotToTouchByGC;
//            gcTime = std::max(gcTime, kMinGCTimeSlice);
//            GarbageCollector::CollectIncremental(gcTime * 1000000000);
//            time = GetTimeSinceStartup();
//            timeToWait = frameTime - (time - m_LastSyncEnd);
//        }

        if (behavior != SyncInUpdateTime)
            return;

#if PLATFORM_ANDROID
        if (AndroidSync())
            return;
#endif

        // Wait a bit less (0.1ms), to accomodate for small fluctuations
        timeToWait -= 0.0001;

        // Don't support perfhud in batch/automated mode
        bool isTimeStopped = CompareApproximatelyD(time, m_LastSyncEnd); //&& IsHumanControllingUs();
        if (!isTimeStopped && time - m_LastSyncEnd < frameTime)
        {
#if SUPPORT_THREADS
            CurrentThread::SleepForSeconds(timeToWait);
#endif
            int i = 0;
            double start = GetTimeSinceStartup();
            // do the last adjustment with a busy wait
            do
            {
                time = GetTimeSinceStartup();
                if (++i >= 1000)
                {
                    // When using PerfHUD ES together with the NVIDIA time extension
                    // the time might be stopped, thus causing a diff of 0.
                    if ((time - start) == 0)
                    {
                        return;
                    }
                    // Need to reset "start" here just in case we started to
                    // busy wait and then the time was stopped while busy waiting
                    start = time;
                    i = 0;
                }

//                // Give others some time in spinning loop
//                Thread::YieldProcessor();
            }
            while (time - m_LastSyncEnd < frameTime);
        }
    }
}

void TimeManager::Sync(SyncPosition syncPosition)
{
    SyncBehaviour syncBehaviour = GetSyncBehaviour();
    switch (syncBehaviour)
    {
        case NoSync:
//            if (syncPosition == InUpdateTime)
//            {
//                // If we don't sync, allow the incremental GC to use the default timeslice for collection.
//                // This gives better results then to only let it collect when it does so by itself, since the risk of falling back
//                // to a full collection is higher in that case.
//                GarbageCollector::CollectIncrementalForRemainingTimeSliceBudget();
//            }
            break;

        case SyncOutsideOfPlayerLoop:
            if (syncPosition == BeforePlayerLoop)
                StartSyncFrame();
            else if (syncPosition == AfterPlayerLoop)
                EndSyncFrame(syncBehaviour);
            break;

        case SyncInUpdateTime:
            if (syncPosition == InUpdateTime)
            {
                EndSyncFrame(syncBehaviour);
                StartSyncFrame();
            }
            break;

        case SyncInWaitForPresent:
            if (syncPosition == AfterWaitForPresent)
                StartSyncFrame();
            else if (syncPosition == BeforeWaitForPresent)
                EndSyncFrame(syncBehaviour);
            break;
    }
}

void TimeManager::Update(double timeSinceStartup)
{
    Assert(!m_UseFixedTimeStep);
    m_FrameCount++;
    m_RenderFrameCount++;
    if (m_SetTimeManually)
        return;

    // PROFILER_AUTO(gTimeManagerUpdate);

    double unscaledTime = timeSinceStartup - m_RealZeroTime;
    m_DynamicTime.m_UnscaledDeltaTime = unscaledTime - m_DynamicTime.m_CurFrameUnscaledTime;
    m_DynamicTime.m_CurFrameUnscaledTime = unscaledTime;

#if PLATFORM_ANDROID
    // On Android Jelly Bean +, with vsync on: Use last vsync time instead for more stable animation
    IVRDevice* vrDevice = GetIVRDevice();
    bool disableVsync = vrDevice && vrDevice->GetActive() && vrDevice->GetDisableVSync();

    int wantedFrameRate = GetActualTargetFrameRate();
    if (wantedFrameRate > 0 && !disableVsync)
    {
        double t = GetVSyncTime(wantedFrameRate, m_FirstFrameAfterReset);
        if (t > 0)
            timeSinceStartup = t;
    }
#endif

    double time = timeSinceStartup - m_ZeroTime;

    if (m_CaptureDeltaTime > 0.0F)
    {
        // Capture delta time is always constant
        DEBUG_MSG("time: setting time using capture delta time of %f, timescale %f\n", m_CaptureDeltaTime, m_TimeScale);
        time = m_DynamicTime.m_CurFrameTime + m_CaptureDeltaTime * m_TimeScale;
    }
    else if (m_FirstFrameAfterReset)
    {
        // Don't do anything to delta time the first frame!
        m_FirstFrameAfterReset = false;
        return;
    }
    else if (m_FirstFrameAfterPause)
    {
        // When coming out of a pause / startup / scene load we don't want to have a spike in delta time.
        // So just default to kStartupDeltaTime.
        DEBUG_MSG("time: setting time first frame after pause\n");
        time = m_DynamicTime.m_CurFrameTime + kStartupDeltaTime * m_TimeScale;
    }
    else if (time - m_DynamicTime.m_CurFrameTime > m_MaximumTimestep)
    {
        // clamp the delta time in case a frame takes too long.
        DEBUG_MSG("time: maximum dt (was %f)\n", time - m_DynamicTime.m_CurFrameTime);
        time = m_DynamicTime.m_CurFrameTime + m_MaximumTimestep * m_TimeScale;
    }
    else if (time - m_DynamicTime.m_CurFrameTime < kMinimumDeltaTime)
    {
        // clamp the delta time in case a frame goes to fast! (prevent delta time being zero)
        DEBUG_MSG("time: minimum dt (was %f)\n", time - m_DynamicTime.m_CurFrameTime);
        time = m_DynamicTime.m_CurFrameTime + kMinimumDeltaTime * m_TimeScale;
    }
    else if (!CompareApproximately(m_TimeScale, 1.0F))
    {
        // Handle time scale
        DEBUG_MSG("time: time scale path, delta %f\n", time - m_DynamicTime.m_CurFrameTime);
        float deltaTime = time - m_DynamicTime.m_CurFrameTime;
        time = m_DynamicTime.m_CurFrameTime + deltaTime * m_TimeScale;
    }

    m_DynamicTime.m_LastFrameTime = m_DynamicTime.m_CurFrameTime;
    m_DynamicTime.m_CurFrameTime = time;
    m_DynamicTime.m_DeltaTime = m_DynamicTime.m_CurFrameTime - m_DynamicTime.m_LastFrameTime;
    m_DynamicTime.m_InvDeltaTime = CalcInvDeltaTime(m_DynamicTime.m_DeltaTime);
    CalcSmoothDeltaTime(m_DynamicTime);

    m_ActiveTime = m_DynamicTime;

    // Recalc zeroTime. If time was not modified above, it will have no effect
    m_ZeroTime = timeSinceStartup - m_DynamicTime.m_CurFrameTime;

    if (m_FirstFrameAfterPause)
    {
        m_FirstFrameAfterPause = false;
        // This is not a real delta time so don't include in smoothed time
        m_DynamicTime.m_SmoothingWeight = 0.0f;
    }
}

void TimeManager::SetTime(double time)
{
    Assert(!m_UseFixedTimeStep);
    m_DynamicTime.m_LastFrameTime = m_DynamicTime.m_CurFrameTime;
    m_DynamicTime.m_CurFrameTime = time;
    m_DynamicTime.m_DeltaTime = m_DynamicTime.m_CurFrameTime - m_DynamicTime.m_LastFrameTime;

    m_DynamicTime.m_InvDeltaTime = CalcInvDeltaTime(m_DynamicTime.m_DeltaTime);
    CalcSmoothDeltaTime(m_DynamicTime);

    m_ActiveTime = m_DynamicTime;

    // Sync m_ZeroTime with timemanager time
    m_ZeroTime = GetTimeSinceStartup() - m_DynamicTime.m_CurFrameTime;
    #if DEBUG_TIME_MANAGER
    printf_console("time: set to %f, sync zero to %f\n", time, m_ZeroTime);
    #endif
}

bool TimeManager::HasFixedTimeStep()
{
    if (m_FixedTime.m_CurFrameTime + m_FixedTime.m_DeltaTime > m_DynamicTime.m_CurFrameTime && !m_FirstFixedFrameAfterReset)
    {
        return false;
    }
    return true;
}

bool TimeManager::StepFixedTime()
{
    if (!HasFixedTimeStep())
    {
        m_ActiveTime = m_DynamicTime;
        m_UseFixedTimeStep = false;
        return false;
    }

    m_FixedTime.m_LastFrameTime = m_FixedTime.m_CurFrameTime;
    if (!m_FirstFixedFrameAfterReset)
    {
        m_FixedTime.m_CurFrameTime += m_FixedTime.m_DeltaTime;
    }
    if (m_TimeScale != 0)
    {
        // The purpose of unscaled time is to have time that increments unaffected
        // by time scale so that e.g. menu animations etc. can still work normally
        // while a game's time is in slow motion or entirely paused.
        // For dynamic frames, recording the realtime at the start of the frame works
        // for that, but for fixed frames it's not as simple. In realtime, fixed
        // frames are not spread evenly, but are all executed right after each other.
        // We want Time.fixedUnscaledTime to be spread evenly, like Time.fixedTime,
        // but just unaffected by time scale. We also want it to be in sync with
        // Time.unscaledTime. We do this by using Time.unscaledTime as a starting
        // point but add the delta in game time between the current dynamic frame
        // time and the current fixed frame time.
        // Since those are both in scaled time, we need to divide by the time scale.
        double dynamicToFixedTimeDelta = m_FixedTime.m_CurFrameTime - m_DynamicTime.m_CurFrameTime;
        double unscaledTime = m_DynamicTime.m_CurFrameUnscaledTime
            + (dynamicToFixedTimeDelta / m_TimeScale);

        m_FixedTime.m_UnscaledDeltaTime = unscaledTime - m_FixedTime.m_CurFrameUnscaledTime;
        m_FixedTime.m_CurFrameUnscaledTime = unscaledTime;
    }

    m_ActiveTime = m_FixedTime;
    m_UseFixedTimeStep = true;
    m_FirstFixedFrameAfterReset = false;

    return true;
}

void TimeManager::ResetTime(bool isPlayMode)
{
    Assert(!m_UseFixedTimeStep);
    m_DynamicTime.m_CurFrameTime = 0.0F;
    m_DynamicTime.m_LastFrameTime = 0.0F;
    m_DynamicTime.m_CurFrameUnscaledTime = 0.0F;
    if (isPlayMode)
    {
        m_DynamicTime.m_DeltaTime = 0.02F;
        m_DynamicTime.m_InvDeltaTime = 1.0F / m_DynamicTime.m_DeltaTime;
        m_DynamicTime.m_UnscaledDeltaTime = 0.02F;
    }
    else
    {
        m_DynamicTime.m_DeltaTime = 0.0F;
        m_DynamicTime.m_InvDeltaTime = 0.0F;
    }
    m_DynamicTime.m_SmoothDeltaTime = 0.0F;
    m_DynamicTime.m_SmoothingWeight = 0.0F;

    m_FixedTime.m_CurFrameTime = 0.0F;
    m_FixedTime.m_LastFrameTime = 0.0F;
    m_FixedTime.m_CurFrameUnscaledTime = 0.0F;
    // Dont erase the fixed delta time
    m_FixedTime.m_UnscaledDeltaTime = m_FixedTime.m_DeltaTime;
    m_FixedTime.m_InvDeltaTime = 1.0F / m_FixedTime.m_DeltaTime;

    m_ActiveTime = m_DynamicTime;

    m_FirstFrameAfterReset = true;
    m_FirstFrameAfterPause = true;
    m_FirstFixedFrameAfterReset = true;

    m_FrameCount = 0;
    m_RenderFrameCount = 0;
    m_ZeroTime = GetTimeSinceStartup();
    m_RealZeroTime = m_ZeroTime;
    #if DEBUG_TIME_MANAGER
    printf_console("time: startup, zero time %f\n", m_RealZeroTime);
    #endif
    m_SceneLoadOffset = 0.0F;
    m_CaptureDeltaTime = 0.0F;

    // GlobalCallbacks::Get().resetTime.Invoke();
}

template<class TransferFunction>
void TimeManager::Transfer(TransferFunction& transfer)
{
    Super::Transfer(transfer);
    transfer.Transfer(m_FixedTime.m_DeltaTime, "Fixed Timestep");
    transfer.Transfer(m_MaximumTimestep, "Maximum Allowed Timestep");
    transfer.Transfer(m_TimeScale, "m_TimeScale");
    transfer.Transfer(m_MaximumParticleTimestep, "Maximum Particle Timestep");
}

void TimeManager::SetFixedDeltaTime(float fixedStep)
{
    fixedStep = ::clamp<float>(fixedStep, 0.0001F, 10.0F);
    m_FixedTime.m_DeltaTime = fixedStep;
    m_FixedTime.m_InvDeltaTime = 1.0F / m_FixedTime.m_DeltaTime;
    m_FixedTime.m_SmoothDeltaTime = m_FixedTime.m_DeltaTime;

    SetMaximumDeltaTime(m_MaximumTimestep);
}

void TimeManager::SetMaximumDeltaTime(float maxStep)
{
    m_MaximumTimestep = std::max<float>(maxStep, m_FixedTime.m_DeltaTime);
}

void TimeManager::SetMaximumParticleDeltaTime(float maxStep)
{
    m_MaximumParticleTimestep = std::max<float>(maxStep, m_FixedTime.m_DeltaTime);
}

void TimeManager::AwakeFromLoad(AwakeFromLoadMode awakeMode)
{
    Super::AwakeFromLoad(awakeMode);

    m_FixedTime.m_InvDeltaTime = 1.0F / m_FixedTime.m_DeltaTime;
    m_FixedTime.m_SmoothDeltaTime = m_FixedTime.m_DeltaTime;
}

//void TimeManager::CheckConsistency()
//{
//    Super::CheckConsistency();
//
//    m_FixedTime.m_DeltaTime = clamp<float>(m_FixedTime.m_DeltaTime, 0.0001F, 10.0F);
//    m_MaximumTimestep = std::max<float>(m_MaximumTimestep, m_FixedTime.m_DeltaTime);
//    m_MaximumParticleTimestep = std::max<float>(m_MaximumParticleTimestep, m_FixedTime.m_DeltaTime);
//}

void TimeManager::DidFinishLoadingScene()
{
    m_SceneLoadOffset = -m_DynamicTime.m_CurFrameTime;
    // Trying to reconstruct what was intended here, this seems plausible:
    m_FirstFrameAfterPause = m_FirstFrameAfterReset = true;
}

void TimeManager::SetTimeScale(float scale)
{
    const float lowerLimit = 0.0f;      // discard negative values
    const float upperLimit = 100.0f;    // disallow large scale factors in the editor

    if (::isnan(scale))
    {
        ErrorStringMsg("Time.timeScale cannot be NaN");
        return;
    }

    if (scale < lowerLimit)
    {
        ErrorStringMsg("Time.timeScale is out of range. The value cannot be less than %.1f", lowerLimit);
        return;
    }

    // We limit the upper value only in the editor to avoid accidental large values that would make the update loop unreponsive.
    // In the player we don't have an upper limit to allow offline simulation.
    if (HUAHUO_EDITOR && scale > upperLimit)
    {
        ErrorStringMsg("Time.timeScale is out of range. When running in the editor this value needs to be less than or equal to %.1f", upperLimit);
        return;
    }

    m_TimeScale = scale;
    SetDirty();
}

double TimeManager::GetRealtime()
{
    return GetTimeSinceStartup() - m_RealZeroTime;
}

template<class TransferFunc>
void TimeManager::TransferState(TransferFunc& transfer)
{
    TRANSFER(m_FirstFrameAfterReset);
    TRANSFER(m_FirstFrameAfterPause);
    TRANSFER(m_FirstFixedFrameAfterReset);

    TRANSFER(m_FrameCount);
    TRANSFER(m_RenderFrameCount);
    TRANSFER(m_CullFrameCount);

    TRANSFER(m_CaptureDeltaTime);
    TRANSFER(m_ZeroTime);
    TRANSFER(m_RealZeroTime);
    TRANSFER(m_SceneLoadOffset);

    TRANSFER(m_SetTimeManually);
    TRANSFER(m_UseFixedTimeStep);
    TRANSFER(m_TimeScale);
    TRANSFER(m_MaximumTimestep);
    TRANSFER(m_MaximumParticleTimestep);

    TRANSFER(m_LastSyncEnd);

    TRANSFER(m_DynamicTime.m_CurFrameTime);
    TRANSFER(m_DynamicTime.m_LastFrameTime);
    TRANSFER(m_DynamicTime.m_CurFrameUnscaledTime);
    TRANSFER(m_DynamicTime.m_DeltaTime);
    TRANSFER(m_DynamicTime.m_UnscaledDeltaTime);
    TRANSFER(m_DynamicTime.m_SmoothDeltaTime);
    TRANSFER(m_DynamicTime.m_SmoothingWeight);
    TRANSFER(m_DynamicTime.m_InvDeltaTime);

    TRANSFER(m_FixedTime.m_CurFrameTime);
    TRANSFER(m_FixedTime.m_LastFrameTime);
    TRANSFER(m_FixedTime.m_CurFrameUnscaledTime);
    TRANSFER(m_FixedTime.m_DeltaTime);
    TRANSFER(m_FixedTime.m_UnscaledDeltaTime);
    TRANSFER(m_FixedTime.m_SmoothDeltaTime);
    TRANSFER(m_FixedTime.m_SmoothingWeight);
    TRANSFER(m_FixedTime.m_InvDeltaTime);

    TRANSFER(m_ActiveTime.m_CurFrameTime);
    TRANSFER(m_ActiveTime.m_LastFrameTime);
    TRANSFER(m_ActiveTime.m_CurFrameUnscaledTime);
    TRANSFER(m_ActiveTime.m_DeltaTime);
    TRANSFER(m_ActiveTime.m_UnscaledDeltaTime);
    TRANSFER(m_ActiveTime.m_SmoothDeltaTime);
    TRANSFER(m_ActiveTime.m_SmoothingWeight);
    TRANSFER(m_ActiveTime.m_InvDeltaTime);
}

IMPLEMENT_REGISTER_CLASS(TimeManager, 5);
IMPLEMENT_OBJECT_SERIALIZE(TimeManager);

IMPLEMENT_STATE_SYNCHRONIZE(TimeManager)
GET_MANAGER(TimeManager)
GET_MANAGER_PTR(TimeManager)
