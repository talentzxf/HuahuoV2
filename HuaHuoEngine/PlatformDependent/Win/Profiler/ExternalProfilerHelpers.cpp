#include "ExternalProfilerHelpers.h"

#if ENABLE_PROFILER
#include "PixHelpers.h"
#include "NsightHelpers.h"

bool WasAppLaunchedByExternalGPUDebugger()
{
    return (WasAppLaunchedUnderPixGpuCapture() || WasAppLaunchedUnderNsightGpuCapture());
}

#endif
