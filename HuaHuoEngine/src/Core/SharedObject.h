//
// Created by VincentZhang on 5/21/2022.
//

#ifndef HUAHUOENGINE_SHAREDOBJECT_H
#define HUAHUOENGINE_SHAREDOBJECT_H
#include "ConstructorUtility.h"
#include "SharedObjectPtr.h"
#include "Memory/MemoryMacros.h"
#include "Threads/ExtendedAtomicOps.h"

// SharedObject should be used with SharedObjectPtr (Runtime/Core/SharedObjectPtr.h), which provides automatic
// reference counting.

// SharedObject is a base class for reference counted objects for either multi-threaded and single-threaded use.
// Choose the mode using the ThreadSafe template argument, or use ThreadSharedObject or SingleThreadedSharedObject.
// Single-threaded objects have asserts in debug builds that AddRef() and Release() are called on the correct thread.
// This is a template class in order to keep the destructor non-virtual. If you derive from T, you need a virtual ~T().

// GI objects have automatic AddRef()/Release() management through RefcountedData in a RefcountedDataHandle.
// Should we do a similar thing for non-GI objects?

// Validation that objects are alive when calling AddRef() and Release(). Costs an extra int per object to track state.
#define VALIDATE_SHARED_OBJECTS (ENABLE_ASSERTIONS && !UNITY_RELEASE)

// Validation that single-threaded objects are used from the correct thread.
#define VALIDATE_SINGLE_THREADED_SHARED_OBJECT_USAGE (ENABLE_ASSERTIONS && !UNITY_RELEASE)

#if VALIDATE_SINGLE_THREADED_SHARED_OBJECT_USAGE
template<bool Enabled>
class CorrectThreadValidator
{
public:
    CorrectThreadValidator() : m_ThreadID(CurrentThread::GetID()) {}

    bool IsCorrectThread() const { return CurrentThread::GetID() == m_ThreadID; }

private:
    ThreadId m_ThreadID;
};

template<>
class CorrectThreadValidator<false>
{
public:
    bool IsCorrectThread() const { return true; }
};
#endif // VALIDATE_SINGLE_THREADED_SHARED_OBJECT_USAGE

// SharedObject has pluggable behaviors for construction and when the refcount of the tracked object goes to zero.
// This is the default implementation that creates and destroys the object. Other uses can be object pooling etc.
template<typename T>
class SharedObjectFactory
{
public:
    using AutoLabel = AutoLabelConstructor<T>;

    template<typename ... TArgs>
    static core::SharedObjectPtr<T> CreateSharedObject(MemLabelId memLabelId, TArgs&& ... args)
    {
        T* obj = HUAHUO_NEW(T, memLabelId)(memLabelId, std::forward<TArgs>(args)...);    // ref count = 1
        core::SharedObjectPtr<T> ptr(obj);                                              // ref count = 2
        obj->Release();
        return ptr;
    }

    // Allows access to the copy constructor
    static core::SharedObjectPtr<T> CopySharedObject(const T& other, MemLabelId label)
    {
        T* obj = (T*)HUAHUO_MALLOC(label, sizeof(T));
        AutoLabel::construct_args(obj, label, other);                               // ref count = 1
        core::SharedObjectPtr<T> ptr(obj);                                            // ref count = 2
        obj->Release();                                                               // ref count = 1
        return ptr;
    }

    static void Destroy(T *obj, MemLabelId memLabel)
    {
        obj->~T();
        UNITY_FREE(memLabel, obj);
    }
};


template<typename T, bool ThreadSafe, typename TFactoryOps = SharedObjectFactory<T> >
class SharedObject
{
public:

    template<typename U>
    friend class core::SharedObjectPtr;

    typedef TFactoryOps TObjectFactoryOps;  // Used to allow core::CreateShared to call TObjectFactoryOps::Create
    inline void AddRef() const;
    inline void Release() const;
    inline int  GetRefCount() const;
    inline MemLabelRef GetMemLabel() const { return m_MemLabel; }


protected:

    // Used to throw an informative compiler time error, telling the developer they need to use SharedObjectPtr with a
    // template T that is derived from SharedObject, if they do not do so.
    typedef void* SharedObjectDerived;

    SharedObject(MemLabelId label) : m_MemLabel(label), m_RefCount(1) { InitializeSharedObject(); }
    SharedObject(const SharedObject& other) : m_MemLabel(other.m_MemLabel), m_RefCount(1) { InitializeSharedObject(); }

    void SetMemLabel(MemLabelId label) { m_MemLabel = label; }

    void ValidateThreadID() const;

    // The pooled variants can call this to reset the SharedObject base to initial state.
    void BaseReset()
    {
        InitializeSharedObject();
    }

    MemLabelId m_MemLabel;

private:
    SharedObject& operator=(const SharedObject&); // No assignment

    inline void InitializeSharedObject();

#if VALIDATE_SHARED_OBJECTS
    inline int ValidateSharedObject(const char* funcName) const;
    enum
    {
        kSharedObjectOK      = 0x4B4F4B4F, // "OKOK"
    };
    mutable int m_SOState;
#endif

#if VALIDATE_SINGLE_THREADED_SHARED_OBJECT_USAGE
    CorrectThreadValidator<!ThreadSafe> m_ThreadValidator;
#endif

    mutable int m_RefCount;
};

template<typename T, bool ThreadSafe, typename TFactoryOps>
void SharedObject<T, ThreadSafe, TFactoryOps>::AddRef() const
{
    bool valid = true;
#if VALIDATE_SHARED_OBJECTS
    valid = (ValidateSharedObject("AddRef()") == kSharedObjectOK);
#endif
    // If object is invalid, thread ID probably is too
    if (valid)
        ValidateThreadID();

    if (ThreadSafe)
        atomic_retain(&m_RefCount);
    else
        ++m_RefCount;
}

template<typename T, bool ThreadSafe, typename TFactoryOps>
void SharedObject<T, ThreadSafe, TFactoryOps>::Release() const
{
    bool valid = true;
#if VALIDATE_SHARED_OBJECTS
    int preReleaseState = ValidateSharedObject("Release()");
    valid = (preReleaseState == kSharedObjectOK);
#endif
    // If object is invalid, thread ID probably is too
    if (valid)
        ValidateThreadID();

    if ((ThreadSafe && atomic_release(&m_RefCount)) ||
        (!ThreadSafe && --m_RefCount == 0))
    {
#if VALIDATE_SHARED_OBJECTS
        if (ThreadSafe)
        {
            int postReleaseState = atomic_exchange_explicit(&m_SOState, 0, ::memory_order_relaxed);
            AssertFormatMsg(postReleaseState == preReleaseState, "State changed while releasing object: %p (new state %08X)", this, postReleaseState);
        }
#endif
        MemLabelId memLabel = m_MemLabel;
        T* obj = static_cast<T*>(const_cast<SharedObject<T, ThreadSafe, TFactoryOps> *>(this));
        TObjectFactoryOps::Destroy(obj, memLabel);
    }
}

template<typename T, bool ThreadSafe, typename TFactoryOps>
int SharedObject<T, ThreadSafe, TFactoryOps>::GetRefCount() const
{
    return ThreadSafe ? atomic_load_explicit(&m_RefCount, ::memory_order_relaxed) : m_RefCount;
}

template<typename T, bool ThreadSafe, typename TFactoryOps>
void SharedObject<T, ThreadSafe, TFactoryOps>::ValidateThreadID() const
{
#if VALIDATE_SINGLE_THREADED_SHARED_OBJECT_USAGE
    AssertFormatMsg(m_ThreadValidator.IsCorrectThread(), "Non-thread safe object used from wrong thread: %p", this);
#endif
}

template<typename T, bool ThreadSafe, typename TFactoryOps>
void SharedObject<T, ThreadSafe, TFactoryOps>::InitializeSharedObject()
{
#if VALIDATE_SHARED_OBJECTS
    m_SOState = kSharedObjectOK;
#endif
}

#if VALIDATE_SHARED_OBJECTS
template<typename T, bool ThreadSafe, typename TFactoryOps>
int SharedObject<T, ThreadSafe, TFactoryOps>::ValidateSharedObject(const char* funcName) const
{
    int state = ThreadSafe ? atomic_load_explicit(&m_SOState, ::memory_order_relaxed) : m_SOState;
    AssertFormatMsg(state == kSharedObjectOK, "Called %s on an invalid object: %p (state %08X)", funcName, this, state);
    return state;
}

#endif

template<typename T, typename TFactoryOps = SharedObjectFactory<T> >
class ThreadSharedObject : public SharedObject<T, true, TFactoryOps>
{
protected:
    ThreadSharedObject(MemLabelId label) : SharedObject<T, true, TFactoryOps>(label) {}
    ThreadSharedObject(const ThreadSharedObject& other) : SharedObject<T, true, TFactoryOps>(other) {}
};

template<typename T, typename TFactoryOps = SharedObjectFactory<T> >
class SingleThreadedSharedObject : public SharedObject<T, false, TFactoryOps>
{
protected:
    SingleThreadedSharedObject(MemLabelId label) : SharedObject<T, false, TFactoryOps>(label) {}
    SingleThreadedSharedObject(const SingleThreadedSharedObject& other) : SharedObject<T, false, TFactoryOps>(other) {}
};

#endif //HUAHUOENGINE_SHAREDOBJECT_H
