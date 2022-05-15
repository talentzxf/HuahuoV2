#pragma once

static const int SYSTEMBITS     = 4;
static const int INDEXBITS      = 12;
static const int REFCOUNTBITS   = 16;

static const int SYSTEMMASK     = (1 << SYSTEMBITS) - 1;
static const int INDEXMASK      = (1 << INDEXBITS) - 1;
static const int REFCOUNTMASK   = (1 << REFCOUNTBITS) - 1;

static const int SYSTEMSHIFT    = 32            - SYSTEMBITS;
static const int INDEXSHIFT     = SYSTEMSHIFT   - INDEXBITS;
static const int REFCOUNTSHIFT  = INDEXSHIFT    - REFCOUNTBITS;

template<typename Handle>
static inline Handle* MAKEHANDLE(int systemindex, int index, int refcount)
{
    AUDIO_LOGSCOPE(AUDIO_DEBUG_FILTER_FAKEMOD_CORE);
    assert(((systemindex & ~SYSTEMMASK) | (index & ~INDEXMASK) | (refcount & ~REFCOUNTMASK)) == 0);
    return (Handle*)(
        ((unsigned long)(systemindex & SYSTEMMASK) << SYSTEMSHIFT) |
        ((unsigned long)(index & INDEXMASK) << INDEXSHIFT) |
        ((unsigned long)(refcount & REFCOUNTMASK) << REFCOUNTSHIFT)
    );
}

#define ValidateLog AUDIO_LOGMSG


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

template<typename T>
class LinkChain
{
    AUDIO_MARKER_DATA(AUDIO_DEBUG_FILTER_FAKEMOD_CORE, "FkmdLcEI", "FkmdLcEF", LinkChain)
public:
    LinkChain(T* owner)
        : m_owner(owner)
        , m_prev(this)
        , m_next(this)
    {
        AUDIO_LOGSCOPE();
    }

    ~LinkChain()
    {
        AUDIO_LOGSCOPE();
        if (m_next != this)
            RemoveFromList();
    }

public:
    typedef LinkChain<T> Node;
    class Iterator
    {
        AUDIO_MARKER_DATA(AUDIO_DEBUG_FILTER_FAKEMOD_CORE, "FkmdLcII", "FkmdLcIF", Iterator)
    public:
        Iterator(const Node* node)
            : m_node(node)
        {
            AUDIO_LOGSCOPE();
        }

        inline void operator++()
        {
            AUDIO_LOGSCOPE();
            m_node = m_node->m_next;
        }

        inline bool operator==(const Iterator& i)
        {
            AUDIO_LOGSCOPE();
            return m_node == i.m_node;
        }

        inline bool operator!=(const Iterator& i)
        {
            AUDIO_LOGSCOPE();
            return m_node != i.m_node;
        }

        T* operator->() const
        {
            AUDIO_LOGSCOPE();
            return (m_node == NULL) ? NULL : m_node->m_owner;
        }

    protected:
        const Node* m_node;
    };
public:
    inline bool Empty() const
    {
        AUDIO_LOGSCOPE();
        return m_next == this;
    }

    inline void Reset()
    {
        AUDIO_LOGSCOPE();
        m_prev = this;
        m_next = this;
    }

    inline void Add(Node* node)
    {
        AUDIO_LOGSCOPE();
        assert(m_owner == NULL);
        node->RemoveFromList();
        node->m_prev = m_prev;
        node->m_next = this;
        m_prev->m_next = node;
        m_prev = node;
    }

    inline void RemoveFromList()
    {
        AUDIO_LOGSCOPE();
        assert(m_owner != NULL);
        m_prev->m_next = m_next;
        m_next->m_prev = m_prev;
        m_prev = this;
        m_next = this;
    }

    inline T* FirstPtr() const
    {
        AUDIO_LOGSCOPE();
        return (m_next != this) ? m_next->m_owner : NULL;
    }

    inline Iterator First() const
    {
        AUDIO_LOGSCOPE();
        return Iterator(m_next);
    }

    inline Iterator Last() const
    {
        AUDIO_LOGSCOPE();
        return Iterator(m_prev);
    }

    inline Iterator End() const
    {
        AUDIO_LOGSCOPE();
        return Iterator(this);
    }

public:
    T* m_owner;
    Node* m_prev;
    Node* m_next;
};

template<typename T>
class TArray
{
public:
    TArray()
        : m_data(NULL)
        , m_size(0)
    {
    }

    ~TArray()
    {
        delete[] m_data;
    }

public:
    void SetSize(size_t size)
    {
        delete[] m_data;
        m_data = new T[size];
        m_size = size;
    }

    T& operator[](size_t index) const
    {
        assert(index >= 0);
        assert(index < m_size);
        return m_data[index];
    }

    size_t Size() const
    {
        return m_size;
    }

protected:
    T* m_data;
    size_t m_size;
};

namespace FMOD
{
    class SystemI;
}

template<typename Handle, typename Implementation>
class ObjectPool
{
    AUDIO_MARKER_DATA(AUDIO_DEBUG_FILTER_FAKEMOD_CORE, "FkmdObPI", "FkmdObPF", ObjectPool)
public:
    ObjectPool(FMOD::SystemI* system, int size);
    ~ObjectPool();
public:
    void SetSize(int newsize);
    Implementation* GetFree(int reuseindex = -1);
    Implementation* Validate(Handle* handle);
    void Check();
    FMOD_RESULT Release(Implementation* obj);
public:
    FMOD::SystemI* m_system;
    int m_numallocated;
    int m_numfree;
    int m_numused;
    LinkChain<Implementation> m_free; // This is the sentinel element of the list of free channels
    LinkChain<Implementation> m_used; // This is the sentinel element of the list of used channels
    TArray<Implementation*> m_pool;
};

static const int MAXSYSTEMS = 4;
static FMOD::SystemI* systemobjects[MAXSYSTEMS] = { NULL };
static int numsystemobjects = 0;

template<typename T>
static inline const char* GetObjectTypeName()
{
    AUDIO_LOGSCOPE(AUDIO_DEBUG_FILTER_FAKEMOD_CORE);
    return __FUNCTION__;
}

template<typename Handle, typename Implementation>
ObjectPool<Handle, Implementation>::ObjectPool(FMOD::SystemI* system, int size)
    : m_system(system)
    , m_numallocated(0)
    , m_numfree(0)
    , m_numused(0)
    , m_used(NULL)
    , m_free(NULL)
{
    AUDIO_LOGSCOPE();
    SetSize(size);
}

template<typename Handle, typename Implementation>
ObjectPool<Handle, Implementation>::~ObjectPool()
{
    AUDIO_LOGSCOPE();
    for (int n = 0; n < m_pool.Size(); n++)
        if (m_pool[n] != NULL)
            delete m_pool[n];
}

template<typename Handle, typename Implementation>
void ObjectPool<Handle, Implementation>::SetSize(int newsize)
{
    // Note: will delete all existing objects
    AUDIO_LOGSCOPE();
    for (int i = 0; i < m_pool.Size(); i++)
        delete m_pool[i];
    m_pool.SetSize(newsize);
    for (int i = 0; i < newsize; i++)
        m_pool[i] = NULL;
    m_used.Reset();
    m_free.Reset();
    m_numallocated = 0;
    m_numfree = 0;
    m_numused = 0;
}
