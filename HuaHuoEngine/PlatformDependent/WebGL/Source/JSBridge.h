#pragma once

inline core::string GetStringFromJS(UInt32(*func)(char*, UInt32))
{
    core::string str(func(0, 0) + 1, 0); // allocate string matching required size plus NULL terminator
    func((char*)str.data(), str.size()); // fill string data
    str.resize(str.size() - 1); // strip NULL terminator
    return str;
}

inline core::string GetStringFromJS(UInt32(*func)(UInt32, char*, UInt32), UInt32 index)
{
    core::string str(func(index, 0, 0) + 1, 0); // allocate string matching required size plus NULL terminator
    func(index, (char*)str.data(), str.size()); // fill string data
    str.resize(str.size() - 1); // strip NULL terminator
    return str;
}

void GetCanvasClientSize(double *outWidth, double *outHeight);

extern "C"
{
    void JS_FileSystem_Initialize(void);
    void JS_FileSystem_Sync(void);
    void JS_FileSystem_Terminate(void);

    void JS_Profiler_InjectJobs(void);

    UInt32 JS_SystemInfo_HasWebGL(void);
    UInt32 JS_SystemInfo_HasCursorLock(void);
    UInt32 JS_SystemInfo_HasFullscreen(void);
    UInt32 JS_SystemInfo_IsMobile(void);
    void JS_SystemInfo_GetScreenSize(double *outWidth, double *outHeight);
    void JS_SystemInfo_GetCanvasClientSize(const char *domElementSelector, double *outWidth, double *outHeight);
    double JS_SystemInfo_GetPreferredDevicePixelRatio(void);
    bool JS_SystemInfo_GetMatchWebGLToCanvasSize(void);
    UInt32 JS_SystemInfo_GetDocumentURL(char* buffer, UInt32 bufferSize);
    UInt32 JS_SystemInfo_GetStreamingAssetsURL(char* buffer, UInt32 bufferSize);
    UInt32 JS_SystemInfo_GetBrowserName(char* buffer, UInt32 bufferSize);
    UInt32 JS_SystemInfo_GetBrowserVersion(void);
    UInt32 JS_SystemInfo_GetBrowserVersionString(char* buffer, UInt32 bufferSize);
    UInt32 JS_SystemInfo_GetLanguage(char* buffer, UInt32 bufferSize);
    UInt32 JS_SystemInfo_GetOS(char* buffer, UInt32 bufferSize);
    UInt32 JS_SystemInfo_GetMemory(void);
    UInt32 JS_SystemInfo_GetGPUInfo(char* buffer, UInt32 bufferSize);

    void JS_Sound_Init(void);
    UInt32 JS_Sound_ReleaseInstance(UInt32 instance);
    UInt32 JS_Sound_Load(void* audioPtr, UInt32 length);
    UInt32 JS_Sound_Load_PCM(UInt32 channels, UInt32 length, UInt32 sampleRate, float *ptr);
    UInt32 JS_Sound_Create_Channel(void (*callback)(void*), void* userData);
    void JS_Sound_Play(UInt32 buffer, UInt32 channel, double offset, double delay);
    void JS_Sound_Stop(UInt32 channel, double delay);
    void JS_Sound_SetPosition(UInt32 channel, double x, double y, double z);
    void JS_Sound_SetVolume(UInt32 channel, double volume);
    void JS_Sound_SetPitch(UInt32 channel, double pitch);
    void JS_Sound_SetListenerPosition(double x, double y, double z);
    void JS_Sound_SetListenerOrientation(double x, double y, double z, double xUp, double yUp, double zUp);
    void JS_Sound_SetLoop(UInt32 channel, bool loop);
    void JS_Sound_SetLoopPoints(UInt32 channel, double loopStart, double loopEnd);
    void JS_Sound_Set3D(UInt32 channel, bool threeD);
    UInt32 JS_Sound_GetLoadState(UInt32 buffer);
    UInt32 JS_Sound_GetLength(UInt32 buffer);
    void JS_Sound_ResumeIfNeeded(void);

    void JS_Log_Dump(const char* condition, UInt32 type);
    void JS_Log_StackTrace(char* buffer, UInt32 bufferSize);

    void JS_Cursor_SetShow(bool show);
    void JS_Cursor_SetImage(void *data, UInt32 size);

    void JS_Eval_EvalJS(const char* code);
    void JS_Eval_OpenURL(const char* url);
    UInt32 JS_Eval_SetInterval(void (*em_arg_callback_func)(void*), void *arg, int millis);
    void JS_Eval_ClearInterval(UInt32 id);
    UInt32 JS_Eval_SetTimeout(void (*em_arg_callback_func)(void*), void *arg, int millis);
    void JS_Eval_ClearTimeout(UInt32 id);

    UInt32 JS_WebCam_IsSupported(void);
    void JS_WebCam_EnumerateDevices(void);

    UInt32 JS_WebCamVideo_GetNumDevices(void);
    UInt32 JS_WebCamVideo_GetDeviceName(UInt32 deviceId, char* buffer, UInt32 bufferSize);
    UInt32 JS_WebCamVideo_CanPlay(UInt32 deviceId);
    void JS_WebCamVideo_Start(UInt32 deviceId);
    void JS_WebCamVideo_Stop(UInt32 deviceId);
    UInt32 JS_WebCamVideo_GetNativeWidth(UInt32 deviceId);
    UInt32 JS_WebCamVideo_GetNativeHeight(UInt32 deviceId);
    void JS_WebCamVideo_GrabFrame(UInt32 deviceId, void* buffer, UInt32 destWidth, UInt32 destHeight);

    UInt32 JS_WebRequest_Create(const char* url, const char* method);
    void JS_WebRequest_SetTimeout(UInt32 request, UInt32 timeout);
    void JS_WebRequest_SetRequestHeader(UInt32 request, const char* header, const char* value);
    void JS_WebRequest_SetResponseHandler(UInt32 request, void *ref, void (*onresponse)(void*, int, void*, UInt32, char*, int));
    void JS_WebRequest_SetProgressHandler(UInt32 request, void *ref, void (*onprogress)(void*, UInt32, UInt32));
    void JS_WebRequest_Send(UInt32 request, void *ptr, UInt32 length);
    UInt32 JS_WebRequest_GetResponseHeaders(UInt32 request, char* buffer, UInt32 bufferSize);
    UInt32 JS_WebRequest_GetStatusLine(UInt32 request, char* buffer, UInt32 bufferSize);
    void JS_WebRequest_Abort(UInt32 request);
    void JS_WebRequest_Release(UInt32 request);

    UInt32 JS_Video_Create(const char* url);
    void JS_Video_Destroy(UInt32 video);
    bool JS_Video_UpdateToTexture(UInt32 video, void *tex);
    void JS_Video_Play(UInt32 video);
    void JS_Video_Pause(UInt32 video);
    void JS_Video_Seek(UInt32 video, double time);
    void JS_Video_SetLoop(UInt32 video, bool loop);
    void JS_Video_SetMute(UInt32 video, bool mute);
    void JS_Video_SetPlaybackRate(UInt32 video, double rate);
    UInt16 JS_Video_GetNumAudioTracks(UInt32 video);
    void JS_Video_EnableAudioTrack(UInt32 video, UInt16 trackIndex, bool enabled);
    const char* JS_Video_GetAudioLanguageCode(UInt32 video, UInt16 trackIndex);
    void JS_Video_SetVolume(UInt32 video, double rate);
    UInt32 JS_Video_Height(UInt32 video);
    UInt32 JS_Video_Width(UInt32 video);
    double JS_Video_Time(UInt32 video);
    double JS_Video_Duration(UInt32 video);
    bool JS_Video_IsReady(UInt32 video);
    bool JS_Video_IsPlaying(UInt32 video);
    void JS_Video_SetErrorHandler(UInt32 video, void *ref, void (*onerror)(void*, int));
    void JS_Video_SetReadyHandler(UInt32 video, void *ref, void (*onready)(void*));
    void JS_Video_SetEndedHandler(UInt32 video, void *ref, void (*onended)(void*));
    void JS_Video_SetSeekedOnceHandler(UInt32 video, void *ref, void (*onseeked)(void*));
    bool JS_Video_CanPlayFormat(const char* format);
}
