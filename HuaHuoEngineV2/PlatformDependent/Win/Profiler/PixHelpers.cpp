#include "UnityPrefix.h"
#include "PixHelpers.h"
#include <wrl/client.h>
#include "IDXGraphicsAnalysisHelper.h"

REGHANDLE PixCaptureStateCallbackRegister(PixCaptureStateCallbackFuncPtr callback, void* callbackContext)
{
    // GUID was provided by PIX team as this feature is not supported by an API yet.
    REGHANDLE eventHandle = 0;
    GUID providerId = { 0x8819ca17, 0x7faa, 0x4184, { 0xaf, 0xb7, 0xa4, 0x12, 0x0c, 0xa7, 0x93, 0xcd } };
    ULONG r = EventRegister(&providerId, callback, callbackContext, &eventHandle);
    DebugAssertMsg(r == ERROR_SUCCESS, "Unable to register PIX event.");

    return eventHandle;
}

void PixCaptureStateCallbackUnregister(REGHANDLE handle)
{
    ULONG r = EventUnregister(handle);
    DebugAssertMsg(r == ERROR_SUCCESS, "Unable to unregister PIX event");
}

// GPU captures

Microsoft::WRL::ComPtr<IDXGraphicsAnalysis> graphicsAnalysis;

bool GraphicsAnalysisInterfaceExists(PFN_DXGI_GET_DEBUG_INTERFACE_1 getDebugInterface)
{
    return GraphicsAnalysisInterfaceExists(getDebugInterface, graphicsAnalysis.GetAddressOf());
}

#if ENABLE_PROFILER
// Method taken from https://devblogs.microsoft.com/pix/programmatic-capture/
bool WasAppLaunchedUnderPixGpuCapture()
{
    if (graphicsAnalysis.Get() != NULL)
        return true;

    return GetDXGraphicsAnalysisInterface(graphicsAnalysis.GetAddressOf());
}

void BeginPIXGPUCapture()
{
    if (WasAppLaunchedUnderPixGpuCapture())
    {
        graphicsAnalysis->BeginCapture();
    }
}

void EndPIXGPUCapture()
{
    if (WasAppLaunchedUnderPixGpuCapture())
    {
        graphicsAnalysis->EndCapture();
    }
}

bool IsPIXAttached()
{
    return WasAppLaunchedUnderPixGpuCapture();
}

#endif
