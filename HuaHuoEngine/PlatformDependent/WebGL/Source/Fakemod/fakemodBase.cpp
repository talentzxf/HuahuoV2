#include "fakemod.h"

FMOD_RESULT F_API FMOD_System_Create(FMOD_SYSTEM **handle)
{
    AUDIO_LOGSCOPE(AUDIO_DEBUG_FILTER_FAKEMOD_CORE);
    FMOD::SystemI* system = new FMOD::SystemI(++numsystemobjects);
    systemobjects[numsystemobjects] = system;
    system->m_index = numsystemobjects;
    system->m_handle = MAKEHANDLE<FMOD::System>(system->m_index, 0, system->m_refcount);
    *handle = (FMOD_SYSTEM*)system->m_handle;
    FMOD_RETURN(FMOD_OK);
}

FMOD_RESULT F_API FMOD_Debug_SetLevel(FMOD_DEBUGLEVEL level)
{
    AUDIO_LOGSCOPE(AUDIO_DEBUG_FILTER_FAKEMOD_CORE);
    FMOD_RETURN(FMOD_OK);
}

FMOD_RESULT F_API FMOD_Debug_GetLevel(FMOD_DEBUGLEVEL *level)
{
    AUDIO_LOGSCOPE(AUDIO_DEBUG_FILTER_FAKEMOD_CORE);
    FMOD_RETURN(FMOD_OK);
}

FMOD_RESULT F_API FMOD_Memory_Initialize(void* poolmem, int poollen, FMOD_MEMORY_ALLOCCALLBACK useralloc, FMOD_MEMORY_REALLOCCALLBACK userrealloc, FMOD_MEMORY_FREECALLBACK userfree, FMOD_MEMORY_TYPE memtypeflags)
{
    AUDIO_LOGSCOPE(AUDIO_DEBUG_FILTER_FAKEMOD_CORE);
    FMOD_RETURN(FMOD_OK);
}

FMOD_RESULT F_API FMOD_Memory_GetStats(int *currentalloced, int *maxalloced, FMOD_BOOL blocking)
{
    AUDIO_LOGSCOPE(AUDIO_DEBUG_FILTER_FAKEMOD_CORE);
    *currentalloced = 100000;
    *maxalloced = 100000;
    FMOD_RETURN(FMOD_OK);
}

namespace FMOD
{
    #if ENABLE_STATE_DEBUGGING

    void SystemI::ValidateState()
    {
        //AUDIO_LOGSCOPE(AUDIO_DEBUG_FILTER_FAKEMOD_CORE);
        ValidateLog("Validate SystemI: m_index=%d numsounds=%d numvirtchannels=%d m_numrealchannels=%d", m_index, m_sounds.m_numallocated, m_channels.m_numallocated, m_numrealchannels);
        AUDIO_INDENTSCOPE();
        /*  for(int n = 0; n < m_sounds.Size(); n++)
            {
                if(m_sounds[n] != NULL)
                {
                    ValidateLog("  Sound %4d: [%s]", n, (n < m_sounds.m_numused) ? "active" : "inactive");
                    AUDIO_INDENTSCOPE();
                    m_sounds[n]->ValidateState();
                }
            }
            for(int n = 0; n < m_virtchannels.Size(); n++)
            {
                if(m_virtchannels[n] != NULL)
                {
                    ValidateLog("  Virtual channel %4d: [%s]", n, (n < m_sounds.m_numused) ? "active" : "inactive");
                    AUDIO_INDENTSCOPE();
                    m_virtchannels[n]->ValidateState();
                }
            }*/
        LinkChain<ChannelI>::Iterator c = m_realchannels.First();
        while (c != m_realchannels.End())
        {
            AUDIO_INDENTSCOPE();
            c->ValidateState();
            ++c;
        }
    }

    void SoundI::ValidateState()
    {
        //AUDIO_LOGSCOPE(AUDIO_DEBUG_FILTER_FAKEMOD_CORE);
        ValidateLog("Validate SoundI: m_index=%d m_refcount=%d m_numchannels=%d", m_index, m_refcount, m_numchannels);
    }

    void ChannelI::ValidateState()
    {
        //AUDIO_LOGSCOPE(AUDIO_DEBUG_FILTER_FAKEMOD_CORE);
        ValidateLog("Validate ChannelI: m_index=%d m_refcount=%d", m_index, m_refcount);
    }

    void ChannelGroupI::ValidateState()
    {
        //AUDIO_LOGSCOPE(AUDIO_DEBUG_FILTER_FAKEMOD_CORE);
        //ValidateLog("Validate ChannelI: m_index=%d m_refcount=%d", m_index, m_refcount);
    }

    #endif

    SystemI* SystemI::GetSystemFromNonSystemHandle(void* handle)
    {
        AUDIO_LOGSCOPE_STATIC(AUDIO_DEBUG_FILTER_FAKEMOD_CORE);
        uintptr_t val = (uintptr_t)handle;
        uintptr_t systemindex = (val >> SYSTEMSHIFT) & SYSTEMMASK;
        if (systemobjects[systemindex] == NULL)
        {
            FMOD_LOGINVALIDHANDLE("Attempt to use invalid system handle with index=%d (index is outside of valid range)", systemindex);
            return NULL;
        }
        return systemobjects[systemindex];
    }
}
