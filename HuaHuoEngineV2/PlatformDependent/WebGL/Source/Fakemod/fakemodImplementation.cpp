/*
To generate an empty stub for this file (when fmod function delcarations change):

Paste the code below into a file named fakemodstub.cpp, and then run:
gcc -E fakemodstub.cpp

#include "fakemodMacros.h"

#define FMOD_IMPLEMENT_METHOD(klass,method,...) \
FMOD_RESULT klass##I::method(PARAMETER_LIST(DECLARE_PARAMETER,__VA_ARGS__)) {\
    AUDIO_LOGSCOPE_STATEVAL();\
    FMOD_RETURN(FMOD_OK); \
}
#include "api/fakemod.inc"
*/
#include "fakemod.h"

#include "Runtime/Input/TimeManager.h"
#include "PlatformDependent/WebGL/Source/JSBridge.h"

UInt64 FakeGetTimeInSamples()
{
    return GetTimeSinceStartup() * FAKE_SAMPLERATE;
}

namespace FMOD
{
    SystemI::SystemI(int index)
        : FMOD_INIT_CLASS(NULL)
        , m_numrealchannels(0)
        , m_maxrealchannels(0)
        , m_realchannels(NULL)
        , m_masterchannelgroup(NULL)
        , m_rolloffcallback(NULL)
        , m_systemcallback(NULL)
        , m_dspbufferlength(1024)
        , m_dspnumbuffers(2)
        , m_channels(this, 1024)
        , m_channelgroups(this, 8192)
        , m_sounds(this, 4096)
        , m_dsps(this, 1024)
        , m_reverbs(this, 1024)
        , m_dopplerscale(1.0f)
        , m_distancefactor(1.0f)
        , m_rolloffscale(1.0f)
    {
        AUDIO_LOGSCOPE_STATEVAL();

        m_listenerposition.x = 0;
        m_listenerposition.y = 0;
        m_listenerposition.z = 0;

        m_listenervelocity.x = 0;
        m_listenervelocity.y = 0;
        m_listenervelocity.z = 0;

        m_listenerforward.x = 0;
        m_listenerforward.y = 0;
        m_listenerforward.z = 0;

        m_listenerup.x = 0;
        m_listenerup.y = 0;
        m_listenerup.z = 0;

        setSoftwareChannels(32);
    }

    SystemI::~SystemI()
    {
        AUDIO_LOGSCOPE_STATEVAL();
    }

    SoundI::SoundI(SystemI* system)
        : FMOD_INIT_CLASS(system)
        , m_numchannels(0)
        , m_channels(NULL)
        , m_mode(0)
        , m_frequency(0)
        , m_instance(0)
        , m_loopstart(0)
        , m_loopend(0)
        , m_format(FMOD_SOUND_FORMAT_NONE)
    {
        AUDIO_LOGSCOPE_STATEVAL();
    }

    SoundI::~SoundI()
    {
        if (m_instance)
            JS_Sound_ReleaseInstance(m_instance);
        AUDIO_LOGSCOPE_STATEVAL();
    }

    void SoundI::StopAllChannels()
    {
        AUDIO_LOGSCOPE_STATEVAL();
        while (!m_channels.Empty())
        {
            m_channels.First()->stop();
        }
    }

    void FakeModChannelCallback(void* channel)
    {
        ((ChannelI*)channel)->EndCallback();
    }

    ChannelI::ChannelI(SystemI* system)
        : FMOD_INIT_CLASS(system)
        , m_mode(0)
        , m_starttimeinsamples(0)
        , m_pausetimeinsamples(0)
        , m_stoptimeinsamples(0)
        , m_length(0)
        , m_ismuted(false)
        , m_ispaused(false)
        , m_isvirtual(false)
        , m_sound(NULL)
        , m_channelgroup(NULL)
        , m_soundchain(this)
        , m_realchannelchain(this)
        , m_volume(1.0f)
        , m_panlevel(1.0f)
        , m_3Ddopplerlevel(1.0f)
        , m_dopplerpitch(1.0f)
        , m_delay(0.0f)
        , m_channelcallback(NULL)
        , m_frequency(FAKE_SAMPLERATE)
    {
        AUDIO_LOGSCOPE_STATEVAL();
        m_position.x = 0;
        m_position.y = 0;
        m_position.z = 0;

        m_velocity.x = 0;
        m_velocity.y = 0;
        m_velocity.z = 0;

        m_instance = JS_Sound_Create_Channel(FakeModChannelCallback, this);
        m_audibility = (rand() % 1000) * 0.001f;
    }

    void ChannelI::EndCallback()
    {
        if (SystemI::Validate<Channel, ChannelI>(m_handle) != this)
        {
            // we already cleaned up.
            return;
        }

        AUDIO_LOGSCOPE_STATEVAL();
        m_soundchain.RemoveFromList();
        m_realchannelchain.RemoveFromList();
        --m_system->m_numrealchannels;
        if (m_sound != NULL)
        {
            m_sound = NULL;
        }

        if (m_channelcallback)
            m_channelcallback((FMOD_CHANNEL*)m_handle, FMOD_CHANNEL_CALLBACKTYPE_END, NULL, NULL);

        // Maybe don't release here directly
        m_system->Release<Channel, ChannelI>(this);
    }

    ChannelI::~ChannelI()
    {
        if (m_instance)
            JS_Sound_ReleaseInstance(m_instance);
        AUDIO_LOGSCOPE_STATEVAL();
    }

    ChannelGroupI::ChannelGroupI(SystemI* system)
        : FMOD_INIT_CLASS(system)
        , m_audibilityFactor(1.0f)
        , m_channels(NULL)
        , m_parent(NULL)
        , m_subgroups(NULL)
        , m_parentpool(this)
        , m_volume(1.0f)
    {
        AUDIO_LOGSCOPE_STATEVAL();
    }

    ChannelGroupI::~ChannelGroupI()
    {
        AUDIO_LOGSCOPE_STATEVAL();
    }

    DSPI::DSPI(SystemI* system)
        : FMOD_INIT_CLASS(system)
    {
        AUDIO_LOGSCOPE_STATEVAL();
    }

    DSPI::~DSPI()
    {
        AUDIO_LOGSCOPE_STATEVAL();
    }

    ReverbI::ReverbI(SystemI* system)
        : FMOD_INIT_CLASS(system)
    {
        AUDIO_LOGSCOPE_STATEVAL();
    }

    ReverbI::~ReverbI()
    {
        AUDIO_LOGSCOPE_STATEVAL();
    }

    FMOD_RESULT ChannelGroupI::addDSP(DSP* dsp , DSPConnection** con)
    {
        FMOD_RETURN(FMOD_OK);
    }

    FMOD_RESULT ChannelGroupI::addGroup(ChannelGroup* group)
    {
        ChannelGroupI* subgroup = SystemI::Validate<ChannelGroup, ChannelGroupI>(group);
        if (subgroup != NULL)
        {
            m_subgroups.Add(&subgroup->m_parentpool);
            subgroup->m_parent = this;
        }
        FMOD_RETURN(FMOD_OK);
    }

    FMOD_RESULT ChannelGroupI::get3DOcclusion(float * directocclusion , float * reverbocclusion)
    {
        FMOD_RETURN(FMOD_ERR_UNIMPLEMENTED);
    }

    FMOD_RESULT ChannelGroupI::getAudibilityFactor(float *factor)
    {
        if (factor != NULL)
            *factor = m_audibilityFactor;
        FMOD_RETURN(FMOD_OK);
    }

    FMOD_RESULT ChannelGroupI::getChannel(int index , Channel ** channel)
    {
        FMOD_RETURN(FMOD_ERR_UNIMPLEMENTED);
    }

    FMOD_RESULT ChannelGroupI::getDSPHead(DSP** dsp)
    {
        if (dsp != NULL)
            *dsp = NULL;
        FMOD_RETURN(FMOD_OK);
    }

    FMOD_RESULT ChannelGroupI::getGroup(int index , ChannelGroup ** group)
    {
        FMOD_RETURN(FMOD_ERR_UNIMPLEMENTED);
    }

    FMOD_RESULT ChannelGroupI::getMemoryInfo(unsigned int memorybits , unsigned int event_memorybits, unsigned int * memoryused, FMOD_MEMORY_USAGE_DETAILS * memoryused_details)
    {
        FMOD_RETURN(FMOD_ERR_UNIMPLEMENTED);
    }

    FMOD_RESULT ChannelGroupI::getMute(bool * mute)
    {
        FMOD_RETURN(FMOD_ERR_UNIMPLEMENTED);
    }

    FMOD_RESULT ChannelGroupI::getName(char * name , int namelen)
    {
        FMOD_RETURN(FMOD_ERR_UNIMPLEMENTED);
    }

    FMOD_RESULT ChannelGroupI::getNumChannels(int * numchannels)
    {
        if (numchannels != NULL)
            *numchannels = 0;
        FMOD_RETURN(FMOD_OK);
    }

    FMOD_RESULT ChannelGroupI::getNumGroups(int * numgroups)
    {
        if (numgroups != NULL)
            *numgroups = 0;
        FMOD_RETURN(FMOD_OK);
    }

    FMOD_RESULT ChannelGroupI::getParentGroup(ChannelGroup** group)
    {
        if (group != NULL)
            *group = (m_parent == NULL) ? NULL : m_parent->m_handle;
        FMOD_RETURN(FMOD_OK);
    }

    FMOD_RESULT ChannelGroupI::getPaused(bool * paused)
    {
        FMOD_RETURN(FMOD_ERR_UNIMPLEMENTED);
    }

    FMOD_RESULT ChannelGroupI::getPitch(float * pitch)
    {
        FMOD_RETURN(FMOD_ERR_UNIMPLEMENTED);
    }

    FMOD_RESULT ChannelGroupI::getSpectrum(float* spectrumarray , int numvalues, int channeloffset, FMOD_DSP_FFT_WINDOW windowtype)
    {
        FMOD_RETURN(FMOD_OK);
    }

    FMOD_RESULT ChannelGroupI::getSystemObject(System ** system)
    {
        FMOD_RETURN(FMOD_ERR_UNIMPLEMENTED);
    }

    FMOD_RESULT ChannelGroupI::getUserData(void ** userdata)
    {
        FMOD_RETURN(FMOD_ERR_UNIMPLEMENTED);
    }

    FMOD_RESULT ChannelGroupI::getVolume(float * volume)
    {
        if (volume != NULL)
            *volume = m_volume;
        FMOD_RETURN(FMOD_OK);
    }

    FMOD_RESULT ChannelGroupI::getWaveData(float* wavearray , int numsamples, int channeloffset)
    {
        FMOD_RETURN(FMOD_OK);
    }

    FMOD_RESULT ChannelGroupI::override3DAttributes(const FMOD_VECTOR * pos , const FMOD_VECTOR * vel)
    {
        FMOD_RETURN(FMOD_ERR_UNIMPLEMENTED);
    }

    FMOD_RESULT ChannelGroupI::overrideFrequency(float frequency)
    {
        FMOD_RETURN(FMOD_ERR_UNIMPLEMENTED);
    }

    FMOD_RESULT ChannelGroupI::overridePan(float pan)
    {
        FMOD_RETURN(FMOD_ERR_UNIMPLEMENTED);
    }

    FMOD_RESULT ChannelGroupI::overrideReverbProperties(const FMOD_REVERB_CHANNELPROPERTIES * prop)
    {
        FMOD_RETURN(FMOD_ERR_UNIMPLEMENTED);
    }

    FMOD_RESULT ChannelGroupI::overrideSpeakerMix(float frontleft , float frontright, float center, float lfe, float backleft, float backright, float sideleft, float sideright)
    {
        FMOD_RETURN(FMOD_ERR_UNIMPLEMENTED);
    }

    FMOD_RESULT ChannelGroupI::overrideVolume(float volume)
    {
        FMOD_RETURN(FMOD_ERR_UNIMPLEMENTED);
    }

    FMOD_RESULT ChannelGroupI::release()
    {
        m_system->Release<ChannelGroup, ChannelGroupI>(this);
        FMOD_RETURN(FMOD_OK);
    }

    FMOD_RESULT ChannelGroupI::set3DOcclusion(float directocclusion , float reverbocclusion)
    {
        FMOD_RETURN(FMOD_ERR_UNIMPLEMENTED);
    }

    FMOD_RESULT ChannelGroupI::setAudibilityFactor(float factor)
    {
        m_audibilityFactor = factor;
        FMOD_RETURN(FMOD_OK);
    }

    FMOD_RESULT ChannelGroupI::setMute(bool mute)
    {
        FMOD_RETURN(FMOD_OK);
    }

    FMOD_RESULT ChannelGroupI::setPaused(bool paused)
    {
        FMOD_RETURN(FMOD_OK);
    }

    FMOD_RESULT ChannelGroupI::setPitch(float pitch)
    {
        FMOD_RETURN(FMOD_OK);
    }

    FMOD_RESULT ChannelGroupI::setUserData(void * userdata)
    {
        FMOD_RETURN(FMOD_ERR_UNIMPLEMENTED);
    }

    FMOD_RESULT ChannelGroupI::setVolume(float volume)
    {
        m_volume = volume;
        FMOD_RETURN(FMOD_OK);
    }

    FMOD_RESULT ChannelGroupI::stop()
    {
        FMOD_RETURN(FMOD_ERR_UNIMPLEMENTED);
    }

    FMOD_RESULT ChannelI::addDSP(DSP * dsp , DSPConnection ** connection)
    {
        FMOD_RETURN(FMOD_ERR_UNIMPLEMENTED);
    }

    FMOD_RESULT ChannelI::get3DAttributes(FMOD_VECTOR * pos , FMOD_VECTOR * vel)
    {
        FMOD_RETURN(FMOD_ERR_UNIMPLEMENTED);
    }

    FMOD_RESULT ChannelI::get3DConeOrientation(FMOD_VECTOR * orientation)
    {
        FMOD_RETURN(FMOD_ERR_UNIMPLEMENTED);
    }

    FMOD_RESULT ChannelI::get3DConeSettings(float * insideconeangle , float * outsideconeangle, float * outsidevolume)
    {
        FMOD_RETURN(FMOD_ERR_UNIMPLEMENTED);
    }

    FMOD_RESULT ChannelI::get3DCustomRolloff(FMOD_VECTOR ** points , int * numpoints)
    {
        FMOD_RETURN(FMOD_ERR_UNIMPLEMENTED);
    }

    FMOD_RESULT ChannelI::get3DDistanceFilter(bool * custom , float * customLevel, float * centerFreq)
    {
        FMOD_RETURN(FMOD_ERR_UNIMPLEMENTED);
    }

    FMOD_RESULT ChannelI::get3DDopplerLevel(float * level)
    {
        if (level != NULL)
            *level = m_3Ddopplerlevel;
        FMOD_RETURN(FMOD_OK);
    }

    FMOD_RESULT ChannelI::get3DMinMaxDistance(float * mindistance , float * maxdistance)
    {
        FMOD_RETURN(FMOD_ERR_UNIMPLEMENTED);
    }

    FMOD_RESULT ChannelI::get3DOcclusion(float * directocclusion , float * reverbocclusion)
    {
        FMOD_RETURN(FMOD_ERR_UNIMPLEMENTED);
    }

    FMOD_RESULT ChannelI::get3DPanLevel(float * level)
    {
        FMOD_RETURN(FMOD_ERR_UNIMPLEMENTED);
    }

    FMOD_RESULT ChannelI::get3DSpread(float * angle)
    {
        FMOD_RETURN(FMOD_ERR_UNIMPLEMENTED);
    }

    FMOD_RESULT ChannelI::getAudibility(float* audibility)
    {
        if (audibility != NULL)
            *audibility = m_audibility;
        FMOD_RETURN(FMOD_OK);
    }

    FMOD_RESULT ChannelI::getChannelGroup(ChannelGroup** channelgroup)
    {
        if (channelgroup != NULL)
            *channelgroup = (m_channelgroup == NULL) ? NULL : m_channelgroup->m_handle;
        FMOD_RETURN(FMOD_OK);
    }

    FMOD_RESULT ChannelI::getCurrentSound(Sound** sound)
    {
        if (sound != NULL)
            *sound = (m_sound == NULL) ? NULL : m_sound->m_handle;
        FMOD_RETURN(FMOD_OK);
    }

    FMOD_RESULT ChannelI::getDelay(FMOD_DELAYTYPE delaytype , unsigned int* delay_hi, unsigned int* delay_lo)
    {
        if (delay_hi != NULL)
            *delay_hi = 0;
        if (delay_lo != NULL)
            *delay_lo = 0;
        FMOD_RETURN(FMOD_OK);
    }

    FMOD_RESULT ChannelI::getDSPHead(DSP** dsp)
    {
        if (dsp != NULL)
            *dsp = NULL;
        FMOD_RETURN(FMOD_OK);
    }

    FMOD_RESULT ChannelI::getFrequency(float * frequency)
    {
        if (frequency != NULL)
            *frequency = m_frequency;
        FMOD_RETURN(FMOD_OK);
    }

    FMOD_RESULT ChannelI::getIndex(int * index)
    {
        FMOD_RETURN(FMOD_ERR_UNIMPLEMENTED);
    }

    FMOD_RESULT ChannelI::getInputChannelMix(float * levels , int numlevels)
    {
        FMOD_RETURN(FMOD_ERR_UNIMPLEMENTED);
    }

    FMOD_RESULT ChannelI::getLoopCount(int * loopcount)
    {
        FMOD_RETURN(FMOD_ERR_UNIMPLEMENTED);
    }

    FMOD_RESULT ChannelI::getLoopPoints(unsigned int * loopstart , FMOD_TIMEUNIT loopstarttype, unsigned int * loopend, FMOD_TIMEUNIT loopendtype)
    {
        FMOD_RETURN(FMOD_ERR_UNIMPLEMENTED);
    }

    FMOD_RESULT ChannelI::getLowPassGain(float * gain)
    {
        FMOD_RETURN(FMOD_ERR_UNIMPLEMENTED);
    }

    FMOD_RESULT ChannelI::getMemoryInfo(unsigned int memorybits , unsigned int event_memorybits, unsigned int * memoryused, FMOD_MEMORY_USAGE_DETAILS * memoryused_details)
    {
        FMOD_RETURN(FMOD_ERR_UNIMPLEMENTED);
    }

    FMOD_RESULT ChannelI::getMode(FMOD_MODE* mode)
    {
        if (mode != NULL)
            *mode = m_mode;
        FMOD_RETURN(FMOD_OK);
    }

    FMOD_RESULT ChannelI::getMute(bool* muted)
    {
        if (muted != NULL)
            *muted = m_ismuted;
        FMOD_RETURN(FMOD_OK);
    }

    FMOD_RESULT ChannelI::getPan(float * pan)
    {
        FMOD_RETURN(FMOD_ERR_UNIMPLEMENTED);
    }

    FMOD_RESULT ChannelI::getPaused(bool* paused)
    {
        if (paused != NULL)
            *paused = m_ispaused;
        FMOD_RETURN(FMOD_OK);
    }

    FMOD_RESULT ChannelI::getPosition(unsigned int* position , FMOD_TIMEUNIT timeunit)
    {
        SInt64 t = m_ispaused ? m_pausetimeinsamples : FakeGetTimeInSamples();
        SInt64 delta = (t - m_starttimeinsamples) * ((m_frequency * m_dopplerpitch) / (float)FAKE_SAMPLERATE);
        if (delta < 0)
            delta = 0;
        delta %= m_length;

        switch (timeunit)
        {
            case FMOD_TIMEUNIT_PCM:
                if (position != NULL)
                    *position = delta;
                FMOD_RETURN(FMOD_OK);
            case FMOD_TIMEUNIT_MS:
                if (position != NULL)
                    *position = (unsigned int)(delta * 1000.0 / (double)FAKE_SAMPLERATE);
                FMOD_RETURN(FMOD_OK);
            default:
                FMOD_UNIMPLEMENTED();
                FMOD_RETURN(FMOD_ERR_INVALID_PARAM);
        }
        FMOD_RETURN(FMOD_OK);
    }

    FMOD_RESULT ChannelI::getPriority(int * priority)
    {
        FMOD_RETURN(FMOD_ERR_UNIMPLEMENTED);
    }

    FMOD_RESULT ChannelI::getReverbProperties(FMOD_REVERB_CHANNELPROPERTIES* props)
    {
        FMOD_RETURN(FMOD_OK);
    }

    FMOD_RESULT ChannelI::getSpeakerLevels(FMOD_SPEAKER speaker , float * levels, int numlevels)
    {
        FMOD_RETURN(FMOD_ERR_UNIMPLEMENTED);
    }

    FMOD_RESULT ChannelI::getSpeakerMix(float * frontleft , float * frontright, float * center, float * lfe, float * backleft, float * backright, float * sideleft, float * sideright)
    {
        FMOD_RETURN(FMOD_ERR_UNIMPLEMENTED);
    }

    FMOD_RESULT ChannelI::getSpectrum(float* spectrumarray , int numvalues, int channeloffset, FMOD_DSP_FFT_WINDOW windowtype)
    {
        FMOD_RETURN(FMOD_OK);
    }

    FMOD_RESULT ChannelI::getSystemObject(System ** system)
    {
        FMOD_RETURN(FMOD_ERR_UNIMPLEMENTED);
    }

    FMOD_RESULT ChannelI::getUserData(void** userdata)
    {
        if (userdata != NULL)
            *userdata = m_userdata;
        FMOD_RETURN(FMOD_OK);
    }

    FMOD_RESULT ChannelI::getVolume(float * volume)
    {
        FMOD_RETURN(FMOD_ERR_UNIMPLEMENTED);
    }

    FMOD_RESULT ChannelI::getWaveData(float* wavearray , int numsamples, int channeloffset)
    {
        FMOD_RETURN(FMOD_OK);
    }

    FMOD_RESULT ChannelI::isPlaying(bool* isplaying)
    {
        if (isplaying != NULL)
            *isplaying = m_sound != NULL && !m_ispaused;
        FMOD_RETURN(FMOD_OK);
    }

    FMOD_RESULT ChannelI::isVirtual(bool* isvirtual)
    {
        if (isvirtual != NULL)
            *isvirtual = m_isvirtual;
        FMOD_RETURN(FMOD_OK);
    }

#define FMOD_Vector_Subtract(_a, _b, _r) \
     (_r)->x = (_a)->x - (_b)->x;          \
     (_r)->y = (_a)->y - (_b)->y;          \
     (_r)->z = (_a)->z - (_b)->z;

#define FMOD_Vector_DotProduct(_a, _b)                              \
    (((_a)->x * (_b)->x) + ((_a)->y * (_b)->y) + ((_a)->z * (_b)->z))


#define FMOD_Vector_GetLengthFast(_x)  sqrt(FMOD_Vector_DotProduct((_x), (_x)))


    FMOD_RESULT ChannelI::set3DAttributes(const FMOD_VECTOR* pos , const FMOD_VECTOR* vel)
    {
        if (pos)
        {
            m_position = *pos;
            JS_Sound_SetPosition(m_instance, pos->x, pos->y, pos->z);
        }

        if (vel)
            m_velocity = *vel;

        UpdateDopplerPitch();

        setVolume(m_volume);
        setFrequency(m_frequency);
        FMOD_RETURN(FMOD_OK);
    }

    FMOD_RESULT ChannelI::set3DConeOrientation(FMOD_VECTOR* pos)
    {
        FMOD_RETURN(FMOD_OK);
    }

    FMOD_RESULT ChannelI::set3DConeSettings(float insideconeangle , float outsideconeangle, float outsidevolume)
    {
        FMOD_RETURN(FMOD_OK);
    }

    FMOD_RESULT ChannelI::set3DCustomRolloff(FMOD_VECTOR * points , int numpoints)
    {
        FMOD_RETURN(FMOD_ERR_UNIMPLEMENTED);
    }

    FMOD_RESULT ChannelI::set3DDistanceFilter(bool custom , float customLevel, float centerFreq)
    {
        FMOD_RETURN(FMOD_ERR_UNIMPLEMENTED);
    }

    FMOD_RESULT ChannelI::set3DDopplerLevel(float dopplerlevel)
    {
        m_3Ddopplerlevel = dopplerlevel;
        FMOD_RETURN(FMOD_OK);
    }

    FMOD_RESULT ChannelI::set3DMinMaxDistance(float mindistance , float maxdistance)
    {
        FMOD_RETURN(FMOD_OK);
    }

    FMOD_RESULT ChannelI::set3DOcclusion(float directocclusion , float reverbocclusion)
    {
        FMOD_RETURN(FMOD_ERR_UNIMPLEMENTED);
    }

    FMOD_RESULT ChannelI::set3DPanLevel(float panlevel)
    {
        JS_Sound_Set3D(m_instance, panlevel > 0);
        m_panlevel = panlevel;
        FMOD_RETURN(FMOD_OK);
    }

    FMOD_RESULT ChannelI::set3DSpread(float spread)
    {
        FMOD_RETURN(FMOD_OK);
    }

    FMOD_RESULT ChannelI::setCallback(FMOD_CHANNEL_CALLBACK callback)
    {
        m_channelcallback = callback;
        FMOD_RETURN(FMOD_OK);
    }

    FMOD_RESULT ChannelI::setChannelGroup(ChannelGroup* channelgroup)
    {
        m_channelgroup = SystemI::Validate<ChannelGroup, ChannelGroupI>(channelgroup);
        FMOD_RETURN(FMOD_OK);
    }

    FMOD_RESULT ChannelI::setDelay(FMOD_DELAYTYPE delaytype , unsigned int delay_hi, unsigned int delay_lo)
    {
        UInt64 time = (UInt64)(((UInt64)delay_hi) << 32) + delay_lo;
        UInt64 curTime = FakeGetTimeInSamples();
        float delay = ((float)(time - curTime)) / FAKE_SAMPLERATE;
        if (curTime > time)
            delay = 0;
        switch (delaytype)
        {
            case FMOD_DELAYTYPE_DSPCLOCK_START:
            {
                if (m_starttimeinsamples < time)
                {
                    m_starttimeinsamples = time;
                    JS_Sound_Play(m_sound->m_instance, m_instance, 0, delay);
                    if (m_stoptimeinsamples > curTime)
                        JS_Sound_Stop(m_instance, ((float)(m_stoptimeinsamples - curTime)) / FAKE_SAMPLERATE);
                    JS_Sound_SetLoopPoints(m_instance, m_sound->m_loopstart, m_sound->m_loopend);
                }

                FMOD_RETURN(FMOD_OK);
            }

            case FMOD_DELAYTYPE_DSPCLOCK_END:
            {
                m_stoptimeinsamples = time;
                if (m_stoptimeinsamples != 0)
                    JS_Sound_Stop(m_instance, delay);
                FMOD_RETURN(FMOD_OK);
            }

            default:
                FMOD_RETURN(FMOD_ERR_UNIMPLEMENTED);
        }
    }

    FMOD_RESULT ChannelI::setFrequency(float frequency)
    {
        // zero frequency will cause a divide by zero, generating an "integer result unrepresentable" wasm exception later on.
        frequency = std::max(frequency, std::numeric_limits<float>::epsilon());
        if (frequency != m_frequency)
            UpdateStartPositionForPitchChange(frequency, m_dopplerpitch);

        FMOD_RETURN(FMOD_OK);
    }

    FMOD_RESULT ChannelI::setInputChannelMix(float * levels , int numlevels)
    {
        FMOD_RETURN(FMOD_ERR_UNIMPLEMENTED);
    }

    FMOD_RESULT ChannelI::setLoopCount(int loopcount)
    {
        FMOD_RETURN(FMOD_ERR_UNIMPLEMENTED);
    }

    FMOD_RESULT ChannelI::setLoopPoints(unsigned int loopstart , FMOD_TIMEUNIT loopstarttype, unsigned int loopend, FMOD_TIMEUNIT loopendtype)
    {
        FMOD_RETURN(FMOD_ERR_UNIMPLEMENTED);
    }

    FMOD_RESULT ChannelI::setLowPassGain(float gain)
    {
        FMOD_RETURN(FMOD_ERR_UNIMPLEMENTED);
    }

    FMOD_RESULT ChannelI::setMode(FMOD_MODE mode)
    {
        const FMOD_MODE loopmask = (FMOD_LOOP_NORMAL | FMOD_LOOP_OFF);
        if (mode & loopmask)
        {
            m_mode = (m_mode & ~loopmask) | (mode & loopmask);
            JS_Sound_SetLoop(m_instance, mode & FMOD_LOOP_NORMAL);
        }

        FMOD_RETURN(FMOD_OK);
    }

    FMOD_RESULT ChannelI::setMute(bool muted)
    {
        m_ismuted = muted;
        FMOD_RETURN(FMOD_OK);
    }

    FMOD_RESULT ChannelI::setPan(float pan)
    {
        FMOD_RETURN(FMOD_OK);
    }

    void ChannelI::UpdatePitch()
    {
        JS_Sound_SetPitch(m_instance, m_ispaused ? 0.0000001 : (m_frequency * m_dopplerpitch) / (float)FAKE_SAMPLERATE);
    }

    void ChannelI::UpdateDopplerPitch()
    {
        float dopplerpitch = 1.0f;
        float        dopplerscale, distancescale, rolloffscale;
        FMOD_VECTOR listenerPos, listenerVelo;
        FMOD_RESULT result = m_system->get3DListenerAttributes(0 , &listenerPos, &listenerVelo, NULL, NULL);
        if (result != FMOD_OK)
            return;

        result = m_system->get3DSettings(&dopplerscale, &distancescale, &rolloffscale);
        if (result != FMOD_OK)
            return;

        dopplerscale *= m_3Ddopplerlevel * m_panlevel;

    #define SPEED_OF_SOUND 340.0f

        if (dopplerscale > 0)
        {
            FMOD_VECTOR relativeVelocity, directionToSound;

            FMOD_Vector_Subtract(&m_velocity, &listenerVelo, &relativeVelocity);
            FMOD_Vector_Subtract(&m_position, &listenerPos, &directionToSound);

            float length = FMOD_Vector_GetLengthFast(&directionToSound);

            float velocityAwayFromListener = 0.0f;
            if (length > 0.0f)
                velocityAwayFromListener = FMOD_Vector_DotProduct(&relativeVelocity, &directionToSound) / length;

            dopplerpitch  = SPEED_OF_SOUND * distancescale;
            dopplerpitch -= (velocityAwayFromListener * dopplerscale);
            dopplerpitch /= (SPEED_OF_SOUND * distancescale);
        }

        if (dopplerpitch < .000001f)
            dopplerpitch = .000001f;

        if (dopplerpitch != m_dopplerpitch)
            UpdateStartPositionForPitchChange(m_frequency, dopplerpitch);
    }

    void ChannelI::UpdateStartPositionForPitchChange(float frequency, float dopplerpitch)
    {
        SInt64 t = m_ispaused ? m_pausetimeinsamples : FakeGetTimeInSamples();
        SInt64 scaleddelta = (t - m_starttimeinsamples) * ((m_frequency * m_dopplerpitch) / (float)FAKE_SAMPLERATE);
        m_frequency = frequency;
        m_dopplerpitch = dopplerpitch;
        m_starttimeinsamples = t - scaleddelta * (FAKE_SAMPLERATE / (float)(m_frequency * m_dopplerpitch));
        UpdatePitch();
    }

    FMOD_RESULT ChannelI::setPaused(bool paused)
    {
        if (m_ispaused != paused)
        {
            m_ispaused = paused;

            if (m_ispaused)
                m_pausetimeinsamples = FakeGetTimeInSamples();
            else
                m_starttimeinsamples += FakeGetTimeInSamples() - m_pausetimeinsamples;

            // This is a bad hack to fake pausing by setting the pitch to 0.0000001 (as zero is not allowed).
            // The Web Audio API has no built-in support for pausing (which seems to be one hell of an oversight.),
            // and other commonly suggested workarounds based on remembering start and pause times of sounds, and seeking
            // have other issues with looping and pitch, so I chose this simpler hack instead.

            UpdatePitch();
        }
        FMOD_RETURN(FMOD_OK);
    }

    FMOD_RESULT ChannelI::setPosition(unsigned int position , FMOD_TIMEUNIT timeunit)
    {
        switch (timeunit)
        {
            case FMOD_TIMEUNIT_PCM:
                break;
            case FMOD_TIMEUNIT_MS:
                position *= (float)FAKE_SAMPLERATE / 1000;
                break;
            default:
                FMOD_UNIMPLEMENTED();
                FMOD_RETURN(FMOD_ERR_INVALID_PARAM);
        }
        if (position > m_length)
            position = m_length;
        UInt64 curTime = FakeGetTimeInSamples();
        m_starttimeinsamples = curTime - position * (FAKE_SAMPLERATE / (float)m_frequency);
        if (m_ispaused)
            m_pausetimeinsamples = curTime;
        JS_Sound_Play(m_sound->m_instance, m_instance, position / (float)FAKE_SAMPLERATE, 0);
        if (m_stoptimeinsamples > curTime)
            JS_Sound_Stop(m_instance, ((float)(m_stoptimeinsamples - curTime)) / FAKE_SAMPLERATE);
        JS_Sound_SetLoopPoints(m_instance, m_sound->m_loopstart, m_sound->m_loopend);
        UpdatePitch();
        setMode(m_mode);
        FMOD_RETURN(FMOD_OK);
    }

    FMOD_RESULT ChannelI::setPriority(int priority)
    {
        FMOD_RETURN(FMOD_OK);
    }

    FMOD_RESULT ChannelI::setReverbProperties(const FMOD_REVERB_CHANNELPROPERTIES* props)
    {
        FMOD_RETURN(FMOD_OK);
    }

    FMOD_RESULT ChannelI::setSpeakerLevels(FMOD_SPEAKER speaker , float * levels, int numlevels)
    {
        FMOD_RETURN(FMOD_ERR_UNIMPLEMENTED);
    }

    FMOD_RESULT ChannelI::setSpeakerMix(float frontleft , float frontright, float center, float lfe, float backleft, float backright, float sideleft, float sideright)
    {
        FMOD_RETURN(FMOD_ERR_UNIMPLEMENTED);
    }

    FMOD_RESULT ChannelI::setUserData(void* userdata)
    {
        m_userdata = userdata;
        FMOD_RETURN(FMOD_OK);
    }

    FMOD_RESULT ChannelI::setVolume(float volume)
    {
        m_volume = volume;
        if (m_system->m_rolloffcallback)
        {
            float x = m_position.x - m_system->m_listenerposition.x;
            float y = m_position.y - m_system->m_listenerposition.y;
            float z = m_position.z - m_system->m_listenerposition.z;
            float distance = sqrt(x * x + y * y + z * z);
            float rolloff = m_system->m_rolloffcallback((FMOD_CHANNEL*)m_handle, distance);
            volume *= 1.0f + (rolloff - 1.0f) * m_panlevel;
        }
        ChannelGroupI* channelgroup = m_channelgroup;
        while (channelgroup)
        {
            float groupVolume;
            channelgroup->getVolume(&groupVolume);
            float groupAudibilityFactor;
            channelgroup->getAudibilityFactor(&groupAudibilityFactor);

            ChannelGroup* channelgroupHandle;
            channelgroup->getParentGroup(&channelgroupHandle);
            channelgroup = SystemI::Validate<ChannelGroup, ChannelGroupI>(channelgroupHandle);

            volume *= groupVolume * groupAudibilityFactor;
        }
        if (m_ismuted)
            volume = 0;
        JS_Sound_SetVolume(m_instance, volume);
        FMOD_RETURN(FMOD_OK);
    }

    FMOD_RESULT ChannelI::stop()
    {
        JS_Sound_Stop(m_instance, 0);
        EndCallback();
        FMOD_RETURN(FMOD_OK);
    }

    FMOD_RESULT DSPI::addInput(DSP * target , DSPConnection ** connection)
    {
        FMOD_RETURN(FMOD_ERR_UNIMPLEMENTED);
    }

    FMOD_RESULT DSPI::disconnectAll(bool inputs , bool outputs)
    {
        FMOD_RETURN(FMOD_OK);
    }

    FMOD_RESULT DSPI::disconnectFrom(DSP * target)
    {
        FMOD_RETURN(FMOD_ERR_UNIMPLEMENTED);
    }

    FMOD_RESULT DSPI::getActive(bool* active)
    {
        if (active != NULL)
            *active = false;
        FMOD_RETURN(FMOD_OK);
    }

    FMOD_RESULT DSPI::getBypass(bool* bypass)
    {
        if (bypass != NULL)
            *bypass = false;
        FMOD_RETURN(FMOD_OK);
    }

    FMOD_RESULT DSPI::getDefaults(float * frequency , float * volume, float * pan, int * priority)
    {
        FMOD_RETURN(FMOD_ERR_UNIMPLEMENTED);
    }

    FMOD_RESULT DSPI::getInfo(char * name , unsigned int * version, int * channels, int * configwidth, int * configheight)
    {
        FMOD_RETURN(FMOD_ERR_UNIMPLEMENTED);
    }

    FMOD_RESULT DSPI::getInput(int index , DSP ** input, DSPConnection ** inputconnection)
    {
        FMOD_RETURN(FMOD_ERR_UNIMPLEMENTED);
    }

    FMOD_RESULT DSPI::getMemoryInfo(unsigned int memorybits , unsigned int event_memorybits, unsigned int * memoryused, FMOD_MEMORY_USAGE_DETAILS * memoryused_details)
    {
        FMOD_RETURN(FMOD_ERR_UNIMPLEMENTED);
    }

    FMOD_RESULT DSPI::getNumInputs(int * numinputs)
    {
        FMOD_RETURN(FMOD_ERR_UNIMPLEMENTED);
    }

    FMOD_RESULT DSPI::getNumOutputs(int * numoutputs)
    {
        FMOD_RETURN(FMOD_ERR_UNIMPLEMENTED);
    }

    FMOD_RESULT DSPI::getNumParameters(int * numparams)
    {
        FMOD_RETURN(FMOD_ERR_UNIMPLEMENTED);
    }

    FMOD_RESULT DSPI::getOutput(int index , DSP ** output, DSPConnection ** outputconnection)
    {
        FMOD_RETURN(FMOD_ERR_UNIMPLEMENTED);
    }

    FMOD_RESULT DSPI::getParameter(int index , float* value, char *valuestr, int valuestrlen)
    {
        FMOD_RETURN(FMOD_OK);
    }

    FMOD_RESULT DSPI::getParameterInfo(int index , char * name, char * label, char * description, int descriptionlen, float * min, float * max)
    {
        FMOD_RETURN(FMOD_ERR_UNIMPLEMENTED);
    }

    FMOD_RESULT DSPI::getSpeakerActive(FMOD_SPEAKER speaker , bool * active)
    {
        FMOD_RETURN(FMOD_ERR_UNIMPLEMENTED);
    }

    FMOD_RESULT DSPI::getSystemObject(System ** system)
    {
        FMOD_RETURN(FMOD_ERR_UNIMPLEMENTED);
    }

    FMOD_RESULT DSPI::getType(FMOD_DSP_TYPE * type)
    {
        FMOD_RETURN(FMOD_ERR_UNIMPLEMENTED);
    }

    FMOD_RESULT DSPI::getUserData(void** userdata)
    {
        if (userdata != NULL)
            *userdata = m_userdata;
        FMOD_RETURN(FMOD_OK);
    }

    FMOD_RESULT DSPI::release()
    {
        m_system->Release<DSP, DSPI>(this);
        FMOD_RETURN(FMOD_OK);
    }

    FMOD_RESULT DSPI::remove()
    {
        FMOD_RETURN(FMOD_OK);
    }

    FMOD_RESULT DSPI::reset()
    {
        FMOD_RETURN(FMOD_ERR_UNIMPLEMENTED);
    }

    FMOD_RESULT DSPI::setActive(bool active)
    {
        FMOD_RETURN(FMOD_OK);
    }

    FMOD_RESULT DSPI::setBypass(bool bypass)
    {
        FMOD_RETURN(FMOD_OK);
    }

    FMOD_RESULT DSPI::setDefaults(float frequency , float volume, float pan, int priority)
    {
        FMOD_RETURN(FMOD_ERR_UNIMPLEMENTED);
    }

    FMOD_RESULT DSPI::setParameter(int index , float value)
    {
        FMOD_RETURN(FMOD_OK);
    }

    FMOD_RESULT DSPI::setSpeakerActive(FMOD_SPEAKER speaker , bool active)
    {
        FMOD_RETURN(FMOD_ERR_UNIMPLEMENTED);
    }

    FMOD_RESULT DSPI::setUserData(void * userdata)
    {
        FMOD_RETURN(FMOD_ERR_UNIMPLEMENTED);
    }

    FMOD_RESULT DSPI::showConfigDialog(void * hwnd , bool show)
    {
        FMOD_RETURN(FMOD_ERR_UNIMPLEMENTED);
    }

    FMOD_RESULT DSPConnection::setMix(float volume)
    {
        FMOD_RETURN(FMOD_ERR_UNIMPLEMENTED);
    }

    FMOD_RESULT ReverbI::get3DAttributes(FMOD_VECTOR * position , float * mindistance, float * maxdistance)
    {
        FMOD_RETURN(FMOD_ERR_UNIMPLEMENTED);
    }

    FMOD_RESULT ReverbI::getActive(bool * active)
    {
        FMOD_RETURN(FMOD_ERR_UNIMPLEMENTED);
    }

    FMOD_RESULT ReverbI::getMemoryInfo(unsigned int memorybits , unsigned int event_memorybits, unsigned int * memoryused, FMOD_MEMORY_USAGE_DETAILS * memoryused_details)
    {
        FMOD_RETURN(FMOD_ERR_UNIMPLEMENTED);
    }

    FMOD_RESULT ReverbI::getProperties(FMOD_REVERB_PROPERTIES * properties)
    {
        FMOD_RETURN(FMOD_ERR_UNIMPLEMENTED);
    }

    FMOD_RESULT ReverbI::getUserData(void ** userdata)
    {
        FMOD_RETURN(FMOD_ERR_UNIMPLEMENTED);
    }

    FMOD_RESULT ReverbI::release()
    {
        m_system->Release<Reverb, ReverbI>(this);
        FMOD_RETURN(FMOD_OK);
    }

    FMOD_RESULT ReverbI::set3DAttributes(const FMOD_VECTOR* pos , float mindistance, float maxdistance)
    {
        FMOD_RETURN(FMOD_OK);
    }

    FMOD_RESULT ReverbI::setActive(bool active)
    {
        FMOD_RETURN(FMOD_OK);
    }

    FMOD_RESULT ReverbI::setProperties(const FMOD_REVERB_PROPERTIES* prop)
    {
        FMOD_RETURN(FMOD_OK);
    }

    FMOD_RESULT ReverbI::setUserData(void * userdata)
    {
        FMOD_RETURN(FMOD_ERR_UNIMPLEMENTED);
    }

    FMOD_RESULT SoundI::addSyncPoint(unsigned int offset , FMOD_TIMEUNIT offsettype, const char * name, FMOD_SYNCPOINT ** point)
    {
        FMOD_RETURN(FMOD_ERR_UNIMPLEMENTED);
    }

    FMOD_RESULT SoundI::deleteSyncPoint(FMOD_SYNCPOINT * point)
    {
        FMOD_RETURN(FMOD_ERR_UNIMPLEMENTED);
    }

    FMOD_RESULT SoundI::get3DConeSettings(float * insideconeangle , float * outsideconeangle, float * outsidevolume)
    {
        FMOD_RETURN(FMOD_ERR_UNIMPLEMENTED);
    }

    FMOD_RESULT SoundI::get3DCustomRolloff(FMOD_VECTOR ** points , int * numpoints)
    {
        FMOD_RETURN(FMOD_ERR_UNIMPLEMENTED);
    }

    FMOD_RESULT SoundI::get3DMinMaxDistance(float * min , float * max)
    {
        FMOD_RETURN(FMOD_ERR_UNIMPLEMENTED);
    }

    FMOD_RESULT SoundI::getDefaults(float* frequency , float* volume, float* pan, int* priority)
    {
        if (frequency != NULL)
            *frequency = FAKE_SAMPLERATE;
        if (volume != NULL)
            *volume = 1.0f;
        if (pan != NULL)
            *pan = 1.0f;
        if (priority != NULL)
            *priority = 128;
        FMOD_RETURN(FMOD_OK);
    }

    FMOD_RESULT SoundI::getFormat(FMOD_SOUND_TYPE* type , FMOD_SOUND_FORMAT* format, int* channels, int* bits)
    {
        if (type != NULL)
            *type = FMOD_SOUND_TYPE_UNKNOWN;
        if (format != NULL)
            *format = m_format;
        if (channels != NULL)
            *channels = m_numchannels;
        if (bits != NULL)
            *bits = 32;
        FMOD_RETURN(FMOD_OK);
    }

    FMOD_RESULT SoundI::getLength(unsigned int* length , FMOD_TIMEUNIT timeunit)
    {
        if (length == NULL)
            FMOD_RETURN(FMOD_ERR_INVALID_PARAM);

        UInt64 soundlength = JS_Sound_GetLength(m_instance);

        switch (timeunit)
        {
            case FMOD_TIMEUNIT_PCM:
                *length = soundlength;
                FMOD_RETURN(FMOD_OK);
            case FMOD_TIMEUNIT_MS:
                *length = (unsigned int)(soundlength * 1000.0 / (double)FAKE_SAMPLERATE);
                FMOD_RETURN(FMOD_OK);
            case FMOD_TIMEUNIT_PCMBYTES:
                *length = (unsigned int)(soundlength * 4);
                FMOD_RETURN(FMOD_OK);
            case FMOD_TIMEUNIT_RAWBYTES:
                *length = (unsigned int)(soundlength * 4);
                FMOD_RETURN(FMOD_OK);
            default:
                FMOD_UNIMPLEMENTED();
                FMOD_RETURN(FMOD_ERR_INVALID_PARAM);
        }
        FMOD_RETURN(FMOD_OK);
    }

    FMOD_RESULT SoundI::getLoopCount(int * loopcount)
    {
        FMOD_RETURN(FMOD_ERR_UNIMPLEMENTED);
    }

    FMOD_RESULT SoundI::getLoopPoints(unsigned int * loopstart , FMOD_TIMEUNIT loopstarttype, unsigned int * loopend, FMOD_TIMEUNIT loopendtype)
    {
        FMOD_RETURN(FMOD_ERR_UNIMPLEMENTED);
    }

    FMOD_RESULT SoundI::getMemoryInfo(unsigned int memorybits , unsigned int event_memorybits, unsigned int* memoryused, FMOD_MEMORY_USAGE_DETAILS* memoryused_details)
    {
        if (memoryused != NULL)
            *memoryused = 0;
        FMOD_RETURN(FMOD_OK);
    }

    FMOD_RESULT SoundI::getMode(FMOD_MODE* mode)
    {
        if (mode != NULL)
            *mode = 0;
        FMOD_RETURN(FMOD_OK);
    }

    FMOD_RESULT SoundI::getMusicChannelVolume(int channel , float * volume)
    {
        FMOD_RETURN(FMOD_ERR_UNIMPLEMENTED);
    }

    FMOD_RESULT SoundI::getMusicNumChannels(int* numchannels)
    {
        if (numchannels != NULL)
            *numchannels = 2;
        FMOD_RETURN(FMOD_OK);
    }

    FMOD_RESULT SoundI::getMusicSpeed(float * speed)
    {
        FMOD_RETURN(FMOD_ERR_UNIMPLEMENTED);
    }

    FMOD_RESULT SoundI::getName(char * name , int namelen)
    {
        FMOD_RETURN(FMOD_ERR_UNIMPLEMENTED);
    }

    FMOD_RESULT SoundI::getNumSubSounds(int * numsubsounds)
    {
        if (numsubsounds != NULL)
            *numsubsounds = 0;
        FMOD_RETURN(FMOD_OK);
    }

    FMOD_RESULT SoundI::getNumSyncPoints(int * numsyncpoints)
    {
        FMOD_RETURN(FMOD_ERR_UNIMPLEMENTED);
    }

    FMOD_RESULT SoundI::getNumTags(int * numtags , int * numtagsupdated)
    {
        FMOD_RETURN(FMOD_ERR_UNIMPLEMENTED);
    }

    FMOD_RESULT SoundI::getOpenState(FMOD_OPENSTATE * openstate , unsigned int * percentbuffered, bool * starving, bool * diskbusy)
    {
        if (openstate)
            *openstate = (FMOD_OPENSTATE)JS_Sound_GetLoadState(m_instance);
        if (percentbuffered)
            *percentbuffered = 100;
        if (starving)
            *starving = false;
        if (diskbusy)
            *diskbusy = false;

        FMOD_RETURN(FMOD_OK);
    }

    FMOD_RESULT SoundI::getSoundGroup(SoundGroup ** soundgroup)
    {
        FMOD_RETURN(FMOD_ERR_UNIMPLEMENTED);
    }

    FMOD_RESULT SoundI::getSubSound(int index , Sound ** subsound)
    {
        FMOD_RETURN(FMOD_ERR_UNIMPLEMENTED);
    }

    FMOD_RESULT SoundI::getSyncPoint(int index , FMOD_SYNCPOINT ** point)
    {
        FMOD_RETURN(FMOD_ERR_UNIMPLEMENTED);
    }

    FMOD_RESULT SoundI::getSyncPointInfo(FMOD_SYNCPOINT * point , char * name, int namelen, unsigned int * offset, FMOD_TIMEUNIT offsettype)
    {
        FMOD_RETURN(FMOD_ERR_UNIMPLEMENTED);
    }

    FMOD_RESULT SoundI::getSystemObject(System ** system)
    {
        FMOD_RETURN(FMOD_ERR_UNIMPLEMENTED);
    }

    FMOD_RESULT SoundI::getTag(const char * name , int index, FMOD_TAG * tag)
    {
        FMOD_RETURN(FMOD_ERR_UNIMPLEMENTED);
    }

    FMOD_RESULT SoundI::getUserData(void** userdata)
    {
        if (userdata != NULL)
            *userdata = m_userdata;
        FMOD_RETURN(FMOD_OK);
    }

    FMOD_RESULT SoundI::getVariations(float * frequencyvar , float * volumevar, float * panvar)
    {
        FMOD_RETURN(FMOD_ERR_UNIMPLEMENTED);
    }

    FMOD_RESULT SoundI::lock(unsigned int offset , unsigned int length, void** ptr1, void** ptr2, unsigned int* len1, unsigned int* len2)
    {
        // We don't really support SoundI::lock.
        // This function only allocates memory which can then be loaded into the sound by SoundI::unlock
        // to make AudioClip.SetData work. Other use cases are not supported.

        if (ptr1 != NULL)
            *ptr1 = UNITY_MALLOC(kMemTempAlloc, length);
        if (len1 != NULL)
            *len1 = length;
        if (ptr2 != NULL)
            *ptr2 = NULL;
        if (len2 != NULL)
            *len2 = 0;

        FMOD_RETURN(FMOD_OK);
    }

    FMOD_RESULT SoundI::readData(void* data , unsigned int lenbytes, unsigned int* read)
    {
        memset(data, 0, lenbytes);
        FMOD_RETURN(FMOD_OK);
    }

    FMOD_RESULT SoundI::release()
    {
        StopAllChannels();
        if (m_instance)
            JS_Sound_ReleaseInstance(m_instance);
        m_instance = 0;
        m_system->Release<Sound, SoundI>(this);
        FMOD_RETURN(FMOD_OK);
    }

    FMOD_RESULT SoundI::seekData(unsigned int pos)
    {
        FMOD_RETURN(FMOD_OK);
    }

    FMOD_RESULT SoundI::set3DConeSettings(float insideconeangle , float outsideconeangle, float outsidevolume)
    {
        FMOD_RETURN(FMOD_ERR_UNIMPLEMENTED);
    }

    FMOD_RESULT SoundI::set3DCustomRolloff(FMOD_VECTOR * points , int numpoints)
    {
        FMOD_RETURN(FMOD_ERR_UNIMPLEMENTED);
    }

    FMOD_RESULT SoundI::set3DMinMaxDistance(float min , float max)
    {
        FMOD_RETURN(FMOD_ERR_UNIMPLEMENTED);
    }

    FMOD_RESULT SoundI::setDefaults(float frequency , float volume, float pan, int priority)
    {
        FMOD_RETURN(FMOD_ERR_UNIMPLEMENTED);
    }

    FMOD_RESULT SoundI::setLoopCount(int loopcount)
    {
        FMOD_RETURN(FMOD_ERR_UNIMPLEMENTED);
    }

    FMOD_RESULT SoundI::setLoopPoints(unsigned int loopstart , FMOD_TIMEUNIT loopstarttype, unsigned int loopend, FMOD_TIMEUNIT loopendtype)
    {
        switch (loopstarttype)
        {
            case FMOD_TIMEUNIT_PCM:
                m_loopstart = loopstart / (float)FAKE_SAMPLERATE;
                break;
            case FMOD_TIMEUNIT_MS:
                m_loopstart = loopstart * 0.001;
                break;
            default:
                FMOD_UNIMPLEMENTED();
                FMOD_RETURN(FMOD_ERR_INVALID_PARAM);
        }
        switch (loopendtype)
        {
            case FMOD_TIMEUNIT_PCM:
                m_loopend = loopend / (float)FAKE_SAMPLERATE;
                break;
            case FMOD_TIMEUNIT_MS:
                m_loopend = loopend * 0.001;
                break;
            default:
                FMOD_UNIMPLEMENTED();
                FMOD_RETURN(FMOD_ERR_INVALID_PARAM);
        }
        FMOD_RETURN(FMOD_OK);
    }

    FMOD_RESULT SoundI::setMode(FMOD_MODE mode)
    {
        m_mode = mode;
        FMOD_RETURN(FMOD_OK);
    }

    FMOD_RESULT SoundI::setMusicChannelVolume(int channel , float volume)
    {
        FMOD_RETURN(FMOD_ERR_UNIMPLEMENTED);
    }

    FMOD_RESULT SoundI::setMusicSpeed(float speed)
    {
        FMOD_RETURN(FMOD_ERR_UNIMPLEMENTED);
    }

    FMOD_RESULT SoundI::setSoundGroup(SoundGroup * soundgroup)
    {
        FMOD_RETURN(FMOD_ERR_UNIMPLEMENTED);
    }

    FMOD_RESULT SoundI::setSubSound(int index , Sound * subsound)
    {
        FMOD_RETURN(FMOD_ERR_UNIMPLEMENTED);
    }

    FMOD_RESULT SoundI::setSubSoundSentence(int * subsoundlist , int numsubsounds)
    {
        FMOD_RETURN(FMOD_ERR_UNIMPLEMENTED);
    }

    FMOD_RESULT SoundI::setUserData(void* userdata)
    {
        m_userdata = userdata;
        FMOD_RETURN(FMOD_OK);
    }

    FMOD_RESULT SoundI::setVariations(float frequencyvar , float volumevar, float panvar)
    {
        FMOD_RETURN(FMOD_ERR_UNIMPLEMENTED);
    }

    FMOD_RESULT SoundI::unlock(void* ptr1 , void* ptr2, unsigned int len1, unsigned int len2)
    {
        // We don't really support SoundI::unlock.
        // This function only supports loading the complete sound from memory allocated by SoundI::lock
        // to make AudioClip.SetData work. Other use cases are not supported.
        if (ptr2 != NULL || len2 != 0)
            FMOD_RETURN(FMOD_ERR_INVALID_PARAM);

        float* deinterlaced = (float*)UNITY_MALLOC(kMemTempAlloc, len1);
        size_t channelSize = len1 / (m_numchannels * sizeof(float));
        for (int channel = 0; channel < m_numchannels; channel++)
        {
            for (int i = 0; i < channelSize; i++)
                deinterlaced[i + channel * channelSize] = ((float*)ptr1)[i * m_numchannels + channel];
        }

        if (m_instance)
            JS_Sound_ReleaseInstance(m_instance);

        m_instance = JS_Sound_Load_PCM(m_numchannels, channelSize, m_frequency, deinterlaced);

        UNITY_FREE(kMemTempAlloc, deinterlaced);
        UNITY_FREE(kMemTempAlloc, ptr1);

        FMOD_RETURN(FMOD_OK);
    }

    FMOD_RESULT SystemI::addDSP(DSP * dsp , DSPConnection ** connection)
    {
        FMOD_RETURN(FMOD_ERR_UNIMPLEMENTED);
    }

    FMOD_RESULT SystemI::attachFileSystem(FMOD_FILE_OPENCALLBACK useropen , FMOD_FILE_CLOSECALLBACK userclose, FMOD_FILE_READCALLBACK userread, FMOD_FILE_SEEKCALLBACK userseek)
    {
        FMOD_RETURN(FMOD_ERR_UNIMPLEMENTED);
    }

    FMOD_RESULT SystemI::close()
    {
        FMOD_RETURN(FMOD_OK);
    }

    FMOD_RESULT SystemI::createChannelGroup(const char* name , ChannelGroup** handle)
    {
        assert(handle != NULL);
        ChannelGroupI* channelgroupi = m_channelgroups.GetFree();
        if (channelgroupi == NULL)
            FMOD_RETURN(FMOD_ERR_INVALID_HANDLE);
        *handle = channelgroupi->m_handle;
        FMOD_RETURN(FMOD_OK);
    }

    FMOD_RESULT SystemI::createDSP(FMOD_DSP_DESCRIPTION* desc , DSP** dsp)
    {
        DSPI* dspi = m_dsps.GetFree();
        if (dspi == NULL)
            FMOD_RETURN(FMOD_ERR_INVALID_PARAM);
        if (dsp != NULL)
            *dsp = dspi->m_handle;
        FMOD_RETURN(FMOD_OK);
    }

    FMOD_RESULT SystemI::createDSPByPlugin(unsigned int handle , DSP ** dsp)
    {
        FMOD_RETURN(FMOD_ERR_UNIMPLEMENTED);
    }

    FMOD_RESULT SystemI::createDSPByType(FMOD_DSP_TYPE type , DSP** dsp)
    {
        DSPI* dspi = m_dsps.GetFree();
        if (dspi == NULL)
            FMOD_RETURN(FMOD_ERR_INVALID_PARAM);
        if (dsp != NULL)
            *dsp = dspi->m_handle;
        FMOD_RETURN(FMOD_OK);
    }

    FMOD_RESULT SystemI::createGeometry(int maxpolygons , int maxvertices, Geometry ** geometry)
    {
        FMOD_RETURN(FMOD_ERR_UNIMPLEMENTED);
    }

    FMOD_RESULT SystemI::createReverb(Reverb** handle)
    {
        assert(handle != NULL);
        ReverbI* reverbi = m_reverbs.GetFree();
        if (reverbi == NULL)
            FMOD_RETURN(FMOD_ERR_INVALID_HANDLE);
        *handle = reverbi->m_handle;
        FMOD_RETURN(FMOD_OK);
    }

    FMOD_RESULT SystemI::createSound(const char* name , FMOD_MODE mode, FMOD_CREATESOUNDEXINFO* exinfo, Sound** handle)
    {
        SoundI* soundi = m_sounds.GetFree();
        if (soundi == NULL)
            FMOD_RETURN(FMOD_ERR_INVALID_HANDLE);
        if (handle == NULL)
            FMOD_RETURN(FMOD_ERR_INVALID_PARAM);
        *handle = soundi->m_handle;
        soundi->m_userdata = exinfo->userdata;
        if (mode & FMOD_OPENMEMORY)
        {
            soundi->m_instance = JS_Sound_Load((void*)name, exinfo->length);
            strcpy(soundi->m_name, "");
        }
        else if (mode & FMOD_CREATESAMPLE && exinfo->pcmreadcallback)
        {
            float* data = (float*)UNITY_MALLOC(kMemTempAlloc, exinfo->length);
            size_t bufferSize = 16384;
            for (int i = 0; i < exinfo->length; i += bufferSize)
            {
                bufferSize = std::min<size_t>(bufferSize, exinfo->length - i);
                exinfo->pcmreadcallback((FMOD_SOUND*)*handle, (char*)data + i, bufferSize);
                exinfo->pcmsetposcallback((FMOD_SOUND*)*handle, 0, (i + bufferSize) / (exinfo->numchannels * sizeof(float)), FMOD_TIMEUNIT_PCM);
            }

            float* deinterlaced = (float*)UNITY_MALLOC(kMemTempAlloc, exinfo->length);
            size_t channelSize = exinfo->length / (exinfo->numchannels * sizeof(float));
            for (int channel = 0; channel < exinfo->numchannels; channel++)
            {
                for (int i = 0; i < channelSize; i++)
                    deinterlaced[i + channel * channelSize] = data[i * exinfo->numchannels + channel];
            }
            soundi->m_numchannels = exinfo->numchannels;
            soundi->m_frequency = exinfo->defaultfrequency;
            soundi->m_instance = JS_Sound_Load_PCM(exinfo->numchannels, channelSize, exinfo->defaultfrequency, deinterlaced);
            soundi->m_format = FMOD_SOUND_FORMAT_PCMFLOAT;

            UNITY_FREE(kMemTempAlloc, data);
            UNITY_FREE(kMemTempAlloc, deinterlaced);
            strcpy(soundi->m_name, name);
        }
        else
        {
            void *handle;
            void *userdata;
            unsigned int filesize;
            FMOD_RESULT res;

            if (exinfo->useropen)
                res = exinfo->useropen(name, 0, &filesize, &handle, &userdata);
            else
                res = m_useropen(name, 0, &filesize, &handle, &userdata);
            if (res != FMOD_OK)
                FMOD_RETURN(res);

            if (exinfo->length)
                filesize = exinfo->length;

            if (exinfo->userseek)
                res = exinfo->userseek(handle, exinfo->fileoffset, userdata);
            else
                res = m_userseek(handle, exinfo->fileoffset, userdata);
            if (res != FMOD_OK)
                FMOD_RETURN(res);

            UInt8* buffer = (UInt8*)UNITY_MALLOC(kMemTempAlloc, filesize);

            unsigned int pos = 0;
            while (pos < filesize)
            {
                unsigned int read;
                if (exinfo->userread)
                    res = exinfo->userread(handle, buffer + pos, filesize - pos, &read, userdata);
                else
                    res = m_userread(handle, buffer + pos, filesize - pos, &read, userdata);
                if (res != FMOD_OK)
                    FMOD_RETURN(res);
                pos += read;
            }

            if (exinfo->userclose)
                res = exinfo->userclose(handle, userdata);
            else
                res = m_userclose(handle, userdata);
            if (res != FMOD_OK)
                FMOD_RETURN(res);

            soundi->m_instance = JS_Sound_Load((void*)buffer, filesize);

            UNITY_FREE(kMemTempAlloc, buffer);
            strcpy(soundi->m_name, name);
        }
        FMOD_RETURN(FMOD_OK);
    }

    FMOD_RESULT SystemI::createSoundGroup(const char * name , SoundGroup ** soundgroup)
    {
        FMOD_RETURN(FMOD_ERR_UNIMPLEMENTED);
    }

    FMOD_RESULT SystemI::createStream(const char * name_or_data , FMOD_MODE mode, FMOD_CREATESOUNDEXINFO * exinfo, Sound ** sound)
    {
        FMOD_RETURN(FMOD_ERR_UNIMPLEMENTED);
    }

    FMOD_RESULT SystemI::get3DListenerAttributes(int listener , FMOD_VECTOR * pos, FMOD_VECTOR * vel, FMOD_VECTOR * forward, FMOD_VECTOR * up)
    {
        assert(listener == 0); // we only support one listener in fakemod for now.

        if (pos)
            *pos = m_listenerposition;

        if (vel)
            *vel = m_listenervelocity;

        if (forward)
            *forward = m_listenerforward;

        if (up)
            *up = m_listenerup;

        FMOD_RETURN(FMOD_OK);
    }

    FMOD_RESULT SystemI::get3DNumListeners(int * numlisteners)
    {
        FMOD_RETURN(FMOD_ERR_UNIMPLEMENTED);
    }

    FMOD_RESULT SystemI::get3DSettings(float * dopplerscale , float * distancefactor, float * rolloffscale)
    {
        if (dopplerscale)
            *dopplerscale = m_dopplerscale;

        if (distancefactor)
            *distancefactor = m_distancefactor;

        if (rolloffscale)
            *rolloffscale = m_rolloffscale;

        FMOD_RETURN(FMOD_OK);
    }

    FMOD_RESULT SystemI::get3DSpeakerPosition(FMOD_SPEAKER speaker , float * x, float * y, bool * active)
    {
        FMOD_RETURN(FMOD_ERR_UNIMPLEMENTED);
    }

    FMOD_RESULT SystemI::getAdvancedSettings(FMOD_ADVANCEDSETTINGS * settings)
    {
        FMOD_RETURN(FMOD_ERR_UNIMPLEMENTED);
    }

    FMOD_RESULT SystemI::getCDROMDriveName(int drive , char * drivename, int drivenamelen, char * scsiname, int scsinamelen, char * devicename, int devicenamelen)
    {
        FMOD_RETURN(FMOD_ERR_UNIMPLEMENTED);
    }

    FMOD_RESULT SystemI::getChannel(int channelid , Channel ** channel)
    {
        FMOD_RETURN(FMOD_ERR_UNIMPLEMENTED);
    }

    FMOD_RESULT SystemI::getChannelsPlaying(int* numchannels)
    {
        if (numchannels != NULL)
            *numchannels = m_numrealchannels;
        FMOD_RETURN(FMOD_OK);
    }

    FMOD_RESULT SystemI::getCPUUsage(float* dsp , float* stream, float* geometry, float* update, float* total)
    {
        if (dsp != NULL)
            *dsp = 60.0f;
        if (stream != NULL)
            *stream = 30.0f;
        if (geometry != NULL)
            *geometry = 0.0f;
        if (update != NULL)
            *update = 10.0f;
        FMOD_RETURN(FMOD_OK);
    }

    FMOD_RESULT SystemI::getDriver(int* driver)
    {
        if (driver != NULL)
            *driver = 0;
        FMOD_RETURN(FMOD_OK);
    }

    FMOD_RESULT SystemI::getDriverCaps(int id , FMOD_CAPS* caps, int* controlpaneloutputrate, FMOD_SPEAKERMODE* controlpanelspeakermode)
    {
        if (controlpanelspeakermode != NULL)
            *controlpanelspeakermode = FMOD_SPEAKERMODE_STEREO;
        FMOD_RETURN(FMOD_OK);
    }

    FMOD_RESULT SystemI::getDriverInfo(int id , char * name, int namelen, FMOD_GUID * guid)
    {
        FMOD_RETURN(FMOD_ERR_UNIMPLEMENTED);
    }

    FMOD_RESULT SystemI::getDriverInfoW(int id , short * name, int namelen, FMOD_GUID * guid)
    {
        FMOD_RETURN(FMOD_ERR_UNIMPLEMENTED);
    }

    FMOD_RESULT SystemI::getDSPBufferSize(unsigned int* bufferlength , int* numbuffers)
    {
        if (bufferlength != NULL)
            *bufferlength = m_dspbufferlength;
        if (numbuffers != NULL)
            *numbuffers = m_dspnumbuffers;
        FMOD_RETURN(FMOD_OK);
    }

    FMOD_RESULT SystemI::getDSPClock(unsigned int* clock_hi , unsigned int* clock_lo)
    {
        UInt64 t = FakeGetTimeInSamples();
        if (clock_hi != NULL)
            *clock_hi = (unsigned int)(t >> 32);
        if (clock_lo != NULL)
            *clock_lo = (unsigned int)(t & 0xFFFFFFFF);
        FMOD_RETURN(FMOD_OK);
    }

    FMOD_RESULT SystemI::getDSPHead(DSP ** dsp)
    {
        FMOD_RETURN(FMOD_ERR_UNIMPLEMENTED);
    }

    FMOD_RESULT SystemI::getGeometryOcclusion(const FMOD_VECTOR * listener , const FMOD_VECTOR * source, float * direct, float * reverb)
    {
        FMOD_RETURN(FMOD_ERR_UNIMPLEMENTED);
    }

    FMOD_RESULT SystemI::getGeometrySettings(float * maxworldsize)
    {
        FMOD_RETURN(FMOD_ERR_UNIMPLEMENTED);
    }

    FMOD_RESULT SystemI::getHardwareChannels(int * numhardwarechannels)
    {
        FMOD_RETURN(FMOD_ERR_UNIMPLEMENTED);
    }

    FMOD_RESULT SystemI::getMasterChannelGroup(ChannelGroup** channelgroup)
    {
        assert(channelgroup != NULL);
        assert(m_masterchannelgroup != NULL);
        *channelgroup = m_masterchannelgroup->m_handle;
        FMOD_RETURN(FMOD_OK);
    }

    FMOD_RESULT SystemI::getMasterSoundGroup(SoundGroup ** soundgroup)
    {
        FMOD_RETURN(FMOD_ERR_UNIMPLEMENTED);
    }

    FMOD_RESULT SystemI::getMemoryInfo(unsigned int memorybits , unsigned int event_memorybits, unsigned int* memoryused, FMOD_MEMORY_USAGE_DETAILS* memoryused_details)
    {
        if (memoryused != NULL)
            *memoryused = 100000;
        FMOD_RETURN(FMOD_OK);
    }

    FMOD_RESULT SystemI::getNetworkProxy(char * proxy , int proxylen)
    {
        FMOD_RETURN(FMOD_ERR_UNIMPLEMENTED);
    }

    FMOD_RESULT SystemI::getNetworkTimeout(int * timeout)
    {
        FMOD_RETURN(FMOD_ERR_UNIMPLEMENTED);
    }

    FMOD_RESULT SystemI::getNumCDROMDrives(int * numdrives)
    {
        FMOD_RETURN(FMOD_ERR_UNIMPLEMENTED);
    }

    FMOD_RESULT SystemI::getNumDrivers(int* numdrivers)
    {
        if (numdrivers != NULL)
            *numdrivers = 1;
        FMOD_RETURN(FMOD_OK);
    }

    FMOD_RESULT SystemI::getNumPlugins(FMOD_PLUGINTYPE plugintype , int * numplugins)
    {
        FMOD_RETURN(FMOD_ERR_UNIMPLEMENTED);
    }

    FMOD_RESULT SystemI::getOutput(FMOD_OUTPUTTYPE * output)
    {
        FMOD_RETURN(FMOD_ERR_UNIMPLEMENTED);
    }

    FMOD_RESULT SystemI::getOutputByPlugin(unsigned int * handle)
    {
        FMOD_RETURN(FMOD_ERR_UNIMPLEMENTED);
    }

    FMOD_RESULT SystemI::getOutputHandle(void ** handle)
    {
        FMOD_RETURN(FMOD_ERR_UNIMPLEMENTED);
    }

    FMOD_RESULT SystemI::getPluginHandle(FMOD_PLUGINTYPE plugintype , int index, unsigned int * handle)
    {
        FMOD_RETURN(FMOD_ERR_UNIMPLEMENTED);
    }

    FMOD_RESULT SystemI::getPluginInfo(unsigned int handle , FMOD_PLUGINTYPE * plugintype, char * name, int namelen, unsigned int * version)
    {
        FMOD_RETURN(FMOD_ERR_UNIMPLEMENTED);
    }

    FMOD_RESULT SystemI::getRecordDriverCaps(int id , FMOD_CAPS* caps, int* minfrequency, int* maxfrequency)
    {
        if (minfrequency != NULL)
            *minfrequency = 44100;
        if (maxfrequency != NULL)
            *maxfrequency = 48000;
        FMOD_RETURN(FMOD_OK);
    }

    FMOD_RESULT SystemI::getRecordDriverInfo(int id , char* name, int namelen, FMOD_GUID* guid)
    {
        strcpy(name, "record");
        memset(guid, 0, sizeof(FMOD_GUID));
        FMOD_RETURN(FMOD_OK);
    }

    FMOD_RESULT SystemI::getRecordDriverInfoW(int id , short * name, int namelen, FMOD_GUID * guid)
    {
        FMOD_RETURN(FMOD_ERR_UNIMPLEMENTED);
    }

    FMOD_RESULT SystemI::getRecordNumDrivers(int* numdrivers)
    {
        if (numdrivers != NULL)
            *numdrivers = 1;
        FMOD_RETURN(FMOD_OK);
    }

    FMOD_RESULT SystemI::getRecordPosition(int id , unsigned int* position)
    {
        if (position != NULL)
            *position = 0;
        FMOD_RETURN(FMOD_OK);
    }

    FMOD_RESULT SystemI::getReverbAmbientProperties(FMOD_REVERB_PROPERTIES * prop)
    {
        FMOD_RETURN(FMOD_ERR_UNIMPLEMENTED);
    }

    FMOD_RESULT SystemI::getReverbProperties(FMOD_REVERB_PROPERTIES * prop)
    {
        FMOD_RETURN(FMOD_ERR_UNIMPLEMENTED);
    }

    FMOD_RESULT SystemI::getSoftwareChannels(int * numsoftwarechannels)
    {
        FMOD_RETURN(FMOD_ERR_UNIMPLEMENTED);
    }

    FMOD_RESULT SystemI::getSoftwareFormat(int* samplerate , FMOD_SOUND_FORMAT* format, int* numoutputchannels, int* maxinputchannels, FMOD_DSP_RESAMPLER* resamplemethod, int* bits)
    {
        if (samplerate != NULL)
            *samplerate = FAKE_SAMPLERATE;
        if (numoutputchannels != NULL)
            *numoutputchannels = 2;
        if (maxinputchannels != NULL)
            *maxinputchannels = 2;
        FMOD_RETURN(FMOD_OK);
    }

    FMOD_RESULT SystemI::getSoundRAM(int * currentalloced , int * maxalloced, int * total)
    {
        FMOD_RETURN(FMOD_ERR_UNIMPLEMENTED);
    }

    FMOD_RESULT SystemI::getSpeakerMode(FMOD_SPEAKERMODE * speakermode)
    {
        if (speakermode != NULL)
            *speakermode = FMOD_SPEAKERMODE_STEREO;
        FMOD_RETURN(FMOD_OK);
    }

    FMOD_RESULT SystemI::getSpectrum(float * spectrumarray , int numvalues, int channeloffset, FMOD_DSP_FFT_WINDOW windowtype)
    {
        FMOD_RETURN(FMOD_ERR_UNIMPLEMENTED);
    }

    FMOD_RESULT SystemI::getStreamBufferSize(unsigned int* filebuffersize , FMOD_TIMEUNIT* filebuffersizetype)
    {
        if (filebuffersize != NULL)
            *filebuffersize = 0;
        if (filebuffersizetype != NULL)
            *filebuffersize = FMOD_TIMEUNIT_RAWBYTES;
        FMOD_RETURN(FMOD_OK);
    }

    FMOD_RESULT SystemI::getUserData(void ** userdata)
    {
        FMOD_RETURN(FMOD_ERR_UNIMPLEMENTED);
    }

    FMOD_RESULT SystemI::getVersion(unsigned int* version)
    {
        if (version == NULL)
            FMOD_RETURN(FMOD_ERR_INVALID_PARAM);
        *version = 0x44207;
        FMOD_RETURN(FMOD_OK);
    }

    FMOD_RESULT SystemI::getWaveData(float * wavearray , int numvalues, int channeloffset)
    {
        FMOD_RETURN(FMOD_ERR_UNIMPLEMENTED);
    }

    FMOD_RESULT SystemI::init(int maxchannels , FMOD_INITFLAGS flags, void *extradriverdata)
    {
        JS_Sound_Init();
        m_channels.SetSize(maxchannels);
        m_masterchannelgroup = m_channelgroups.GetFree();
        if (m_masterchannelgroup == NULL)
            FMOD_RETURN(FMOD_ERR_INTERNAL);
        FMOD_RETURN(FMOD_OK);
    }

    FMOD_RESULT SystemI::isRecording(int id , bool* isrecording)
    {
        if (isrecording != NULL)
            *isrecording = false;
        FMOD_RETURN(FMOD_OK);
    }

    FMOD_RESULT SystemI::loadGeometry(const void * data , int datasize, Geometry ** geometry)
    {
        FMOD_RETURN(FMOD_ERR_UNIMPLEMENTED);
    }

    FMOD_RESULT SystemI::loadPlugin(const char * filename , unsigned int * handle, unsigned int priority)
    {
        FMOD_RETURN(FMOD_ERR_UNIMPLEMENTED);
    }

    FMOD_RESULT SystemI::lockDSP()
    {
        FMOD_RETURN(FMOD_ERR_UNIMPLEMENTED);
    }

    FMOD_RESULT SystemI::playDSP(FMOD_CHANNELINDEX channelindex , DSP* dsp, bool paused, Channel** handle)
    {
        assert(handle != NULL);
        DSPI* dspi = SystemI::Validate<DSP, DSPI>(dsp);
        if (dspi == NULL)
            FMOD_RETURN(FMOD_ERR_INVALID_HANDLE);
        ChannelI* channeli = m_channels.GetFree();
        if (channeli == NULL)
            FMOD_RETURN(FMOD_ERR_CHANNEL_ALLOC);
        *handle = channeli->m_handle;
        channeli->m_sound = NULL;
        channeli->m_channelgroup = NULL;
        channeli->m_length = FAKE_SAMPLERATE;
        channeli->m_starttimeinsamples = FakeGetTimeInSamples();
        m_realchannels.Add(&channeli->m_realchannelchain);
        ++m_numrealchannels;
        FMOD_RETURN(FMOD_OK);
    }

    FMOD_RESULT SystemI::playSound(FMOD_CHANNELINDEX channelindex , Sound* sound, bool paused, Channel** handle)
    {
        assert(handle != NULL);
        SoundI* soundi = SystemI::Validate<Sound, SoundI>(sound);
        if (soundi == NULL)
            FMOD_RETURN(FMOD_ERR_INVALID_HANDLE);
        ChannelI* channeli = m_channels.GetFree();
        if (channeli == NULL)
            FMOD_RETURN(FMOD_ERR_CHANNEL_ALLOC);
        *handle = channeli->m_handle;
        soundi->m_channels.Add(&channeli->m_soundchain);
        // Even if the sound is to start paused (and will be scheduled to play later),
        // start the sound here. The channel will be immediately paused below.
        // That ensures that the sound playback state is set up appropriately to
        // resume/pause the execution later.
        JS_Sound_Play(soundi->m_instance, channeli->m_instance, 0, 0);
        JS_Sound_SetLoopPoints(channeli->m_instance, soundi->m_loopstart, soundi->m_loopend);
        channeli->m_sound = soundi;
        channeli->m_channelgroup = NULL;
        channeli->m_length = JS_Sound_GetLength(soundi->m_instance);
        channeli->m_starttimeinsamples = FakeGetTimeInSamples();
        channeli->m_pausetimeinsamples = channeli->m_starttimeinsamples;
        channeli->m_frequency = FAKE_SAMPLERATE;
        m_realchannels.Add(&channeli->m_realchannelchain);
        ++m_numrealchannels;
        channeli->setPaused(paused);
        FMOD_RETURN(FMOD_OK);
    }

    FMOD_RESULT SystemI::recordStart(int id , Sound * sound, bool loop)
    {
        FMOD_RETURN(FMOD_ERR_UNIMPLEMENTED);
    }

    FMOD_RESULT SystemI::recordStop(int id)
    {
        FMOD_RETURN(FMOD_ERR_UNIMPLEMENTED);
    }

    FMOD_RESULT SystemI::registerCodec(FMOD_CODEC_DESCRIPTION * description , unsigned int * handle, unsigned int priority)
    {
        FMOD_RETURN(FMOD_ERR_UNIMPLEMENTED);
    }

    FMOD_RESULT SystemI::registerDSP(FMOD_DSP_DESCRIPTION * description , unsigned int * handle)
    {
        FMOD_RETURN(FMOD_ERR_UNIMPLEMENTED);
    }

    FMOD_RESULT SystemI::release()
    {
        FMOD_RETURN(FMOD_OK);
    }

    FMOD_RESULT SystemI::set3DListenerAttributes(int listener , const FMOD_VECTOR* pos, const FMOD_VECTOR* vel, const FMOD_VECTOR* forward, const FMOD_VECTOR* up)
    {
        m_listenerposition = *pos;
        m_listenervelocity = *vel;
        m_listenerforward = *forward;
        m_listenerup = *up;

        JS_Sound_SetListenerPosition(pos->x, pos->y, pos->z);
        JS_Sound_SetListenerOrientation(forward->x, forward->y, forward->z, up->x, up->y, up->z);

        FMOD_RETURN(FMOD_OK);
    }

    FMOD_RESULT SystemI::set3DNumListeners(int numlisteners)
    {
        FMOD_RETURN(FMOD_ERR_UNIMPLEMENTED);
    }

    FMOD_RESULT SystemI::set3DRolloffCallback(FMOD_3D_ROLLOFFCALLBACK callback)
    {
        m_rolloffcallback = callback;
        FMOD_RETURN(FMOD_OK);
    }

    FMOD_RESULT SystemI::set3DSettings(float dopplerscale , float distancefactor, float rolloffscale)
    {
        m_dopplerscale = dopplerscale;
        m_distancefactor = distancefactor;
        m_rolloffscale = rolloffscale;
        FMOD_RETURN(FMOD_OK);
    }

    FMOD_RESULT SystemI::set3DSpeakerPosition(FMOD_SPEAKER speaker , float x, float y, bool active)
    {
        FMOD_RETURN(FMOD_ERR_UNIMPLEMENTED);
    }

    FMOD_RESULT SystemI::setAdvancedSettings(FMOD_ADVANCEDSETTINGS * settings)
    {
        FMOD_RETURN(FMOD_OK);
    }

    FMOD_RESULT SystemI::setCallback(FMOD_SYSTEM_CALLBACK callback)
    {
        m_systemcallback = callback;
        FMOD_RETURN(FMOD_OK);
    }

    FMOD_RESULT SystemI::setDriver(int driver)
    {
        FMOD_RETURN(FMOD_OK);
    }

    FMOD_RESULT SystemI::setDSPBufferSize(unsigned int bufferlength , int numbuffers)
    {
        m_dspbufferlength = bufferlength;
        m_dspnumbuffers = numbuffers;
        FMOD_RETURN(FMOD_OK);
    }

    FMOD_RESULT SystemI::setFileSystem(FMOD_FILE_OPENCALLBACK useropen , FMOD_FILE_CLOSECALLBACK userclose, FMOD_FILE_READCALLBACK userread, FMOD_FILE_SEEKCALLBACK userseek, FMOD_FILE_ASYNCREADCALLBACK userasyncread, FMOD_FILE_ASYNCCANCELCALLBACK userasynccancel, int blockalign)
    {
        m_useropen = useropen;
        m_userclose = userclose;
        m_userread = userread;
        m_userseek = userseek;

        FMOD_RETURN(FMOD_OK);
    }

    FMOD_RESULT SystemI::setGeometrySettings(float maxworldsize)
    {
        FMOD_RETURN(FMOD_ERR_UNIMPLEMENTED);
    }

    FMOD_RESULT SystemI::setHardwareChannels(int numhardwarechannels)
    {
        FMOD_RETURN(FMOD_ERR_UNIMPLEMENTED);
    }

    FMOD_RESULT SystemI::setNetworkProxy(const char * proxy)
    {
        FMOD_RETURN(FMOD_ERR_UNIMPLEMENTED);
    }

    FMOD_RESULT SystemI::setNetworkTimeout(int timeout)
    {
        FMOD_RETURN(FMOD_ERR_UNIMPLEMENTED);
    }

    FMOD_RESULT SystemI::setOutput(FMOD_OUTPUTTYPE outputtype)
    {
        FMOD_RETURN(FMOD_OK);
    }

    FMOD_RESULT SystemI::setOutputByPlugin(unsigned int handle)
    {
        FMOD_RETURN(FMOD_ERR_UNIMPLEMENTED);
    }

    FMOD_RESULT SystemI::setPluginPath(const char * path)
    {
        FMOD_RETURN(FMOD_ERR_UNIMPLEMENTED);
    }

    FMOD_RESULT SystemI::setReverbAmbientProperties(FMOD_REVERB_PROPERTIES * prop)
    {
        FMOD_RETURN(FMOD_ERR_UNIMPLEMENTED);
    }

    FMOD_RESULT SystemI::setReverbProperties(const FMOD_REVERB_PROPERTIES * prop)
    {
        FMOD_RETURN(FMOD_ERR_UNIMPLEMENTED);
    }

    FMOD_RESULT SystemI::setSoftwareChannels(int numsoftwarechannels)
    {
        m_maxrealchannels = numsoftwarechannels;
        FMOD_RETURN(FMOD_OK);
    }

    FMOD_RESULT SystemI::setSoftwareFormat(int samplerate , FMOD_SOUND_FORMAT format, int numoutputchannels, int maxinputchannels, FMOD_DSP_RESAMPLER resamplemethod)
    {
        FMOD_RETURN(FMOD_OK);
    }

    FMOD_RESULT SystemI::setSpeakerMode(FMOD_SPEAKERMODE speakermode)
    {
        FMOD_RETURN(FMOD_OK);
    }

    FMOD_RESULT SystemI::setStreamBufferSize(unsigned int filebuffersize , FMOD_TIMEUNIT filebuffersizetype)
    {
        FMOD_RETURN(FMOD_OK);
    }

    FMOD_RESULT SystemI::setUserData(void * userdata)
    {
        FMOD_RETURN(FMOD_ERR_UNIMPLEMENTED);
    }

    FMOD_RESULT SystemI::unloadPlugin(unsigned int handle)
    {
        FMOD_RETURN(FMOD_ERR_UNIMPLEMENTED);
    }

    FMOD_RESULT SystemI::unlockDSP()
    {
        FMOD_RETURN(FMOD_ERR_UNIMPLEMENTED);
    }

    FMOD_RESULT SystemI::update()
    {
        // Sort channels here
        m_channels.Check();
        m_channelgroups.Check();
        m_sounds.Check();
        m_dsps.Check();
        m_reverbs.Check();
        FMOD_RETURN(FMOD_OK);
    }
}
