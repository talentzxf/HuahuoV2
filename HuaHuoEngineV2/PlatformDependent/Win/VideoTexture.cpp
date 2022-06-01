#include "UnityPrefix.h"
#include "Runtime/Graphics/Image.h"
#include "Runtime/Misc/HasNoReferencesAttribute.h"
#include "Runtime/Video/VideoTexture.h"
#include "Runtime/Input/TimeManager.h"
#include "Runtime/GfxDevice/GfxDevice.h"
#include "Runtime/Profiler/Profiler.h"

#if ENABLE_WEBCAM

#include "External/videoInput/videoInput.h"

REGISTER_TYPE_ATTRIBUTES(WebCamTexture, (HasNoReferences, ()));
IMPLEMENT_REGISTER_CLASS(WebCamTexture, 158);

PROFILER_INFORMATION(gWebcamUpdate, "Webcam.Update", kProfilerOther);
PROFILER_INFORMATION(gWebcamUploadTexture, "Webcam.UploadTexture", kProfilerOther);

static videoInput* g_VideoInput;
static int g_nDevices = 0;

struct PlatformDependentWebCamTextureData
{
    int     m_Device;
};

void WebCamTexture::InitializeClass()
{
    g_VideoInput = UNITY_NEW(videoInput, kMemWebCam);
}

void WebCamTexture::CleanupClass()
{
    UNITY_DELETE(g_VideoInput, kMemWebCam);
}

void WebCamTexture::InitDeviceList()
{
    g_nDevices = g_VideoInput->listDevices();
}

void WebCamTexture::GetDeviceNames(MonoWebCamDevices& devices,
    bool forceUpdate)
{
    if (g_nDevices == 0 || forceUpdate)
        InitDeviceList();

    devices.clear();
    for (int i = 0; i < g_nDevices; i++)
    {
        MonoWebCamDevice device;
        device.name = scripting_string_new(g_VideoInput->getDeviceName(i));
        device.flags = kWebCamFrontFacing;
        EnsureUniqueName(device, devices);
        devices.push_back(device);
    }
}

void WebCamTexture::Create()
{
    m_VT = new PlatformDependentWebCamTextureData();

    m_VT->m_Device = GetDeviceIdFromDeviceList(m_DeviceName);
    if (m_VT->m_Device == -1)
        return;

    m_IsCreated = false;
    if (m_RequestedFPS > 0)
    {
        g_VideoInput->setIdealFramerate(m_VT->m_Device, m_RequestedFPS);
    }
    bool bOk = g_VideoInput->setupDevice(m_VT->m_Device, m_RequestedWidth, m_RequestedHeight);

    if (bOk == true)
    {
        m_IsCreated = true;
        InitVideoMemory(g_VideoInput->getWidth(m_VT->m_Device), g_VideoInput->getHeight(m_VT->m_Device));
    }
    else
    {
        m_IsCreated = false;
        m_VT->m_Device = -1;
    }
}

void WebCamTexture::Update()
{
    PROFILER_AUTO(gWebcamUpdate);
    if (m_IsCreated)
    {
        int result = g_VideoInput->isFrameNew(m_VT->m_Device);
        if (result)
        {
            if (result == -1)
            {
                ErrorString("Error capturing camera feed. Maybe the camera has been disconnected?");
                Stop();
            }
            else
            {
                PROFILER_AUTO(gWebcamUploadTexture);
                g_VideoInput->getPixels(m_VT->m_Device, (UInt8*)GetImageBuffer(), GetDataWidth() * 4);
                UploadTextureData();
            }
        }
    }
}

void WebCamTexture::Play()
{
    if (!m_IsCreated)
        Create();
    if (m_IsCreated)
        BaseVideoTexture::Play();
}

void WebCamTexture::Pause()
{
    BaseVideoTexture::Pause();
}

void WebCamTexture::Stop()
{
    if (m_VT)
    {
        g_VideoInput->stopDevice(m_VT->m_Device);
        m_IsCreated = false;
        ReleaseVideoMemory();
    }
    BaseVideoTexture::Stop();
}

void WebCamTexture::ThreadedCleanup()
{
    if (m_VT)
        delete m_VT;
}

#endif
