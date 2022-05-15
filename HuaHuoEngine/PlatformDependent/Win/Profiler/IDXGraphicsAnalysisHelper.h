#ifndef __DX_GRAPHICS_ANALYZER_HELPER__
#define __DX_GRAPHICS_ANALYZER_HELPER__

#include "Windows.h"
#include "evntprov.h"
#include <dxgi1_3.h>

// This is declared in DXGraphicsAnalysis.h but hidden for Win7 builds.
interface DECLSPEC_UUID("9f251514-9d4d-4902-9d60-18988ab7d4b5") DECLSPEC_NOVTABLE IDXGraphicsAnalysis : public IUnknown
{
    STDMETHOD_(void, BeginCapture)() PURE;
    STDMETHOD_(void, EndCapture)() PURE;
};

// This function determines if the IDXGraphicsAnalysis exists - which only happens when the app was launched by an external GPU debugger like PIX.
// for PIX GPU captures
typedef HRESULT(WINAPI* PFN_DXGI_GET_DEBUG_INTERFACE_1)(UINT flags, REFIID ridd, void** pDebug);
bool GraphicsAnalysisInterfaceExists(PFN_DXGI_GET_DEBUG_INTERFACE_1 getDebugInterface, IDXGraphicsAnalysis** graphicsAnalysis);
bool GetDXGraphicsAnalysisInterface(IDXGraphicsAnalysis** graphicsAnalysis);
#endif //__DX_GRAPHICS_ANALYZER_HELPER__
