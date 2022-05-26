//
// Created by VincentZhang on 5/13/2022.
//

#include "GfxDevice.h"
#include "Logging/LogAssert.h"
#include "Job/Jobs.h"
#include <cstdlib>
#include "Graphics/RenderSurface.h"

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

RenderSurfaceBase* GfxDevice::AllocRenderSurface(bool colorSurface)
{
    const size_t memSz = RenderSurfaceStructMemorySize(colorSurface);
    RenderSurfaceBase* ret = (RenderSurfaceBase*)HUAHUO_MALLOC_ALIGNED(kMemGfxDevice, memSz, 16);
    ::memset(ret, 0x00, memSz);
    ret->samples = 1;
    ret->colorSurface = colorSurface;
    return ret;
}

void GfxDevice::DeallocRenderSurface(RenderSurfaceBase* rs)
{
#if DEBUGMODE
    const size_t memSz = RenderSurfaceStructMemorySize(rs->colorSurface);
    ::memset(rs, 0xFD, memSz);
#endif

    HUAHUO_FREE(kMemGfxDevice, rs);
}

void GfxDevice::DestroyRenderSurface(RenderSurfaceHandle& rsh)
{
    RenderSurfaceBase* rs = rsh.object;

    // lots of GfxDevice implementations had this if, so we keep it
    if (rs == 0)
        return;

    // we disallow destroying backbuffer through high level interface
    if (rs->backBuffer)
        return;

//    // TODO@MT: Waits for all async render jobs to complete before allowing the texture to be deleted.
//    GfxDeviceWaitForAllRenderJobsToComplete();

//    bool unused = (rs->flags & kSurfaceCreateNeverUsed) || (!rs->colorSurface && (rs->flags & kSurfaceCreateNoDepth));
//    if ((rs->flags & kSurfaceCreateDynamicScale) && !unused)
//    {
//        ScalableBufferManager::GetInstance().UnregisterRenderSurface(rs, false);
//    }

    DestroyRenderSurfacePlatform(rs);
    DeallocRenderSurface(rs);
    rsh.object = 0;
}
