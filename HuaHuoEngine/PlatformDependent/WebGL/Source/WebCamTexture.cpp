#include "UnityPrefix.h"

#if ENABLE_WEBCAM

#include "Runtime/Misc/HasNoReferencesAttribute.h"
#include "Runtime/Scripting/ScriptingUtility.h"
#include "Runtime/Video/VideoTexture.h"
#include "JSBridge.h"

struct PlatformDependentWebCamTextureData
{
    int deviceId;
};

REGISTER_TYPE_ATTRIBUTES(WebCamTexture, (HasNoReferences, ()));
IMPLEMENT_REGISTER_CLASS(WebCamTexture, 158);

void WebCamTexture::GetDeviceNames(MonoWebCamDevices& devices,
    bool forceUpdate)
{
    devices.clear();

    if (JS_WebCam_IsSupported())
    {
        UInt32 numDevices = JS_WebCamVideo_GetNumDevices();

        for (UInt32 index = 0; index < numDevices; index++)
        {
            MonoWebCamDevice device;

            core::string deviceName = GetStringFromJS(JS_WebCamVideo_GetDeviceName, index);
            device.name = scripting_string_new(deviceName.c_str());

            EnsureUniqueName(device, devices);
            devices.push_back(device);
        }
    }
}

void WebCamTexture::Create()
{
    if (JS_WebCam_IsSupported())
    {
        int deviceId = GetDeviceIdFromDeviceList(m_DeviceName);
        if (deviceId != -1)
        {
            JS_WebCamVideo_Start(deviceId);

            if (m_VT == NULL)
                m_VT = new PlatformDependentWebCamTextureData();

            m_VT->deviceId = deviceId;

            m_IsCreated = true;
        }
    }
}

void WebCamTexture::Update()
{
    if (IsPlaying() && JS_WebCamVideo_CanPlay(m_VT->deviceId))
    {
        UInt32 nativeVideoWidth = JS_WebCamVideo_GetNativeWidth(m_VT->deviceId);
        UInt32 nativeVideoHeight = JS_WebCamVideo_GetNativeHeight(m_VT->deviceId);

        AssertMsg(nativeVideoWidth > 0 && nativeVideoHeight > 0, "Video cannot be played yet");

        // Use requested texture size, otherwise use native one
        UInt32 destVideoWidth = m_RequestedWidth ? m_RequestedWidth : nativeVideoWidth;
        UInt32 destVideoHeight = m_RequestedHeight ? m_RequestedHeight : nativeVideoHeight;

        UInt32* pixels = (UInt32*)GetImageBuffer();
        if (!pixels)
        {
            InitVideoMemory(destVideoWidth, destVideoHeight);

            pixels = (UInt32*)GetImageBuffer();
        }

        if (pixels)
        {
            JS_WebCamVideo_GrabFrame(m_VT->deviceId, (void*)pixels, destVideoWidth, destVideoHeight);

            GraphicsFormat texFormat = GetGraphicsFormat(GetBufferTextureFormat(), kTexColorSpaceLinear);
            ImageReference ref(GetDataWidth(), GetDataHeight(), GetRowSize(GetDataWidth(), texFormat), texFormat, pixels);
            ref.FlipImageY();

            UploadTextureData();
        }
    }
}

void WebCamTexture::Play()
{
    if (!m_IsCreated)
        Create();

    if (m_IsCreated)
    {
        BaseVideoTexture::Play();
    }
}

void WebCamTexture::Pause()
{
    AssertMsg(m_IsCreated, "WebCam not initialized yet.");

    BaseVideoTexture::Pause();
}

void WebCamTexture::Stop()
{
    if (m_IsCreated)
    {
        m_IsCreated = false;

        JS_WebCamVideo_Stop(m_VT->deviceId);

        ReleaseVideoMemory();
    }

    BaseVideoTexture::Stop();
}

void WebCamTexture::Cleanup()
{
    Stop();

    delete m_VT;
    m_VT = NULL;
}

void WebCamTexture::ThreadedCleanup()
{
    Cleanup();
}

void WebCamTexture::InitializeClass()
{
    // Required to be consistent across multiple platforms
}

void WebCamTexture::CleanupClass()
{
    // Required to be consistent across multiple platforms
}

TextureFormat WebCamTexture::GetBufferTextureFormat() const
{
    return kTexFormatRGBA32;
}

#endif // ENABLE_WEBCAM
