//
// Created by VincentZhang on 5/13/2022.
//

#include "GfxDevice.h"
#include "Logging/LogAssert.h"
#include <cstdlib>

static GfxDevice* g_MainGfxDevice = NULL;

GfxDevice& GetGfxDevice()
{
//    __FAKEABLE_FUNCTION__(GetGfxDevice, ());
#if GFX_TLS_DEVICE
    Assert(g_ThreadedGfxDevice);
    return *g_ThreadedGfxDevice;
#elif ENABLE_MULTITHREADED_CODE
    AssertMsg(CurrentThread::IsMainThread(), "GetGfxDevice() should only be called from main thread");
#endif
    Assert(g_MainGfxDevice);
    return *g_MainGfxDevice;
}

bool IsGfxDevice()
{
    return g_MainGfxDevice != NULL;
}


GfxDevice::GfxDevice()
{
}

GfxDevice::~GfxDevice()
{
//    OnDelete();
}

void SetGfxDevice(GfxDevice* device)
{
#if GFX_TLS_DEVICE
    g_MainGfxDevice = device;
    g_ThreadedGfxDevice = device;
#else
    g_MainGfxDevice = device;
#endif
}
