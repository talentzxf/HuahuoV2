        FMOD_IMPLEMENT_METHOD(System, release,(,))

        // Pre-init functions.
        FMOD_IMPLEMENT_METHOD(System, setOutput,(FMOD_OUTPUTTYPE ,output))
        FMOD_IMPLEMENT_METHOD(System, getOutput,(FMOD_OUTPUTTYPE *,output))
        FMOD_IMPLEMENT_METHOD(System, getNumDrivers,(int *,numdrivers))
        FMOD_IMPLEMENT_METHOD(System, getDriverInfo,(int ,id),(char *,name),(int ,namelen),(FMOD_GUID *,guid))
        FMOD_IMPLEMENT_METHOD(System, getDriverInfoW,(int ,id),(short *,name),(int ,namelen),(FMOD_GUID *,guid))
        FMOD_IMPLEMENT_METHOD(System, getDriverCaps,(int ,id),(FMOD_CAPS *,caps),(int *,controlpaneloutputrate),(FMOD_SPEAKERMODE *,controlpanelspeakermode))
        FMOD_IMPLEMENT_METHOD(System, setDriver,(int ,driver))
        FMOD_IMPLEMENT_METHOD(System, getDriver,(int *,driver))
        FMOD_IMPLEMENT_METHOD(System, setHardwareChannels,(int ,numhardwarechannels))
        FMOD_IMPLEMENT_METHOD(System, setSoftwareChannels,(int ,numsoftwarechannels))
        FMOD_IMPLEMENT_METHOD(System, getSoftwareChannels,(int *,numsoftwarechannels))
        FMOD_IMPLEMENT_METHOD(System, setSoftwareFormat,(int ,samplerate),(FMOD_SOUND_FORMAT ,format),(int ,numoutputchannels),(int ,maxinputchannels),(FMOD_DSP_RESAMPLER ,resamplemethod))
        FMOD_IMPLEMENT_METHOD(System, getSoftwareFormat,(int *,samplerate),(FMOD_SOUND_FORMAT *,format),(int *,numoutputchannels),(int *,maxinputchannels),(FMOD_DSP_RESAMPLER *,resamplemethod),(int *,bits))
        FMOD_IMPLEMENT_METHOD(System, setDSPBufferSize,(unsigned int ,bufferlength),(int ,numbuffers))
        FMOD_IMPLEMENT_METHOD(System, getDSPBufferSize,(unsigned int *,bufferlength),(int *,numbuffers))
        FMOD_IMPLEMENT_METHOD(System, setFileSystem,(FMOD_FILE_OPENCALLBACK ,useropen),(FMOD_FILE_CLOSECALLBACK ,userclose),(FMOD_FILE_READCALLBACK ,userread),(FMOD_FILE_SEEKCALLBACK ,userseek),(FMOD_FILE_ASYNCREADCALLBACK ,userasyncread),(FMOD_FILE_ASYNCCANCELCALLBACK ,userasynccancel),(int ,blockalign))
        FMOD_IMPLEMENT_METHOD(System, attachFileSystem,(FMOD_FILE_OPENCALLBACK ,useropen),(FMOD_FILE_CLOSECALLBACK ,userclose),(FMOD_FILE_READCALLBACK ,userread),(FMOD_FILE_SEEKCALLBACK ,userseek))
        FMOD_IMPLEMENT_METHOD(System, setAdvancedSettings,(FMOD_ADVANCEDSETTINGS *,settings))
        FMOD_IMPLEMENT_METHOD(System, getAdvancedSettings,(FMOD_ADVANCEDSETTINGS *,settings))
        FMOD_IMPLEMENT_METHOD(System, setSpeakerMode,(FMOD_SPEAKERMODE ,speakermode))
        FMOD_IMPLEMENT_METHOD(System, getSpeakerMode,(FMOD_SPEAKERMODE *,speakermode))
        FMOD_IMPLEMENT_METHOD(System, setCallback,(FMOD_SYSTEM_CALLBACK ,callback))

        // Plug-in support
        FMOD_IMPLEMENT_METHOD(System, setPluginPath,(const char *,path))
        FMOD_IMPLEMENT_METHOD(System, loadPlugin,(const char *,filename),(unsigned int *,handle),(unsigned int ,priority))
        FMOD_IMPLEMENT_METHOD(System, unloadPlugin,(unsigned int ,handle))
        FMOD_IMPLEMENT_METHOD(System, getNumPlugins,(FMOD_PLUGINTYPE ,plugintype),(int *,numplugins))
        FMOD_IMPLEMENT_METHOD(System, getPluginHandle,(FMOD_PLUGINTYPE ,plugintype),(int ,index),(unsigned int *,handle))
        FMOD_IMPLEMENT_METHOD(System, getPluginInfo,(unsigned int ,handle),(FMOD_PLUGINTYPE *,plugintype),(char *,name),(int ,namelen),(unsigned int *,version))
        FMOD_IMPLEMENT_METHOD(System, setOutputByPlugin,(unsigned int ,handle))
        FMOD_IMPLEMENT_METHOD(System, getOutputByPlugin,(unsigned int *,handle))
        FMOD_IMPLEMENT_METHOD(System, createDSPByPlugin,(unsigned int ,handle),(DSP **,dsp))
        FMOD_IMPLEMENT_METHOD(System, registerCodec,(FMOD_CODEC_DESCRIPTION *,description),(unsigned int *,handle),(unsigned int ,priority))
        FMOD_IMPLEMENT_METHOD(System, registerDSP,(FMOD_DSP_DESCRIPTION *,description),(unsigned int *,handle))

        // Init/Close
        FMOD_IMPLEMENT_METHOD(System, init,(int ,maxchannels),(FMOD_INITFLAGS ,flags),(void *,extradriverdata))
        FMOD_IMPLEMENT_METHOD(System, close,(,))

        // General post-init system functions
        FMOD_IMPLEMENT_METHOD(System, update,(,))        /* IMPORTANT! CALL THIS ONCE PER FRAME! */

        FMOD_IMPLEMENT_METHOD(System, set3DSettings,(float ,dopplerscale),(float ,distancefactor),(float ,rolloffscale))
        FMOD_IMPLEMENT_METHOD(System, get3DSettings,(float *,dopplerscale),(float *,distancefactor),(float *,rolloffscale))
        FMOD_IMPLEMENT_METHOD(System, set3DNumListeners,(int ,numlisteners))
        FMOD_IMPLEMENT_METHOD(System, get3DNumListeners,(int *,numlisteners))
        FMOD_IMPLEMENT_METHOD(System, set3DListenerAttributes,(int ,listener),(const FMOD_VECTOR *,pos),(const FMOD_VECTOR *,vel),(const FMOD_VECTOR *,forward),(const FMOD_VECTOR *,up))
        FMOD_IMPLEMENT_METHOD(System, get3DListenerAttributes,(int ,listener),(FMOD_VECTOR *,pos),(FMOD_VECTOR *,vel),(FMOD_VECTOR *,forward),(FMOD_VECTOR *,up))
        FMOD_IMPLEMENT_METHOD(System, set3DRolloffCallback,(FMOD_3D_ROLLOFFCALLBACK ,callback))
        FMOD_IMPLEMENT_METHOD(System, set3DSpeakerPosition,(FMOD_SPEAKER ,speaker),(float ,x),(float ,y),(bool ,active))
        FMOD_IMPLEMENT_METHOD(System, get3DSpeakerPosition,(FMOD_SPEAKER ,speaker),(float *,x),(float *,y),(bool *,active))

        FMOD_IMPLEMENT_METHOD(System, setStreamBufferSize,(unsigned int ,filebuffersize),(FMOD_TIMEUNIT ,filebuffersizetype))
        FMOD_IMPLEMENT_METHOD(System, getStreamBufferSize,(unsigned int *,filebuffersize),(FMOD_TIMEUNIT *,filebuffersizetype))

        // System information functions.
        FMOD_IMPLEMENT_METHOD(System, getVersion,(unsigned int *,version))
        FMOD_IMPLEMENT_METHOD(System, getOutputHandle,(void **,handle))
        FMOD_IMPLEMENT_METHOD(System, getChannelsPlaying,(int *,channels))
        FMOD_IMPLEMENT_METHOD(System, getHardwareChannels,(int *,numhardwarechannels))
        FMOD_IMPLEMENT_METHOD(System, getCPUUsage,(float *,dsp),(float *,stream),(float *,geometry),(float *,update),(float *,total))
        FMOD_IMPLEMENT_METHOD(System, getSoundRAM,(int *,currentalloced),(int *,maxalloced),(int *,total))
        FMOD_IMPLEMENT_METHOD(System, getNumCDROMDrives,(int *,numdrives))
        FMOD_IMPLEMENT_METHOD(System, getCDROMDriveName,(int ,drive),(char *,drivename),(int ,drivenamelen),(char *,scsiname),(int ,scsinamelen),(char *,devicename),(int ,devicenamelen))
        FMOD_IMPLEMENT_METHOD(System, getSpectrum,(float *,spectrumarray),(int ,numvalues),(int ,channeloffset),(FMOD_DSP_FFT_WINDOW ,windowtype))
        FMOD_IMPLEMENT_METHOD(System, getWaveData,(float *,wavearray),(int ,numvalues),(int ,channeloffset))

        // Sound/DSP/Channel/FX creation and retrieval.
        FMOD_IMPLEMENT_METHOD(System, createSound,(const char *,name_or_data),(FMOD_MODE ,mode),(FMOD_CREATESOUNDEXINFO *,exinfo),(Sound **,sound))
        FMOD_IMPLEMENT_METHOD(System, createStream,(const char *,name_or_data),(FMOD_MODE ,mode),(FMOD_CREATESOUNDEXINFO *,exinfo),(Sound **,sound))
        FMOD_IMPLEMENT_METHOD(System, createDSP,(FMOD_DSP_DESCRIPTION *,description),(DSP **,dsp))
        FMOD_IMPLEMENT_METHOD(System, createDSPByType,(FMOD_DSP_TYPE ,type),(DSP **,dsp))
        FMOD_IMPLEMENT_METHOD(System, createChannelGroup,(const char *,name),(ChannelGroup **,channelgroup))
        FMOD_IMPLEMENT_METHOD(System, createSoundGroup,(const char *,name),(SoundGroup **,soundgroup))
        FMOD_IMPLEMENT_METHOD(System, createReverb,(Reverb **,reverb))

        FMOD_IMPLEMENT_METHOD(System, playSound,(FMOD_CHANNELINDEX ,channelid),(Sound *,sound),(bool ,paused),(Channel **,channel))
        FMOD_IMPLEMENT_METHOD(System, playDSP,(FMOD_CHANNELINDEX ,channelid),(DSP *,dsp),(bool ,paused),(Channel **,channel))
        FMOD_IMPLEMENT_METHOD(System, getChannel,(int ,channelid),(Channel **,channel))
        FMOD_IMPLEMENT_METHOD(System, getMasterChannelGroup,(ChannelGroup **,channelgroup))
        FMOD_IMPLEMENT_METHOD(System, getMasterSoundGroup,(SoundGroup **,soundgroup))

        // Reverb API
        FMOD_IMPLEMENT_METHOD(System, setReverbProperties,(const FMOD_REVERB_PROPERTIES *,prop))
        FMOD_IMPLEMENT_METHOD(System, getReverbProperties,(FMOD_REVERB_PROPERTIES *,prop))
        FMOD_IMPLEMENT_METHOD(System, setReverbAmbientProperties,(FMOD_REVERB_PROPERTIES *,prop))
        FMOD_IMPLEMENT_METHOD(System, getReverbAmbientProperties,(FMOD_REVERB_PROPERTIES *,prop))

        // System level DSP access.
        FMOD_IMPLEMENT_METHOD(System, getDSPHead,(DSP **,dsp))
        FMOD_IMPLEMENT_METHOD(System, addDSP,(DSP *,dsp),(DSPConnection **,connection))
        FMOD_IMPLEMENT_METHOD(System, lockDSP,(,))
        FMOD_IMPLEMENT_METHOD(System, unlockDSP,(,))
        FMOD_IMPLEMENT_METHOD(System, getDSPClock,(unsigned int *,hi),(unsigned int *,lo))

        // Recording API.
        FMOD_IMPLEMENT_METHOD(System, getRecordNumDrivers,(int *,numdrivers))
        FMOD_IMPLEMENT_METHOD(System, getRecordDriverInfo,(int ,id),(char *,name),(int ,namelen),(FMOD_GUID *,guid))
        FMOD_IMPLEMENT_METHOD(System, getRecordDriverInfoW,(int ,id),(short *,name),(int ,namelen),(FMOD_GUID *,guid))
        FMOD_IMPLEMENT_METHOD(System, getRecordDriverCaps,(int ,id),(FMOD_CAPS *,caps),(int *,minfrequency),(int *,maxfrequency))
        FMOD_IMPLEMENT_METHOD(System, getRecordPosition,(int ,id),(unsigned int *,position))

        FMOD_IMPLEMENT_METHOD(System, recordStart,(int ,id),(Sound *,sound),(bool ,loop))
        FMOD_IMPLEMENT_METHOD(System, recordStop,(int ,id))
        FMOD_IMPLEMENT_METHOD(System, isRecording,(int ,id),(bool *,recording))

        // Geometry API.
        FMOD_IMPLEMENT_METHOD(System, createGeometry,(int ,maxpolygons),(int ,maxvertices),(Geometry **,geometry))
        FMOD_IMPLEMENT_METHOD(System, setGeometrySettings,(float ,maxworldsize))
        FMOD_IMPLEMENT_METHOD(System, getGeometrySettings,(float *,maxworldsize))
        FMOD_IMPLEMENT_METHOD(System, loadGeometry,(const void *,data),(int ,datasize),(Geometry **,geometry))
        FMOD_IMPLEMENT_METHOD(System, getGeometryOcclusion,(const FMOD_VECTOR *,listener),(const FMOD_VECTOR *,source),(float *,direct),(float *,reverb))

        // Network functions.
        FMOD_IMPLEMENT_METHOD(System, setNetworkProxy,(const char *,proxy))
        FMOD_IMPLEMENT_METHOD(System, getNetworkProxy,(char *,proxy),(int ,proxylen))
        FMOD_IMPLEMENT_METHOD(System, setNetworkTimeout,(int ,timeout))
        FMOD_IMPLEMENT_METHOD(System, getNetworkTimeout,(int *,timeout))

        // Userdata set/get.
        FMOD_IMPLEMENT_METHOD(System, setUserData,(void *,userdata))
        FMOD_IMPLEMENT_METHOD(System, getUserData,(void **,userdata))

        FMOD_IMPLEMENT_METHOD(System, getMemoryInfo,(unsigned int ,memorybits),(unsigned int ,event_memorybits),(unsigned int *,memoryused),(FMOD_MEMORY_USAGE_DETAILS *,memoryused_details))
