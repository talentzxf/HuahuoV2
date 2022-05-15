#pragma once

#include "Source/Baselib_InlineImplementation.h"
#include "Include/Cpp/Atomic.h"
#include "Include/C/Baselib_Timer.h"

#import <mach/mach_time.h>
#include <sys/sysctl.h>
#include <stdio.h>

namespace DarwinApi
{
    BASELIB_INLINE_IMPL Baselib_Timer_TickToNanosecondConversionRatio Baselib_Timer_GetTicksToNanosecondsConversionRatio()
    {
        mach_timebase_info_data_t timebaseInfo;
        mach_timebase_info(&timebaseInfo);
        return Baselib_Timer_TickToNanosecondConversionRatio { timebaseInfo.numer, timebaseInfo.denom };
    }

    BASELIB_INLINE_IMPL Baselib_Timer_Ticks Baselib_Timer_GetHighPrecisionTimerTicks()
    {
        return mach_absolute_time();
    }
}


namespace detail
{
    static baselib::atomic<timeval> cachedTimeAtBoot;

    static inline timeval GetWallClockTimeAtBoot(const timeval& now)
    {
        // The value of wall clock time at boot changes if the clock is altered (e.g. user change or NTP)
        //
        // Querying is costly, so we cache the value. If this function is
        // called e.g. at least once every frame, we will in most cases only update
        // the cache if the system clock has been altered.


        // Use relaxed atomics to ensure we don't have torn read / writes
        static baselib::atomic<timeval> s_lastTime;
        timeval lastTime = s_lastTime.load(baselib::memory_order_relaxed);

        timeval timeAtBoot;

        timeval difference;
        timersub(&now, &lastTime, &difference);
        // Overflow when difference is bigger than about 292 000 years. ;)
        const int64_t diffMicroseconds = (difference.tv_sec * Baselib_MicrosecondsPerSecond) + difference.tv_usec;

        // We will hit this if the system clock has changed, or if this function hasn't been called for 33 ms.
        const long maxForwardJump_us = 33000;

        const bool wallClockHasJumpedBackward = diffMicroseconds < 0;
        const bool wallClockHasJumpedForward = diffMicroseconds > maxForwardJump_us;
        const bool cachedBootTimeNeedsUpdating = wallClockHasJumpedBackward || wallClockHasJumpedForward;
        if (cachedBootTimeNeedsUpdating)
        {
            size_t size = sizeof(timeAtBoot);
            sysctl((int[]) { CTL_KERN, KERN_BOOTTIME }, 2, &timeAtBoot, &size, NULL, 0);
            cachedTimeAtBoot.store(timeAtBoot, baselib::memory_order_relaxed);
        }
        else
        {
            timeAtBoot = cachedTimeAtBoot.load(baselib::memory_order_relaxed);
        }

        s_lastTime.store(now, baselib::memory_order_relaxed);

        return timeAtBoot;
    }

    static double SecondsSinceBoot()
    {
        // clock_gettime is not available in iOS 9, hence we cannot use it.
        // The value of KERN_BOOTTIME will change if the system clock is changed,
        // however, so will the time of gettimeofday, and hence the difference
        // will still be valid.
        timeval now;
        gettimeofday(&now, NULL);

        const timeval timeAtBoot = GetWallClockTimeAtBoot(now);

        timeval difference;
        timersub(&now, &timeAtBoot, &difference);

        return difference.tv_sec + difference.tv_usec * 1e-6;
    }

    // This construct is here to ensure that Baselib_Timer_GetTimeSinceStartupInSeconds is called during static initialization
    static struct Darwin_Timer_StartupTimeInitializer
    {
        Darwin_Timer_StartupTimeInitializer()
        {
            Baselib_Timer_GetTimeSinceStartupInSeconds();
        }
    } darwin_Timer_StartupTimeInitializer;
}

#define DONT_STRIP(address) UNUSED(address)

namespace DarwinApi
{
    BASELIB_INLINE_IMPL double Baselib_Timer_GetTimeSinceStartupInSeconds()
    {
        DONT_STRIP(::detail::darwin_Timer_StartupTimeInitializer);
        const double secondsSinceBoot = ::detail::SecondsSinceBoot();
        // The reason for having this as a scoped static and not a file static is to avoid the ordering
        // problems of file static initialization.
        static const double secondsSinceBootAtStartup = secondsSinceBoot;
        return secondsSinceBoot - secondsSinceBootAtStartup;
    }
}
