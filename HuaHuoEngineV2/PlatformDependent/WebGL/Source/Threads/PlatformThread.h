#pragma once

#if SUPPORT_THREADS

#include "Runtime/Threads/Posix/PlatformThread.h"

#include <emscripten/threading.h>

inline void YieldProcessorJS()
{
    // temporary yield implementation
    if (emscripten_is_main_browser_thread())
        emscripten_main_thread_process_queued_calls();
}

#endif // SUPPORT_THREADS
