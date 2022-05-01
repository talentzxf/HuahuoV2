//
// Created by VincentZhang on 4/29/2022.
//

#ifndef PERSISTENTMANAGER_PPTR_H
#define PERSISTENTMANAGER_PPTR_H
#include "TypeSystem/Object.h"
#include "Utilities/Hash.h"

#define ALLOW_AUTOLOAD_PPTR_DEREFERNCE (UNITY_EDITOR || 1)

template<class T>
class PPtr
{
    InstanceID  m_InstanceID;
#if !UNITY_RELEASE
    mutable T*          m_DEBUGPtr;
#endif

public:

    inline void AssignObject(const Object* o);

    static const char* GetTypeString();
    static bool MightContainPPtr() { return true; }
    static bool AllowTransferOptimization() { return false; }

    template<class TransferFunction>
    void Transfer(TransferFunction& transfer);

    // Assignment
    explicit PPtr(InstanceID instanceID)
    {
        m_InstanceID = instanceID;
#if !UNITY_RELEASE
        m_DEBUGPtr = NULL;
#endif
    }

    PPtr(const T* o)                               { AssignObject(o); }

//    template<class U>
//    PPtr<T>(const PPtr<U>&o)
//    {
//#if UNITY_RELEASE
//        // This is here to force a compile error in case a down cast from U to T is attempted
//        T* dummy = reinterpret_cast<U*>(0);
//        UNUSED(dummy);
//#else
//        m_DEBUGPtr = o.GetDEBUGPtr();
//#endif
//
//        m_InstanceID = o.GetInstanceID();
//    }

    PPtr()
    {
#if !UNITY_RELEASE
        m_DEBUGPtr = NULL;
#endif
        m_InstanceID = InstanceID_None;
    }

    PPtr& operator=(const T* o)                   { AssignObject(o); return *this; }

    template<class U>
    PPtr& operator=(const PPtr<U>& o)
    {
#if UNITY_RELEASE
        // This is here to force a compile error in case a down cast from U to T is attempted
        T* dummy = reinterpret_cast<U*>(0);
        UNUSED(dummy);
#else
        m_DEBUGPtr = o.GetDEBUGPtr();
#endif

        m_InstanceID = o.GetInstanceID();
        return *this;
    }

    void SetInstanceID(InstanceID instanceID)      { m_InstanceID = instanceID; }
    InstanceID GetInstanceID() const               { return m_InstanceID; }

#if !UNITY_RELEASE
    T* GetDEBUGPtr() const { return m_DEBUGPtr; }
#endif

    void AssignObjectFromInstanceID(InstanceID instanceID, bool threadedLoading) { m_InstanceID = instanceID; }

    // Comparison
    bool operator<(const PPtr& p) const   { return m_InstanceID < p.m_InstanceID; }
    bool operator==(const PPtr& p) const   { return m_InstanceID == p.m_InstanceID; }
    bool operator!=(const PPtr& p) const   { return m_InstanceID != p.m_InstanceID; }

    // MSVC gets confused whether it should use operator bool(), or operator T* with implicit
    // comparison to NULL. So we add explicit functions and use them instead.
    bool IsNull() const;
    bool IsValid() const;
    bool IsEmpty() const;

    T* ForceLoadPtr() const;


    operator T*() const;
    T* operator->() const;
    T& operator*() const;
};

template<typename T> class PtrToType;
template<typename T> class PtrToType<T*>
{
public:
    typedef T value_type;
};
template<typename T> class PtrToType<const T*>
{
public:
    typedef T value_type;
};


template<class T, class U>
T dynamic_pptr_cast(U* ptr)
{
    typedef typename PtrToType<T>::value_type Type;
    T castedPtr = (T)(ptr);
    if (castedPtr && castedPtr->template Is<Type>())
        return castedPtr;
    else
        return NULL;
}

template<class T, class U>
T dynamic_pptr_cast(const PPtr<U>& ptr)
{
    U* o = ptr;
    return dynamic_pptr_cast<T>(o);
}

template<class T> inline
T dynamic_instanceID_cast(InstanceID instanceID)
{
    Object* o = PPtr<Object>(instanceID);
    return dynamic_pptr_cast<T>(o);
}

template<class T, class U>
PPtr<T> assert_pptr_cast(const PPtr<U>& ptr)
{
#if DEBUGMODE
    U* u = ptr;
    Assert(!(dynamic_pptr_cast<U*>(u) == NULL && u != NULL));
#endif
    return PPtr<T>(ptr.GetInstanceID());
}

// Enables boost::mem_fn to use PPtr properly, needed for boost::bind
template<typename T> inline T * get_pointer(PPtr<T> const & p)
{
    return p;
}

template<class T>
inline void PPtr<T>::AssignObject(const Object* o)
{
    if (o == NULL)
        m_InstanceID = InstanceID_None;
    else
        m_InstanceID = o->GetInstanceID();
#if !UNITY_RELEASE
    m_DEBUGPtr = (T*)(o);
#endif
}

template<class T> inline
T* PPtr<T>::ForceLoadPtr() const
{
    if (GetInstanceID() == InstanceID_None)
        return NULL;

    Object* temp = Object::IDToPointer(GetInstanceID());

    if (temp == NULL)
        temp = ReadObjectFromPersistentManager(GetInstanceID());

#if !UNITY_RELEASE
    m_DEBUGPtr = (T*)(temp);
#endif

#if DEBUGMODE || UNITY_EDITOR
    T* casted = dynamic_pptr_cast<T*>(temp);
    if (casted == temp)
        return casted;
    else
    {
        ErrorStringObject(Format("PPtr cast failed when dereferencing! Casting from %s to %s!", temp->GetTypeName(), TypeOf<T>()->GetName()), temp);
        return casted;
    }
#else
    return static_cast<T*>(temp);
#endif
}

template<class T> inline
PPtr<T>::operator T*() const
{
    if (GetInstanceID() == InstanceID_None)
        return NULL;

    Object* temp = Object::IDToPointer(GetInstanceID());

#if ALLOW_AUTOLOAD_PPTR_DEREFERNCE
    if (temp == NULL)
        temp = ReadObjectFromPersistentManager(GetInstanceID());
#endif

#if !UNITY_RELEASE
    m_DEBUGPtr = (T*)(temp);
#endif

#if DEBUGMODE || UNITY_EDITOR
    T* casted = dynamic_pptr_cast<T*>(temp);
    if (casted == temp)
        return casted;
    else
    {
        ErrorStringObject(Format("PPtr cast failed when dereferencing! Casting from %s to %s!", temp->GetTypeName(), TypeOf<T>()->GetName()), temp);
        return casted;
    }
#else
    return static_cast<T*>(temp);
#endif
}

template<class T> inline
T* PPtr<T>::operator->() const
{
    Object* temp = Object::IDToPointer(GetInstanceID());

#if ALLOW_AUTOLOAD_PPTR_DEREFERNCE
    if (temp == NULL)
        temp = ReadObjectFromPersistentManager(GetInstanceID());
#endif

#if !UNITY_RELEASE
    m_DEBUGPtr = (T*)(temp);
#endif

#if DEBUGMODE || UNITY_EDITOR
    T* casted = dynamic_pptr_cast<T*>(temp);
    if (casted != NULL)
        return casted;
    else
    {
        if (temp != NULL)
        {
            ErrorStringObject(Format("PPtr cast failed when dereferencing! Casting from %s to %s!", temp->GetTypeName(), TypeOf<T>()->GetName()), temp);
        }
        else
        {
            ErrorString("Dereferencing NULL PPtr!");
        }
        return casted;
    }
#else
    return static_cast<T*>(temp);
#endif
}

template<class T> inline
T& PPtr<T>::operator*() const
{
    Object* temp = Object::IDToPointer(GetInstanceID());

#if ALLOW_AUTOLOAD_PPTR_DEREFERNCE
    if (temp == NULL)
        temp = ReadObjectFromPersistentManager(GetInstanceID());
#endif

#if !UNITY_RELEASE
    m_DEBUGPtr = (T*)(temp);
#endif

#if DEBUGMODE || UNITY_EDITOR
    T* casted = dynamic_pptr_cast<T*>(temp);
    if (casted != NULL)
        return *casted;
    else
    {
        if (temp != NULL)
        {
            ErrorStringObject(Format("PPtr cast failed when dereferencing! Casting from %s to %s!", temp->GetTypeName(), TypeOf<T>()->GetName()), temp);
        }
        else
        {
            ErrorString("Dereferencing NULL PPtr!");
        }
        ANALYSIS_ASSUME(casted);
        return *casted;
    }
#else
    return *static_cast<T*>(temp);
#endif
}

template<class T> inline
bool PPtr<T>::IsNull() const
{
    T* casted = *this;
    return casted == NULL;
}

template<class T> inline
bool PPtr<T>::IsValid() const
{
    T* casted = *this;
    return casted != NULL;
}

template<class T> inline
bool PPtr<T>::IsEmpty() const
{
    return m_InstanceID == InstanceID_None;
}

template<class T> inline
const char* PPtr<T>::GetTypeString()
{
    return T::GetPPtrTypeString();
}

template<class TransferFunction>
void TransferPPtr(InstanceID& instanceId, TransferFunction& transfer)
{
    LocalSerializedObjectIdentifier localIdentifier;

    if (transfer.NeedsInstanceIDRemapping())
    {
        Assert(transfer.IsWriting() || transfer.IsReading());

        if (transfer.IsReading())
        {
            transfer.Transfer(localIdentifier.localSerializedFileIndex, "m_FileID", kHideInEditorMask | kDontAnimate);
            transfer.Transfer(localIdentifier.localIdentifierInFile, "m_PathID", kHideInEditorMask | kDontAnimate);
            LocalSerializedObjectIdentifierToInstanceID(localIdentifier, instanceId);
        }
        else if (transfer.IsWriting())
        {
            InstanceIDToLocalSerializedObjectIdentifier(instanceId, localIdentifier);
            transfer.Transfer(localIdentifier.localSerializedFileIndex, "m_FileID", kHideInEditorMask | kDontAnimate);
            transfer.Transfer(localIdentifier.localIdentifierInFile, "m_PathID", kHideInEditorMask | kDontAnimate);
        }
        else
        {
            transfer.Transfer(localIdentifier.localSerializedFileIndex, "m_FileID", kHideInEditorMask | kDontAnimate);
            transfer.Transfer(localIdentifier.localIdentifierInFile, "m_PathID", kHideInEditorMask | kDontAnimate);
        }
    }
    else
    {
        transfer.Transfer(instanceId, "m_FileID", kHideInEditorMask | kDontAnimate);
        transfer.Transfer(localIdentifier.localIdentifierInFile, "m_PathID", kHideInEditorMask | kDontAnimate);
    }
}

template<class T>
template<class TransferFunction> inline
void PPtr<T>::Transfer(TransferFunction& transfer)
{
    TransferPPtr(m_InstanceID, transfer);
}

///// This function may not be called unless you use SetObjectLockForRead  / UnlockObjectCreation
///// or SetObjectLookupReadOnly/ReleaseObjectLookupReadOnly on main thread while the job is running
//template<typename T>
//T* PPtrToObjectDontLoadLockTaken(PPtr<T> pptr)
//{
//    if (pptr.GetInstanceID() == InstanceID_None)
//        return NULL;
//
//    Object* temp = Object::IDToPointerLockTaken(pptr.GetInstanceID());
//
//#if DEBUGMODE || UNITY_EDITOR
//    T* casted = dynamic_pptr_cast<T*>(temp);
//    if (casted == temp)
//        return casted;
//    else
//    {
//        ErrorStringObject(Format("PPtr cast failed when dereferencing! Casting from %s to %s!", temp->GetTypeName(), TypeOf<T>()->GetName()), temp);
//        return casted;
//    }
//#else
//    return static_cast<T*>(temp);
//#endif
//}
//
//template<typename T>
//T* PPtrToObjectDontLoadThreadSafe(PPtr<T> pptr)
//{
//    if (pptr.GetInstanceID() == InstanceID_None)
//        return NULL;
//
//    Object* temp = Object::IDToPointerThreadSafe(pptr.GetInstanceID());
//
//#if DEBUGMODE || UNITY_EDITOR
//    T* casted = dynamic_pptr_cast<T*>(temp);
//    if (casted == temp)
//        return casted;
//    else
//    {
//        ErrorStringObject(Format("PPtr cast failed when dereferencing! Casting from %s to %s!", temp->GetTypeName(), TypeOf<T>()->GetName()), temp);
//        return casted;
//    }
//#else
//    return static_cast<T*>(temp);
//#endif
//}

template<typename T>
T* PPtrToObjectDontLoadMainThread(PPtr<T> pptr)
{
    if (pptr.GetInstanceID() == InstanceID_None)
        return NULL;

    Object* temp = Object::IDToPointer(pptr.GetInstanceID());

#if DEBUGMODE || UNITY_EDITOR
    T* casted = dynamic_pptr_cast<T*>(temp);
    if (casted == temp)
        return casted;
    else
    {
        ErrorStringObject(Format("PPtr cast failed when dereferencing! Casting from %s to %s!", temp->GetTypeName(), TypeOf<T>()->GetName()), temp);
        return casted;
    }
#else
    return static_cast<T*>(temp);
#endif
}

namespace core
{
    template<class T>
    struct hash<PPtr<T> >
    {
        UInt32 operator()(const PPtr<T>& p) const
        {
            hash<InstanceID> hasher;
            return hasher(p.GetInstanceID());
        }
    };
}

#endif //PERSISTENTMANAGER_PPTR_H
