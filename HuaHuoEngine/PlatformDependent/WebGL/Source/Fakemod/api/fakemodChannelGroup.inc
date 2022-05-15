        FMOD_IMPLEMENT_METHOD(ChannelGroup, release,(,))
        FMOD_IMPLEMENT_METHOD(ChannelGroup, getSystemObject,(System **,system))

        // Channelgroup scale values.,(changes attributes relative to the ,channels),(doesn't overwrite ,them)
#ifdef UNITY_EXTRA_FUNCTIONALITY
        FMOD_IMPLEMENT_METHOD(ChannelGroup, setAudibilityFactor,(float ,factor))
        FMOD_IMPLEMENT_METHOD(ChannelGroup, getAudibilityFactor,(float *,factor))
#endif
        FMOD_IMPLEMENT_METHOD(ChannelGroup, setVolume,(float ,volume))
        FMOD_IMPLEMENT_METHOD(ChannelGroup, getVolume,(float *,volume))
        FMOD_IMPLEMENT_METHOD(ChannelGroup, setPitch,(float ,pitch))
        FMOD_IMPLEMENT_METHOD(ChannelGroup, getPitch,(float *,pitch))
        FMOD_IMPLEMENT_METHOD(ChannelGroup, set3DOcclusion,(float ,directocclusion),(float ,reverbocclusion))
        FMOD_IMPLEMENT_METHOD(ChannelGroup, get3DOcclusion,(float *,directocclusion),(float *,reverbocclusion))
        FMOD_IMPLEMENT_METHOD(ChannelGroup, setPaused,(bool ,paused))
        FMOD_IMPLEMENT_METHOD(ChannelGroup, getPaused,(bool *,paused))
        FMOD_IMPLEMENT_METHOD(ChannelGroup, setMute,(bool ,mute))
        FMOD_IMPLEMENT_METHOD(ChannelGroup, getMute,(bool *,mute))

        // Channelgroup override values.,(recursively overwrites whatever settings the channels ,had)
        FMOD_IMPLEMENT_METHOD(ChannelGroup, stop,(,))
        FMOD_IMPLEMENT_METHOD(ChannelGroup, overrideVolume,(float ,volume))
        FMOD_IMPLEMENT_METHOD(ChannelGroup, overrideFrequency,(float ,frequency))
        FMOD_IMPLEMENT_METHOD(ChannelGroup, overridePan,(float ,pan))
        FMOD_IMPLEMENT_METHOD(ChannelGroup, overrideReverbProperties,(const FMOD_REVERB_CHANNELPROPERTIES *,prop))
        FMOD_IMPLEMENT_METHOD(ChannelGroup, override3DAttributes,(const FMOD_VECTOR *,pos),(const FMOD_VECTOR *,vel))
        FMOD_IMPLEMENT_METHOD(ChannelGroup, overrideSpeakerMix,(float ,frontleft),(float ,frontright),(float ,center),(float ,lfe),(float ,backleft),(float ,backright),(float ,sideleft),(float ,sideright))

        // Nested channel groups.
        FMOD_IMPLEMENT_METHOD(ChannelGroup, addGroup,(ChannelGroup *,group))
        FMOD_IMPLEMENT_METHOD(ChannelGroup, getNumGroups,(int *,numgroups))
        FMOD_IMPLEMENT_METHOD(ChannelGroup, getGroup,(int ,index),(ChannelGroup **,group))
        FMOD_IMPLEMENT_METHOD(ChannelGroup, getParentGroup,(ChannelGroup **,group))

        // DSP functionality only for channel groups playing sounds created with FMOD_SOFTWARE.
        FMOD_IMPLEMENT_METHOD(ChannelGroup, getDSPHead,(DSP **,dsp))
        FMOD_IMPLEMENT_METHOD(ChannelGroup, addDSP,(DSP *,dsp),(DSPConnection **,connection))

        // Information only functions.
        FMOD_IMPLEMENT_METHOD(ChannelGroup, getName,(char *,name),(int ,namelen))
        FMOD_IMPLEMENT_METHOD(ChannelGroup, getNumChannels,(int *,numchannels))
        FMOD_IMPLEMENT_METHOD(ChannelGroup, getChannel,(int ,index),(Channel **,channel))
        FMOD_IMPLEMENT_METHOD(ChannelGroup, getSpectrum,(float *,spectrumarray),(int ,numvalues),(int ,channeloffset),(FMOD_DSP_FFT_WINDOW ,windowtype))
        FMOD_IMPLEMENT_METHOD(ChannelGroup, getWaveData,(float *,wavearray),(int ,numvalues),(int ,channeloffset))

        // Userdata set/get.
        FMOD_IMPLEMENT_METHOD(ChannelGroup, setUserData,(void *,userdata))
        FMOD_IMPLEMENT_METHOD(ChannelGroup, getUserData,(void **,userdata))

        FMOD_IMPLEMENT_METHOD(ChannelGroup, getMemoryInfo,(unsigned int ,memorybits),(unsigned int ,event_memorybits),(unsigned int *,memoryused),(FMOD_MEMORY_USAGE_DETAILS *,memoryused_details))
 