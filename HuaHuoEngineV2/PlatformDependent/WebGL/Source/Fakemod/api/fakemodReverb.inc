        FMOD_IMPLEMENT_METHOD(Reverb, release,(,))

        // Reverb manipulation.
        FMOD_IMPLEMENT_METHOD(Reverb, set3DAttributes,(const FMOD_VECTOR *,position),(float ,mindistance),(float ,maxdistance))
        FMOD_IMPLEMENT_METHOD(Reverb, get3DAttributes,(FMOD_VECTOR *,position),(float *,mindistance),(float *,maxdistance))
        FMOD_IMPLEMENT_METHOD(Reverb, setProperties,(const FMOD_REVERB_PROPERTIES *,properties))
        FMOD_IMPLEMENT_METHOD(Reverb, getProperties,(FMOD_REVERB_PROPERTIES *,properties))
        FMOD_IMPLEMENT_METHOD(Reverb, setActive,(bool ,active))
        FMOD_IMPLEMENT_METHOD(Reverb, getActive,(bool *,active))

        // Userdata set/get.
        FMOD_IMPLEMENT_METHOD(Reverb, setUserData,(void *,userdata))
        FMOD_IMPLEMENT_METHOD(Reverb, getUserData,(void **,userdata))

        FMOD_IMPLEMENT_METHOD(Reverb, getMemoryInfo,(unsigned int ,memorybits),(unsigned int ,event_memorybits),(unsigned int *,memoryused),(FMOD_MEMORY_USAGE_DETAILS *,memoryused_details))
