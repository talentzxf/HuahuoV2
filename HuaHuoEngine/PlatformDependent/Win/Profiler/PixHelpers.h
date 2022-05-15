#pragma once

#include "Windows.h"
#include "evntprov.h"


// Timing captures

// Pointer to callback function, called from a system thread.
//
// PENABLEDCALLBACK defined in evntprov.h
//
// IsEnabled > 0 means a timing capture has started.
// CallbackContext is user data passed to callback. Caller is responsible for ensuring correct barriers and thread safety as function is called
// in another thread.
//
// All other paramaters are currently unused.
typedef PENABLECALLBACK PixCaptureStateCallbackFuncPtr;

// Register a callback directly to PIX when it attaches and detaches for CPU captures.
//
// Returns handle to callback, which caller must unregister.
REGHANDLE PixCaptureStateCallbackRegister(PixCaptureStateCallbackFuncPtr callback, void* callbackContext);

// Unregister a callback, using handle provided above
void PixCaptureStateCallbackUnregister(REGHANDLE callbackHandle);

// GPU captures
#if ENABLE_PROFILER
// This function is slow, and should be once (at startup) to determine if application was launched under PIX for GPU capture
bool WasAppLaunchedUnderPixGpuCapture();
#endif
