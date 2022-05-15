#include "UnityPrefix.h"

#include "VideoCaptureBindings.h"
#include "VideoCapture.h"


Unity::VideoCapture* VideoCaptureBindings::Instantiate(bool showHolograms, ScriptingObjectPtr callback)
{
    return Unity::VideoCapture::Instantiate(showHolograms, callback);
}

dynamic_array<Resolution> VideoCaptureBindings::GetSupportedResolutions()
{
    return Unity::VideoCapture::GetSupportedResolutions();
}

dynamic_array<float> VideoCaptureBindings::GetSupportedFrameRatesForResolution(int resolutionWidth, int resolutionHeight)
{
    return Unity::VideoCapture::GetSupportedFrameRatesForResolution(resolutionWidth, resolutionHeight);
}

void VideoCaptureBindings::Dispose(Unity::VideoCapture* videoCapture)
{
    videoCapture->Dispose();
}

void VideoCaptureBindings::DisposeThreaded(Unity::VideoCapture* videoCapture)
{
    videoCapture->DisposeThreaded();
}

void VideoCaptureBindings::StartVideoMode(Unity::VideoCapture* videoCapture, Unity::CameraParameters cameraParameters, Unity::AudioMixerMode audioState, const ScriptingObjectPtr onPhotoModeStartedCallback)
{
    videoCapture->StartVideoMode(cameraParameters, audioState, onPhotoModeStartedCallback);
}

void VideoCaptureBindings::StopVideoMode(Unity::VideoCapture* videoCapture, const ScriptingObjectPtr onPhotoModeStoppedCallback)
{
    videoCapture->StopVideoMode(onPhotoModeStoppedCallback);
}

void VideoCaptureBindings::StartRecordingVideoToDisk(Unity::VideoCapture* videoCapture, const ICallString & filename, const ScriptingObjectPtr callback)
{
    videoCapture->StartRecordingVideoToDisk(filename, callback);
}

void VideoCaptureBindings::StopRecordingVideoToDisk(Unity::VideoCapture* videoCapture, const ScriptingObjectPtr callback)
{
    videoCapture->StopRecordingVideoToDisk(callback);
}

bool VideoCaptureBindings::IsRecording(Unity::VideoCapture* videoCapture)
{
    return videoCapture->IsRecording();
}

void* VideoCaptureBindings::GetUnsafePointerToVideoDeviceController(Unity::VideoCapture* videoCapture)
{
    return videoCapture->GetUnsafePointerToVideoDeviceController();
}
