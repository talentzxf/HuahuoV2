        FMOD_IMPLEMENT_METHOD(Sound, release,(,))
        FMOD_IMPLEMENT_METHOD(Sound, getSystemObject,(System **,system))

        // Standard sound manipulation functions.
        FMOD_IMPLEMENT_METHOD(Sound, lock,(unsigned int ,offset),(unsigned int ,length),(void **,ptr1),(void **,ptr2),(unsigned int *,len1),(unsigned int *,len2))
        FMOD_IMPLEMENT_METHOD(Sound, unlock,(void *,ptr1),(void *,ptr2),(unsigned int ,len1),(unsigned int ,len2))
        FMOD_IMPLEMENT_METHOD(Sound, setDefaults,(float ,frequency),(float ,volume),(float ,pan),(int ,priority))
        FMOD_IMPLEMENT_METHOD(Sound, getDefaults,(float *,frequency),(float *,volume),(float *,pan),( int *,priority))
        FMOD_IMPLEMENT_METHOD(Sound, setVariations,(float ,frequencyvar),(float ,volumevar),(float ,panvar))
        FMOD_IMPLEMENT_METHOD(Sound, getVariations,(float *,frequencyvar),(float *,volumevar),(float *,panvar))
        FMOD_IMPLEMENT_METHOD(Sound, set3DMinMaxDistance,(float ,min),(float ,max))
        FMOD_IMPLEMENT_METHOD(Sound, get3DMinMaxDistance,(float *,min),(float *,max))
        FMOD_IMPLEMENT_METHOD(Sound, set3DConeSettings,(float ,insideconeangle),(float ,outsideconeangle),(float ,outsidevolume))
        FMOD_IMPLEMENT_METHOD(Sound, get3DConeSettings,(float *,insideconeangle),(float *,outsideconeangle),(float *,outsidevolume))
        FMOD_IMPLEMENT_METHOD(Sound, set3DCustomRolloff,(FMOD_VECTOR *,points),(int ,numpoints))
        FMOD_IMPLEMENT_METHOD(Sound, get3DCustomRolloff,(FMOD_VECTOR **,points),(int *,numpoints))
        FMOD_IMPLEMENT_METHOD(Sound, setSubSound,(int ,index),(Sound *,subsound))
        FMOD_IMPLEMENT_METHOD(Sound, getSubSound,(int ,index),(Sound **,subsound))
        FMOD_IMPLEMENT_METHOD(Sound, setSubSoundSentence,(int *,subsoundlist),(int ,numsubsounds))
        FMOD_IMPLEMENT_METHOD(Sound, getName,(char *,name),(int ,namelen))
        FMOD_IMPLEMENT_METHOD(Sound, getLength,(unsigned int *,length),(FMOD_TIMEUNIT ,lengthtype))
        FMOD_IMPLEMENT_METHOD(Sound, getFormat,(FMOD_SOUND_TYPE *,type),(FMOD_SOUND_FORMAT *,format),(int *,channels),(int *,bits))
        FMOD_IMPLEMENT_METHOD(Sound, getNumSubSounds,(int *,numsubsounds))
        FMOD_IMPLEMENT_METHOD(Sound, getNumTags,(int *,numtags),(int *,numtagsupdated))
        FMOD_IMPLEMENT_METHOD(Sound, getTag,(const char *,name),(int ,index),(FMOD_TAG *,tag))
        FMOD_IMPLEMENT_METHOD(Sound, getOpenState,(FMOD_OPENSTATE *,openstate),(unsigned int *,percentbuffered),(bool *,starving),(bool *,diskbusy))
        FMOD_IMPLEMENT_METHOD(Sound, readData,(void *,buffer),(unsigned int ,lenbytes),(unsigned int *,read))
        FMOD_IMPLEMENT_METHOD(Sound, seekData,(unsigned int ,pcm))

        FMOD_IMPLEMENT_METHOD(Sound, setSoundGroup,(SoundGroup *,soundgroup))
        FMOD_IMPLEMENT_METHOD(Sound, getSoundGroup,(SoundGroup **,soundgroup))

        // Synchronization point API.  These points can come from markers embedded in wav files),(and can also generate channel callbacks.
        FMOD_IMPLEMENT_METHOD(Sound, getNumSyncPoints,(int *,numsyncpoints))
        FMOD_IMPLEMENT_METHOD(Sound, getSyncPoint,(int ,index),(FMOD_SYNCPOINT **,point))
        FMOD_IMPLEMENT_METHOD(Sound, getSyncPointInfo,(FMOD_SYNCPOINT *,point),(char *,name),(int ,namelen),(unsigned int *,offset),(FMOD_TIMEUNIT ,offsettype))
        FMOD_IMPLEMENT_METHOD(Sound, addSyncPoint,(unsigned int ,offset),(FMOD_TIMEUNIT ,offsettype),(const char *,name),(FMOD_SYNCPOINT **,point))
        FMOD_IMPLEMENT_METHOD(Sound, deleteSyncPoint,(FMOD_SYNCPOINT *,point))

        // Functions also in Channel class but here they are the 'default' to save having to change it in Channel all the time.
        FMOD_IMPLEMENT_METHOD(Sound, setMode,(FMOD_MODE ,mode))
        FMOD_IMPLEMENT_METHOD(Sound, getMode,(FMOD_MODE *,mode))
        FMOD_IMPLEMENT_METHOD(Sound, setLoopCount,(int ,loopcount))
        FMOD_IMPLEMENT_METHOD(Sound, getLoopCount,(int *,loopcount))
        FMOD_IMPLEMENT_METHOD(Sound, setLoopPoints,(unsigned int ,loopstart),(FMOD_TIMEUNIT ,loopstarttype),(unsigned int ,loopend),(FMOD_TIMEUNIT ,loopendtype))
        FMOD_IMPLEMENT_METHOD(Sound, getLoopPoints,(unsigned int *,loopstart),(FMOD_TIMEUNIT ,loopstarttype),(unsigned int *,loopend),(FMOD_TIMEUNIT ,loopendtype))

        // For MOD/S3M/XM/IT/MID sequenced formats only.
        FMOD_IMPLEMENT_METHOD(Sound, getMusicNumChannels,(int *,numchannels))
        FMOD_IMPLEMENT_METHOD(Sound, setMusicChannelVolume,(int ,channel),(float ,volume))
        FMOD_IMPLEMENT_METHOD(Sound, getMusicChannelVolume,(int ,channel),(float *,volume))
        FMOD_IMPLEMENT_METHOD(Sound, setMusicSpeed,(float ,speed))
        FMOD_IMPLEMENT_METHOD(Sound, getMusicSpeed,(float *,speed))

        // Userdata set/get.
        FMOD_IMPLEMENT_METHOD(Sound, setUserData,(void *,userdata))
        FMOD_IMPLEMENT_METHOD(Sound, getUserData,(void **,userdata))

        FMOD_IMPLEMENT_METHOD(Sound, getMemoryInfo,(unsigned int ,memorybits),(unsigned int ,event_memorybits),(unsigned int *,memoryused),(FMOD_MEMORY_USAGE_DETAILS *,memoryused_details))
