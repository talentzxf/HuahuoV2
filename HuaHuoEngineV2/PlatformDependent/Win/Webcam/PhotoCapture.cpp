#include "UnityPrefix.h"

#include "External/Windows10/src/ComHelpers.h"
#include "External/Windows10/src/SynchronousAction.h"
#include "External/Windows10/src/WinRTFunctions.h"
#include "PhotoCapture.h"

#include "PhotoCaptureFrame.h"
#include "MixedRealityCaptureVideoEffect.h"
#include "PhotoCaptureOperation.h"
#include "ComPtr.h"
#include "IMFSampleImageFileWriter.h"
#include "PlatformDependent/Win/Webcam/WebCam.h"
#include "Runtime/Core/Callbacks/GlobalCallbacks.h"
#include "Runtime/Graphics/ScreenManager.h"
#include "Runtime/Interfaces/IVRDevice.h"
#include "Runtime/Math/Matrix4x4.h"
#include "Runtime/Scripting/ScriptingInvocation.h"
#include "Runtime/Scripting/CommonScriptingClasses.h"
#include "Runtime/Scripting/ScriptingExportUtility.h"
#include "Runtime/Utilities/EnumTraits.h"
#include "Runtime/Threads/CurrentThread.h"

#if PLATFORM_WINRT
#include "PlatformDependent/MetroPlayer/MetroCapabilities.h"
#endif

#include <mfidl.h>

namespace UnityWinRTBase
{
    using namespace UnityWinRTBase::Windows::Foundation;
    using namespace UnityWinRTBase::Windows::Foundation::Collections;
    using namespace UnityWinRTBase::Windows::Foundation::Numerics;
    using namespace UnityWinRTBase::Windows::Media;
    using namespace UnityWinRTBase::Windows::Media::Capture;
    using namespace UnityWinRTBase::Windows::Media::Devices;
    using namespace UnityWinRTBase::Windows::Media::MediaProperties;
    using namespace UnityWinRTBase::Windows::Perception::Spatial;
    using namespace UnityWinRTBase::Windows::Devices::Enumeration;
}

struct OnCreatedPhotoCaptureResourceEventData
{
    Unity::PhotoCapture* instance;
};

struct OnStartedPhotoModeEventData
{
    Unity::PhotoCapture* instance;
    int callbackId;
};

struct OnStoppedPhotoModeEventData
{
    Unity::PhotoCapture* instance;
};

struct OnCapturedPhotoToMemoryEventData
{
    Unity::PhotoCapture* instance;
    Unity::PhotoCaptureOperationParameters *photoCaptureOperationParams;
    IMFSample* mfSample;

    void Destroy()
    {
        mfSample->Release();

        if (photoCaptureOperationParams != NULL)
        {
            photoCaptureOperationParams->callbackGCHandle.ReleaseAndClear();

            UNITY_DELETE(photoCaptureOperationParams, kMemWebCam);
            photoCaptureOperationParams = NULL;
        }
    }
};

struct OnPhotoCaptureProcessNextTaskEventData
{
    Unity::PhotoCapture* instance;
};

struct OnPhotoCaptureDestructionEventData
{
    Unity::PhotoCapture* instance;
};

struct OnNotifyScriptAboutErrorEventData
{
    Unity::PhotoCapture* instance;
    Unity::PhotoCapture::OperationType operationType;
    HRESULT hr;
    ScriptingBackendNativeGCHandle callbackGCHandle;

    void Destroy()
    {
        ScriptingGCHandle::FromScriptingBackendNativeGCHandle(callbackGCHandle).ReleaseAndClear();
    }
};

REGISTER_EVENT_ID(0xE01867225D6E42F1, 0xA29D5C6CFEF6DBE1, OnCreatedPhotoCaptureResourceEventData)
REGISTER_EVENT_ID(0x355AF7E3A4044B1C, 0x80FC05808E8A0310, OnStartedPhotoModeEventData)
REGISTER_EVENT_ID(0xFDD34FB465DF4E57, 0x9E7CCCA154331A39, OnStoppedPhotoModeEventData)
REGISTER_EVENT_ID(0x8876C60C5DD24D7C, 0xAFC34336F5E5323F, OnPhotoCaptureProcessNextTaskEventData)
REGISTER_EVENT_ID(0xF5BA6C160F4D40D3, 0x985E2D8BCC3FB4CB, OnPhotoCaptureDestructionEventData)
REGISTER_EVENT_ID_WITH_CLEANUP(0x00C1C2CEE3A14A1A, 0x9CB83D211BB8751C, OnNotifyScriptAboutErrorEventData)
REGISTER_EVENT_ID_WITH_CLEANUP(0xE3E3F40B18D04D91, 0x93AA04BFB81D4410, OnCapturedPhotoToMemoryEventData)

namespace Unity
{
    PhotoCapture::PhotoModeState PhotoCapture::s_PhotoModeState = PhotoCapture::PhotoModeState::Deactivated;
    void* PhotoCapture::s_ActivateCaptureInstance = NULL;

    dynamic_array<Resolution> PhotoCapture::GetSupportedResolutions()
    {
        dynamic_array<Resolution> supportedResolutions(kMemTempAlloc);

#if PLATFORM_WINRT
        if (!metro::Capabilities::IsSupported(metro::Capabilities::kWebCam, "because you're using PhotoCapture"))
            return supportedResolutions;
#endif

        win::ComPtr<UnityWinRTBase::IInspectable> mediaCaptureInspectable;
        win::ComPtr<UnityWinRTBase::IMediaCapture> mediaCapture;

        HRESULT hr = UnityWinRTBase::RoActivateInstance(UnityWinRTBase::HStringReference(L"Windows.Media.Capture.MediaCapture"), &mediaCaptureInspectable);
        if (FAILED(hr))
        {
            WarningString("Warning: failed to create Windows::Media::Capture::MediaCapture object. PhotoCapture requires running on at least Windows 8.1.");
            return supportedResolutions;
        }

#define CheckHR(hr) do { if (FAILED(hr)) return supportedResolutions; } while (false)

        hr = mediaCaptureInspectable.As(&mediaCapture);
        CheckHR(hr);

        win::ComPtr<UnityWinRTBase::IAsyncAction> asyncAction;

        // InitializeWithSettingsAsync throws exceptions in editor
        // but works correctly on the HoloLens.  We want to use
        // this API on the HoloLens so that we do not require the
        // microphone capability when using the PhotoCapture API.
        // When running in editor, we use InitializeAsync which
        // works as intended.
#if UNITY_EDITOR
        hr = mediaCapture->InitializeAsync(&asyncAction);
        CheckHR(hr);
#else
        win::ComPtr<UnityWinRTBase::IInspectable> settingsInspectable;
        win::ComPtr<UnityWinRTBase::IMediaCaptureInitializationSettings> settings;
        hr = UnityWinRTBase::RoActivateInstance(UnityWinRTBase::HStringReference(L"Windows.Media.Capture.MediaCaptureInitializationSettings"), &settingsInspectable);
        CheckHR(hr);

        hr = settingsInspectable.As(&settings);
        CheckHR(hr);

        hr = settings->put_StreamingCaptureMode(UnityWinRTBase::Windows::Media::Capture::StreamingCaptureMode_Video);
        CheckHR(hr);

        hr = mediaCapture->InitializeWithSettingsAsync(settings.Get(), &asyncAction);
        CheckHR(hr);
#endif

        hr = UnityWinRTBase::SynchronousAction::Wait(kMemWebCam, asyncAction);
        CheckHR(hr);

        win::ComPtr<UnityWinRTBase::IVideoDeviceController> videoController;
        hr = mediaCapture->get_VideoDeviceController(&videoController);
        CheckHR(hr);

        win::ComPtr<UnityWinRTBase::IMediaDeviceController> mediaController;
        hr = videoController.As(&mediaController);
        CheckHR(hr);

        win::ComPtr<UnityWinRTBase::IVectorView<UnityWinRTBase::IMediaEncodingProperties*> > encodingProperties;
        hr = mediaController->GetAvailableMediaStreamProperties(UnityWinRTBase::MediaStreamType_Photo, &encodingProperties);
        CheckHR(hr);

        uint32_t count;
        hr = encodingProperties->get_Size(&count);
        CheckHR(hr);

        dynamic_array<UnityWinRTBase::IMediaEncodingProperties*> properties(kMemTempAlloc);
        properties.resize_uninitialized(count);

        uint32_t actualCount;
        hr = encodingProperties->GetMany(0, count, properties.data(), &actualCount);
        CheckHR(hr);
        AssertMsg(actualCount == count, "Invalid encoding property count for resolutions");

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

    PhotoCapture * PhotoCapture::Instantiate(bool showHolograms, ScriptingObjectPtr callback)
    {
#if PLATFORM_WINRT
        if (!metro::Capabilities::IsSupported(metro::Capabilities::kWebCam, "because you're using PhotoCapture"))
            return NULL;
#endif

        win::ComPtr<UnityWinRTBase::IInspectable> mediaCaptureInspectable;
        win::ComPtr<UnityWinRTBase::IMediaCapture> mediaCapture;

        HRESULT hr = UnityWinRTBase::RoActivateInstance(UnityWinRTBase::HStringReference(L"Windows.Media.Capture.MediaCapture"), &mediaCaptureInspectable);
        if (FAILED(hr))
        {
            WarningString("Warning: failed to create Windows::Media::Capture::MediaCapture object. PhotoCapture requires running on at least Windows 8.1.");
            return nullptr;
        }

        hr = mediaCaptureInspectable.As(&mediaCapture);
        if (FAILED(hr))
        {
            ErrorString("Failed to cast Windows::Media::Capture::MediaCapture object to IMediaCapture."); // Something has gone terribly wrong
            return nullptr;
        }

        return UNITY_NEW(PhotoCapture, kMemWebCam)(showHolograms, callback, mediaCapture);
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

#define VerifyCriticalHR(hr, message) \
do \
{ \
    if (FAILED(hr)) \
    { \
        AssertFormatMsg(false, message, hr); \
        m_HasErrors = true; \
        return; \
    } \
} \
while (false)

#define VerifyCriticalHRWithReturnValue(hr, returnValue, message) \
do \
{ \
    if (FAILED(hr)) \
    { \
        AssertFormatMsg(false, message, hr); \
        m_HasErrors = true; \
        return returnValue; \
    } \
} \
while (false)

    // If this fails, we log an error and just continue with the next task
#define VerifyHR(hr, message) \
do \
{ \
    if (FAILED(hr)) \
    { \
        AssertFormatMsg(false, message, hr); \
        return; \
    } \
} \
while (false)

#if UNITY_EDITOR
    dynamic_array<PhotoCapture*> PhotoCapture::s_PhotoCaptureInstances(kMemWebCam);
#endif

    //---------------------------------------------------------------
    // HoloLens firmware currently has hologram capture functionality disabled.
    // To prevent error spam, we will set m_ShowHolograms to false until
    // Microsoft re-enables the functionality.
    PhotoCapture::PhotoCapture(bool showHolograms, const ScriptingObjectPtr onCreateCallback, UnityWinRTBase::Windows::Media::Capture::IMediaCapture* mediaCapture) :
        m_Tasks(*this),
        m_OnCreateCallbackGCHandle(onCreateCallback, GCHANDLE_STRONG),
        m_OnPhotoModeStartedGCHandle(),
        m_OnPhotoModeStoppedGCHandle(),
        m_PhotoProperties(NULL),
        m_MediaCapture(mediaCapture),
        m_EncodingProperties(NULL),
        m_LowLagPhotoCapture(NULL),
        m_ShowHolograms(showHolograms),
        m_HasErrors(false),
        m_IsDisposed(false)
    {
#if UNITY_EDITOR
        m_ExitedPlayMode = false;
        m_IsQueuedForDestruction = false;
        s_PhotoCaptureInstances.push_back(this);
#endif

        GlobalEventQueue::GetInstance().AddHandler(m_OnCreatedPhotoCaptureResourceEventDelegate.SetObject(this));
        GlobalEventQueue::GetInstance().AddHandler(m_OnStartedPhotoModeEventDelegate.SetObject(this));
        GlobalEventQueue::GetInstance().AddHandler(m_OnStoppedPhotoModeEventDelegate.SetObject(this));
        GlobalEventQueue::GetInstance().AddHandler(m_OnCapturedPhotoToMemoryEventDelegate.SetObject(this));
        GlobalEventQueue::GetInstance().AddHandler(m_OnProcessNextTaskEventDelegate.SetObject(this));
        GlobalEventQueue::GetInstance().AddHandler(m_OnNotifyScriptAboutErrorEventDelegate.SetObject(this));

        static UnityEventQueue::StaticFunctionEventHandler<OnPhotoCaptureDestructionEventData> s_DestroyEventDelegate(&PhotoCapture::HandleDestructionEvent);
        static bool s_StaticInitializationComplete = false;
        if (!s_StaticInitializationComplete)
        {
            s_StaticInitializationComplete = true;
            GlobalEventQueue::GetInstance().AddHandler(&s_DestroyEventDelegate);
#if UNITY_EDITOR
            GlobalCallbacks::Get().exitPlayModeAfterOnEnableInEditMode.Register(&PhotoCapture::MarkExitedPlayMode);
            GlobalCallbacks::Get().domainUnloadComplete.Register(&PhotoCapture::DestroyAllPhotoCaptureWebCams);
#endif
        }

        m_Tasks.Initialize();
        m_Tasks.AddTask(&PhotoCapture::InitializeAsync);
    }

    //---------------------------------------------------------------
    void PhotoCapture::InitializeAsync()
    {
        win::ComPtr<UnityWinRTBase::IInspectable> initializationSettingsInspectable;
        win::ComPtr<UnityWinRTBase::IMediaCaptureInitializationSettings> initializationSettings;
        win::ComPtr<UnityWinRTBase::IAsyncAction> asyncAction;

        HRESULT hr = S_OK;

        // InitializeWithSettingsAsync throws exceptions in editor
        // but works correctly on the HoloLens.  We want to use
        // this API on the HoloLens so that we do not require the
        // microphone capability when using the PhotoCapture API.
        // When running in editor, we use InitializeAsync which
        // works as intended.
#if UNITY_EDITOR
        hr = GetMediaCapture()->InitializeAsync(&asyncAction);
        VerifyHRAndNotifyScript(OperationType::Create, hr, m_OnCreateCallbackGCHandle);
#else
        win::ComPtr<UnityWinRTBase::IInspectable> settingsInspectable;
        win::ComPtr<UnityWinRTBase::IMediaCaptureInitializationSettings> settings;
        hr = UnityWinRTBase::RoActivateInstance(UnityWinRTBase::HStringReference(L"Windows.Media.Capture.MediaCaptureInitializationSettings"), &settingsInspectable);
        VerifyHRAndNotifyScript(OperationType::Create, hr, m_OnCreateCallbackGCHandle);

        hr = settingsInspectable.As(&settings);
        VerifyHRAndNotifyScript(OperationType::Create, hr, m_OnCreateCallbackGCHandle);

        hr = settings->put_StreamingCaptureMode(UnityWinRTBase::Windows::Media::Capture::StreamingCaptureMode_Video);
        VerifyHRAndNotifyScript(OperationType::Create, hr, m_OnCreateCallbackGCHandle);

        hr = GetMediaCapture()->InitializeWithSettingsAsync(settings.Get(), &asyncAction);
        VerifyHRAndNotifyScript(OperationType::Create, hr, m_OnCreateCallbackGCHandle);
#endif

        hr = asyncAction->put_Completed(this);
        VerifyHRAndNotifyScript(OperationType::Create, hr, m_OnCreateCallbackGCHandle);
    }

#if UNITY_EDITOR
    void PhotoCapture::MarkExitedPlayMode()
    {
        WebCam::GetInstance().SetWebCamMode(WebCam::None);
        s_PhotoModeState = PhotoModeState::Deactivated;
        s_ActivateCaptureInstance = NULL;

        for (size_t i = 0; i < s_PhotoCaptureInstances.size(); i++)
        {
            s_PhotoCaptureInstances[i]->m_ExitedPlayMode = true;
        }
    }

    void PhotoCapture::DestroyAllPhotoCaptureWebCams()
    {
        dynamic_array<PhotoCapture*> webcams(s_PhotoCaptureInstances, kMemTempAlloc);

        for (size_t i = 0; i < webcams.size(); i++)
        {
            if (!webcams[i]->m_IsQueuedForDestruction)
                webcams[i]->Dispose();
        }
    }

#endif

    //---------------------------------------------------------------
    void PhotoCapture::Dispose()
    {
        AssertMsg(CurrentThread::IsMainThread(), "Can not dispose of a PhotoCapture object off the main thread.");

        if (m_IsDisposed)
            return;

        m_IsDisposed = true;

        if (s_ActivateCaptureInstance == this)
        {
            s_ActivateCaptureInstance = NULL;
            WebCam::GetInstance().SetWebCamMode(WebCam::None);
        }

        if (s_PhotoModeState != PhotoModeState::Deactivating &&
            s_PhotoModeState != PhotoModeState::Deactivated &&
            m_LowLagPhotoCapture != nullptr)
        {
            DeactivateLowLagCamera(false);
        }

        GlobalEventQueue::GetInstance().RemoveHandler(&m_OnCreatedPhotoCaptureResourceEventDelegate);
        GlobalEventQueue::GetInstance().RemoveHandler(&m_OnStartedPhotoModeEventDelegate);
        GlobalEventQueue::GetInstance().RemoveHandler(&m_OnStoppedPhotoModeEventDelegate);
        GlobalEventQueue::GetInstance().RemoveHandler(&m_OnCapturedPhotoToMemoryEventDelegate);
        GlobalEventQueue::GetInstance().RemoveHandler(&m_OnProcessNextTaskEventDelegate);
        GlobalEventQueue::GetInstance().RemoveHandler(&m_OnNotifyScriptAboutErrorEventDelegate);

        m_OnCreateCallbackGCHandle.ReleaseAndClear();
        m_OnPhotoModeStartedGCHandle.ReleaseAndClear();
        m_OnPhotoModeStoppedGCHandle.ReleaseAndClear();

#if UNITY_EDITOR
        for (size_t i = 0; i < s_PhotoCaptureInstances.size(); i++)
        {
            if (s_PhotoCaptureInstances[i] == this)
            {
                s_PhotoCaptureInstances[i] = s_PhotoCaptureInstances[s_PhotoCaptureInstances.size() - 1];
                s_PhotoCaptureInstances.pop_back();
                break;
            }
        }
#endif

        Release();
    }

    //---------------------------------------------------------------
    void PhotoCapture::DisposeThreaded()
    {
#if UNITY_EDITOR
        m_IsQueuedForDestruction = true;
#endif

        OnPhotoCaptureDestructionEventData eventData = { this };
        GlobalEventQueue::GetInstance().SendEvent(eventData);
    }

    //---------------------------------------------------------------
    void* PhotoCapture::GetUnsafePointerToVideoDeviceController()
    {
        UnityWinRTBase::IVideoDeviceController *videoController;
        HRESULT hr = GetMediaCapture()->get_VideoDeviceController(&videoController);
        AssertFormatMsg(SUCCEEDED(hr), "Failed to retrieve the video device controller pointer from media capture! [0x%x]", hr);

        return videoController;
    }

    //---------------------------------------------------------------
    void PhotoCapture::NotifyScriptOfErrorOnMainThread(OperationType operationType, HRESULT hr, ScriptingGCHandle callbackGCHandle)
    {
        if (CurrentThread::IsMainThread())
        {
            NotifyScriptOfError(operationType, hr, callbackGCHandle);
        }
        else
        {
            OnNotifyScriptAboutErrorEventData eventData = { this, operationType, hr, ScriptingGCHandle::ToScriptingBackendNativeGCHandle(callbackGCHandle) };
            GlobalEventQueue::GetInstance().SendEvent(eventData);
        }
    }

    //---------------------------------------------------------------
    void PhotoCapture::NotifyScriptOfError(OperationType operationType, HRESULT hr, ScriptingGCHandle callbackGCHandle)
    {
        NotifyScriptOfError(operationType, hr, NULL, callbackGCHandle.Resolve());
    }

    //---------------------------------------------------------------
    void PhotoCapture::NotifyScriptOfError(OperationType operationType, HRESULT hr, const char* message, ScriptingObjectPtr callback)
    {
        ScriptingInvocation scriptingInvocation;

        if (message != NULL)
        {
            ErrorString(message);
        }

        if (operationType == OperationType::Create)
        {
            scriptingInvocation = ScriptingInvocation(GetCoreScriptingClasses().invokeOnCreatedResourceDelegate);
            scriptingInvocation.AddObject(callback);
            scriptingInvocation.AddIntPtr(NULL);
        }
        else if (operationType == OperationType::StartPhotoMode)
        {
            scriptingInvocation = ScriptingInvocation(GetCoreScriptingClasses().invokeOnPhotoModeStartedDelegate);
            scriptingInvocation.AddObject(callback);
            scriptingInvocation.AddInt64(hr);
        }
        else if (operationType == OperationType::StopPhotoMode)
        {
            scriptingInvocation = ScriptingInvocation(GetCoreScriptingClasses().invokeOnPhotoModeStoppedDelegate);
            scriptingInvocation.AddObject(callback);
            scriptingInvocation.AddInt64(hr);
        }
        else if (operationType == OperationType::TakePhotoAsyncToMemory)
        {
            scriptingInvocation = ScriptingInvocation(GetCoreScriptingClasses().invokeOnCapturedPhotoToMemoryDelegate);
            scriptingInvocation.AddObject(callback);
            scriptingInvocation.AddInt64(hr);
            scriptingInvocation.AddIntPtr(NULL);
        }
        else if (operationType == OperationType::TakePhotoAsyncToDisk)
        {
            scriptingInvocation = ScriptingInvocation(GetCoreScriptingClasses().invokeOnCapturedPhotoToDiskDelegate);
            scriptingInvocation.AddObject(callback);
            scriptingInvocation.AddInt64(hr);
        }

        scriptingInvocation.Invoke();
    }

    //---------------------------------------------------------------
    void PhotoCapture::ProcessNextTaskOnMainThread()
    {
        if (CurrentThread::IsMainThread())
        {
            m_Tasks.ProcessNextTask();
        }
        else
        {
            OnPhotoCaptureProcessNextTaskEventData e = { this };
            GlobalEventQueue::GetInstance().SendEvent(e);
        }
    }

    //---------------------------------------------------------------
    void PhotoCapture::HandleEvent(const OnPhotoCaptureProcessNextTaskEventData& eventData)
    {
        if (eventData.instance != this)
            return;

        m_Tasks.ProcessNextTask();
    }

    //---------------------------------------------------------------
    void PhotoCapture::HandleEvent(const OnNotifyScriptAboutErrorEventData& eventData)
    {
        if (eventData.instance != this)
            return;

        NotifyScriptOfError(eventData.operationType, eventData.hr, ScriptingGCHandle::FromScriptingBackendNativeGCHandle(eventData.callbackGCHandle));
    }

    //---------------------------------------------------------------
    void PhotoCapture::HandleDestructionEvent(const OnPhotoCaptureDestructionEventData & buffer)
    {
        buffer.instance->Dispose();
    }

    //---------------------------------------------------------------
    void PhotoCapture::OnCreatedPhotoCaptureResource()
    {
        OnCreatedPhotoCaptureResourceEventData eventData = { this };
        GlobalEventQueue::GetInstance().SendEvent(eventData);
    }

    //---------------------------------------------------------------
    void PhotoCapture::HandleEvent(const OnCreatedPhotoCaptureResourceEventData& eventData)
    {
        if (eventData.instance != this)
        {
            return;
        }

        if (!IsWebCameraAvailable())
        {
            NotifyScriptOfError(OperationType::Create, E_FAIL, "No web camera is available for use.", m_OnCreateCallbackGCHandle.Resolve());
            this->Dispose();
            return;
        }

        ScriptingGCHandle callback = m_OnCreateCallbackGCHandle;
        m_OnCreateCallbackGCHandle.Clear();

        ScriptingInvocation scriptingInvocation(GetCoreScriptingClasses().invokeOnCreatedResourceDelegate);
        scriptingInvocation.AddObject(callback.Resolve());
        scriptingInvocation.AddIntPtr(eventData.instance);
        scriptingInvocation.Invoke();

        Assert(callback.HasTarget());
        callback.ReleaseAndClear();
    }

    //---------------------------------------------------------------
    void PhotoCapture::StartPhotoMode(const CameraParameters &cameraParameters, const ScriptingObjectPtr callback)
    {
        if (WebCam::GetInstance().GetWebCamMode() == WebCam::PhotoMode)
        {
            NotifyScriptOfError(OperationType::StartPhotoMode, E_FAIL, "Photo Mode has already been started.", callback);
            return;
        }

        if (WebCam::GetInstance().GetWebCamMode() == WebCam::VideoMode)
        {
            NotifyScriptOfError(OperationType::StartPhotoMode, E_FAIL, "Can not start Photo Mode because Video Mode has already been started.", callback);
            return;
        }

        if (s_PhotoModeState != PhotoModeState::Deactivated || s_ActivateCaptureInstance != NULL)
        {
            NotifyScriptOfError(OperationType::StartPhotoMode, E_FAIL, "Photo Mode has already been started.", callback);
            return;
        }

        WebCam::GetInstance().SetWebCamMode(WebCam::PhotoMode);
        s_ActivateCaptureInstance = this;

        s_PhotoModeState = PhotoModeState::Activating;


        // Make sure we free an existing callback before creating a new one
        // just in case we were put into a bad state during the previous invocation.
        m_OnPhotoModeStartedGCHandle.ReleaseAndClear();
        m_OnPhotoModeStartedGCHandle.AcquireStrong(callback);

        m_CameraParameters = cameraParameters;
        SetupEncodingProperties(m_CameraParameters, m_EncodingProperties);

        m_Tasks.AddTask(&PhotoCapture::SetResolution);

        if (m_ShowHolograms)
        {
            m_Tasks.AddTask(&PhotoCapture::AddPhotoEffectAsync);
        }

        m_Tasks.AddTask(&PhotoCapture::ActivateLowLagCamera);
    }

    //---------------------------------------------------------------
    void PhotoCapture::OnStartedPhotoMode()
    {
        OnStartedPhotoModeEventData eventData = { this };
        GlobalEventQueue::GetInstance().SendEvent(eventData);
    }

    //---------------------------------------------------------------
    void PhotoCapture::HandleEvent(const OnStartedPhotoModeEventData& eventData)
    {
        if (eventData.instance != this)
        {
            return;
        }

        s_PhotoModeState = PhotoModeState::Activated;

        ScriptingGCHandle callback = m_OnPhotoModeStartedGCHandle;
        m_OnPhotoModeStartedGCHandle.Clear();

        ScriptingInvocation scriptingInvocation(GetCoreScriptingClasses().invokeOnPhotoModeStartedDelegate);
        scriptingInvocation.AddObject(callback.Resolve());
        scriptingInvocation.AddInt64(S_OK);
        scriptingInvocation.Invoke();

        Assert(callback.HasTarget());
        callback.ReleaseAndClear();
    }

    //---------------------------------------------------------------
    void PhotoCapture::StopPhotoMode(const ScriptingObjectPtr callback)
    {
        if (WebCam::GetInstance().GetWebCamMode() == WebCam::VideoMode)
        {
            NotifyScriptOfError(OperationType::StopPhotoMode, E_FAIL, "Can not stop Photo Mode because the web camera is currently in Video Mode.", callback);
            return;
        }

        if (s_PhotoModeState != PhotoModeState::Activated)
        {
            NotifyScriptOfError(OperationType::StopPhotoMode, E_FAIL, "Photo Mode has not been started.", callback);
            return;
        }

        if (s_ActivateCaptureInstance != this)
        {
            NotifyScriptOfError(OperationType::StopPhotoMode, E_FAIL, "Only the capture instance that started the photo mode can stop the photo mode.", callback);
            return;
        }

        s_PhotoModeState = PhotoModeState::Deactivating;

        // Make sure we free an existing callback before creating a new one
        // just in case we were put into a bad state during the previous invocation.
        m_OnPhotoModeStoppedGCHandle.ReleaseAndClear();
        m_OnPhotoModeStoppedGCHandle.AcquireStrong(callback);

        m_Tasks.AddTask(&PhotoCapture::RemovePhotoEffectAsync);
        m_Tasks.AddTask(&PhotoCapture::DeactivateLowLagCamera);
    }

    //---------------------------------------------------------------
    void PhotoCapture::OnStoppedPhotoMode()
    {
        OnStoppedPhotoModeEventData eventData = { this };
        GlobalEventQueue::GetInstance().SendEvent(eventData);
    }

    //---------------------------------------------------------------
    void PhotoCapture::HandleEvent(const OnStoppedPhotoModeEventData& eventData)
    {
        if (eventData.instance != this)
        {
            return;
        }

        s_ActivateCaptureInstance = NULL;
        WebCam::GetInstance().SetWebCamMode(WebCam::None);

        ScriptingGCHandle callback = m_OnPhotoModeStoppedGCHandle;
        m_OnPhotoModeStoppedGCHandle.Clear();

        s_PhotoModeState = PhotoModeState::Deactivated;
        ScriptingInvocation scriptingInvocation(GetCoreScriptingClasses().invokeOnPhotoModeStoppedDelegate);
        scriptingInvocation.AddObject(callback.Resolve());
        scriptingInvocation.AddInt64(S_OK);
        scriptingInvocation.Invoke();

        Assert(callback.HasTarget());
        callback.ReleaseAndClear();
    }

    //---------------------------------------------------------------
    void PhotoCapture::CapturePhotoToMemory(const ScriptingObjectPtr callback)
    {
        if (m_HasErrors)
        {
            NotifyScriptOfError(OperationType::TakePhotoAsyncToMemory, E_FAIL, "Failed to capture frame: PhotoCapture is in an erroneous state.", callback);
            return;
        }

        if (WebCam::GetInstance().GetWebCamMode() == WebCam::VideoMode)
        {
            NotifyScriptOfError(OperationType::TakePhotoAsyncToMemory, E_FAIL, "Can not capture a photo while in video mode.", callback);
            return;
        }

        if (WebCam::GetInstance().GetWebCamMode() != WebCam::PhotoMode)
        {
            NotifyScriptOfError(OperationType::TakePhotoAsyncToMemory, E_FAIL, "StartPhotoModeAsync must be called prior to taking a photo.", callback);
            return;
        }

        if (s_ActivateCaptureInstance != this)
        {
            NotifyScriptOfError(OperationType::TakePhotoAsyncToMemory, E_FAIL, "Only the capture instance that started the photo mode can take photos.", callback);
            return;
        }

        PhotoCaptureOperationParameters *captureParams = UNITY_NEW(PhotoCaptureOperationParameters, kMemWebCam);
        captureParams->callbackGCHandle.AcquireStrong(callback);
        captureParams->filepath = NULL;
        captureParams->saveToDisk = false;

        AtomicNode* atomicNode = UNITY_NEW(AtomicNode, kMemWebCam);
        atomicNode->data[0] = reinterpret_cast<void*>(captureParams);

        m_CaptureRequests.Enqueue(atomicNode);
        m_Tasks.AddTask(&PhotoCapture::DoCapturePhotoToMemoryAsync);
    }

    //---------------------------------------------------------------
    void PhotoCapture::DoCapturePhotoToMemoryAsync()
    {
        AtomicNode* atomicNode = m_CaptureRequests.Dequeue();
        AssertMsg(atomicNode != NULL, "Failed to dequeue a capture request.");

        PhotoCaptureOperationParameters *captureParams = reinterpret_cast<PhotoCaptureOperationParameters*>(atomicNode->data[0]);
        UNITY_DELETE(atomicNode, kMemWebCam);

        PhotoCaptureOperation::Execute(this, m_LowLagPhotoCapture, captureParams);
    }

    //---------------------------------------------------------------
    void PhotoCapture::OnPhotoCapturedToMemory(IMFSample* mfSample, PhotoCaptureOperationParameters *photoCaptureOperationParams)
    {
        if (m_IsDisposed)
        {
            UNITY_DELETE(photoCaptureOperationParams, kMemWebCam);
            return;
        }

#if UNITY_EDITOR
        if (m_IsQueuedForDestruction || m_ExitedPlayMode)
        {
            UNITY_DELETE(photoCaptureOperationParams, kMemWebCam);
            return;
        }
#endif

        mfSample->AddRef();

        // If we need to save this captured photo to disk, go ahead and save it here
        // so we don't pay this cost on the main thread.
        if (photoCaptureOperationParams->saveToDisk)
        {
            WriteIMFSampleToDisk(*photoCaptureOperationParams->filepath, mfSample, EnumTraits::FromInt<PhotoCaptureFileOutputFormat>(photoCaptureOperationParams->fileOutputFormat), photoCaptureOperationParams->callbackGCHandle);
        }

        OnCapturedPhotoToMemoryEventData eventData = { this, photoCaptureOperationParams, mfSample };
        GlobalEventQueue::GetInstance().SendEvent(eventData);
    }

    //---------------------------------------------------------------
    void PhotoCapture::HandleEvent(const OnCapturedPhotoToMemoryEventData& eventData)
    {
        if (eventData.instance != this)
        {
            return;
        }

#if UNITY_EDITOR
        if (m_ExitedPlayMode || m_IsQueuedForDestruction)
            return;
#endif

        if (m_IsDisposed)
        {
            return;
        }

        if (!eventData.photoCaptureOperationParams->saveToDisk)
        {
            OnCapturedPhotoToMemoryStage2(eventData.photoCaptureOperationParams->callbackGCHandle, eventData.mfSample);
        }
        else
        {
            OnCapturedPhotoToDisk(eventData.photoCaptureOperationParams->callbackGCHandle);
        }

        if (m_IsDisposed)
        {
            ErrorString("Error: You have disposed PhotoCapture in a frame ready callback!");
            return;
        }

        ProcessNextTaskOnMainThread();
    }

    static void PrintMatrix(const char* mtxName, const Matrix4x4f& matrix)
    {
        printf_console(
            "%s:\n %- 02.4f %- 02.4f %- 02.4f %- 02.4f\n %- 02.4f %- 02.4f %- 02.4f %- 02.4f\n %- 02.4f %- 02.4f %- 02.4f %- 02.4f\n %- 02.4f %- 02.4f %- 02.4f %- 02.4f\n",
            mtxName,
            matrix.Get(0, 0), matrix.Get(0, 1), matrix.Get(0, 2), matrix.Get(0, 3),
            matrix.Get(1, 0), matrix.Get(1, 1), matrix.Get(1, 2), matrix.Get(1, 3),
            matrix.Get(2, 0), matrix.Get(2, 1), matrix.Get(2, 2), matrix.Get(2, 3),
            matrix.Get(3, 0), matrix.Get(3, 1), matrix.Get(3, 2), matrix.Get(3, 3));
    }

    //---------------------------------------------------------------
    void PhotoCapture::OnCapturedPhotoToMemoryStage2(ScriptingGCHandle callbackGCHandle, IMFSample *mfSample)
    {
        Matrix4x4f projectionMatrix, viewToWorldInCameraCoordsMatrix;

        bool hasCameraMatrices = false;
        if (GetIXRWindowsLocatableCamera())
            hasCameraMatrices = GetIXRWindowsLocatableCamera()->TryGetCameraMatricesFromNativeData((void*)mfSample, projectionMatrix, viewToWorldInCameraCoordsMatrix);

        // Otherwise, fallback to identity matrices
        if (!hasCameraMatrices)
        {
            viewToWorldInCameraCoordsMatrix.SetIdentity();
            projectionMatrix.SetIdentity();
        }

        DWORD bufferCount;
        mfSample->GetBufferCount(&bufferCount);
        AssertFormatMsg(bufferCount > 0, "Somewhat unexpected: bufferCount = 0x%d", bufferCount);

        win::ComPtr<IMFMediaBuffer> photoBuffer;
        mfSample->GetBufferByIndex(0, &photoBuffer);
        PhotoCaptureFrame* captureFrame = PhotoCaptureFrame::Create(viewToWorldInCameraCoordsMatrix, projectionMatrix, mfSample, photoBuffer, m_CameraParameters.m_CameraResolutionWidth, m_CameraParameters.m_CameraResolutionHeight, EnumTraits::FromInt<CapturePixelFormat>(m_CameraParameters.m_PixelFormat), hasCameraMatrices);

        ScriptingInvocation scriptingInvocation(GetCoreScriptingClasses().invokeOnCapturedPhotoToMemoryDelegate);
        scriptingInvocation.AddObject(callbackGCHandle.Resolve());
        scriptingInvocation.AddInt64(S_OK);
        scriptingInvocation.AddIntPtr(captureFrame);
        scriptingInvocation.Invoke();
    }

    //---------------------------------------------------------------
    void PhotoCapture::CapturePhotoToDisk(const ICallString &filename, PhotoCaptureFileOutputFormat fileOutputFormat, const ScriptingObjectPtr callback)
    {
        if (m_HasErrors)
        {
            NotifyScriptOfError(OperationType::TakePhotoAsyncToDisk, E_FAIL, "Failed to capture frame: PhotoCapture is in an erroneous state.", callback);
            return;
        }

        if (WebCam::GetInstance().GetWebCamMode() == WebCam::VideoMode)
        {
            NotifyScriptOfError(OperationType::TakePhotoAsyncToDisk, E_FAIL, "Can not capture a photo while in video mode.", callback);
            return;
        }

        if (WebCam::GetInstance().GetWebCamMode() != WebCam::PhotoMode)
        {
            NotifyScriptOfError(OperationType::TakePhotoAsyncToDisk, E_FAIL, "StartPhotoModeAsync must be called prior to taking a photo.", callback);
            return;
        }

        if (s_ActivateCaptureInstance != this)
        {
            NotifyScriptOfError(OperationType::TakePhotoAsyncToDisk, E_FAIL, "Only the capture instance that started the photo mode can take photos.", callback);
            return;
        }

        if (fileOutputFormat == PhotoCaptureFileOutputFormat::JPG)
        {
            if (m_CameraParameters.m_PixelFormat != CapturePixelFormat::BGRA32 && m_CameraParameters.m_PixelFormat != CapturePixelFormat::NV12)
            {
                NotifyScriptOfError(OperationType::TakePhotoAsyncToDisk, E_FAIL, "The capture pixel format must be BGRA32 or NV12 to save the captured image in the JPG format.", callback);
                return;
            }
        }
        else if (fileOutputFormat == PhotoCaptureFileOutputFormat::PNG)
        {
            if (m_CameraParameters.m_PixelFormat != CapturePixelFormat::BGRA32)
            {
                NotifyScriptOfError(OperationType::TakePhotoAsyncToDisk, E_FAIL, "The capture pixel format must be BGRA32 to save the captured image in the PNG format.", callback);
                return;
            }
        }

        PhotoCaptureOperationParameters *captureParams = UNITY_NEW(PhotoCaptureOperationParameters, kMemWebCam);
        captureParams->callbackGCHandle.AcquireStrong(callback);
        captureParams->filepath = UNITY_NEW(core::wstring, kMemWebCam)((wchar_t*)filename.GetRawCharBuffer());
        captureParams->saveToDisk = true;
        captureParams->fileOutputFormat = fileOutputFormat;

        AtomicNode* atomicNode = UNITY_NEW(AtomicNode, kMemWebCam);
        atomicNode->data[0] = reinterpret_cast<void*>(captureParams);

        m_CaptureRequests.Enqueue(atomicNode);
        m_Tasks.AddTask(&PhotoCapture::DoCapturePhotoToMemoryAsync);
    }

    //---------------------------------------------------------------
    void PhotoCapture::OnCapturedPhotoToDisk(ScriptingGCHandle callbackGCHandle)
    {
        ScriptingInvocation scriptingInvocation(GetCoreScriptingClasses().invokeOnCapturedPhotoToDiskDelegate);
        scriptingInvocation.AddObject(callbackGCHandle.Resolve());
        scriptingInvocation.AddInt64(S_OK);
        scriptingInvocation.Invoke();
    }

    //---------------------------------------------------------------
    void PhotoCapture::WriteIMFSampleToDisk(const core::wstring &filename, IMFSample* mfSample, PhotoCaptureFileOutputFormat fileOutputFormat, ScriptingGCHandle callbackGCHandle)
    {
        HRESULT hr = S_OK;

        if (fileOutputFormat == PhotoCaptureFileOutputFormat::JPG)
        {
            if (m_CameraParameters.m_PixelFormat == CapturePixelFormat::NV12)
            {
                hr = Unity::IMFSampleImageFileWriter::ExportNV12AsJpeg(filename, m_CameraParameters.m_CameraResolutionWidth, m_CameraParameters.m_CameraResolutionHeight, mfSample);
            }
            else if (m_CameraParameters.m_PixelFormat == CapturePixelFormat::BGRA32)
            {
                hr = Unity::IMFSampleImageFileWriter::ExportBGRA32AsJpeg(filename, m_CameraParameters.m_CameraResolutionWidth, m_CameraParameters.m_CameraResolutionHeight, mfSample);
            }
        }
        else
        {
            if (m_CameraParameters.m_PixelFormat == CapturePixelFormat::BGRA32)
            {
                hr = Unity::IMFSampleImageFileWriter::ExportBGRA32AsPng(filename, m_CameraParameters.m_CameraResolutionWidth, m_CameraParameters.m_CameraResolutionHeight, mfSample);
            }
        }
        VerifyHRAndNotifyScript(OperationType::TakePhotoAsyncToDisk, hr, callbackGCHandle);
    }

    //---------------------------------------------------------------
    HRESULT STDMETHODCALLTYPE PhotoCapture::Invoke(UnityWinRTBase::IMediaCapture* sender,
        UnityWinRTBase::IMediaCaptureFailedEventArgs* errorEventArgs)
    {
        ProcessNextTaskOnMainThread();

        return S_OK;
    }

    //---------------------------------------------------------------
    HRESULT STDMETHODCALLTYPE PhotoCapture::Invoke(UnityWinRTBase::IAsyncAction* asyncAction,
        UnityWinRTBase::AsyncStatus asyncStatus)
    {
        Unity::ClassAsyncTask<PhotoCapture>::AsyncTaskOperation currentOperation = m_Tasks.GetCurrentOperation();

        if (asyncStatus != UnityWinRTBase::Completed)
        {
            const char* failedAction;

            bool isCriticalError = true;

            OperationType operationType = OperationType::Unknown;
            ScriptingGCHandle callbackGCHandle;
            if (currentOperation == &PhotoCapture::InitializeAsync)
            {
                failedAction = "Failed to initialize IMediaCapture (hr = 0x%X)";
                operationType = OperationType::Create;
                callbackGCHandle = m_OnCreateCallbackGCHandle;
            }
            else if (currentOperation == &PhotoCapture::SetResolution)
            {
                failedAction = "Failed to set capture resolution (hr = 0x%X)";
                operationType = OperationType::StartPhotoMode;
                callbackGCHandle = m_OnPhotoModeStartedGCHandle;
            }
            else
            {
                AssertMsg(false, "PhotoCapture::Invoke got called from an unknown action");
                failedAction = "Failed to complete unknown action (hr = 0x%X)";
            }

            if (isCriticalError == true)
            {
                win::ComPtr<UnityWinRTBase::IAsyncInfo> asyncInfo;
                HRESULT hr = asyncAction->QueryInterface(__uuidof(UnityWinRTBase::IAsyncInfo), &asyncInfo);
                if (callbackGCHandle.HasTarget())
                {
                    VerifyHRAndNotifyScriptWithReturnValue(operationType, hr, m_OnCreateCallbackGCHandle, hr);
                }
                else
                {
                    VerifyCriticalHRWithReturnValue(hr, S_OK, "Failed to cast IAsyncAction to IAsyncInfo (hr = 0x%X)");
                }

                HRESULT errorCode;
                hr = asyncInfo->get_ErrorCode(&errorCode);
                VerifyCriticalHRWithReturnValue(hr, S_OK, "Failed to get ErrorCode from IAsyncInfo (hr = 0x%X)");
                VerifyCriticalHRWithReturnValue(errorCode, S_OK, failedAction);
            }
        }
        else
        {
            // The SetResolution operation will come through here.
            if (currentOperation == &PhotoCapture::InitializeAsync)
            {
                OnCreatedPhotoCaptureResource();
            }

            if (currentOperation == &PhotoCapture::DeactivateLowLagCamera)
            {
                OnStoppedPhotoMode();
            }
        }

        ProcessNextTaskOnMainThread();

        return S_OK;
    }

    //---------------------------------------------------------------
    HRESULT PhotoCapture::Invoke(UnityWinRTBase::IAsyncOperation<UnityWinRTBase::IMediaExtension*>* asyncOperation,
        UnityWinRTBase::AsyncStatus asyncStatus)
    {
        ProcessNextTaskOnMainThread();

        return S_OK;
    }

    //---------------------------------------------------------------
    HRESULT PhotoCapture::Invoke(UnityWinRTBase::IAsyncOperation<UnityWinRTBase::LowLagPhotoCapture*>* asyncOperation,
        UnityWinRTBase::AsyncStatus asyncStatus)
    {
        HRESULT hr;

        if (asyncStatus != UnityWinRTBase::Completed)
        {
            win::ComPtr<UnityWinRTBase::IAsyncInfo> asyncInfo;
            hr = asyncOperation->QueryInterface(__uuidof(UnityWinRTBase::IAsyncInfo), &asyncInfo);
            VerifyHRAndNotifyScriptWithReturnValue(OperationType::StartPhotoMode, hr, m_OnCreateCallbackGCHandle, hr);

            HRESULT errorCode;
            hr = asyncInfo->get_ErrorCode(&errorCode);
            VerifyCriticalHRWithReturnValue(hr, S_OK, "Failed to get ErrorCode from IAsyncInfo (hr = 0x%X)");
            VerifyCriticalHRWithReturnValue(errorCode, S_OK, "Failed to prepare for photo capture (hr = 0x%X)");
        }

        hr = asyncOperation->GetResults(&m_LowLagPhotoCapture);
        VerifyHRAndNotifyScriptWithReturnValue(OperationType::StartPhotoMode, hr, m_OnPhotoModeStartedGCHandle, hr);

        if (m_Tasks.GetCurrentOperation() == &PhotoCapture::ActivateLowLagCamera)
        {
            OnStartedPhotoMode();
        }

        ProcessNextTaskOnMainThread();

        return S_OK;
    }

    //---------------------------------------------------------------
    void PhotoCapture::SetupEncodingProperties(const CameraParameters & cameraParameters, win::ComPtr<UnityWinRTBase::Windows::Media::MediaProperties::IImageEncodingProperties> & encodingProperties)
    {
        HRESULT hr;

        if (cameraParameters.m_PixelFormat == CapturePixelFormat::BGRA32 || cameraParameters.m_PixelFormat == CapturePixelFormat::NV12)
        {
            win::ComPtr<UnityWinRTBase::IImageEncodingPropertiesStatics2> propertiesFactory;
            hr = UnityWinRTBase::RoGetActivationFactory(UnityWinRTBase::HStringReference(L"Windows.Media.MediaProperties.ImageEncodingProperties"),
                __uuidof(UnityWinRTBase::IImageEncodingPropertiesStatics2), &propertiesFactory);
            VerifyHRAndNotifyScript(OperationType::StartPhotoMode, hr, m_OnPhotoModeStartedGCHandle);

            UnityWinRTBase::MediaPixelFormat pixelFormat = cameraParameters.m_PixelFormat == CapturePixelFormat::BGRA32 ? UnityWinRTBase::MediaPixelFormat_Bgra8 : UnityWinRTBase::MediaPixelFormat_Nv12;

            hr = propertiesFactory->CreateUncompressed(pixelFormat, &encodingProperties);
            VerifyHRAndNotifyScript(OperationType::StartPhotoMode, hr, m_OnPhotoModeStartedGCHandle);
        }
        else
        {
            win::ComPtr<UnityWinRTBase::IImageEncodingPropertiesStatics> propertiesFactory;
            hr = UnityWinRTBase::RoGetActivationFactory(UnityWinRTBase::HStringReference(L"Windows.Media.MediaProperties.ImageEncodingProperties"),
                __uuidof(UnityWinRTBase::IImageEncodingPropertiesStatics), &propertiesFactory);
            VerifyHRAndNotifyScript(OperationType::StartPhotoMode, hr, m_OnPhotoModeStartedGCHandle);

            if (cameraParameters.m_PixelFormat == CapturePixelFormat::JPEG)
            {
                hr = propertiesFactory->CreateJpeg(&encodingProperties);
                VerifyHRAndNotifyScript(OperationType::StartPhotoMode, hr, m_OnPhotoModeStartedGCHandle);
            }
            else
            {
                hr = propertiesFactory->CreatePng(&encodingProperties);
                VerifyHRAndNotifyScript(OperationType::StartPhotoMode, hr, m_OnPhotoModeStartedGCHandle);
            }
        }

        hr = encodingProperties->put_Width(cameraParameters.m_CameraResolutionWidth);
        VerifyHRAndNotifyScript(OperationType::StartPhotoMode, hr, m_OnPhotoModeStartedGCHandle);

        hr = encodingProperties->put_Height(cameraParameters.m_CameraResolutionHeight);
        VerifyHRAndNotifyScript(OperationType::StartPhotoMode, hr, m_OnPhotoModeStartedGCHandle);
    }

    //---------------------------------------------------------------
    void PhotoCapture::SetResolution()
    {
        win::ComPtr<UnityWinRTBase::IVideoDeviceController> videoController;
        HRESULT hr = GetMediaCapture()->get_VideoDeviceController(&videoController);
        VerifyHRAndNotifyScript(OperationType::StartPhotoMode, hr, m_OnPhotoModeStartedGCHandle);

        win::ComPtr<UnityWinRTBase::IMediaDeviceController> mediaController;
        hr = videoController.As(&mediaController);
        VerifyHRAndNotifyScript(OperationType::StartPhotoMode, hr, m_OnPhotoModeStartedGCHandle);

        win::ComPtr<UnityWinRTBase::IVectorView<UnityWinRTBase::IMediaEncodingProperties*> > encodingProperties;
        hr = mediaController->GetAvailableMediaStreamProperties(UnityWinRTBase::MediaStreamType_Photo, &encodingProperties);
        VerifyHRAndNotifyScript(OperationType::StartPhotoMode, hr, m_OnPhotoModeStartedGCHandle);

        uint32_t count;
        hr = encodingProperties->get_Size(&count);
        VerifyHRAndNotifyScript(OperationType::StartPhotoMode, hr, m_OnPhotoModeStartedGCHandle);

        win::ComPtr<UnityWinRTBase::IMediaEncodingProperties> properties;

        for (uint32_t i = 0; i < count; i++)
        {
            win::ComPtr<UnityWinRTBase::IVideoEncodingProperties> videoEncodingProperties;

            hr = encodingProperties->GetAt(i, &properties);
            VerifyHRAndNotifyScript(OperationType::StartPhotoMode, hr, m_OnPhotoModeStartedGCHandle);

            hr = properties.As(&videoEncodingProperties);
            if (FAILED(hr))
                continue;

            uint32_t width, height;

            hr = videoEncodingProperties->get_Width(&width);
            VerifyHRAndNotifyScript(OperationType::StartPhotoMode, hr, m_OnPhotoModeStartedGCHandle);

            hr = videoEncodingProperties->get_Height(&height);
            VerifyHRAndNotifyScript(OperationType::StartPhotoMode, hr, m_OnPhotoModeStartedGCHandle);

            if (m_CameraParameters.m_CameraResolutionWidth == static_cast<int>(width) && m_CameraParameters.m_CameraResolutionHeight == static_cast<int>(height))
                break;
        }

        win::ComPtr<UnityWinRTBase::IAsyncAction> asyncAction;
        hr = mediaController->SetMediaStreamPropertiesAsync(UnityWinRTBase::MediaStreamType_Photo, properties, &asyncAction);
        VerifyHRAndNotifyScript(OperationType::StartPhotoMode, hr, m_OnPhotoModeStartedGCHandle);

        hr = asyncAction->put_Completed(this);
        VerifyHRAndNotifyScript(OperationType::StartPhotoMode, hr, m_OnPhotoModeStartedGCHandle);
    }

    //---------------------------------------------------------------
    void PhotoCapture::AddPhotoEffectAsync()
    {
#define ProcessNextTaskIfFailed(hr) do { if (FAILED(hr)) { ProcessNextTaskOnMainThread(); return; } } while (false)

        win::ComPtr<UnityWinRTBase::IMediaCapture4> mediaCapture4;
        HRESULT hr = GetMediaCapture().As(&mediaCapture4);
        ProcessNextTaskIfFailed(hr); // We're running below Windows 10. Don't add video effect and continue.

        win::ComPtr<MixedRealityCaptureVideoEffect> effect;
        effect.Attach(UNITY_NEW(MixedRealityCaptureVideoEffect, kMemWebCam)(UnityWinRTBase::MediaStreamType_Photo, m_CameraParameters.m_HologramOpacity));

        win::ComPtr<UnityWinRTBase::IAsyncOperation<UnityWinRTBase::IMediaExtension*> > asyncOperation;
        hr = mediaCapture4->AddVideoEffectAsync(effect, UnityWinRTBase::MediaStreamType_Photo, &asyncOperation);
        ProcessNextTaskIfFailed(hr);

        hr = asyncOperation->put_Completed(this);
        ProcessNextTaskIfFailed(hr);

#undef ProcessNextTaskIfFailed
    }

    //---------------------------------------------------------------
    void PhotoCapture::RemovePhotoEffectAsync()
    {
#define ProcessNextTaskIfFailed(hr) do { if (FAILED(hr)) { ProcessNextTaskOnMainThread(); return; } } while (false)

        win::ComPtr<UnityWinRTBase::IMediaCapture4> mediaCapture4;
        HRESULT hr = GetMediaCapture().As(&mediaCapture4);
        ProcessNextTaskIfFailed(hr); // We're running below Windows 10. Don't add video effect and continue.

        win::ComPtr<UnityWinRTBase::IAsyncAction> asyncOperation;
        hr = GetMediaCapture()->ClearEffectsAsync(UnityWinRTBase::MediaStreamType_Photo, &asyncOperation);
        ProcessNextTaskIfFailed(hr);

        hr = asyncOperation->put_Completed(this);
        ProcessNextTaskIfFailed(hr);

#undef ProcessNextTaskIfFailed
    }

    //---------------------------------------------------------------
    void PhotoCapture::ActivateLowLagCamera()
    {
        win::ComPtr<UnityWinRTBase::IMediaCapture2> mediaCapture2;
        HRESULT hr = GetMediaCapture().As(&mediaCapture2);
        VerifyHRAndNotifyScript(OperationType::StartPhotoMode, hr, m_OnPhotoModeStartedGCHandle);

        win::ComPtr<UnityWinRTBase::IAsyncOperation<UnityWinRTBase::LowLagPhotoCapture*> > asyncOperation;
        hr = mediaCapture2->PrepareLowLagPhotoCaptureAsync(m_EncodingProperties, &asyncOperation);
        VerifyHRAndNotifyScript(OperationType::StartPhotoMode, hr, m_OnPhotoModeStartedGCHandle);

        hr = asyncOperation->put_Completed(this);
        VerifyHRAndNotifyScript(OperationType::StartPhotoMode, hr, m_OnPhotoModeStartedGCHandle);
    }

    //---------------------------------------------------------------
    void PhotoCapture::DeactivateLowLagCamera()
    {
        DeactivateLowLagCamera(true);
    }

    //---------------------------------------------------------------
    void PhotoCapture::DeactivateLowLagCamera(bool notifyMainThreadWhenComplete)
    {
        //chances are this means we m_HasErrors = true.
        if (m_LowLagPhotoCapture == nullptr)
        {
            return;
        }

        // Free LowLatencyPhotoCapture resources
        win::ComPtr<UnityWinRTBase::IAsyncAction> asyncAction;
        HRESULT hr = m_LowLagPhotoCapture->FinishAsync(&asyncAction);
        VerifyHRAndNotifyScript(OperationType::StopPhotoMode, hr, m_OnPhotoModeStoppedGCHandle);
        if (notifyMainThreadWhenComplete)
        {
            hr = asyncAction->put_Completed(this);
            VerifyHRAndNotifyScript(OperationType::StopPhotoMode, hr, m_OnPhotoModeStoppedGCHandle);
        }
        m_LowLagPhotoCapture = nullptr;


        if (s_ActivateCaptureInstance == this && (s_PhotoModeState == PhotoModeState::Activating || s_PhotoModeState == PhotoModeState::Activated))
        {
            ErrorString("StopPhotoMode should be called before disposing the active photo capture instance.");
            s_PhotoModeState = PhotoModeState::Deactivated;
            s_ActivateCaptureInstance = NULL;
            return;
        }
    }

    //---------------------------------------------------------------
    bool PhotoCapture::IsWebCameraAvailable()
    {
        win::ComPtr<UnityWinRTBase::IVideoDeviceController> videoDeviceController;
        HRESULT hr = GetMediaCapture()->get_VideoDeviceController(&videoDeviceController);
        if (FAILED(hr))
        {
            return false;
        }

        win::ComPtr<UnityWinRTBase::IMediaDeviceController> mediaDeviceController;
        hr = (videoDeviceController).As(&mediaDeviceController);
        if (FAILED(hr))
        {
            return false;
        }

        win::ComPtr<UnityWinRTBase::IVectorView<UnityWinRTBase::IMediaEncodingProperties*> > encodingProperties;
        hr = mediaDeviceController->GetAvailableMediaStreamProperties(UnityWinRTBase::Windows::Media::Capture::MediaStreamType_Photo, &encodingProperties);
        if (FAILED(hr))
        {
            return false;
        }

        return true;
    }

#undef VerifyHR
#undef VerifyAsyncHR
#undef VerifyHRAndNotifyScript
#undef VerifyHRAndNotifyScriptWithReturnValue
} // end namespace Unity
