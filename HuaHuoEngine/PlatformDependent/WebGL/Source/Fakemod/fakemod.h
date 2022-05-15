#pragma once

#include "UnityPrefix.h"
#include "Modules/Audio/Public/correct_fmod_includer.h"
#include "Modules/Audio/Public/AudioFeatureDefines.h"

#define ENABLE_STATE_DEBUGGING 0

#include "fakemodMacros.h"
#include "fakemodObjects.h"

namespace FMOD
{
    class SystemI;
    class ChannelI;
    class ChannelGroupI;
    class SoundI;
    class DSPI;
    class ReverbI;

    // Class definitions:

#define FMOD_IMPLEMENT_METHOD(klass, method, ...) \
FMOD_RESULT method(PARAMETER_LIST(DECLARE_PARAMETER,__VA_ARGS__));

    class SystemI
    {
    public:
        SystemI(int index);
        ~SystemI();
        FMOD_IMPLEMENT_CLASS(System)
#include "api/fakemodSystem.inc"
        static SystemI * GetSystemFromNonSystemHandle(void* handle); // no system validation here, since refcount is used by handle itself
        template<typename Handle, typename Implementation> inline ObjectPool<Handle, Implementation>* GetObjectPool();
        template<typename Handle, typename Implementation> FMOD_RESULT Release(Implementation* obj)
        {
            AUDIO_LOGSCOPE_STATIC(AUDIO_DEBUG_FILTER_FAKEMOD_CORE);
            return GetObjectPool<Handle, Implementation>()->Release(obj);
        }

        template<typename Handle, typename Implementation> static inline Implementation* Validate(Handle* handle)
        {
            AUDIO_LOGSCOPE_STATIC(AUDIO_DEBUG_FILTER_FAKEMOD_CORE);
            SystemI* system = GetSystemFromNonSystemHandle(handle);
            if (system == NULL)
                return NULL;
            ObjectPool<Handle, Implementation>* pool = system->GetObjectPool<Handle, Implementation>();
            return pool->Validate(handle);
        }

    protected:
        AUDIO_MARKER_DATA(AUDIO_DEBUG_FILTER_FAKEMOD_SYSTEM, "FkmdSysI", "FkmdSysF", FMOD::SystemI)
        int m_numrealchannels;
        int m_maxrealchannels;
        int m_dspbufferlength;
        int m_dspnumbuffers;
        float m_dopplerscale;
        float m_distancefactor;
        float m_rolloffscale;
        LinkChain<ChannelI> m_realchannels; // This is the sentinel element of the list of real channels
        ObjectPool<Channel, ChannelI> m_channels;
        ObjectPool<ChannelGroup, ChannelGroupI> m_channelgroups;
        ObjectPool<Sound, SoundI> m_sounds;
        ObjectPool<DSP, DSPI> m_dsps;
        ObjectPool<Reverb, ReverbI> m_reverbs;
        ChannelGroupI* m_masterchannelgroup;

        FMOD_VECTOR m_listenerposition;
        FMOD_VECTOR m_listenervelocity;
        FMOD_VECTOR m_listenerforward;
        FMOD_VECTOR m_listenerup;
        FMOD_3D_ROLLOFFCALLBACK m_rolloffcallback;
        FMOD_SYSTEM_CALLBACK m_systemcallback;
        FMOD_FILE_OPENCALLBACK m_useropen;
        FMOD_FILE_CLOSECALLBACK m_userclose;
        FMOD_FILE_READCALLBACK m_userread;
        FMOD_FILE_SEEKCALLBACK m_userseek;
        friend class ChannelI;
    };

    template<> inline ObjectPool<System, SystemI>* SystemI::GetObjectPool<System, SystemI>() { assert(0); return NULL; }
    template<> inline ObjectPool<Channel, ChannelI>* SystemI::GetObjectPool<Channel, ChannelI>() { AUDIO_LOGSCOPE_STATIC(AUDIO_DEBUG_FILTER_FAKEMOD_CORE); return &m_channels; }
    template<> inline ObjectPool<ChannelGroup, ChannelGroupI>* SystemI::GetObjectPool<ChannelGroup, ChannelGroupI>() { AUDIO_LOGSCOPE_STATIC(AUDIO_DEBUG_FILTER_FAKEMOD_CORE); return &m_channelgroups; }
    template<> inline ObjectPool<Sound, SoundI>* SystemI::GetObjectPool<Sound, SoundI>() { AUDIO_LOGSCOPE_STATIC(AUDIO_DEBUG_FILTER_FAKEMOD_CORE); return &m_sounds; }
    template<> inline ObjectPool<DSP, DSPI>* SystemI::GetObjectPool<DSP, DSPI>() { AUDIO_LOGSCOPE_STATIC(AUDIO_DEBUG_FILTER_FAKEMOD_CORE); return &m_dsps; }
    template<> inline ObjectPool<Reverb, ReverbI>* SystemI::GetObjectPool<Reverb, ReverbI>() { AUDIO_LOGSCOPE_STATIC(AUDIO_DEBUG_FILTER_FAKEMOD_CORE); return &m_reverbs; }
    template<> inline SystemI* SystemI::Validate(System* handle)
    {
        AUDIO_LOGSCOPE_STATIC(AUDIO_DEBUG_FILTER_FAKEMOD_CORE);
        SystemI* system = GetSystemFromNonSystemHandle(handle);
        if (system == NULL)
            return NULL;
        uintptr_t val = (uintptr_t)handle;
        uintptr_t refcount = (val >> REFCOUNTSHIFT) & REFCOUNTMASK;
        if (system->m_refcount != refcount)
        {
#if AUDIO_ENABLE_DEBUGGING
            uintptr_t index = (val >> INDEXSHIFT) & INDEXMASK;
            FMOD_LOGINVALIDHANDLE("Attempt to use invalid system handle with index=%d (refcount mismatch %d != %d)", index, refcount, system->m_refcount);
#endif
            return NULL;
        }
        return system;
    }

    class ChannelI
    {
    public:
        ChannelI(SystemI* system);
        ~ChannelI();
        FMOD_IMPLEMENT_CLASS(Channel)
#include "api/fakemodChannel.inc"
        void EndCallback();
    protected:
        AUDIO_MARKER_DATA(AUDIO_DEBUG_FILTER_FAKEMOD_CHANNEL, "FkMdChnI", "FkMdChnF", FMOD::ChannelI)
        float m_audibility;
        LinkChain<ChannelI> m_soundchain; // This ChannelI is part of a chain of ChannelI's owned by m_sound
        LinkChain<ChannelI> m_realchannelchain; // This ChannelI is part of the chain of real channels
        SoundI* m_sound;
        ChannelGroupI* m_channelgroup;
        FMOD_MODE m_mode;
        FMOD_VECTOR m_position;
        FMOD_VECTOR m_velocity;
        SInt64 m_starttimeinsamples;
        SInt64 m_stoptimeinsamples;
        SInt64 m_pausetimeinsamples;
        SInt64 m_length;
        float m_volume;
        float m_frequency;
        float m_panlevel;
        float m_3Ddopplerlevel;
        float m_dopplerpitch;
        float m_delay;
        bool m_ismuted;
        bool m_ispaused;
        bool m_isvirtual;
        UInt32 m_instance;
        FMOD_CHANNEL_CALLBACK m_channelcallback;
        void UpdatePitch();
        void UpdateDopplerPitch();
        void UpdateStartPositionForPitchChange(float frequency, float dopplerpitch);
        friend class SystemI;
    };

    class ChannelGroupI
    {
    public:
        ChannelGroupI(SystemI* system);
        ~ChannelGroupI();
        FMOD_IMPLEMENT_CLASS(ChannelGroup)
#include "api/fakemodChannelGroup.inc"
    protected:
        AUDIO_MARKER_DATA(AUDIO_DEBUG_FILTER_FAKEMOD_CHANNELGROUP, "FkMdChgI", "FkMdChgF", FMOD::ChannelGroupI)
        int m_numchannels;
        float m_audibilityFactor;
        float m_volume;
        ChannelGroupI* m_parent;
        LinkChain<ChannelGroupI> m_parentpool;
        LinkChain<ChannelGroupI> m_subgroups; // Channel groups under this channel group
        LinkChain<ChannelI> m_channels; // Channels under this channel group
    };

    class DSPI
    {
    public:
        DSPI(SystemI* system);
        ~DSPI();
        FMOD_IMPLEMENT_CLASS(DSP)
#include "api/fakemodDSP.inc"
    protected:
        AUDIO_MARKER_DATA(AUDIO_DEBUG_FILTER_FAKEMOD_DSP, "FkMdDSPI", "FkMdDSPF", FMOD::DSPI)
        friend class SystemI;
    };

    class ReverbI
    {
    public:
        ReverbI(SystemI* system);
        ~ReverbI();
        FMOD_IMPLEMENT_CLASS(Reverb)
#include "api/fakemodReverb.inc"
    protected:
        AUDIO_MARKER_DATA(AUDIO_DEBUG_FILTER_FAKEMOD_REVERB, "FkMdRvbI", "FkMdRvbF", FMOD::ReverbI)
        friend class SystemI;
    };

    class SoundI
    {
    public:
        SoundI(SystemI* system);
        ~SoundI();
        FMOD_IMPLEMENT_CLASS(Sound)
#include "api/fakemodSound.inc"
        void StopAllChannels();
    protected:
        AUDIO_MARKER_DATA(AUDIO_DEBUG_FILTER_FAKEMOD_SOUND, "FkMdSndI", "FkMdSndF", FMOD::SoundI)
        int m_numchannels;
        int m_frequency;
        FMOD_MODE m_mode;
        FMOD_SOUND_FORMAT m_format;
        LinkChain<ChannelI> m_channels; // Channels that this SoundI owns
        char m_name[256];
        UInt32 m_instance;
        double m_loopstart;
        double m_loopend;
        friend class SystemI;
        friend class ChannelI;
    };

#undef FMOD_IMPLEMENT_METHOD
}
