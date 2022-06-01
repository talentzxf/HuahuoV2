//
// Created by VincentZhang on 5/15/2022.
//

#include "TimeHelper.h"
#include "C/Baselib_Timer.h"
#include "Cpp/Time.h"
#include "Cpp/Stopwatch.h"

#if !WEB_ENV

#include <algorithm>
#include <windef.h>
#include <WinBase.h>

#ifndef DONT_STRIP
#define DONT_STRIP(address) UNUSED(address)
#endif

namespace WinApi
{
    static inline Baselib_Timer_TickToNanosecondConversionRatio Baselib_Timer_GetTicksToNanosecondsConversionRatio()
    {
        LARGE_INTEGER frequency = {0};
        BOOL result = QueryPerformanceFrequency(&frequency);
        BaselibAssert(result == TRUE);
        return Baselib_Timer_TickToNanosecondConversionRatio { Baselib_NanosecondsPerSecond, static_cast<uint64_t>(frequency.QuadPart) };
    }

    static inline Baselib_Timer_Ticks Baselib_Timer_GetHighPrecisionTimerTicks()
    {
        LARGE_INTEGER start = {0};
        BOOL result = QueryPerformanceCounter(&start);
        BaselibAssert(result == TRUE);
        return start.QuadPart;
    }
}

Baselib_Timer_TickToNanosecondConversionRatio Baselib_Timer_GetTicksToNanosecondsConversionRatio()
{
    return WinApi::Baselib_Timer_GetTicksToNanosecondsConversionRatio();
}


Baselib_Timer_Ticks Baselib_Timer_GetHighPrecisionTimerTicks()
{
    return WinApi::Baselib_Timer_GetHighPrecisionTimerTicks();
}

namespace Common
{
    static double Baselib_Timer_GetTimeSinceStartupInSeconds()
    {
        // DONT_STRIP(detail::commonStartupTimeInitializer);
        // The reason for having this as a scoped static and not a file static is to avoid the ordering
        // problems of file static initialization.
        static const baselib::Stopwatch s_Stopwatch = baselib::Stopwatch::StartNew();
        return std::chrono::duration<double>(s_Stopwatch.GetElapsedTime()).count();
    }
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