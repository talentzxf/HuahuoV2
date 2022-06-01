#pragma once

#include "Include/Cpp/Stopwatch.h"

namespace Common
{
    static double Baselib_Timer_GetTimeSinceStartupInSeconds();

namespace detail
{
    // This construct is here to ensure that GetTimeSinceStartup_Common is called during static initialization
    static struct CommonStartupTimeInitializer
    {
        CommonStartupTimeInitializer()
        {
            Common::Baselib_Timer_GetTimeSinceStartupInSeconds();
        }
    } commonStartupTimeInitializer;
}
}

#ifndef DONT_STRIP
#define DONT_STRIP(address) UNUSED(address)
#endif

namespace Common
{
    static double Baselib_Timer_GetTimeSinceStartupInSeconds()
    {
        DONT_STRIP(detail::commonStartupTimeInitializer);
        // The reason for having this as a scoped static and not a file static is to avoid the ordering
        // problems of file static initialization.
        static const baselib::Stopwatch s_Stopwatch = baselib::Stopwatch::StartNew();
        return std::chrono::duration<double>(s_Stopwatch.GetElapsedTime()).count();
    }
}
