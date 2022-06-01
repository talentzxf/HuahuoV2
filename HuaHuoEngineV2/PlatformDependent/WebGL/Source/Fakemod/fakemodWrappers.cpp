#include "fakemod.h"

namespace FMOD
{
#define FMOD_IMPLEMENT_METHOD(klass, method, ...) \
FMOD_RESULT klass::method(PARAMETER_LIST(DECLARE_PARAMETER,__VA_ARGS__))\
{\
    AUDIO_LOGSCOPE_HANDLE(AUDIO_DEBUG_FILTER_FAKEMOD_HANDLE);\
    klass##I* p = SystemI::Validate<klass, klass##I>(this);\
    if(p == NULL)\
        FMOD_RETURN(FMOD_ERR_INVALID_HANDLE);\
    FMOD_RESULT r = p->method(PARAMETER_LIST(INVOKE_PARAMETER,__VA_ARGS__));\
    FMOD_RETURN(r);\
}

#include "api/fakemod.inc"

#undef FMOD_IMPLEMENT_METHOD
}
