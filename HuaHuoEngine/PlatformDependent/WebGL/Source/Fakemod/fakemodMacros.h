#pragma once
#include <assert.h>

#if ENABLE_STATE_DEBUGGING

#define STATEDEBUG_DECL(classname) \
    void ValidateState();\
class ScopedStateValidator\
{\
public:\
    ScopedStateValidator(classname##I* obj)\
    : m_obj(obj)\
{\
    m_obj->ValidateState();\
}\
    ~ScopedStateValidator()\
{\
    m_obj->ValidateState();\
}\
protected:\
    classname##I* m_obj;\
};
#define AUDIO_LOGSCOPE_STATEVAL() \
    AUDIO_LOGSCOPE(AUDIO_DEBUG_FILTER_FAKEMOD_CORE);\
    ScopedStateValidator __stateval(this);

#else

#define STATEDEBUG_DECL(classname)
#define AUDIO_LOGSCOPE_STATEVAL() \
    AUDIO_LOGSCOPE(AUDIO_DEBUG_FILTER_FAKEMOD_CORE);

#endif

#define FMOD_UNIMPLEMENTED() \
    if(1)\
{\
    printf("ERROR: Unimplemented FMOD functionality in %s line %d!", __FUNCTION__, __LINE__);\
    assert(0);\
}\
    else\
    printf("Remember ;")

#define FMOD_BREAK() assert(0)

#define FMOD_RETURN(...) \
{\
    if(1)\
{\
    FMOD_RESULT returnResult = __VA_ARGS__;\
    if(returnResult != FMOD_OK)\
{\
    printf("FMOD returns error code %d ("  #__VA_ARGS__  ") executing %s\n", returnResult, __FUNCTION__);\
    if(1 && returnResult != FMOD_ERR_INVALID_HANDLE) FMOD_BREAK();\
}\
    return returnResult;\
}\
    else\
    printf("Remember ;");\
}


// Non-critical errors
#define FMOD_LOGINVALIDHANDLE(...) \
    if(1)\
{\
    AUDIO_LOGMSG_STATIC(__VA_ARGS__);\
    if(0) FMOD_BREAK();\
}\
    else\
    printf("Remember ;")

#define FMOD_IMPLEMENT_CLASS(classname) \
    typedef classname Handle;\
    STATEDEBUG_DECL(classname)\
    SystemI* m_system;\
    void* m_userdata;\
    int m_index;\
    int m_refcount;\
    bool m_isfree;\
    classname* m_handle;\
    LinkChain<classname##I> m_pool;

#define FMOD_INIT_CLASS(systemptr) \
    m_system(systemptr)\
    , m_index(0)\
    , m_refcount(0)\
    , m_handle(NULL)\
    , m_pool(this)\
    , m_userdata(NULL)\
    , m_isfree(true)

#define FAKE_SAMPLERATE 44100

#define FE_1(WHAT, PARAM, X) WHAT(PARAM, X)
#define FE_2(WHAT, PARAM, X, ...) WHAT(PARAM, X)FE_1(WHAT, PARAM, __VA_ARGS__)
#define FE_3(WHAT, PARAM, X, ...) WHAT(PARAM, X)FE_2(WHAT, PARAM, __VA_ARGS__)
#define FE_4(WHAT, PARAM, X, ...) WHAT(PARAM, X)FE_3(WHAT, PARAM, __VA_ARGS__)
#define FE_5(WHAT, PARAM, X, ...) WHAT(PARAM, X)FE_4(WHAT, PARAM, __VA_ARGS__)
#define FE_6(WHAT, PARAM, X, ...) WHAT(PARAM, X)FE_5(WHAT, PARAM, __VA_ARGS__)
#define FE_7(WHAT, PARAM, X, ...) WHAT(PARAM, X)FE_6(WHAT, PARAM, __VA_ARGS__)
#define FE_8(WHAT, PARAM, X, ...) WHAT(PARAM, X)FE_7(WHAT, PARAM, __VA_ARGS__)
//... repeat as needed

#define GET_MACRO(_1, _2, _3, _4, _5, _6, _7, _8, NAME, ...) NAME
#define FOR_EACH(action, param, ...) \
  GET_MACRO(__VA_ARGS__,FE_8,FE_7,FE_6,FE_5,FE_4,FE_3,FE_2,FE_1)(action,param,__VA_ARGS__)

#define PARAMETER_LIST_IMPL(type, name) type name
#define PARAMETER_LIST_NEXT(action, signature) , action signature
#define PARAMETER_LIST_FIRST(action, signature) action signature
#define PARAMETER_LIST_NONE none
#define PARAMETER_LIST_MULTIPLE(action, x, ...) PARAMETER_LIST_FIRST(action,x) FOR_EACH(PARAMETER_LIST_NEXT,action,__VA_ARGS__)
#define PARAMETER_LIST(action, ...) \
  GET_MACRO(__VA_ARGS__,PARAMETER_LIST_MULTIPLE,PARAMETER_LIST_MULTIPLE,PARAMETER_LIST_MULTIPLE,PARAMETER_LIST_MULTIPLE,PARAMETER_LIST_MULTIPLE,PARAMETER_LIST_MULTIPLE,PARAMETER_LIST_MULTIPLE,PARAMETER_LIST_FIRST)(action,__VA_ARGS__)

#define DECLARE_PARAMETER(type, name) type name
#define INVOKE_PARAMETER(type, name) name
