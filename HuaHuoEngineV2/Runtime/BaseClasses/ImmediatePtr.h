//
// Created by VincentZhang on 4/24/2022.
//

#ifndef HUAHUOENGINE_IMMEDIATEPTR_H
#define HUAHUOENGINE_IMMEDIATEPTR_H

#include "TypeSystem/Object.h"
#include "Utilities/Annotations.h"

template<class T>
class ImmediatePtr
{
    T* m_Ptr;

public:

    static const char* GetTypeString();
    static bool MightContainPPtr()                         { return true; }
    static bool AllowTransferOptimization()                { return false; }

    template<class TransferFunction>
    void Transfer(TransferFunction& transfer);

    // Assignment
    ImmediatePtr(T* o)                                     { m_Ptr = o;  }
    ImmediatePtr(const ImmediatePtr<T>& o)                 { m_Ptr = o.m_Ptr; }
    ImmediatePtr()                                         { m_Ptr = 0; }

    void operator=(T* o)                                  { m_Ptr = o; }
    void operator=(const ImmediatePtr<T>& o)              { m_Ptr = o.m_Ptr; }

    void SetInstanceID(InstanceID instanceID)              { AssignObjectFromInstanceID(instanceID, false); }

    InstanceID GetInstanceID() const
    {
        if (m_Ptr != NULL)
        {
            InstanceID instanceID = m_Ptr->GetInstanceID();
            Assert(!(InstanceID_AsSInt32Ref(instanceID) & 1));
            return instanceID;
        }

        return InstanceID_None;
    }

    void AssignObjectFromInstanceID(InstanceID instanceID, bool threadedLoading)
    {
        Assert(!(InstanceID_AsSInt32Ref(instanceID) & 1));

        T* obj = dynamic_pptr_cast<T*>(PreallocateObjectFromPersistentManager(instanceID, threadedLoading));
        m_Ptr = obj;
    }

    bool operator==(const T* p) const  { return m_Ptr == p; }
    bool operator!=(const T* p) const  { return m_Ptr != p; }

    operator T*() const    { return m_Ptr; }
    T* operator->() const { T* o = m_Ptr; Assert(o != NULL); return o; }
    T& operator*() const  { T* o = m_Ptr; Assert(o != NULL); ANALYSIS_ASSUME(o); return *o; }
};

template<class T> inline
const char* ImmediatePtr<T>::GetTypeString()
{
    return T::GetPPtrTypeString();
}

template<class T>
template<class TransferFunction> inline
void ImmediatePtr<T>::Transfer(TransferFunction& transfer)
{
    LocalSerializedObjectIdentifier localIdentifier;

    if (transfer.NeedsInstanceIDRemapping())
    {
        Assert(transfer.IsWriting() || transfer.IsReading());
        bool threadedLoading = transfer.GetFlags() & kThreadedSerialization;

        if (transfer.IsReading())
        {
            transfer.Transfer(localIdentifier.localSerializedFileIndex, "m_FileID", kHideInEditorMask | kDontAnimate);
            transfer.Transfer(localIdentifier.localIdentifierInFile, "m_PathID", kHideInEditorMask | kDontAnimate);
            InstanceID instanceID;
            LocalSerializedObjectIdentifierToInstanceID(localIdentifier, instanceID);
            AssignObjectFromInstanceID(instanceID, threadedLoading);
        }
        else if (transfer.IsWriting())
        {
            InstanceIDToLocalSerializedObjectIdentifier(GetInstanceID(), localIdentifier);
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
        if (transfer.IsReading())
        {
            transfer.Transfer(localIdentifier.localSerializedFileIndex, "m_FileID", kHideInEditorMask);
            transfer.Transfer(localIdentifier.localIdentifierInFile, "m_PathID", kHideInEditorMask);
            AssignObjectFromInstanceID(InstanceID_Make(localIdentifier.localSerializedFileIndex), false);
        }
        else if (transfer.IsWriting())
        {
            localIdentifier.localSerializedFileIndex = InstanceID_AsSInt32Ref(GetInstanceID());
            localIdentifier.localIdentifierInFile = 0;
            transfer.Transfer(localIdentifier.localSerializedFileIndex, "m_FileID", kHideInEditorMask);
            transfer.Transfer(localIdentifier.localIdentifierInFile, "m_PathID", kHideInEditorMask);
            Assert(localIdentifier.localSerializedFileIndex == InstanceID_AsSInt32Ref(GetInstanceID()));
        }
        else
        {
            transfer.Transfer(localIdentifier.localSerializedFileIndex, "m_FileID", kHideInEditorMask);
            transfer.Transfer(localIdentifier.localIdentifierInFile, "m_PathID", kHideInEditorMask);
        }
    }
}
#endif //HUAHUOENGINE_IMMEDIATEPTR_H
