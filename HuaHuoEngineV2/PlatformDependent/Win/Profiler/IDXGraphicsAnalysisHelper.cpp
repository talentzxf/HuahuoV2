#include "IDXGraphicsAnalysisHelper.h"

bool GraphicsAnalysisInterfaceExists(PFN_DXGI_GET_DEBUG_INTERFACE_1 getDebugInterface, IDXGraphicsAnalysis** graphicsAnalysis)
{
    // Get debug interface
    HRESULT hr = getDebugInterface(0, IID_PPV_ARGS(graphicsAnalysis));

    // hr will be E_NOINTERFACE if not attached for GPU capture
    return hr != E_NOINTERFACE;
}

bool GetDXGraphicsAnalysisInterface(IDXGraphicsAnalysis** graphicsAnalysis)
{
    bool interfaceExists = false;
#if PLATFORM_WINRT
    interfaceExists = GraphicsAnalysisInterfaceExists(DXGIGetDebugInterface1, graphicsAnalysis);
#else

    HMODULE dxgi = LoadLibrary("dxgi.dll");
    if (dxgi)
    {
        PFN_DXGI_GET_DEBUG_INTERFACE_1 func = (PFN_DXGI_GET_DEBUG_INTERFACE_1)GetProcAddress(dxgi, "DXGIGetDebugInterface1");
        if (func)
        {
            interfaceExists = GraphicsAnalysisInterfaceExists(func, graphicsAnalysis);
        }

        FreeLibrary(dxgi);
    }
#endif

    return interfaceExists;
}
