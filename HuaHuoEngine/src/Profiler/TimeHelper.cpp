//
// Created by VincentZhang on 5/15/2022.
//

#include "TimeHelper.h"
#include "C/Baselib_Timer.h"

#if !WEB_ENV
namespace Common
{
    static double Baselib_Timer_GetTimeSinceStartupInSeconds();
}

double Baselib_Timer_GetTimeSinceStartupInSeconds(){
    return Common::Baselib_Timer_GetTimeSinceStartupInSeconds();
}
#endif

// This construct is here to support engine shutdown and reinitialization without binary unload,
// as well as embedding scenarios, where binary load time and engine initialization time can
// be different.
struct GetTimeSinceStartupHelper
{
    GetTimeSinceStartupHelper() { startTime = Baselib_Timer_GetTimeSinceStartupInSeconds(); }
    double GetTimeSinceStartup() { return Baselib_Timer_GetTimeSinceStartupInSeconds() - startTime; }
    double startTime;
};

static GetTimeSinceStartupHelper s_startupHelper;

double GetTimeSinceStartup()
{
    //__FAKEABLE_FUNCTION__(GetTimeSinceStartup, ());
    GetTimeSinceStartupHelper* startupHelper = &s_startupHelper;//.EnsureInitialized();
    return startupHelper->GetTimeSinceStartup();
}