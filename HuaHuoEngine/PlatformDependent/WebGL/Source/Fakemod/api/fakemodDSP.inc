        FMOD_IMPLEMENT_METHOD(DSP, release,(,))
        FMOD_IMPLEMENT_METHOD(DSP, getSystemObject,(System **,system))

        // Connection / disconnection / input and output enumeration.
        FMOD_IMPLEMENT_METHOD(DSP, addInput,(DSP *,target),(DSPConnection **,connection))
        FMOD_IMPLEMENT_METHOD(DSP, disconnectFrom,(DSP *,target))
        FMOD_IMPLEMENT_METHOD(DSP, disconnectAll,(bool ,inputs),(bool ,outputs))
        FMOD_IMPLEMENT_METHOD(DSP, remove,(,))
        FMOD_IMPLEMENT_METHOD(DSP, getNumInputs,(int *,numinputs))
        FMOD_IMPLEMENT_METHOD(DSP, getNumOutputs,(int *,numoutputs))
        FMOD_IMPLEMENT_METHOD(DSP, getInput,(int ,index),(DSP **,input),(DSPConnection **,inputconnection))
        FMOD_IMPLEMENT_METHOD(DSP, getOutput,(int ,index),(DSP **,output),(DSPConnection **,outputconnection))

        // DSP unit control.
        FMOD_IMPLEMENT_METHOD(DSP, setActive,(bool ,active))
        FMOD_IMPLEMENT_METHOD(DSP, getActive,(bool *,active))
        FMOD_IMPLEMENT_METHOD(DSP, setBypass,(bool ,bypass))
        FMOD_IMPLEMENT_METHOD(DSP, getBypass,(bool *,bypass))
        FMOD_IMPLEMENT_METHOD(DSP, setSpeakerActive,(FMOD_SPEAKER ,speaker),(bool ,active))
		FMOD_IMPLEMENT_METHOD(DSP, getSpeakerActive,(FMOD_SPEAKER ,speaker),(bool *,active))
		FMOD_IMPLEMENT_METHOD(DSP, reset,(,))


        // DSP parameter control.
        FMOD_IMPLEMENT_METHOD(DSP, setParameter,(int ,index),(float ,value))
        FMOD_IMPLEMENT_METHOD(DSP, getParameter,(int ,index),(float *,value),(char *,valuestr),(int ,valuestrlen))
        FMOD_IMPLEMENT_METHOD(DSP, getNumParameters,(int *,numparams))
        FMOD_IMPLEMENT_METHOD(DSP, getParameterInfo,(int ,index),(char *,name),(char *,label),(char *,description),(int ,descriptionlen),(float *,min),(float *,max))
        FMOD_IMPLEMENT_METHOD(DSP, showConfigDialog,(void *,hwnd),(bool ,show))

        // DSP attributes.
        FMOD_IMPLEMENT_METHOD(DSP, getInfo,(char *,name),(unsigned int *,version),(int *,channels),(int *,configwidth),(int *,configheight))
        FMOD_IMPLEMENT_METHOD(DSP, getType,(FMOD_DSP_TYPE *,type))
        FMOD_IMPLEMENT_METHOD(DSP, setDefaults,(float ,frequency),(float ,volume),(float ,pan),(int ,priority))
        FMOD_IMPLEMENT_METHOD(DSP, getDefaults,(float *,frequency),(float *,volume),(float *,pan),(int *,priority))

        // Userdata set/get.
        FMOD_IMPLEMENT_METHOD(DSP, setUserData,(void *,userdata))
        FMOD_IMPLEMENT_METHOD(DSP, getUserData,(void **,userdata))

        FMOD_IMPLEMENT_METHOD(DSP, getMemoryInfo,(unsigned int ,memorybits),(unsigned int ,event_memorybits),(unsigned int *,memoryused),(FMOD_MEMORY_USAGE_DETAILS *,memoryused_details))
