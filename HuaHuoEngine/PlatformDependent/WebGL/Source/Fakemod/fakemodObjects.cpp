#include "fakemod.h"

template<typename Handle, typename Implementation>
Implementation* ObjectPool<Handle, Implementation>::GetFree(int reuseindex)
{
    AUDIO_LOGSCOPE();
    Implementation* p;
    if (reuseindex != -1)
    {
        p = m_pool[reuseindex];
        if (p == NULL)
            return NULL;
    }
    else
    {
        p = m_free.FirstPtr();
        if (p == NULL)
        {
            if (m_numallocated >= m_pool.Size())
                return NULL;
            p = new Implementation(m_system);
            p->m_index = m_numallocated;
            m_pool[m_numallocated++] = p;
        }
        else
        {
            m_numfree--;
        }
        m_numused++;
        m_used.Add(&p->m_pool);
    }
    p->m_isfree = false;
    p->m_handle = MAKEHANDLE<Handle>(m_system->m_index, p->m_index, ++p->m_refcount);
    return p;
}

template<typename Handle, typename Implementation>
Implementation* ObjectPool<Handle, Implementation>::Validate(Handle* handle)
{
    AUDIO_LOGSCOPE_STATIC(AUDIO_DEBUG_FILTER_FAKEMOD_CORE);
    FMOD::SystemI* system = FMOD::SystemI::GetSystemFromNonSystemHandle(handle);
    if (system == NULL)
        return NULL;
    uintptr_t val = (uintptr_t)handle;
    uintptr_t index = (val >> INDEXSHIFT) & INDEXMASK;
    if (index > INDEXMASK)
    {
        FMOD_LOGINVALIDHANDLE("Attempt to use invalid %s handle with index=%d (index is outside of valid range)", GetObjectTypeName<Handle>(), index);
        return NULL;
    }
    ObjectPool<Handle, Implementation>* objpool = system->GetObjectPool<Handle, Implementation>();
    Implementation* p = objpool->m_pool[index];
    if (p == NULL)
    {
        FMOD_LOGINVALIDHANDLE("Attempt to use invalid %s handle with index=%d (object was never allocated)", GetObjectTypeName<Handle>(), index);
        return NULL;
    }
    if (p->m_isfree)
    {
        FMOD_LOGINVALIDHANDLE("Attempt to use invalid %s handle with index=%d (object has been freed)", GetObjectTypeName<Handle>(), index);
        return NULL;
    }
    uintptr_t refcount = (val >> REFCOUNTSHIFT) & REFCOUNTMASK;
    if (p->m_refcount != refcount)
    {
        FMOD_LOGINVALIDHANDLE("Attempt to use invalid %s handle with index=%d (refcount mismatch %d != %d)", GetObjectTypeName<Handle>(), index, refcount, p->m_refcount);
        return NULL;
    }
    return p;
}

template<typename Handle, typename Implementation>
void ObjectPool<Handle, Implementation>::Check()
{
    for (int n = 0; n < m_numallocated; n++)
    {
        if (m_pool[n] != NULL)
        {
            AUDIO_ASSERT(m_pool[n]->m_index == n);
            AUDIO_ASSERT((((uintptr_t)m_pool[n]->m_handle >> INDEXSHIFT) & INDEXMASK) == n);
        }
    }
}

template<typename Handle, typename Implementation>
FMOD_RESULT ObjectPool<Handle, Implementation>::Release(Implementation* p)
{
    if (p == NULL)
        FMOD_RETURN(FMOD_ERR_INVALID_HANDLE);
    int index = p->m_index;
    Implementation* old = m_pool[index];
    AUDIO_ASSERT(old->m_index == index);
    AUDIO_ASSERT((((uintptr_t)old->m_handle >> INDEXSHIFT) & INDEXMASK) == index);
    AUDIO_ASSERT(old->m_refcount == p->m_refcount);
    AUDIO_ASSERT(m_numallocated > 0);
    ++old->m_refcount;
    old->m_isfree = true;
    m_free.Add(&old->m_pool);
    m_numused--;
    m_numfree++;
    Check();
    FMOD_RETURN(FMOD_OK);
}

using namespace FMOD;

template class ObjectPool<Channel, ChannelI>;
template class ObjectPool<ChannelGroup, ChannelGroupI>;
template class ObjectPool<Sound, SoundI>;
template class ObjectPool<DSP, DSPI>;
template class ObjectPool<Reverb, ReverbI>;
