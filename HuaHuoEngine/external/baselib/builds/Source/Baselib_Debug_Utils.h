#pragma once

#include "Include/Cpp/CountdownTimer.h"
#include <mutex>

using Baselib_Debug_IsDebuggerAttachedFuncPtr = bool(*)();

namespace detail
{
    static bool cachedState;
    static baselib::CountdownTimer cacheTimeout(baselib::CountdownTimer::InitializeExpired());
    static std::mutex guard;
}

// In cases if native implementation of IsDebuggerAttached is expensive,
// we would like to cache results of it every X seconds, in this case 1 second.
static inline bool Baselib_Debug_Cached_IsDebuggerAttached(Baselib_Debug_IsDebuggerAttachedFuncPtr func)
{
    BaselibAssert(func != nullptr);

    std::lock_guard<std::mutex> guard(detail::guard);

    // Check if least than one second passed, then return cached state.
    if (!detail::cacheTimeout.TimeoutExpired())
        return detail::cachedState;

    detail::cachedState = func();
    detail::cacheTimeout = baselib::CountdownTimer::StartNew(std::chrono::seconds(1));

    return detail::cachedState;
}
