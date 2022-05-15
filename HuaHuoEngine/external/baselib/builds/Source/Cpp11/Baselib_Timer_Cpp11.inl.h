#pragma once

#include "Source/Baselib_InlineImplementation.h"
#include "Include/C/Baselib_Timer.h"
#include "Include/Cpp/CountdownTimer.h"

#include <chrono>
#include <thread>

namespace Cpp11Api
{
    BASELIB_INLINE_IMPL void Baselib_Timer_WaitForAtLeast(uint32_t timeInMilliseconds)
    {
        baselib::timeout_ms timeLeft(timeInMilliseconds);
        const baselib::CountdownTimer timer = baselib::CountdownTimer::StartNew(timeLeft);
        do
        {
            std::this_thread::sleep_for(timeLeft);
            timeLeft = timer.GetTimeLeftInMilliseconds();
        }
        while (timeLeft > baselib::timeout_ms::zero());
    }
}
