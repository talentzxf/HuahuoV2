#pragma once

#if PLATFORM_WIN && !PLATFORM_XBOXONE
#include "External/Windows10/src/ComHelpers.h"
#include "External/Windows10/src/Windows10Interfaces.h"
#include "External/Windows10/src/WinRTCollections.h"
#include "PlatformDependent/Win/ComPtr.h"
#endif

#include "Runtime/EventQueue/GlobalEventQueue.h"
#include "Runtime/Scripting/BindingsDefs.h"
#include "Runtime/Scripting/Scripting.h"
#include "Runtime/Scripting/ScriptingUtility.h"
#include "Runtime/Graphics/Resolution.h"
#include "Runtime/Threads/CurrentThread.h"
#if !PLATFORM_WEBGL
#include "Runtime/Threads/TaskChainProcessor.h"
#endif
#include "CameraParameters.h"

struct IMFSample;
struct OnCreatedPhotoCaptureResourceEventData;
struct OnStartedPhotoModeEventData;
struct OnCapturedPhotoToMemoryEventData;
struct OnStoppedPhotoModeEventData;
struct OnPhotoCaptureProcessNextTaskEventData;
struct OnPhotoCaptureDestructionEventData;
struct OnNotifyScriptAboutErrorEventData;

namespace Unity
{
    struct PhotoCaptureOperationParameters;

    REFLECTABLE_ENUM(PhotoCaptureFileOutputFormat,
        PNG,
        JPG
    );

#if PLATFORM_WIN && !PLATFORM_XBOXONE
    class PhotoCapture :
        private UnityWinRTBase::ComClass<
            UnityWinRTBase::Windows::Media::Capture::IMediaCaptureFailedEventHandler,
            UnityWinRTBase::Windows::Foundation::IAsyncActionCompletedHandler,
            UnityWinRTBase::Windows::Foundation::IAsyncOperationCompletedHandler<UnityWinRTBase::Windows::Media::IMediaExtension*>,
            UnityWinRTBase::Windows::Foundation::IAsyncOperationCompletedHandler<UnityWinRTBase::Windows::Media::Capture::LowLagPhotoCapture*> >
    {
    public:

        static PhotoCapture* Instantiate(bool showHolograms, ScriptingObjectPtr callback);
        static dynamic_array<Resolution> GetSupportedResolutions();

        void Dispose();
        void DisposeThreaded();

        void StartPhotoMode(const CameraParameters &cameraParameters, const ScriptingObjectPtr onPhotoModeStartedCallback);
        void StopPhotoMode(const ScriptingObjectPtr onPhotoModeStoppedCallback);
        void CapturePhotoToDisk(const ICallString &filename, PhotoCaptureFileOutputFormat fileOutputFormat, const ScriptingObjectPtr callback);
        void CapturePhotoToMemory(const ScriptingObjectPtr callback);

        void* GetUnsafePointerToVideoDeviceController();

        REFLECTABLE_ENUM(OperationType,
            Create,
            StartPhotoMode,
            StopPhotoMode,
            TakePhotoAsyncToMemory,
            TakePhotoAsyncToDisk,
            Unknown
        );

    private:

        REFLECTABLE_ENUM(PhotoModeState,
            Activating,
            Activated,
            Deactivating,
            Deactivated
        );

#if UNITY_EDITOR
        static dynamic_array<PhotoCapture*> s_PhotoCaptureInstances; // We need to do this in editor in order to destroy all webcams when we exit playmode
#endif
        static void* s_ActivateCaptureInstance;
        static PhotoModeState s_PhotoModeState;


        CameraParameters m_CameraParameters;
        AtomicQueue m_CaptureRequests;
        TaskChainProcessor<PhotoCapture> m_Tasks;
        ScriptingGCHandle m_OnCreateCallbackGCHandle;
        ScriptingGCHandle m_OnPhotoModeStartedGCHandle;
        ScriptingGCHandle m_OnPhotoModeStoppedGCHandle;
        win::ComPtr<UnityWinRTBase::Windows::Media::MediaProperties::IImageEncodingProperties> m_PhotoProperties;
        win::ComPtr<UnityWinRTBase::Windows::Media::Capture::IMediaCapture> m_MediaCapture;
        win::ComPtr<UnityWinRTBase::Windows::Media::MediaProperties::IImageEncodingProperties> m_EncodingProperties;
        win::ComPtr<UnityWinRTBase::Windows::Media::Capture::ILowLagPhotoCapture> m_LowLagPhotoCapture;
        bool m_ShowHolograms;
        bool m_HasErrors;
        bool m_IsDisposed;

#if UNITY_EDITOR
        bool m_ExitedPlayMode;
        bool m_IsQueuedForDestruction;
#endif

        PhotoCapture(bool showHolograms, const ScriptingObjectPtr onCreateCallback, UnityWinRTBase::Windows::Media::Capture::IMediaCapture* mediaCapture);

#if UNITY_EDITOR
        static void MarkExitedPlayMode();
        static void DestroyAllPhotoCaptureWebCams();
#endif

        void InitializeAsync();
        void SetResolution();
        void AddPhotoEffectAsync();
        void RemovePhotoEffectAsync();
        void ActivateLowLagCamera();
        void DeactivateLowLagCamera();
        void DeactivateLowLagCamera(bool notifyMainThreadWhenComplete);

        void DoCapturePhotoToMemoryAsync();
        void DoCapturePhotoToDiskAsync();

        void NotifyScriptOfErrorOnMainThread(OperationType operationType, HRESULT hr, ScriptingGCHandle callbackGCHandle);
        void NotifyScriptOfError(OperationType operationType, HRESULT hr, ScriptingGCHandle callbackGCHandle);
        void NotifyScriptOfError(OperationType operationType, HRESULT hr, const char* message, ScriptingObjectPtr callback);

        void ProcessNextTaskOnMainThread();

        void OnPhotoCapturedToMemory(IMFSample* mfSample, PhotoCaptureOperationParameters *photoCaptureOperationParams);

        UnityEventQueue::ClassBasedEventHandler<OnCreatedPhotoCaptureResourceEventData, PhotoCapture> m_OnCreatedPhotoCaptureResourceEventDelegate;
        UnityEventQueue::ClassBasedEventHandler<OnStartedPhotoModeEventData, PhotoCapture> m_OnStartedPhotoModeEventDelegate;
        UnityEventQueue::ClassBasedEventHandler<OnStoppedPhotoModeEventData, PhotoCapture> m_OnStoppedPhotoModeEventDelegate;
        UnityEventQueue::ClassBasedEventHandler<OnCapturedPhotoToMemoryEventData, PhotoCapture> m_OnCapturedPhotoToMemoryEventDelegate;
        UnityEventQueue::ClassBasedEventHandler<OnPhotoCaptureProcessNextTaskEventData, PhotoCapture> m_OnProcessNextTaskEventDelegate;
        UnityEventQueue::ClassBasedEventHandler<OnNotifyScriptAboutErrorEventData, PhotoCapture> m_OnNotifyScriptAboutErrorEventDelegate;

        void OnCreatedPhotoCaptureResource();
        void HandleEvent(const OnCreatedPhotoCaptureResourceEventData& eventData);

        void OnStartedPhotoMode();
        void HandleEvent(const OnStartedPhotoModeEventData& eventData);

        void OnStoppedPhotoMode();
        void HandleEvent(const OnStoppedPhotoModeEventData& eventData);

        void OnCapturedPhotoToMemoryStage1(UnityWinRTBase::Windows::Foundation::IAsyncOperation<UnityWinRTBase::Windows::Media::Capture::CapturedPhoto*>* asyncOperation);
        void HandleEvent(const OnCapturedPhotoToMemoryEventData& eventData);

        void OnCapturedPhotoToMemoryStage2(ScriptingGCHandle callbackGCHandle, IMFSample *mfSample);

        void OnCapturedPhotoToDisk(ScriptingGCHandle callbackGCHandle);

        void HandleEvent(const OnPhotoCaptureProcessNextTaskEventData& eventData);
        void HandleEvent(const OnNotifyScriptAboutErrorEventData& eventData);

        static void HandleDestructionEvent(const OnPhotoCaptureDestructionEventData& buffer);

        bool IsWebCameraAvailable();

        inline win::ComPtr<UnityWinRTBase::Windows::Media::Capture::IMediaCapture>& GetMediaCapture() { Assert(CurrentThread::IsMainThread()); return m_MediaCapture; }

        // Interface implementations
        virtual HRESULT STDMETHODCALLTYPE Invoke(UnityWinRTBase::Windows::Media::Capture::IMediaCapture* sender, UnityWinRTBase::Windows::Media::Capture::IMediaCaptureFailedEventArgs* errorEventArgs) override;
        virtual HRESULT STDMETHODCALLTYPE Invoke(UnityWinRTBase::Windows::Foundation::IAsyncAction* asyncAction, UnityWinRTBase::Windows::Foundation::AsyncStatus asyncStatus) override;
        virtual HRESULT STDMETHODCALLTYPE Invoke(UnityWinRTBase::Windows::Foundation::IAsyncOperation<UnityWinRTBase::Windows::Media::IMediaExtension*>* asyncOperation, UnityWinRTBase::Windows::Foundation::AsyncStatus asyncStatus) override;
        virtual HRESULT STDMETHODCALLTYPE Invoke(UnityWinRTBase::Windows::Foundation::IAsyncOperation<UnityWinRTBase::Windows::Media::Capture::LowLagPhotoCapture*>* asyncOperation, UnityWinRTBase::Windows::Foundation::AsyncStatus asyncStatus) override;

        void SetupEncodingProperties(const CameraParameters & cameraParameters, win::ComPtr<UnityWinRTBase::Windows::Media::MediaProperties::IImageEncodingProperties> & encodingProperties);
        void WriteIMFSampleToDisk(const core::wstring &filename, IMFSample* mfSample, PhotoCaptureFileOutputFormat fileOutputFormat, ScriptingGCHandle callbackGCHandle);

        friend class win::ComPtr<PhotoCapture>;
        friend class UnityEventQueue::ClassBasedEventHandler<OnCreatedPhotoCaptureResourceEventData, PhotoCapture>;
        friend class UnityEventQueue::ClassBasedEventHandler<OnStartedPhotoModeEventData, PhotoCapture>;
        friend class UnityEventQueue::ClassBasedEventHandler<OnStoppedPhotoModeEventData, PhotoCapture>;
        friend class UnityEventQueue::ClassBasedEventHandler<OnCapturedPhotoToMemoryEventData, PhotoCapture>;
        friend class UnityEventQueue::ClassBasedEventHandler<OnPhotoCaptureProcessNextTaskEventData, PhotoCapture>;
        friend class UnityEventQueue::ClassBasedEventHandler<OnNotifyScriptAboutErrorEventData, PhotoCapture>;
        friend class PhotoCaptureOperation;
    };
#else
    // we need to stub in a class for the bindings to work, since the above declaration depends on WinRT interfaces
    class PhotoCapture {};
#endif
}

BIND_MANAGED_TYPE_NAME(::Unity::PhotoCaptureFileOutputFormat::ActualEnumType, UnityEngine_Windows_WebCam_PhotoCaptureFileOutputFormat);
BIND_MANAGED_TYPE_NAME(::Unity::PhotoCapture, UnityEngine_Windows_WebCam_PhotoCapture);
