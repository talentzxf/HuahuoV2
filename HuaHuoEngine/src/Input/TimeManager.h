#pragma once

#include "Modules/ExportModules.h"
#include "BaseClasses/GameManager.h"
#include "Serialize/StateSynchronizeDefines.h"

class TimeManager;

TimeManager* GetTimeManagerPtr();
EXPORT_COREMODULE TimeManager& GetTimeManager();

// The time since startup of the player.
double GetTimeSinceStartup();

/*
Time requirements:
- Delta time shall never be less than kMinimumDeltaTime (So people dont get nans when dividing by delta time)
- The first frame when starting up is always at zero and kStartupDeltaTime delta time.
- delta time is clamped to a maximum of kMaximumDeltaTime
- adding up delta time always gives you the current time

- after loading a scene or pausing, the first frames delta time is kStartupDeltaTime
- fixed delta time is always smaller or equal to dynamic time

- When starting up there is always one physics frame before the first display!
*/

class TimeManager : public GlobalGameManager
{
    REGISTER_CLASS(TimeManager);
    DECLARE_OBJECT_SERIALIZE();
    DECLARE_STATE_SYNCHRONIZE(TimeManager);
public:

    struct TimeHolder
    {
        TimeHolder();
        double m_CurFrameTime;
        double m_LastFrameTime;
        double m_CurFrameUnscaledTime;
        float m_DeltaTime;
        float m_UnscaledDeltaTime;
        float m_SmoothDeltaTime;
        float m_SmoothingWeight;
        float m_InvDeltaTime;
    };

    TimeManager(/*MemLabelId label,*/ ObjectCreationMode mode);
    // ~TimeManager (); declared-by-macro

    virtual void Reset() override;


    virtual void AwakeFromLoad(AwakeFromLoadMode mode) override;
    // virtual void CheckConsistency() override;

    virtual void Update(double timeSinceStartup);

    void ResetTime(bool isPlayMode);

    void SetPause(bool pause);


    //  void SetMinimumDeltaTime (float c) { m_MinimumDeltaTime = c; }

    inline double   GetCurTime()  const        { return m_ActiveTime.m_CurFrameTime; }
    inline double   GetTimeSinceSceneLoad()  const     { return m_ActiveTime.m_CurFrameTime + m_SceneLoadOffset; }
    inline float    GetDeltaTime() const       { return m_ActiveTime.m_DeltaTime; }
    inline float    GetSmoothDeltaTime()  const    { return m_ActiveTime.m_SmoothDeltaTime; }

    inline float    GetInvDeltaTime() const        { return m_ActiveTime.m_InvDeltaTime; }
    inline int64_t  GetFrameCount() const      { return m_FrameCount; }

    inline int      GetRenderFrameCount() const { return m_RenderFrameCount; }

    inline double   GetZeroTime() const { return m_ZeroTime; }
    inline double   GetRealZeroTime() const { return m_RealZeroTime; }

    inline float    GetFixedDeltaTime() { return m_FixedTime.m_DeltaTime; }
    inline float    GetInvFixedDeltaTime() { return m_FixedTime.m_InvDeltaTime; }
    void            SetFixedDeltaTime(float fixedStep);
    inline double   GetFixedTime() { return m_FixedTime.m_CurFrameTime; }

    inline double   GetDynamicTime() { return m_DynamicTime.m_CurFrameTime; }

    inline float    GetMaximumDeltaTime() { return m_MaximumTimestep; }
    void            SetMaximumDeltaTime(float maxStep);

    inline float    GetMaximumParticleDeltaTime() { return m_MaximumParticleTimestep; }
    void            SetMaximumParticleDeltaTime(float maxStep);

    inline double   GetUnscaledTime() const { return m_ActiveTime.m_CurFrameUnscaledTime; }
    inline double   GetUnscaledDeltaTime() const { return m_ActiveTime.m_UnscaledDeltaTime; }

    inline double   GetDynamicUnscaledTime() const { return m_DynamicTime.m_CurFrameUnscaledTime; }
    inline double   GetDynamicUnscaledDeltaTime() const { return m_DynamicTime.m_UnscaledDeltaTime; }

    inline double   GetFixedUnscaledTime() const { return m_FixedTime.m_CurFrameUnscaledTime; }
    inline double   GetFixedUnscaledDeltaTime() const { return m_FixedTime.m_UnscaledDeltaTime; }

    /// Steps the fixed time step until the dynamic time is exceeded.
    /// Returns true if it has to be called again to reach the dynamic time
    bool StepFixedTime();
    bool HasFixedTimeStep();

    void SetTimeManually(bool manually) { m_SetTimeManually = manually; }
    void SetTime(double time);
    void SetTimeScale(float scale);
    float GetTimeScale() { return m_TimeScale; }

    inline bool IsUsingFixedTimeStep() const { return m_UseFixedTimeStep; }

    void DidFinishLoadingScene();

    void SetCaptureDeltaTime(float step) { m_CaptureDeltaTime = step; }
    float GetCaptureDeltaTime() { return m_CaptureDeltaTime; }

    double GetRealtime();

    enum SyncPosition
    {
        BeforePlayerLoop,
        AfterPlayerLoop,
        BeforeWaitForPresent,
        AfterWaitForPresent,
        InUpdateTime
    };

    void Sync(SyncPosition syncPosition);

private:

    enum
    {
        kNumFramesToKeepForVSyncTimes = 100
    };
    TimeHolder  m_FixedTime;
    TimeHolder  m_DynamicTime;
    TimeHolder  m_ActiveTime;

    bool        m_FirstFrameAfterReset;
    bool        m_FirstFrameAfterPause;
    bool        m_FirstFixedFrameAfterReset;

    int64_t     m_FrameCount;
    static_assert(sizeof(decltype(m_FrameCount)) * CHAR_BIT == 64, "Will overflow at high frame rates: case 1201708");
    int         m_RenderFrameCount;
    int         m_CullFrameCount;
    float       m_CaptureDeltaTime;
    double      m_ZeroTime;
    double      m_RealZeroTime;
    double      m_SceneLoadOffset;

    bool        m_SetTimeManually;
    bool        m_UseFixedTimeStep;
    float       m_TimeScale;///< How fast compared to the real time does the game time progress (1.0 is realtime, .5 slow motion) range { 0, 100 }
    float       m_MaximumTimestep;
    float       m_MaximumParticleTimestep;

    double      m_LastSyncEnd;
    double      m_FrameSyncEnds[kNumFramesToKeepForVSyncTimes];

    // See GetSyncBehaviour implementation for an explaination of the different modes.
    enum SyncBehaviour
    {
        NoSync,
        SyncOutsideOfPlayerLoop,
        SyncInUpdateTime,
        SyncInWaitForPresent
    };

    SyncBehaviour GetSyncBehaviour() const;
    void StartSyncFrame();
    void EndSyncFrame(SyncBehaviour behavior);
    double GetTargetFrameTime() const;
};

template<TimeManager::SyncPosition positionBefore, TimeManager::SyncPosition positionAfter>
struct SyncTimeManagerHelper
{
    inline SyncTimeManagerHelper()
    {
        GetTimeManager().Sync(positionBefore);
    }

    inline ~SyncTimeManagerHelper()
    {
        GetTimeManager().Sync(positionAfter);
    }
};

inline double   GetCurTime()   { return GetTimeManager().GetCurTime(); }
inline float    GetDeltaTime()         { return GetTimeManager().GetDeltaTime(); }
inline float    GetInvDeltaTime()  { return GetTimeManager().GetInvDeltaTime(); }
float CalcInvDeltaTime(float dt);

const float kMinimumDeltaTime = 0.00001F;
