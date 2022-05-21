#pragma once

#include "Memory/AllocatorLabels.h"
#include <cstdlib>
#include <utility>

// core::SharedObjectPtr - A pointer that automatically adds and releases it's ref count on a (Threaded)SharedObject,
// thus reducing the potential for errors. Destruction of the shared object is automatically carried out when there are
// no remaining SharedObjectPtrs to it.  SharedObjectPtrs can be copied, moved, passed into functions by value or by
// reference, stored in dynamic_arrays, etc. Note that the automatic ref counting is thread safe whenever the
// SharedObject is.
// See also: Runtime/Core/SharedObject.h and Runtime/Core/ThreadSharedObjectPool.h

// A SharedObjectPtr can only be initialised on a class derived from a ThreadSharedObject or SingleThreadedSharedObject.
// Otherwise, a compile time error will be thrown.

// Initial creation of the SharedObject and SharedObjectPtr should be achieved via
// core::SharedObjectPtr<T> = core::CreateShared(MemLabel, ... args), where ... args is a list of construction args for the
// class T.  On default construction or construction with a nullptr, no object is owned and the ref count is 0. On copy
// and assignment, the ref count is increased by 1, and on move it is unaffected.

// The following functions are provided as an interface to the old API of manual ref counting and should only be used on
// such occasions:
//      SharedObjectPtr(T* ptr);
//      T* GetAndAddRef();
//      void SetTo(T* ptr);

// Important Note:
// core::SharedObjectPtr ptr(UNITY_NEW(...)) and ptr.Assign(UNITY_NEW(T, MemLabel)(MemLabel, TArgs)) will lead to
// incorrect reference counting, and thus memory leaks.  See documentation for further info.

namespace core
{
    template<typename T>
    class SharedObjectPtr;

    template<typename T>
    class SharedObjectPtr<const T>
    {
    public:
        // Throws an informative compiler time error to tell the developer they need to use SharedObjectPtr with a
        // template T that is derived from SharedObject, if they do not do so.
        typename T::SharedObjectDerived All_SharedObjectPtrs_must_be_intialised_on_a_class_derived_from_SharedObject;

        SharedObjectPtr() : m_Ptr(NULL) {}

        explicit SharedObjectPtr(const T* ptr) : m_Ptr(ptr) { AddRef(); }

        SharedObjectPtr(const SharedObjectPtr& other)
        {
            m_Ptr = other.m_Ptr;
            AddRef();
        }

        SharedObjectPtr& operator=(const SharedObjectPtr& other)
        {
            Release();
            m_Ptr = other.m_Ptr;
            AddRef();

            return *this;
        }

        SharedObjectPtr(SharedObjectPtr&& other)
        {
            m_Ptr = other.m_Ptr;
            other.m_Ptr = NULL;
        }

        SharedObjectPtr& operator=(SharedObjectPtr&& other)
        {
            if (this != &other)
            {
                Release();
                m_Ptr = other.m_Ptr;
                other.m_Ptr = NULL;
            }
            return *this;
        }

        ~SharedObjectPtr() { Release(); }

        int GetRefCount()
        {
            if (m_Ptr)
                return m_Ptr->GetRefCount();

            return 0;
        }

    protected:

        void AddRef()
        {
            if (m_Ptr)
                m_Ptr->AddRef();
        }

        void Release()
        {
            if (m_Ptr)
                m_Ptr->Release();
        }

    public:

        const T* Get() const { return m_Ptr; }

        MemLabelId GetMemLabel() const { return m_Ptr->GetMemLabel(); }

        const T& operator*() const
        {
            AssertMsg(m_Ptr, "Dereferencing a NULL pointer to data in a SharedObjectPtr");
            return *m_Ptr;
        }

        const T* operator->() const
        {
            AssertMsg(m_Ptr, "Using class member access operator (->) on a null SharedObjectPtr");
            return m_Ptr;
        }

        const T* GetPtrAndAddRef() { AddRef(); return m_Ptr; }

        void Clear() { Release(); m_Ptr = NULL; }

        operator bool() const { return m_Ptr != NULL; }

        void Assign(const T* ptr) { Release(); m_Ptr = ptr; AddRef(); }

        template<typename U>
        bool operator==(const SharedObjectPtr<U>& rhs) const
        {
            return m_Ptr == rhs.m_Ptr;
        }

        template<typename U>
        bool operator!=(const SharedObjectPtr<U>& rhs) const
        {
            return m_Ptr != rhs.m_Ptr;
        }

    protected:
        const T* m_Ptr;
    };

    template<typename U>
    class SharedObjectPtr : public SharedObjectPtr<const U>
    {
        template<typename V>
        friend class core::SharedObjectPtr;


    private:
        typedef SharedObjectPtr<const U> base;
        using base::m_Ptr;
        using base::AddRef;
        using base::Release;
    public:

        SharedObjectPtr() {}

        explicit SharedObjectPtr(U* ptr) : base(ptr) {}

        SharedObjectPtr(const SharedObjectPtr& other) : base(other) {}

        SharedObjectPtr& operator=(const SharedObjectPtr& other)
        {
            base::operator=(other);
            return *this;
        }

        SharedObjectPtr(SharedObjectPtr&& other) : base(std::move(other))
        {
        }

        SharedObjectPtr& operator=(SharedObjectPtr&& other)
        {
            base::operator=(std::move(other));
            return *this;
        }

    public:
        U* Get() const { return const_cast<U*>(m_Ptr); }
        void Assign(U* ptr) { Release(); m_Ptr = ptr; AddRef(); }


        U& operator*() const
        {
            AssertMsg(m_Ptr, "Dereferencing a NULL pointer to data in a SharedObjectPtr");
            return *(Get());
        }

        U* operator->() const
        {
            AssertMsg(m_Ptr, "Using class member access operator (->) on a null SharedObjectPtr");
            return Get();
        }

        U* GetPtrAndAddRef() { AddRef(); return Get(); }
    };


    template<typename T, typename ... TArgs>
    core::SharedObjectPtr<T> CreateShared(const MemLabelId& memLabelId, TArgs&& ... args)
    {
        return T::TObjectFactoryOps::CreateSharedObject(memLabelId, std::forward<TArgs>(args)...);
    }

    template<typename T>
    core::SharedObjectPtr<T> CopyShared(const core::SharedObjectPtr<const T>& other)
    {
        return T::TObjectFactoryOps::CopySharedObject(*other.Get(), other.GetMemLabel());
    }

    template<typename T>
    core::SharedObjectPtr<T> CopyShared(const core::SharedObjectPtr<const T>& other, MemLabelId label)
    {
        return T::TObjectFactoryOps::CopySharedObject(other, label);
    }
}
