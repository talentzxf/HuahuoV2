#pragma once


#include "CameraParameters.h"
#include "MixedRealityCaptureAudioEffect.h"
#include "Runtime/Graphics/Resolution.h"
#include "Runtime/Scripting/BindingsDefs.h"
#include "Runtime/Scripting/ScriptingTypes.h"

namespace Unity
{
    class VideoCapture;
}

// This is a helper class that makes sure the generated bindings files don't get exposed to C++/CX
// Until VideoCapture stops using C++/CX, this will have to stand in between bindings layer and the implementation
class VideoCaptureBindings
{
#if PLATFORM_WIN && !PLATFORM_XBOXONE
public:
    static Unity::VideoCapture* Instantiate(bool showHolograms, ScriptingObjectPtr callback);
    static dynamic_array<Resolution> GetSupportedResolutions();
    static dynamic_array<float> GetSupportedFrameRatesForResolution(int resolutionWidth, int resolutionHeight);

    static void Dispose(Unity::VideoCapture* videoCapture);
    static void DisposeThreaded(Unity::VideoCapture* videoCapture);

    static void StartVideoMode(Unity::VideoCapture* videoCapture, Unity::CameraParameters cameraParameters, Unity::AudioMixerMode audioState, const ScriptingObjectPtr onPhotoModeStartedCallback);
    static void StopVideoMode(Unity::VideoCapture* videoCapture, const ScriptingObjectPtr onPhotoModeStoppedCallback);
    static void StartRecordingVideoToDisk(Unity::VideoCapture* videoCapture, const ICallString & filename, const ScriptingObjectPtr callback);
    static void StopRecordingVideoToDisk(Unity::VideoCapture* videoCapture, const ScriptingObjectPtr callback);
    static bool IsRecording(Unity::VideoCapture* videoCapture);

    static void* GetUnsafePointerToVideoDeviceController(Unity::VideoCapture* videoCapture);
#endif
};

BIND_MANAGED_TYPE_NAME(::Unity::VideoCapture, UnityEngine_Windows_WebCam_VideoCapture);
