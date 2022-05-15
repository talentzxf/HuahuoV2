#pragma once

#include "Runtime/Graphics/Resolution.h"
#include "Runtime/Scripting/ScriptingTypes.h"
#include "Runtime/Scripting/ScriptingUtility.h"
#include "Runtime/Utilities/RuntimeStatic.h"
#include "Runtime/EventQueue/GlobalEventQueue.h"
#include "PlatformDependent/Win/Webcam/CameraParameters.h"
#include "PlatformDependent/Win/Webcam/MixedRealityCaptureAudioEffect.h"

struct OnCreatedVideoCaptureResourceEventData;
struct OnStartedVideoModeEventData;
struct OnStoppedVideoModeEventData;
struct OnStartedRecordingVideoToDiskEventData;
struct OnStoppedRecordingVideoToDiskEventData;
struct OnVideoCaptureDestructionEventData;
struct OnNotifyScriptAboutVideoCaptureErrorEventData;
struct OnVideoCaptureAsyncOperationCompleteEventData;
struct OnVideoCaptureStartRecordingOperationEventData;

namespace Unity
{
#if PLATFORM_WIN && !PLATFORM_XBOXONE
    class VideoCapture
    {
    public:

        static VideoCapture* Instantiate(bool showHolograms, ScriptingObjectPtr callback);
        static dynamic_array<Resolution> GetSupportedResolutions();
        static dynamic_array<float> GetSupportedFrameRatesForResolution(int resolutionWidth, int resolutionHeight);

        void Dispose();
        void DisposeThreaded();

        void StartVideoMode(CameraParameters cameraParameters, Unity::AudioMixerMode audioState, const ScriptingObjectPtr onPhotoModeStartedCallback);
        void StopVideoMode(const ScriptingObjectPtr onPhotoModeStoppedCallback);
        void StartRecordingVideoToDisk(const ICallString & filename, const ScriptingObjectPtr callback);
        void StopRecordingVideoToDisk(const ScriptingObjectPtr callback);
        bool IsRecording();

        void* GetUnsafePointerToVideoDeviceController();

        REFLECTABLE_ENUM(OperationType,
            Create,
            StartVideoMode,
            StopVideoMode,
            StartRecordingVideo,
            StopRecordingVideo,
            Unknown
        );

    private:

        REFLECTABLE_ENUM(RecordingState,
            Uninitialized,
            Idle,
            PreparingToStartRecording,
            Recording,
            PreparingToStopRecording
        );

        REFLECTABLE_ENUM(VideoModeState,
            Activating,
            Activated,
            Deactivating,
            Deactivated
        );

        struct VideoCaptureStatics
        {
            VideoCaptureStatics()
                : activeInstance(nullptr)
                , videoModeState(VideoModeState::Deactivated)
                , staticInitComplete(false)
#if UNITY_EDITOR
                , videoCaptureInstances(kMemWebCam)
#endif
            {}

            VideoCapture* activeInstance;
            VideoModeState videoModeState;
            bool staticInitComplete;

#if UNITY_EDITOR
            // We need to release all VideoCapture instances when exiting Playmode in the Editor
            dynamic_array<VideoCapture*> videoCaptureInstances;
#endif
        };
        static RuntimeStatic<VideoCaptureStatics> s_VideoCaptureStatics;

        win::ComPtr<UnityWinRTBase::Windows::Media::Capture::IMediaCapture> m_MediaCapture;

        CameraParameters m_CameraParameters;
        ScriptingGCHandle m_OnCreateCallbackGCHandle;
        ScriptingGCHandle m_OnVideoModeStartedGCHandle;
        ScriptingGCHandle m_OnVideoModeStoppedGCHandle;
        ScriptingGCHandle m_OnVideoRecordingStartedGCHandle;
        ScriptingGCHandle m_OnVideoRecordingStoppedGCHandle;
        RecordingState m_RecordingState;
        Unity::AudioMixerMode m_AudioState;
        bool m_ShowHolograms;
        bool m_HasErrors;
        bool m_IsDisposed;
        bool m_IsRecording;
#if UNITY_EDITOR
        bool m_IsQueuedForDestruction;
#endif

        VideoCapture(bool showHolograms, const ScriptingObjectPtr onCreateCallback);

        void InitializeAsync();

        // Methods used in async operation "continuation" chains
        void DisableMixedRealityVideoEffectsAsync_StartVideo();
        void DisableMixedRealityVideoEffectsAsync_StopRecording();
        void EnableMixedRealityVideoEffectAsync();
        void EnableMixedRealityAudioEffectAsync();
        void EnableAudioRecording();
        void DisableAudioRecording();
        void DisableMixedRealityVideoEffectsAsync(OperationType operation, ScriptingGCHandle* pGCScriptHandle);
        void SetAudioRecordingEnabled(bool enable);
        void StartActualRecordingAsync(const win::ComPtr<UnityWinRTBase::Windows::Storage::IStorageFile>& storageFile);

        void NotifyScriptOfError(OperationType operationType, HRESULT hr, ScriptingGCHandle callbackGCHandle);
        void NotifyScriptOfError(OperationType operationType, HRESULT hr, const char* message, ScriptingObjectPtr callback);

        void SendOnCreatedVideoCaptureResourceEvent();
        UnityEventQueue::ClassBasedEventHandler<OnCreatedVideoCaptureResourceEventData, VideoCapture> m_OnCreatedVideoCaptureResourceEventDelegate;
        void HandleEvent(const OnCreatedVideoCaptureResourceEventData& eventData);
        friend class UnityEventQueue::ClassBasedEventHandler<OnCreatedVideoCaptureResourceEventData, VideoCapture>;
        void OnCreatedVideoCaptureResource();

        void SendOnStartedVideoModeEvent();
        UnityEventQueue::ClassBasedEventHandler<OnStartedVideoModeEventData, VideoCapture> m_OnStartedVideoModeEventDelegate;
        void HandleEvent(const OnStartedVideoModeEventData& eventData);
        friend class UnityEventQueue::ClassBasedEventHandler<OnStartedVideoModeEventData, VideoCapture>;
        void OnStartedVideoMode();

        void SendOnStoppedVideoModeEvent();
        UnityEventQueue::ClassBasedEventHandler<OnStoppedVideoModeEventData, VideoCapture> m_OnStoppedVideoModeEventDelegate;
        void HandleEvent(const OnStoppedVideoModeEventData& eventData);
        friend class UnityEventQueue::ClassBasedEventHandler<OnStoppedVideoModeEventData, VideoCapture>;
        void OnStoppedVideoMode();

        void SendOnStartedRecordingVideoToDiskModeEvent();
        UnityEventQueue::ClassBasedEventHandler<OnStartedRecordingVideoToDiskEventData, VideoCapture> m_OnStartedRecordingVideoToDiskEventDelegate;
        void HandleEvent(const OnStartedRecordingVideoToDiskEventData& eventData);
        friend class UnityEventQueue::ClassBasedEventHandler<OnStartedRecordingVideoToDiskEventData, VideoCapture>;
        void OnStartedRecordingVideoToDisk();

        void SendOnStoppedRecordingVideoToDiskModeEvent();
        UnityEventQueue::ClassBasedEventHandler<OnStoppedRecordingVideoToDiskEventData, VideoCapture> m_OnStoppedRecordingVideoToDiskEventDelegate;
        void HandleEvent(const OnStoppedRecordingVideoToDiskEventData& eventData);
        friend class UnityEventQueue::ClassBasedEventHandler<OnStoppedRecordingVideoToDiskEventData, VideoCapture>;
        void OnStoppedRecordingVideoToDisk();

        void NotifyScriptOfErrorOnMainThread(OperationType operationType, HRESULT hr, ScriptingGCHandle callbackGCHandle);
        UnityEventQueue::ClassBasedEventHandler<OnNotifyScriptAboutVideoCaptureErrorEventData, VideoCapture> m_OnNotifyScriptAboutErrorEventDelegate;
        void HandleEvent(const OnNotifyScriptAboutVideoCaptureErrorEventData& eventData);
        friend class UnityEventQueue::ClassBasedEventHandler<OnNotifyScriptAboutVideoCaptureErrorEventData, VideoCapture>;

        static void HandleDestructionEvent(const OnVideoCaptureDestructionEventData& eventData);
        static void HandleAsyncOperationCompleteEvent(const OnVideoCaptureAsyncOperationCompleteEventData& eventData);
        static void HandleStartRecordingOperationEvent(const OnVideoCaptureStartRecordingOperationEventData& eventData);

#if UNITY_EDITOR
        static void OnExitPlayMode();
#endif

#else
    // we need to stub in a class for the bindings to work, since the above declaration depends on WinRT interfaces
    class VideoCapture {};
#endif
    };
}
