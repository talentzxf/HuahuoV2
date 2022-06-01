#pragma once

#include "Runtime/Scripting/ScriptingUtility.h"

#if ENABLE_AUDIO

#if PLATFORM_WEBGL
#define ScriptingHandle int
#endif

#include "Modules/Audio/Public/correct_fmod_includer.h"

class AudioChannel
{
public:
    AudioChannel(ScriptingHandle soundObject, bool loop, unsigned int sampleCount);
    ~AudioChannel();
    int set3DSpread(float a) { return 0; }
    int stop();
    void setPaused(bool b);

    FMOD_RESULT getPaused(bool *paused);
    FMOD_RESULT isPlaying(bool *isPlaying);
    void setMute(bool mute);
    int getMute(bool *mute);
    int set3DMinMaxDistance(float dist, float dist2) { return 0; }
    int get3DMinMaxDistance(float *dist, float *dist2) { return 0; }
    void setMode(int mode) {}
    int setVolume(float volume);
    int getPosition(unsigned *position, int mode);
    int setPosition(float pos, int mode);
    void setFrequency(float freq) {}
    int getFrequency(float *freq) { return 0; }
    void setPriority(int prio) {}
    int getPriority(int *prio) { return 0; }
    int set3DPanLevel(float level) { return 0; }
    int set3DConeOrientation(FMOD_VECTOR*) { return 0; }
    void setPan(float pan);
    void getPan(float* pan);
    void setUserData(void* data) {m_UserData = data; }
    void getUserData(void** result) {*result = m_UserData; }

    void SetDistanceVolumeMultiplier(float distanceVolumeMultiplier);
    void CompleteCallBack();
private:
    float m_DistanceVolumeMultiplier;
    float m_Volume;
    void* m_UserData;
    float m_Pan;
    unsigned int m_SampleCount;
    double m_SoundLength;
    double pcmConversion;

    bool m_IsPaused;
    bool m_IsPlaying;
    bool m_IsMuted;

    int m_PausePosition;
    int m_Loop;

    ScriptingHandle m_SoundObject;
    ScriptingHandle m_SoundInstanceData;
    void UpdateFlashVolume();
};

//Audio
extern "C" ScriptingHandle Ext_Sound_Load(void* audioPtr, int length);
extern "C" ScriptingHandle Ext_Sound_Get_InstanceData(ScriptingHandle sound, AudioChannel * instance, void(AudioChannel::*CompleteCallBack)());
extern "C" double Ext_Sound_Get_Length(ScriptingHandle soundObject);

#endif
