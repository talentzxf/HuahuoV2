#include "UnityPrefix.h"

#include "External/Windows10/src/ComHelpers.h"
#include "External/Windows10/src/SynchronousAction.h"
#include "External/Windows10/src/WinRTFunctions.h"
#include "External/Windows10/src/AsyncOperationHandler.h"
#include "VideoCapture.h"
#include "PlatformDependent/Win/ComPtr.h"
#include "PlatformDependent/Win/Webcam/WebCam.h"
#include "Runtime/Core/Callbacks/GlobalCallbacks.h"
#include "Runtime/Scripting/ScriptingInvocation.h"
#include "Runtime/Scripting/ScriptingExportUtility.h"
#include "Runtime/Threads/CurrentThread.h"
#include "MixedRealityCaptureVideoEffect.h"

#if PLATFORM_WINRT
#include "PlatformDependent/MetroPlayer/MetroCapabilities.h"
#endif

namespace UnityWinRTBase
{
    using namespace UnityWinRTBase::Windows::Foundation;
    using namespace UnityWinRTBase::Windows::Foundation::Collections;
    using namespace UnityWinRTBase::Windows::Foundation::Numerics;
    using namespace UnityWinRTBase::Windows::Media;
    using namespace UnityWinRTBase::Windows::Media::Capture;
    using namespace UnityWinRTBase::Windows::Media::Devices;
    using namespace UnityWinRTBase::Windows::Media::MediaProperties;
    using namespace UnityWinRTBase::Windows::Storage;
}

struct OnCreatedVideoCaptureResourceEventData
{
    Unity::VideoCapture* instance;
};
REGISTER_EVENT_ID(0xD12BE8BE011049F1, 0xA1AEEE31D87E1E9B, OnCreatedVideoCaptureResourceEventData);

struct OnStartedVideoModeEventData
{
    Unity::VideoCapture* instance;
};
REGISTER_EVENT_ID(0x308804DDCC7E45F9, 0xA0E4BF464598AD84, OnStartedVideoModeEventData);

struct OnStoppedVideoModeEventData
{
    Unity::VideoCapture* instance;
};
REGISTER_EVENT_ID(0xC962B320ECBD4AFC, 0xB84D94CEE6C84EED, OnStoppedVideoModeEventData);

struct OnStartedRecordingVideoToDiskEventData
{
    Unity::VideoCapture* instance;
    OnStartedRecordingVideoToDiskEventData() : instance(0) {}
    OnStartedRecordingVideoToDiskEventData(Unity::VideoCapture* inst) : instance(inst) {}
};
REGISTER_EVENT_ID(0x5B5E48741A6E4D64, 0xBD75CF2977706E9B, OnStartedRecordingVideoToDiskEventData);

struct OnStoppedRecordingVideoToDiskEventData
{
    Unity::VideoCapture* instance;
    OnStoppedRecordingVideoToDiskEventData() : instance(0) {}
    OnStoppedRecordingVideoToDiskEventData(Unity::VideoCapture* inst) : instance(inst) {}
};
REGISTER_EVENT_ID(0xC2214A3F99C340C2, 0xA50DCF9CFC25C3E8, OnStoppedRecordingVideoToDiskEventData);

struct OnNotifyScriptAboutVideoCaptureErrorEventData
{
    Unity::VideoCapture* instance;
    Unity::VideoCapture::OperationType operationType;
    HRESULT hr;
    ScriptingBackendNativeGCHandle callbackGCHandle;

    void Destroy()
    {
        ScriptingGCHandle::FromScriptingBackendNativeGCHandle(callbackGCHandle).ReleaseAndClear();
    }
};
REGISTER_EVENT_ID_WITH_CLEANUP(0x20AB5E3FF6D34172, 0x8576D5353A550E96, OnNotifyScriptAboutVideoCaptureErrorEventData)

struct OnVideoCaptureDestructionEventData
{
    Unity::VideoCapture* instance;
};
REGISTER_EVENT_ID(0xD9159D04EA8548DA, 0xA22EDCFF97E4D265, OnVideoCaptureDestructionEventData)

struct OnVideoCaptureAsyncOperationCompleteEventData
{
    Unity::VideoCapture* instance;
    void (Unity::VideoCapture::*methodToInvoke)();
};
REGISTER_EVENT_ID(0x91805D112F114FB3, 0xAA18EC61FAB9F076, OnVideoCaptureAsyncOperationCompleteEventData)

struct OnVideoCaptureStartRecordingOperationEventData
{
    Unity::VideoCapture* instance;
    UnityWinRTBase::Windows::Storage::IStorageFile* storageFile;
};
REGISTER_EVENT_ID(0x6DEA2CC85FA043D7, 0xBF42B427265108AA, OnVideoCaptureStartRecordingOperationEventData)


namespace Unity
{
    RuntimeStatic<VideoCapture::VideoCaptureStatics> VideoCapture::VideoCapture::s_VideoCaptureStatics(kMemWebCam, RuntimeStatic<VideoCapture::VideoCaptureStatics>::kConstruction);

    bool CheckForUWPVideoCaptureCapabilities()
    {
#if PLATFORM_WINRT
        const char* explanation = "because you're using VideoCapture which requires both Webcam and Microphone";

        if (!metro::Capabilities::IsSupported(metro::Capabilities::kWebCam, explanation))
            return false;

        if (!metro::Capabilities::IsSupported(metro::Capabilities::kMicrophone, explanation))
            return false;
#endif
        return true;
    }

    dynamic_array<Resolution> VideoCapture::GetSupportedResolutions()
    {
        dynamic_array<Resolution> supportedResolutions(kMemTempAlloc);

        if (!CheckForUWPVideoCaptureCapabilities())
            return supportedResolutions;

        win::ComPtr<UnityWinRTBase::IInspectable> mediaCaptureInspectable;
        win::ComPtr<UnityWinRTBase::IMediaCapture> mediaCapture;

        HRESULT hr = UnityWinRTBase::RoActivateInstance(UnityWinRTBase::HStringReference(L"Windows.Media.Capture.MediaCapture"), &mediaCaptureInspectable);
        if (FAILED(hr))
        {
            WarningString("Warning: failed to create Windows::Media::Capture::MediaCapture object. VideoCapture requires running on at least Windows 8.1.");
            return supportedResolutions;
        }

#define CheckHR(hr) do { if (FAILED(hr)) return supportedResolutions; } while (false)

        hr = mediaCaptureInspectable.As(&mediaCapture);
        CheckHR(hr);

        win::ComPtr<UnityWinRTBase::IAsyncAction> asyncAction;
        hr = mediaCapture->InitializeAsync(&asyncAction);
        CheckHR(hr);

        hr = UnityWinRTBase::SynchronousAction::Wait(kMemWebCam, asyncAction);
        CheckHR(hr);

        win::ComPtr<UnityWinRTBase::IVideoDeviceController> videoController;
        hr = mediaCapture->get_VideoDeviceController(&videoController);
        CheckHR(hr);

        win::ComPtr<UnityWinRTBase::IMediaDeviceController> mediaController;
        hr = videoController.As(&mediaController);
        CheckHR(hr);

        win::ComPtr<UnityWinRTBase::IVectorView<UnityWinRTBase::IMediaEncodingProperties*> > encodingProperties;
        hr = mediaController->GetAvailableMediaStreamProperties(UnityWinRTBase::MediaStreamType_VideoRecord, &encodingProperties);
        CheckHR(hr);

        uint32_t count;
        hr = encodingProperties->get_Size(&count);
        CheckHR(hr);

        dynamic_array<UnityWinRTBase::IMediaEncodingProperties*> properties(kMemTempAlloc);
        properties.resize_uninitialized(count);

        uint32_t actualCount;
        hr = encodingProperties->GetMany(0, count, properties.data(), &actualCount);
        CheckHR(hr);
        Assert(actualCount == count);

        supportedResolutions.reserve(actualCount);

        for (uint32_t i = 0; i < actualCount; i++)
        {
            win::ComPtr<UnityWinRTBase::IMediaEncodingProperties> mediaEncodingProperties;
            win::ComPtr<UnityWinRTBase::IVideoEncodingProperties> videoEncodingProperties;
            mediaEncodingProperties.Attach(properties[i]); // use RAII to release it at the end of this iteration

            hr = mediaEncodingProperties.As(&videoEncodingProperties);
            if (FAILED(hr))
                continue;

            uint32_t width, height;

            hr = videoEncodingProperties->get_Width(&width);
            if (FAILED(hr))
                continue;

            hr = videoEncodingProperties->get_Height(&height);
            if (FAILED(hr))
                continue;

            Resolution resolution;
            resolution.width = width;
            resolution.height = height;
            resolution.refreshRate = 0;
            supportedResolutions.push_back(resolution);
        }

        return supportedResolutions;
#undef CheckHR
    }

    dynamic_array<float> VideoCapture::GetSupportedFrameRatesForResolution(int resolutionWidth, int resolutionHeight)
    {
        dynamic_array<float> supportedFrameRates(kMemTempAlloc);

        if (!CheckForUWPVideoCaptureCapabilities())
            return supportedFrameRates;

        win::ComPtr<UnityWinRTBase::IInspectable> mediaCaptureInspectable;
        win::ComPtr<UnityWinRTBase::IMediaCapture> mediaCapture;

        HRESULT hr = UnityWinRTBase::RoActivateInstance(UnityWinRTBase::HStringReference(L"Windows.Media.Capture.MediaCapture"), &mediaCaptureInspectable);
        if (FAILED(hr))
        {
            WarningString("Warning: failed to create Windows::Media::Capture::MediaCapture object. VideoCapture requires running on at least Windows 8.1.");
            return supportedFrameRates;
        }

#define CheckHR(hr) do { if (FAILED(hr)) return supportedFrameRates; } while (false)

        hr = mediaCaptureInspectable.As(&mediaCapture);
        CheckHR(hr);

        win::ComPtr<UnityWinRTBase::IAsyncAction> asyncAction;
        hr = mediaCapture->InitializeAsync(&asyncAction);
        CheckHR(hr);

        hr = UnityWinRTBase::SynchronousAction::Wait(kMemWebCam, asyncAction);
        CheckHR(hr);

        win::ComPtr<UnityWinRTBase::IVideoDeviceController> videoController;
        hr = mediaCapture->get_VideoDeviceController(&videoController);
        CheckHR(hr);

        win::ComPtr<UnityWinRTBase::IMediaDeviceController> mediaController;
        hr = videoController.As(&mediaController);
        CheckHR(hr);

        win::ComPtr<UnityWinRTBase::IVectorView<UnityWinRTBase::IMediaEncodingProperties*> > encodingProperties;
        hr = mediaController->GetAvailableMediaStreamProperties(UnityWinRTBase::MediaStreamType_VideoRecord, &encodingProperties);
        CheckHR(hr);

        uint32_t count;
        hr = encodingProperties->get_Size(&count);
        CheckHR(hr);

        dynamic_array<UnityWinRTBase::IMediaEncodingProperties*> properties(kMemTempAlloc);
        properties.resize_uninitialized(count);

        uint32_t actualCount;
        hr = encodingProperties->GetMany(0, count, properties.data(), &actualCount);
        CheckHR(hr);
        Assert(actualCount == count);

        supportedFrameRates.reserve(actualCount);

        for (uint32_t i = 0; i < actualCount; i++)
        {
            win::ComPtr<UnityWinRTBase::IMediaEncodingProperties> mediaEncodingProperties;
            win::ComPtr<UnityWinRTBase::IVideoEncodingProperties> videoEncodingProperties;
            mediaEncodingProperties.Attach(properties[i]); // use RAII to release it at the end of this iteration

            hr = mediaEncodingProperties.As(&videoEncodingProperties);
            if (FAILED(hr))
                continue;

            uint32_t width, height;

            hr = videoEncodingProperties->get_Width(&width);
            if (FAILED(hr))
                continue;

            hr = videoEncodingProperties->get_Height(&height);
            if (FAILED(hr))
                continue;

            if (resolutionWidth != width || resolutionHeight != height)
            {
                continue;
            }

            uint32_t fpsNumerator, fpsDenominator;

            win::ComPtr<UnityWinRTBase::IMediaRatio> mediaRatio;
            videoEncodingProperties->get_FrameRate(&mediaRatio);

            mediaRatio->get_Numerator(&fpsNumerator);
            if (FAILED(hr))
                continue;
            mediaRatio->get_Denominator(&fpsDenominator);
            if (FAILED(hr))
                continue;

            float fps = static_cast<float>(fpsNumerator) / fpsDenominator;
            supportedFrameRates.push_back(fps);
        }

        return supportedFrameRates;
#undef CheckHR
    }

    HRESULT GetErrorCodeFromAsyncAction(UnityWinRTBase::IAsyncAction* asyncAction, HRESULT* errorCode)
    {
        DebugAssert(asyncAction != nullptr);
        DebugAssert(errorCode != nullptr);

        win::ComPtr<UnityWinRTBase::IAsyncInfo> asyncInfo;
        HRESULT hr;

        *errorCode = E_FAIL;

        hr = asyncAction->QueryInterface(__uuidof(UnityWinRTBase::IAsyncInfo), &asyncInfo);
        if (SUCCEEDED(hr))
        {
            hr = asyncInfo->get_ErrorCode(errorCode);
        }

        return hr;
    }

    VideoCapture* VideoCapture::Instantiate(bool showHolograms, ScriptingObjectPtr callback)
    {
        if (!CheckForUWPVideoCaptureCapabilities())
            return nullptr;

        return UNITY_NEW(VideoCapture, kMemWebCam)(showHolograms, callback);
    }

    // If this fails, we mark that we're in an error state and return
#define VerifyHRAndNotifyScript(operationType, hr, callbackId) \
do \
{ \
    if (FAILED(hr)) \
    { \
        NotifyScriptOfErrorOnMainThread(operationType, hr, callbackId); \
        m_HasErrors = true; \
        return; \
    } \
} \
while (false)

#define VerifyHRAndNotifyScriptWithReturnValue(operationType, hr, callbackId, returnValue) \
do \
{ \
    if (FAILED(hr)) \
    { \
        NotifyScriptOfErrorOnMainThread(operationType, hr, callbackId); \
        m_HasErrors = true; \
        return returnValue; \
    } \
} \
while (false)


    //---------------------------------------------------------------
    // HoloLens firmware currently has hologram capture functionality disabled.
    // To prevent error spam, we will set m_ShowHolograms to false until
    // Microsoft re-enables the functionality.
    VideoCapture::VideoCapture(bool showHolograms, const ScriptingObjectPtr onCreateCallback)
        : m_OnCreateCallbackGCHandle()
        , m_OnVideoModeStartedGCHandle()
        , m_OnVideoModeStoppedGCHandle()
        , m_OnVideoRecordingStartedGCHandle()
        , m_OnVideoRecordingStoppedGCHandle()
        , m_RecordingState(RecordingState::Uninitialized)
        , m_AudioState(Unity::AudioMixerMode::MicAndLoopback)
        , m_ShowHolograms(showHolograms)
        , m_HasErrors(false)
        , m_IsDisposed(false)
        , m_IsRecording(false)
        , m_MediaCapture(nullptr)
    {
#if UNITY_EDITOR
        m_IsQueuedForDestruction = false;
        s_VideoCaptureStatics->videoCaptureInstances.push_back(this);
#endif

        m_OnCreateCallbackGCHandle.AcquireStrong(onCreateCallback);

        GlobalEventQueue::GetInstance().AddHandler(m_OnCreatedVideoCaptureResourceEventDelegate.SetObject(this));
        GlobalEventQueue::GetInstance().AddHandler(m_OnStartedVideoModeEventDelegate.SetObject(this));
        GlobalEventQueue::GetInstance().AddHandler(m_OnStoppedVideoModeEventDelegate.SetObject(this));
        GlobalEventQueue::GetInstance().AddHandler(m_OnStartedRecordingVideoToDiskEventDelegate.SetObject(this));
        GlobalEventQueue::GetInstance().AddHandler(m_OnStoppedRecordingVideoToDiskEventDelegate.SetObject(this));
        GlobalEventQueue::GetInstance().AddHandler(m_OnNotifyScriptAboutErrorEventDelegate.SetObject(this));

        static UnityEventQueue::StaticFunctionEventHandler<OnVideoCaptureDestructionEventData> s_DestroyEventDelegate(&VideoCapture::HandleDestructionEvent);
        static UnityEventQueue::StaticFunctionEventHandler<OnVideoCaptureAsyncOperationCompleteEventData> s_AsyncHandlerEventDelegate(&VideoCapture::HandleAsyncOperationCompleteEvent);
        static UnityEventQueue::StaticFunctionEventHandler<OnVideoCaptureStartRecordingOperationEventData> s_StartRecordingOperationEventDelegate(&VideoCapture::HandleStartRecordingOperationEvent);

        if (!s_VideoCaptureStatics->staticInitComplete)
        {
            GlobalEventQueue::GetInstance().AddHandler(&s_DestroyEventDelegate);
            GlobalEventQueue::GetInstance().AddHandler(&s_AsyncHandlerEventDelegate);
            GlobalEventQueue::GetInstance().AddHandler(&s_StartRecordingOperationEventDelegate);

#if UNITY_EDITOR
            GlobalCallbacks::Get().exitPlayModeAfterOnEnableInEditMode.Register(&VideoCapture::OnExitPlayMode);
#endif
            s_VideoCaptureStatics->staticInitComplete = true;
        }

        InitializeAsync();
    }

    //---------------------------------------------------------------
    void VideoCapture::InitializeAsync()
    {
        win::ComPtr<UnityWinRTBase::IInspectable> mediaCaptureInspectable;
        win::ComPtr<UnityWinRTBase::IMediaCapture> mediaCapture;
        win::ComPtr<UnityWinRTBase::IAsyncAction> asyncAction;
        HRESULT hr;

        DebugAssert(CurrentThread::IsMainThread());

        hr = UnityWinRTBase::RoActivateInstance(UnityWinRTBase::HStringReference(L"Windows.Media.Capture.MediaCapture"), &mediaCaptureInspectable);
        VerifyHRAndNotifyScript(OperationType::Create, hr, m_OnCreateCallbackGCHandle);

        hr = mediaCaptureInspectable.As(&mediaCapture);
        VerifyHRAndNotifyScript(OperationType::Create, hr, m_OnCreateCallbackGCHandle);

        hr = mediaCapture->InitializeAsync(&asyncAction);
        VerifyHRAndNotifyScript(OperationType::Create, hr, m_OnCreateCallbackGCHandle);

        // InitializeAsync handler doesn't interact with MediaCapture and doesn't need to be run on the main thread
        hr = asyncAction->put_Completed(AsyncActionHandlerFactory::Create([this](
            UnityWinRTBase::Windows::Foundation::IAsyncAction* asyncAction,
            UnityWinRTBase::Windows::Foundation::AsyncStatus asyncStatus)
            {
                if (asyncStatus == UnityWinRTBase::Completed)
                {
                    SendOnCreatedVideoCaptureResourceEvent();
                }
                else
                {
                    HRESULT errorCode;

                    HRESULT hr = GetErrorCodeFromAsyncAction(asyncAction, &errorCode);
                    VerifyHRAndNotifyScriptWithReturnValue(OperationType::Create, hr, m_OnCreateCallbackGCHandle, S_OK);

                    // Report actual error returned from IAsyncInfo
                    VerifyHRAndNotifyScriptWithReturnValue(OperationType::Create, errorCode, m_OnCreateCallbackGCHandle, S_OK);
                }

                return S_OK;
            }));
        VerifyHRAndNotifyScript(OperationType::Create, hr, m_OnCreateCallbackGCHandle);

        m_MediaCapture = std::move(mediaCapture);
    }

    //---------------------------------------------------------------
    void VideoCapture::Dispose()
    {
        DebugAssert(CurrentThread::IsMainThread());

        if (m_IsDisposed)
            return;

        m_IsDisposed = true;

        SetAudioRecordingEnabled(true);

        if (s_VideoCaptureStatics->activeInstance == this)
        {
            s_VideoCaptureStatics->activeInstance = nullptr;
            WebCam::GetInstance().SetWebCamMode(WebCam::None);
        }

        m_RecordingState = RecordingState::Uninitialized;

        GlobalEventQueue::GetInstance().RemoveHandler(&m_OnCreatedVideoCaptureResourceEventDelegate);
        GlobalEventQueue::GetInstance().RemoveHandler(&m_OnStartedVideoModeEventDelegate);
        GlobalEventQueue::GetInstance().RemoveHandler(&m_OnStoppedVideoModeEventDelegate);
        GlobalEventQueue::GetInstance().RemoveHandler(&m_OnStartedRecordingVideoToDiskEventDelegate);
        GlobalEventQueue::GetInstance().RemoveHandler(&m_OnStoppedRecordingVideoToDiskEventDelegate);
        GlobalEventQueue::GetInstance().RemoveHandler(&m_OnNotifyScriptAboutErrorEventDelegate);

        m_OnCreateCallbackGCHandle.ReleaseAndClear();
        m_OnVideoModeStartedGCHandle.ReleaseAndClear();
        m_OnVideoModeStoppedGCHandle.ReleaseAndClear();
        m_OnVideoRecordingStartedGCHandle.ReleaseAndClear();
        m_OnVideoRecordingStoppedGCHandle.ReleaseAndClear();

#if UNITY_EDITOR
        for (auto& inst : s_VideoCaptureStatics->videoCaptureInstances)
        {
            if (inst == this)
            {
                s_VideoCaptureStatics->videoCaptureInstances.erase_swap_back(&inst);
                break;
            }
        }
#endif

        m_MediaCapture = nullptr;
    }

    //---------------------------------------------------------------
    void VideoCapture::DisposeThreaded()
    {
#if UNITY_EDITOR
        m_IsQueuedForDestruction = true;
#endif

        OnVideoCaptureDestructionEventData eventData = { this };
        GlobalEventQueue::GetInstance().SendEvent(eventData);
    }

    //---------------------------------------------------------------
    void VideoCapture::HandleDestructionEvent(const OnVideoCaptureDestructionEventData& eventData)
    {
        eventData.instance->Dispose();
    }

    //---------------------------------------------------------------
    void VideoCapture::HandleAsyncOperationCompleteEvent(const OnVideoCaptureAsyncOperationCompleteEventData& eventData)
    {
#if UNITY_EDITOR
        if (eventData.instance->m_IsQueuedForDestruction || eventData.instance->m_IsDisposed) return;
#endif

        (eventData.instance->*eventData.methodToInvoke)();
    }

    //---------------------------------------------------------------
    void VideoCapture::HandleStartRecordingOperationEvent(const OnVideoCaptureStartRecordingOperationEventData& eventData)
    {
#if UNITY_EDITOR
        if (eventData.instance->m_IsQueuedForDestruction || eventData.instance->m_IsDisposed) return;
#endif

        // The storageFile COM pointer was already ref-counted when event was queued, so just Attach new ComPtr object to it
        win::ComPtr<UnityWinRTBase::Windows::Storage::IStorageFile> storageFile;
        storageFile.Attach(eventData.storageFile);

        eventData.instance->StartActualRecordingAsync(storageFile);
    }

    //---------------------------------------------------------------
    void* VideoCapture::GetUnsafePointerToVideoDeviceController()
    {
        DebugAssert(CurrentThread::IsMainThread());

        if (m_MediaCapture == nullptr)
        {
            ErrorString("Could not retrieve the video device controller pointer because MediaCapture is null!");
            return nullptr;
        }

        win::ComPtr<UnityWinRTBase::Windows::Media::Devices::IVideoDeviceController> videoController;
        HRESULT hr;

        hr = m_MediaCapture->get_VideoDeviceController(&videoController);
        if (FAILED(hr))
        {
            ErrorStringMsg("MediaCapture::get_VideoDeviceController() failed [0x%08x]", hr);
            return nullptr;
        }

        // According to the API Documentation: "The caller is responsible for releasing each instance of the COM pointer."
        videoController->AddRef();
        return videoController;
    }

    //---------------------------------------------------------------
    bool VideoCapture::IsRecording()
    {
        return m_IsRecording;
    }

    //---------------------------------------------------------------
    void VideoCapture::SendOnCreatedVideoCaptureResourceEvent()
    {
        if (GlobalEventQueue::GetInstancePtr())
        {
            OnCreatedVideoCaptureResourceEventData eventData = { this };
            GlobalEventQueue::GetInstance().SendEvent(eventData);
        }
    }

    //---------------------------------------------------------------
    void VideoCapture::HandleEvent(const OnCreatedVideoCaptureResourceEventData& eventData)
    {
        if (eventData.instance != this)
            return;

        OnCreatedVideoCaptureResource();
    }

    //---------------------------------------------------------------
    void VideoCapture::OnCreatedVideoCaptureResource()
    {
        m_RecordingState = RecordingState::Idle;

        ScriptingGCHandle callback = m_OnCreateCallbackGCHandle;
        m_OnCreateCallbackGCHandle.Clear();

        ScriptingInvocation scriptingInvocation(GetCoreScriptingClasses().invokeOnCreatedVideoCaptureResourceDelegate);
        scriptingInvocation.AddObject(callback.Resolve());
        scriptingInvocation.AddIntPtr(this);
        scriptingInvocation.Invoke();

        Assert(callback.HasTarget());
        callback.ReleaseAndClear();
    }

    //---------------------------------------------------------------
    void VideoCapture::StartVideoMode(CameraParameters cameraParameters, Unity::AudioMixerMode audioState, const ScriptingObjectPtr callback)
    {
        DebugAssert(CurrentThread::IsMainThread());

        if (m_HasErrors)
        {
            NotifyScriptOfError(OperationType::StartVideoMode, E_FAIL, "Failed to start video mode: VideoCapture is in an erroneous state.", callback);
            return;
        }

        if (WebCam::GetInstance().GetWebCamMode() == WebCam::VideoMode)
        {
            NotifyScriptOfError(OperationType::StartVideoMode, E_FAIL, "Video Mode has already been started.", callback);
            return;
        }

        if (WebCam::GetInstance().GetWebCamMode() == WebCam::PhotoMode)
        {
            NotifyScriptOfError(OperationType::StartVideoMode, E_FAIL, "Can not start Video Mode because Photo Mode has already been started.", callback);
            return;
        }

        if (m_RecordingState != RecordingState::Idle)
        {
            NotifyScriptOfError(OperationType::StartVideoMode, E_FAIL, "The VideoCapture instance is not in the correct state to begin video mode.", callback);
            return;
        }

        if (s_VideoCaptureStatics->videoModeState != VideoModeState::Deactivated || s_VideoCaptureStatics->activeInstance != nullptr)
        {
            NotifyScriptOfError(OperationType::StartVideoMode, E_FAIL, "Video Mode has already been started.", callback);
            return;
        }

        WebCam::GetInstance().SetWebCamMode(WebCam::VideoMode);
        s_VideoCaptureStatics->activeInstance = this;
        s_VideoCaptureStatics->videoModeState = VideoModeState::Activating;

        // Make sure we free an existing callback before creating a new one
        // just in case we were put into a bad state during the previous invocation.
        m_OnVideoModeStartedGCHandle.ReleaseAndClear();
        m_OnVideoModeStartedGCHandle.AcquireStrong(callback);

        m_CameraParameters = cameraParameters;
        m_AudioState = audioState;

        win::ComPtr<UnityWinRTBase::IVideoDeviceController> videoController;
        win::ComPtr<UnityWinRTBase::IMediaDeviceController> mediaController;
        win::ComPtr<UnityWinRTBase::IVectorView<UnityWinRTBase::IMediaEncodingProperties*> > streamProperties;
        HRESULT hr;

        hr = m_MediaCapture->get_VideoDeviceController(&videoController);
        VerifyHRAndNotifyScript(OperationType::StartVideoMode, hr, m_OnVideoModeStartedGCHandle);

        hr = videoController.As(&mediaController);
        VerifyHRAndNotifyScript(OperationType::StartVideoMode, hr, m_OnVideoModeStartedGCHandle);

        hr = mediaController->GetAvailableMediaStreamProperties(UnityWinRTBase::MediaStreamType_VideoRecord, &streamProperties);
        VerifyHRAndNotifyScript(OperationType::StartVideoMode, hr, m_OnVideoModeStartedGCHandle);

        uint32_t size = 0;
        hr = streamProperties->get_Size(&size);
        if (FAILED(hr))
            size = 0;

        win::ComPtr<UnityWinRTBase::IMediaEncodingProperties> selectedProperty;
        for (auto i = 0; i < size; i++)
        {
            win::ComPtr<UnityWinRTBase::IMediaEncodingProperties> mediaProp;
            win::ComPtr<UnityWinRTBase::IVideoEncodingProperties> videoProp;

            if (FAILED(streamProperties->GetAt(i, &mediaProp)))
                continue;

            if (FAILED(mediaProp.As(&videoProp)))
                continue;

            win::ComPtr<UnityWinRTBase::IMediaRatio> frameRate;
            if (FAILED(videoProp->get_FrameRate(&frameRate)))
                continue;

            UINT32 numerator;
            UINT32 denominator;
            if (FAILED(frameRate->get_Numerator(&numerator)) ||
                FAILED(frameRate->get_Denominator(&denominator)))
                continue;

            UINT32 width;
            UINT32 height;
            if (FAILED(videoProp->get_Width(&width)) ||
                FAILED(videoProp->get_Height(&height)))
                continue;

            float fps = static_cast<float>(numerator) / static_cast<float>(denominator);

            if (m_CameraParameters.m_CameraResolutionWidth == static_cast<int>(width) &&
                m_CameraParameters.m_CameraResolutionHeight == static_cast<int>(height) &&
                CompareApproximately(m_CameraParameters.m_FrameRate, fps))
            {
                selectedProperty = mediaProp;
                break;
            }
        }

        if (selectedProperty == nullptr)
        {
            ErrorStringMsg("Failed to find camera video mode for specified parameters: Width %u, Height %u, Frame Rate %f",
                m_CameraParameters.m_CameraResolutionWidth,
                m_CameraParameters.m_CameraResolutionHeight,
                m_CameraParameters.m_FrameRate);

            VerifyHRAndNotifyScript(OperationType::StartVideoMode, E_FAIL, m_OnVideoModeStartedGCHandle);
            return;
        }

        win::ComPtr<UnityWinRTBase::IAsyncAction> asyncAction;
        hr = mediaController->SetMediaStreamPropertiesAsync(UnityWinRTBase::Windows::Media::Capture::MediaStreamType::MediaStreamType_VideoRecord, selectedProperty, &asyncAction);
        VerifyHRAndNotifyScript(OperationType::StartVideoMode, hr, m_OnVideoModeStartedGCHandle);

        // The remainder of StartVideoMode operations are performed by async handlers to configure MixedReality Video/Audio effects (if available).
        // Each async operation must be completed in sequence...on the main thread! We can only access VideoCapture COM object on the thread
        // that created it, but IAsyncAction/IAsyncOperation handlers are called from the thread pool.
        //
        // So to accomplish this we'll dispatch a OnVideoCaptureAsyncOperationCompleteEventData event at the end of each operation to signal the next
        // in the sequence. The event data holds a VideoCapture instance, i.e. "this", and a pointer to the member function to execute on the main thread.
        //
        // The async chain is as follows:
        // SetMediaStreamPropertiesAsync -> DisableMixedRealityVideoEffectsAsync_StartVideo -> EnableMixedRealityVideoEffectAsync -> EnableMixedRealityAudioEffectAsync -> SendOnStartedVideoModeEvent

        hr = asyncAction->put_Completed(AsyncActionHandlerFactory::Create([this](
            UnityWinRTBase::IAsyncAction* asyncAction,
            UnityWinRTBase::AsyncStatus asyncStatus)
            {
                OnVideoCaptureAsyncOperationCompleteEventData eventData = { this, &VideoCapture::DisableMixedRealityVideoEffectsAsync_StartVideo };
                GlobalEventQueue::GetInstance().SendEvent(eventData);
                return S_OK;
                // End StartRecordToStorageFileAsync() handler
            }));
        VerifyHRAndNotifyScript(OperationType::StartVideoMode, hr, m_OnVideoModeStartedGCHandle);
    }

    //---------------------------------------------------------------
    void VideoCapture::DisableMixedRealityVideoEffectsAsync_StartVideo()
    {
        DisableMixedRealityVideoEffectsAsync(OperationType::StartVideoMode, &m_OnVideoModeStartedGCHandle);
    }

    //---------------------------------------------------------------
    void VideoCapture::DisableMixedRealityVideoEffectsAsync_StopRecording()
    {
        DisableMixedRealityVideoEffectsAsync(OperationType::StopRecordingVideo, &m_OnVideoRecordingStoppedGCHandle);
    }

    //---------------------------------------------------------------
    void VideoCapture::EnableMixedRealityVideoEffectAsync()
    {
        DebugAssert(CurrentThread::IsMainThread());

        if (m_ShowHolograms)
        {
            HRESULT hr;

            win::ComPtr<UnityWinRTBase::IMediaCapture4> mediaCapture4;
            hr = m_MediaCapture.As(&mediaCapture4);
            VerifyHRAndNotifyScript(OperationType::StartVideoMode, hr, m_OnVideoModeStartedGCHandle);

            win::ComPtr<UnityWinRTBase::Windows::Media::Effects::IVideoEffectDefinition> effect;
            win::ComPtr<UnityWinRTBase::IAsyncOperation<UnityWinRTBase::Windows::Media::IMediaExtension*> > asyncVideoEffectOperation;

            effect.Attach(UNITY_NEW(Unity::MixedRealityCaptureVideoEffect, kMemWebCam)(UnityWinRTBase::MediaStreamType_VideoRecord, m_CameraParameters.m_HologramOpacity));
            hr = mediaCapture4->AddVideoEffectAsync(effect, UnityWinRTBase::Windows::Media::Capture::MediaStreamType::MediaStreamType_VideoRecord, &asyncVideoEffectOperation);
            if (FAILED(hr))
            {
                // Output a clear error message because the HR returned by AddVideoEffectAsync (in this case) is useless
                ErrorString("Failed to add MixedReality video effect; 'showHolograms' option is only supported for Windows Mixed Reality apps");
            }
            VerifyHRAndNotifyScript(OperationType::StartVideoMode, hr, m_OnVideoModeStartedGCHandle);

            hr = asyncVideoEffectOperation->put_Completed(AsyncOperationHandlerFactory<UnityWinRTBase::Windows::Media::IMediaExtension*>::Create([this](
                UnityWinRTBase::IAsyncOperation<UnityWinRTBase::Windows::Media::IMediaExtension*>* asyncEffectOperation,
                UnityWinRTBase::Windows::Foundation::AsyncStatus asyncStatus)
                {
                    win::ComPtr<UnityWinRTBase::Windows::Media::IMediaExtension> mediaExtension;
                    HRESULT hr;

                    hr = asyncEffectOperation->GetResults(&mediaExtension);
                    if (FAILED(hr))
                    {
                        ErrorString("Failed to add MixedReality video effect; 'showHolograms' option is only supported for Windows Mixed Reality apps");
                    }
                    VerifyHRAndNotifyScriptWithReturnValue(OperationType::StartVideoMode, hr, m_OnVideoModeStartedGCHandle, S_OK);

                    OnVideoCaptureAsyncOperationCompleteEventData eventData = { this, &VideoCapture::EnableMixedRealityAudioEffectAsync };
                    GlobalEventQueue::GetInstance().SendEvent(eventData);

                    // End AddVideoEffectAsync() handler
                    return S_OK;
                }));
            VerifyHRAndNotifyScript(OperationType::StartVideoMode, hr, m_OnVideoModeStartedGCHandle);
        }
        else
        {
            // No async operation, so just invoke next stage directly
            EnableMixedRealityAudioEffectAsync();
        }
    }

    //---------------------------------------------------------------
    void VideoCapture::EnableMixedRealityAudioEffectAsync()
    {
        DebugAssert(CurrentThread::IsMainThread());

        if (m_AudioState != AudioMixerMode::None)
        {
            HRESULT hr;

            win::ComPtr<UnityWinRTBase::IMediaCapture4> mediaCapture4;
            hr = m_MediaCapture.As(&mediaCapture4);
            VerifyHRAndNotifyScript(OperationType::StartVideoMode, hr, m_OnVideoModeStartedGCHandle);

            win::ComPtr<UnityWinRTBase::Windows::Media::Effects::IAudioEffectDefinition> effect;
            win::ComPtr<UnityWinRTBase::IAsyncOperation<UnityWinRTBase::Windows::Media::IMediaExtension*> > asyncAudioEffectOperation;

            effect.Attach(UNITY_NEW(Unity::MixedRealityCaptureAudioEffect, kMemWebCam)(m_AudioState));
            hr = mediaCapture4->AddAudioEffectAsync(effect, &asyncAudioEffectOperation);
            VerifyHRAndNotifyScript(OperationType::StartVideoMode, hr, m_OnVideoModeStartedGCHandle);

            hr = asyncAudioEffectOperation->put_Completed(AsyncOperationHandlerFactory<UnityWinRTBase::Windows::Media::IMediaExtension*>::Create([this](
                UnityWinRTBase::IAsyncOperation<UnityWinRTBase::Windows::Media::IMediaExtension*>* asyncEffectOperation,
                UnityWinRTBase::Windows::Foundation::AsyncStatus asyncStatus)
                {
                    win::ComPtr<UnityWinRTBase::Windows::Media::IMediaExtension> mediaExtension;
                    HRESULT hr;

                    hr = asyncEffectOperation->GetResults(&mediaExtension);
                    VerifyHRAndNotifyScriptWithReturnValue(OperationType::StartVideoMode, hr, m_OnVideoModeStartedGCHandle, S_OK);

                    // Final action just sends an event to the scripts; can call directly
                    SendOnStartedVideoModeEvent();

                    // End AddAudioEffectAsync() handler
                    return S_OK;
                }));
            VerifyHRAndNotifyScript(OperationType::StartVideoMode, hr, m_OnVideoModeStartedGCHandle);
        }
        else
        {
            // No async operation, so just invoke next stage directly
            SetAudioRecordingEnabled(false);
            SendOnStartedVideoModeEvent();
        }
    }

    //---------------------------------------------------------------
    void VideoCapture::EnableAudioRecording()
    {
        SetAudioRecordingEnabled(true);
    }

    //---------------------------------------------------------------
    void VideoCapture::DisableAudioRecording()
    {
        SetAudioRecordingEnabled(false);
    }

    //---------------------------------------------------------------
    void VideoCapture::DisableMixedRealityVideoEffectsAsync(OperationType operation, ScriptingGCHandle* pGCScriptHandle)
    {
        DebugAssert(CurrentThread::IsMainThread());
        DebugAssert(pGCScriptHandle != nullptr);

        if (m_MediaCapture == nullptr)
            return;

        HRESULT hr;

        win::ComPtr<UnityWinRTBase::IAsyncAction> asyncAction;
        hr = m_MediaCapture->ClearEffectsAsync(UnityWinRTBase::Windows::Media::Capture::MediaStreamType::MediaStreamType_VideoRecord, &asyncAction);
        VerifyHRAndNotifyScript(operation, hr, *pGCScriptHandle);

        hr = asyncAction->put_Completed(AsyncActionHandlerFactory::Create([this, operation, pGCScriptHandle](
            UnityWinRTBase::IAsyncAction* asyncAction,
            UnityWinRTBase::AsyncStatus asyncStatus)
            {
                if (asyncStatus == UnityWinRTBase::Completed)
                {
                    // Signal continuation according to current operation type
                    if (operation == OperationType::StartVideoMode)
                    {
                        OnVideoCaptureAsyncOperationCompleteEventData eventData = { this, &VideoCapture::EnableMixedRealityVideoEffectAsync };
                        GlobalEventQueue::GetInstance().SendEvent(eventData);
                    }
                    else if (operation == OperationType::StopRecordingVideo)
                    {
                        // Sends an event back to the scripts and so can be called directly
                        SendOnStoppedRecordingVideoToDiskModeEvent();
                    }
                }
                else
                {
                    HRESULT errorCode;
                    HRESULT hr = GetErrorCodeFromAsyncAction(asyncAction, &errorCode);
                    if (FAILED(hr))
                        errorCode = hr;

                    VerifyHRAndNotifyScriptWithReturnValue(operation, errorCode, *pGCScriptHandle, S_OK);
                }
                return S_OK;
                // End ClearEffectsAsync handler
            }));
        VerifyHRAndNotifyScript(operation, hr, *pGCScriptHandle);
    }

    //---------------------------------------------------------------
    void VideoCapture::SetAudioRecordingEnabled(bool enable)
    {
        DebugAssert(CurrentThread::IsMainThread());

        if (m_MediaCapture == nullptr)
            return;

        HRESULT hr;
        win::ComPtr<UnityWinRTBase::Windows::Media::Devices::IAudioDeviceController> audioController;
        hr = m_MediaCapture->get_AudioDeviceController(&audioController);
        if (FAILED(hr))
            return;

        audioController->put_Muted(!enable);
    }

    //---------------------------------------------------------------
    void VideoCapture::StartActualRecordingAsync(const win::ComPtr<UnityWinRTBase::Windows::Storage::IStorageFile>& storageFile)
    {
        DebugAssert(CurrentThread::IsMainThread());
        DebugAssert(storageFile != nullptr);

        HRESULT hr;

        win::ComPtr<UnityWinRTBase::IMediaEncodingProfileStatics> profileStatics;
        hr = UnityWinRTBase::RoGetActivationFactory(UnityWinRTBase::HStringReference(L"Windows.Media.MediaProperties.MediaEncodingProfile"), __uuidof(profileStatics), &profileStatics);
        VerifyHRAndNotifyScript(OperationType::StartRecordingVideo, hr, m_OnVideoRecordingStartedGCHandle);

        win::ComPtr<UnityWinRTBase::MediaProperties::IMediaEncodingProfile> mediaProfile;
        hr = profileStatics->CreateMp4(UnityWinRTBase::Windows::Media::MediaProperties::VideoEncodingQuality_Auto, &mediaProfile);
        VerifyHRAndNotifyScript(OperationType::StartRecordingVideo, hr, m_OnVideoRecordingStartedGCHandle);

        win::ComPtr<UnityWinRTBase::IAsyncAction> asyncRecordAction;
        hr = m_MediaCapture->StartRecordToStorageFileAsync(mediaProfile, storageFile, &asyncRecordAction);
        VerifyHRAndNotifyScript(OperationType::StartRecordingVideo, hr, m_OnVideoRecordingStartedGCHandle);

        hr = asyncRecordAction->put_Completed(AsyncActionHandlerFactory::Create([this](
            UnityWinRTBase::IAsyncAction* asyncAction,
            UnityWinRTBase::AsyncStatus asyncStatus)
            {
                if (asyncStatus == UnityWinRTBase::Completed)
                {
                    SendOnStartedRecordingVideoToDiskModeEvent();
                }
                else
                {
                    HRESULT errorCode;

                    HRESULT hr = GetErrorCodeFromAsyncAction(asyncAction, &errorCode);
                    VerifyHRAndNotifyScriptWithReturnValue(OperationType::StartRecordingVideo, hr, m_OnVideoRecordingStartedGCHandle, S_OK);

                    // Report actual error returned from IAsyncInfo
                    VerifyHRAndNotifyScriptWithReturnValue(OperationType::StartRecordingVideo, errorCode, m_OnVideoRecordingStartedGCHandle, S_OK);
                }

                return S_OK;
                // End StartRecordToStorageFileAsync() handler
            }));

        VerifyHRAndNotifyScript(OperationType::StartRecordingVideo, hr, m_OnVideoRecordingStartedGCHandle);
    }

    //---------------------------------------------------------------
    void VideoCapture::SendOnStartedVideoModeEvent()
    {
        if (GlobalEventQueue::GetInstancePtr())
        {
            OnStartedVideoModeEventData eventData = { this };
            GlobalEventQueue::GetInstance().SendEvent(eventData);
        }
    }

    //---------------------------------------------------------------
    void VideoCapture::HandleEvent(const OnStartedVideoModeEventData& eventData)
    {
        if (eventData.instance != this)
            return;

        OnStartedVideoMode();
    }

    //---------------------------------------------------------------
    void VideoCapture::OnStartedVideoMode()
    {
        s_VideoCaptureStatics->videoModeState = VideoModeState::Activated;

        ScriptingGCHandle callback = m_OnVideoModeStartedGCHandle;
        m_OnVideoModeStartedGCHandle.Clear();

        ScriptingInvocation scriptingInvocation(GetCoreScriptingClasses().invokeOnVideoModeStartedDelegate);
        scriptingInvocation.AddObject(callback.Resolve());
        scriptingInvocation.AddInt64(S_OK);
        scriptingInvocation.Invoke();

        Assert(callback.HasTarget());
        callback.ReleaseAndClear();
    }

    //---------------------------------------------------------------
    void VideoCapture::StopVideoMode(const ScriptingObjectPtr callback)
    {
        if (m_HasErrors)
        {
            NotifyScriptOfError(OperationType::StopVideoMode, E_FAIL, "Failed to stop video mode: VideoCapture is in an erroneous state.", callback);
            return;
        }

        if (WebCam::GetInstance().GetWebCamMode() == WebCam::PhotoMode)
        {
            NotifyScriptOfError(OperationType::StopVideoMode, E_FAIL, "Can not stop Video Mode because the web camera is currently in Photo Mode.", callback);
            return;
        }

        if (m_RecordingState != RecordingState::Idle)
        {
            NotifyScriptOfError(OperationType::StopVideoMode, E_FAIL, "The VideoCapture instance is not in the correct state to stop video mode.", callback);
            return;
        }

        if (s_VideoCaptureStatics->videoModeState != VideoModeState::Activated)
        {
            NotifyScriptOfError(OperationType::StopVideoMode, E_FAIL, "Video Mode has not been started.", callback);
            return;
        }

        if (s_VideoCaptureStatics->activeInstance != this)
        {
            NotifyScriptOfError(OperationType::StopVideoMode, E_FAIL, "Only the capture instance that started the video mode can stop the video mode.", callback);
            return;
        }

        s_VideoCaptureStatics->videoModeState = VideoModeState::Deactivating;

        // Make sure we free an existing callback before creating a new one
        // just in case we were put into a bad state during the previous invocation.
        m_OnVideoModeStoppedGCHandle.ReleaseAndClear();
        m_OnVideoModeStoppedGCHandle.AcquireStrong(callback);

        SendOnStoppedVideoModeEvent();
    }

    //---------------------------------------------------------------
    void VideoCapture::SendOnStoppedVideoModeEvent()
    {
        if (GlobalEventQueue::GetInstancePtr())
        {
            OnStoppedVideoModeEventData eventData = { this };
            GlobalEventQueue::GetInstance().SendEvent(eventData);
        }
    }

    //---------------------------------------------------------------
    void VideoCapture::HandleEvent(const OnStoppedVideoModeEventData& eventData)
    {
        if (eventData.instance != this)
            return;

        OnStoppedVideoMode();
    }

    //---------------------------------------------------------------
    void VideoCapture::OnStoppedVideoMode()
    {
        s_VideoCaptureStatics->activeInstance = nullptr;
        WebCam::GetInstance().SetWebCamMode(WebCam::None);
        s_VideoCaptureStatics->videoModeState = VideoModeState::Deactivated;

        ScriptingGCHandle callback = m_OnVideoModeStoppedGCHandle;
        m_OnVideoModeStoppedGCHandle.Clear();

        ScriptingInvocation scriptingInvocation(GetCoreScriptingClasses().invokeOnVideoModeStoppedDelegate);
        scriptingInvocation.AddObject(callback.Resolve());
        scriptingInvocation.AddInt64(S_OK);
        scriptingInvocation.Invoke();

        Assert(callback.HasTarget());
        callback.ReleaseAndClear();
    }

    //---------------------------------------------------------------
    void VideoCapture::SendOnStartedRecordingVideoToDiskModeEvent()
    {
        if (GlobalEventQueue::GetInstancePtr())
        {
            OnStartedRecordingVideoToDiskEventData eventData(this);
            GlobalEventQueue::GetInstance().SendEvent(eventData);
        }
    }

    //---------------------------------------------------------------
    void VideoCapture::HandleEvent(const OnStartedRecordingVideoToDiskEventData& eventData)
    {
        if (eventData.instance != this)
            return;

        OnStartedRecordingVideoToDisk();
    }

    //---------------------------------------------------------------
    void VideoCapture::OnStartedRecordingVideoToDisk()
    {
        m_RecordingState = RecordingState::Recording;

        ScriptingInvocation scriptingInvocation(GetCoreScriptingClasses().invokeOnStartedRecordingVideoToDiskDelegate);
        scriptingInvocation.AddObject(m_OnVideoRecordingStartedGCHandle.Resolve());
        scriptingInvocation.AddInt64(S_OK);
        scriptingInvocation.Invoke();

        Assert(m_OnVideoRecordingStartedGCHandle.HasTarget());
        m_OnVideoRecordingStartedGCHandle.ReleaseAndClear();
    }

    //---------------------------------------------------------------
    void VideoCapture::SendOnStoppedRecordingVideoToDiskModeEvent()
    {
        if (GlobalEventQueue::GetInstancePtr())
        {
            OnStoppedRecordingVideoToDiskEventData eventData(this);
            GlobalEventQueue::GetInstance().SendEvent(eventData);
        }
    }

    //---------------------------------------------------------------
    void VideoCapture::HandleEvent(const OnStoppedRecordingVideoToDiskEventData& eventData)
    {
        if (eventData.instance != this)
            return;

        OnStoppedRecordingVideoToDisk();
    }

    //---------------------------------------------------------------
    void VideoCapture::OnStoppedRecordingVideoToDisk()
    {
        m_RecordingState = RecordingState::Idle;
        m_IsRecording = false;

        ScriptingInvocation scriptingInvocation(GetCoreScriptingClasses().invokeOnStoppedRecordingVideoToDiskDelegate);
        scriptingInvocation.AddObject(m_OnVideoRecordingStoppedGCHandle.Resolve());
        scriptingInvocation.AddInt64(S_OK);
        scriptingInvocation.Invoke();

        Assert(m_OnVideoRecordingStoppedGCHandle.HasTarget());
        m_OnVideoRecordingStoppedGCHandle.ReleaseAndClear();
    }

    //---------------------------------------------------------------
    void VideoCapture::StartRecordingVideoToDisk(const ICallString & filename, const ScriptingObjectPtr callback)
    {
        DebugAssert(CurrentThread::IsMainThread());

        if (m_HasErrors)
        {
            NotifyScriptOfError(OperationType::StartRecordingVideo, E_FAIL, "Failed to start recording: VideoCapture is in an erroneous state.", callback);
            return;
        }

        if (WebCam::GetInstance().GetWebCamMode() == WebCam::PhotoMode)
        {
            NotifyScriptOfError(OperationType::StartRecordingVideo, E_FAIL, "Can not record a video while in photo mode.", callback);
            return;
        }

        if (WebCam::GetInstance().GetWebCamMode() != WebCam::VideoMode)
        {
            NotifyScriptOfError(OperationType::StartRecordingVideo, E_FAIL, "StartVideoModeAsync must be called prior to recording a video.", callback);
            return;
        }

        if (s_VideoCaptureStatics->activeInstance != this)
        {
            NotifyScriptOfError(OperationType::StartRecordingVideo, E_FAIL, "Only the capture instance that started the video mode can record video.", callback);
            return;
        }

        if (m_RecordingState != RecordingState::Idle)
        {
            NotifyScriptOfError(OperationType::StartRecordingVideo, E_FAIL, "The VideoCapture is not in the correct state to start recording video.", callback);
            return;
        }

        m_IsRecording = true;
        m_RecordingState = RecordingState::PreparingToStartRecording;

        m_OnVideoRecordingStartedGCHandle.AcquireStrong(callback);

        HRESULT hr;
        win::ComPtr<UnityWinRTBase::IStorageFolderStatics> folderStatics;
        hr = UnityWinRTBase::RoGetActivationFactory(UnityWinRTBase::HStringReference(L"Windows.Storage.StorageFolder"), __uuidof(folderStatics), &folderStatics);
        VerifyHRAndNotifyScript(OperationType::StartRecordingVideo, hr, m_OnVideoRecordingStartedGCHandle);

        UnityWinRTBase::HString platformFile;
        UnityWinRTBase::HString platformPath;

        // Split out filename into separate 'file' and 'path' strings and copy the chars into Platform string fields
        {
            const wchar_t* rawFilename = reinterpret_cast<const wchar_t*>(filename.GetRawCharBuffer());
            const int len = wcslen(rawFilename);

            // Find the index of the separator char between filename and directory path
            int separatorIndex = 0;
            if (len > 0)
            {
                const wchar_t * endPos = rawFilename + len;
                while (endPos > rawFilename)
                {
                    if (*endPos == L'\\' || *endPos == L'/')
                    {
                        separatorIndex = (int)(endPos - rawFilename);
                        break;
                    }
                    else endPos = endPos - 1;
                }
            }

            if (separatorIndex > 0)
            {
                platformFile = UnityWinRTBase::HString(rawFilename + separatorIndex + 1, len - separatorIndex - 1);
                platformPath = UnityWinRTBase::HString(rawFilename, separatorIndex);
            }
            else
            {
                platformFile = UnityWinRTBase::HString(rawFilename);
            }
        }

        win::ComPtr<UnityWinRTBase::IAsyncOperation<UnityWinRTBase::Windows::Storage::StorageFolder*> > asyncFolderOperation;
        hr = folderStatics->GetFolderFromPathAsync(platformPath, &asyncFolderOperation);
        VerifyHRAndNotifyScript(OperationType::StartRecordingVideo, hr, m_OnVideoRecordingStartedGCHandle);

        // Yay for C++14 syntax allowing move semantics for value-captured variables (file/path HString objects)
        hr = asyncFolderOperation->put_Completed(AsyncOperationHandlerFactory<UnityWinRTBase::Windows::Storage::StorageFolder*>::Create([this, file = std::move(platformFile), path = std::move(platformPath)](
            UnityWinRTBase::IAsyncOperation<UnityWinRTBase::Windows::Storage::StorageFolder*>* asyncFolderOperation,
            UnityWinRTBase::Windows::Foundation::AsyncStatus asyncStatus)
            {
                HRESULT hr;

                // Retrieve the StorageFile and use it to create a new file
                win::ComPtr<UnityWinRTBase::Windows::Storage::IStorageFolder> storageFolder;
                hr = asyncFolderOperation->GetResults(&storageFolder);
                VerifyHRAndNotifyScriptWithReturnValue(OperationType::StartRecordingVideo, hr, m_OnVideoRecordingStartedGCHandle, S_OK);

                win::ComPtr<UnityWinRTBase::IAsyncOperation<UnityWinRTBase::Windows::Storage::StorageFile*> > asyncFileOperation;
                hr = storageFolder->CreateFileAsync(file, UnityWinRTBase::CreationCollisionOption::CreationCollisionOption_GenerateUniqueName, &asyncFileOperation);
                VerifyHRAndNotifyScriptWithReturnValue(OperationType::StartRecordingVideo, hr, m_OnVideoRecordingStartedGCHandle, S_OK);

                hr = asyncFileOperation->put_Completed(AsyncOperationHandlerFactory<UnityWinRTBase::Windows::Storage::StorageFile*>::Create([this](
                    UnityWinRTBase::IAsyncOperation<UnityWinRTBase::Windows::Storage::StorageFile*>* asyncFileOperation,
                    UnityWinRTBase::Windows::Foundation::AsyncStatus asyncStatus)
                    {
                        HRESULT hr;

                        // Retrieve the newly created file and begin video capturing
                        win::ComPtr<UnityWinRTBase::Windows::Storage::IStorageFile> storageFile;
                        hr = asyncFileOperation->GetResults(&storageFile);
                        VerifyHRAndNotifyScriptWithReturnValue(OperationType::StartRecordingVideo, hr, m_OnVideoRecordingStartedGCHandle, S_OK);

                        // m_MediaCapture can only be accessed from the main thread, so send an event to invoke StartActualRecordingAsync
                        // which completes this operation.
                        // NOTE: Must AddRef on storageFile COM object because event data is block-copied (ComPtr object is destroyed)
                        storageFile->AddRef();
                        OnVideoCaptureStartRecordingOperationEventData eventData = { this, storageFile };
                        GlobalEventQueue::GetInstance().SendEvent(eventData);

                        return S_OK;
                        // End CreateFileAsync() handler
                    }));

                VerifyHRAndNotifyScriptWithReturnValue(OperationType::StartRecordingVideo, hr, m_OnVideoRecordingStartedGCHandle, S_OK);
                return S_OK;
                // End GetFolderFromPathAsync() handler
            }));
        VerifyHRAndNotifyScript(OperationType::StartRecordingVideo, hr, m_OnVideoRecordingStartedGCHandle);
    }

    //---------------------------------------------------------------
    void VideoCapture::StopRecordingVideoToDisk(const ScriptingObjectPtr callback)
    {
        DebugAssert(CurrentThread::IsMainThread());

        if (m_HasErrors)
        {
            NotifyScriptOfError(OperationType::StopRecordingVideo, E_FAIL, "Failed to stop recording: VideoCapture is in an erroneous state.", callback);
            return;
        }

        if (WebCam::GetInstance().GetWebCamMode() == WebCam::PhotoMode)
        {
            NotifyScriptOfError(OperationType::StopRecordingVideo, E_FAIL, "Can not record a video while in photo mode.", callback);
            return;
        }

        if (s_VideoCaptureStatics->videoModeState != VideoModeState::Activated)
        {
            NotifyScriptOfError(OperationType::StopRecordingVideo, E_FAIL, "Video Mode has not been started.", callback);
            return;
        }

        if (s_VideoCaptureStatics->activeInstance != this)
        {
            NotifyScriptOfError(OperationType::StopRecordingVideo, E_FAIL, "Only the capture instance that started the video mode can stop recording video.", callback);
            return;
        }

        if (m_RecordingState != RecordingState::Recording)
        {
            NotifyScriptOfError(OperationType::StopRecordingVideo, E_FAIL, "The VideoCapture is not in the correct state to stop recording video.", callback);
            return;
        }

        m_RecordingState = RecordingState::PreparingToStopRecording;

        win::ComPtr<UnityWinRTBase::IAsyncAction> asyncAction;
        HRESULT hr;

        m_OnVideoRecordingStoppedGCHandle.AcquireStrong(callback);
        hr = m_MediaCapture->StopRecordAsync(&asyncAction);
        VerifyHRAndNotifyScript(OperationType::StopRecordingVideo, hr, m_OnVideoRecordingStoppedGCHandle);

        hr = asyncAction->put_Completed(AsyncActionHandlerFactory::Create([this](
            UnityWinRTBase::Windows::Foundation::IAsyncAction* asyncAction,
            UnityWinRTBase::Windows::Foundation::AsyncStatus asyncStatus)
            {
                if (asyncStatus == UnityWinRTBase::Completed)
                {
                    {
                        OnVideoCaptureAsyncOperationCompleteEventData eventData = { this, &VideoCapture::EnableAudioRecording };
                        GlobalEventQueue::GetInstance().SendEvent(eventData);
                    }

                    {
                        OnVideoCaptureAsyncOperationCompleteEventData eventData = { this, &VideoCapture::DisableMixedRealityVideoEffectsAsync_StopRecording };
                        GlobalEventQueue::GetInstance().SendEvent(eventData);
                    }
                }
                else
                {
                    HRESULT errorCode;

                    HRESULT hr = GetErrorCodeFromAsyncAction(asyncAction, &errorCode);
                    VerifyHRAndNotifyScriptWithReturnValue(OperationType::StopRecordingVideo, hr, m_OnVideoRecordingStoppedGCHandle, S_OK);

                    // Report actual error returned from IAsyncInfo
                    VerifyHRAndNotifyScriptWithReturnValue(OperationType::StopRecordingVideo, errorCode, m_OnVideoRecordingStoppedGCHandle, S_OK);
                }

                return S_OK;
                // End StopRecordAsync() handler
            }));

        VerifyHRAndNotifyScript(OperationType::StopRecordingVideo, hr, m_OnVideoRecordingStoppedGCHandle);
    }

    //---------------------------------------------------------------
    void VideoCapture::NotifyScriptOfErrorOnMainThread(OperationType operationType, HRESULT hr, ScriptingGCHandle callbackGCHandle)
    {
        if (CurrentThread::IsMainThread())
        {
            NotifyScriptOfError(operationType, hr, callbackGCHandle);
        }
        else
        {
            OnNotifyScriptAboutVideoCaptureErrorEventData eventData = { this, operationType, hr, ScriptingGCHandle::ToScriptingBackendNativeGCHandle(callbackGCHandle) };
            GlobalEventQueue::GetInstance().SendEvent(eventData);
        }
    }

    //---------------------------------------------------------------
    void VideoCapture::HandleEvent(const OnNotifyScriptAboutVideoCaptureErrorEventData& eventData)
    {
        if (eventData.instance != this)
            return;

        NotifyScriptOfError(eventData.operationType, eventData.hr, ScriptingGCHandle::FromScriptingBackendNativeGCHandle(eventData.callbackGCHandle));
    }

    //---------------------------------------------------------------
    void VideoCapture::NotifyScriptOfError(OperationType operationType, HRESULT hr, ScriptingGCHandle callbackGCHandle)
    {
        NotifyScriptOfError(operationType, hr, NULL, callbackGCHandle.Resolve());
    }

    //---------------------------------------------------------------
    void VideoCapture::NotifyScriptOfError(OperationType operationType, HRESULT hr, const char* message, ScriptingObjectPtr callback)
    {
        ScriptingInvocation scriptingInvocation;

        if (message != NULL)
        {
            ErrorString(message);
        }

        if (operationType == OperationType::Create)
        {
            scriptingInvocation = ScriptingInvocation(GetCoreScriptingClasses().invokeOnCreatedVideoCaptureResourceDelegate);
            scriptingInvocation.AddObject(callback);
            scriptingInvocation.AddIntPtr(NULL);
        }
        else if (operationType == OperationType::StartVideoMode)
        {
            scriptingInvocation = ScriptingInvocation(GetCoreScriptingClasses().invokeOnVideoModeStartedDelegate);
            scriptingInvocation.AddObject(callback);
            scriptingInvocation.AddInt64(hr);
        }
        else if (operationType == OperationType::StopVideoMode)
        {
            scriptingInvocation = ScriptingInvocation(GetCoreScriptingClasses().invokeOnVideoModeStoppedDelegate);
            scriptingInvocation.AddObject(callback);
            scriptingInvocation.AddInt64(hr);
        }
        else if (operationType == OperationType::StartRecordingVideo)
        {
            scriptingInvocation = ScriptingInvocation(GetCoreScriptingClasses().invokeOnStartedRecordingVideoToDiskDelegate);
            scriptingInvocation.AddObject(callback);
            scriptingInvocation.AddInt64(hr);
            scriptingInvocation.AddIntPtr(NULL);
        }
        else if (operationType == OperationType::StopRecordingVideo)
        {
            scriptingInvocation = ScriptingInvocation(GetCoreScriptingClasses().invokeOnStoppedRecordingVideoToDiskDelegate);
            scriptingInvocation.AddObject(callback);
            scriptingInvocation.AddInt64(hr);
        }

        scriptingInvocation.Invoke();
    }

#if UNITY_EDITOR
    void VideoCapture::OnExitPlayMode()
    {
        // Dispose all active VideoCapture instances to ensure webcam streaming/recording is stopped
        WebCam::GetInstance().SetWebCamMode(WebCam::None);

        s_VideoCaptureStatics->videoModeState = VideoModeState::Deactivated;
        s_VideoCaptureStatics->activeInstance = nullptr;

        for (auto& inst : s_VideoCaptureStatics->videoCaptureInstances)
        {
            if (!inst->m_IsQueuedForDestruction)
                inst->Dispose();
        }
    }

#endif

#undef VerifyHRAndNotifyScript
#undef VerifyHRAndNotifyScriptWithReturnValue
} // end namespace Unity
