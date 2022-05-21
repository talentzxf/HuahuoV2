//
// Created by VincentZhang on 5/13/2022.
//

#include "GfxDevice.h"
#include "Logging/LogAssert.h"
#include "Job/Jobs.h"
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


GfxDevice::GfxDevice(MemLabelRef label) :
        m_MemoryLabel(label)
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

void GfxDevice::ExecuteAsync(int count, GfxDeviceAsyncCommand::Func* func, GfxDeviceAsyncCommand::ArgScratch** scratches, const GfxDeviceAsyncCommand::Arg* arg, const JobFence& depends)
{
#if ENABLE_EMULATED_JOB_ASSERT
    g_ThreadRunningEmulatedJob = true;
#endif

    SyncFenceNoClear(depends);

    for (int i = 0; i < count; i++)
    {
        GfxDeviceAsyncCommand::ArgScratch* scratch = scratches[i];
        scratch->device = this;
        func(scratch, arg);
        scratch->ThreadedCleanup();
    }

#if ENABLE_EMULATED_JOB_ASSERT
    g_ThreadRunningEmulatedJob = false;
#endif
}