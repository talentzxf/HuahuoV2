#pragma once

// Small things to start/stop an external profiler from code.
// Normally should be disabled!

#define EXTERNAL_PROFILER_USE_CODE_ANALYST 0
#define EXTERNAL_PROFILER_USE_VTUNE 0


#if EXTERNAL_PROFILER_USE_CODE_ANALYST || EXTERNAL_PROFILER_USE_VTUNE
#define EXTERNAL_PROFILER_ENABLE 1
#else
#define EXTERNAL_PROFILER_ENABLE 0
#endif

#if EXTERNAL_PROFILER_ENABLE
void ExternalProfilerStart();
void ExternalProfilerStop();
#endif
