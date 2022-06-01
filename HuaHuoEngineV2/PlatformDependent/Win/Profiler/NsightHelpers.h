#pragma once

#if ENABLE_PROFILER
// This function is slow, and should be once (at startup) to determine if application was launched under Nsight Graphics for GPU capture
bool WasAppLaunchedUnderNsightGpuCapture();
#endif
