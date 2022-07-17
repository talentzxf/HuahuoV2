#pragma once

#include "TypeSystem/Type.h"
#include "Serialize/TransferFunctions/TransferBase.h"
#include "Serialize/SerializationMetaFlags.h"
#include "Memory/MemoryMacros.h"
#include "Serialize/SerializeTraits.h"
//#include "Runtime/Allocator/MemoryMacros.h"
//#include "Runtime/Scripting/ScriptingTypes.h"
#include <vector>
using namespace std;

template<class T>
class PPtr;
template<class T>
class ImmediatePtr;
struct StreamingInfo;
class ScriptingObjectPtr;

class GenerateIDFunctor
{
public:

    /// Calls GenerateInstanceID for every PPtr that is found while transferring.
    /// oldInstanceID is the instanceID of the PPtr. metaFlag is the ored metaFlag of the the Transfer trace to the currently transferred pptr.
    /// After GenerateInstanceID returns, the PPtr will be set to the returned instanceID
    /// If you dont want to change the PPtr return oldInstanceID
    virtual InstanceID GenerateInstanceID(InstanceID oldInstanceID, ::TransferMetaFlags metaFlag) = 0;
};

//----------------------------------------------------------------------------------------------------------------------
// What is this: Optional functor called during a RemapPPtrTransfer for all ReferencedObject
// Motivation  : SerializedFile needs a way to collect all the types referenced inside a UnityObject.
//----------------------------------------------------------------------------------------------------------------------
class ReferencedObjectFunctor
{
public:
    virtual void ProcessReferencedObject(ScriptingObjectPtr objPtr, ::TransferMetaFlags metaFlag) = 0;
};

////@todo: Think about a smart way to calculate to let the compiler optimize transfer code of non-pptr classes away
//// currently we have  a serializeTraits::MightContainPPtr but this should be somehow automatic!


/// Transfer that scans for PPtrs and optionally allows to replace them in-place.  Is given a GenerateIDFunctor which maps one
/// or more existing instance IDs to new instance IDs and then crawls through an object's transfers looking for PPtr transfers.
/// Does not touch any other data.
class EXPORT_COREMODULE RemapPPtrTransfer : public TransferBase
{
private:

    GenerateIDFunctor*              m_GenerateIDFunctor;
    ReferencedObjectFunctor*        m_ReferencedObjectFunctor;
    UNITY_TEMP_VECTOR(::TransferMetaFlags) m_MetaMaskStack;
    ::TransferMetaFlags               m_CachedMetaMaskStackTop;
    bool                            m_ReadPPtrs;

public:

    RemapPPtrTransfer(TransferInstructionFlags flags, bool readPPtrs);

    void SetGenerateIDFunctor(GenerateIDFunctor* functor)  { m_GenerateIDFunctor = functor; }
    GenerateIDFunctor* GetGenerateIDFunctor() const { return m_GenerateIDFunctor; }

    void SetReferencedObjectFunctor(ReferencedObjectFunctor* functor) { m_ReferencedObjectFunctor = functor; }
    ReferencedObjectFunctor* GetReferencedObjectFunctor() const { return m_ReferencedObjectFunctor;}

    bool IsReadingPPtr()       { return m_ReadPPtrs; }
    bool IsWritingPPtr()       { return true; }
    bool IsRemapPPtrTransfer() { return true; }

    bool IsCloningObject()                        { return (m_Flags & kIsCloningObject) != 0; }

    bool DidReadLastPPtrProperty() { return true; }

    void AddMetaFlag(::TransferMetaFlags flag);

    void PushMetaFlag(::TransferMetaFlags flag);
    void PopMetaFlag();

    template<class T>
    void TransferBase(T& data, ::TransferMetaFlags metaFlag = kNoTransferFlags);
    template<class T>
    void Transfer(T& data, const char* name, ::TransferMetaFlags metaFlag = kNoTransferFlags);
    template<class T>
    void TransferWithTypeString(T& data, const char*, const char*, ::TransferMetaFlags metaFlag = kNoTransferFlags);

    void TransferTypeless(UInt32*, const char*, ::TransferMetaFlags /*metaFlag*/ = kNoTransferFlags) {}

    void TransferTypelessData(UInt32, void*, ::TransferMetaFlags /*metaFlag*/ = kNoTransferFlags) {}

    template<class T>
    void TransferSTLStyleArray(T& data, ::TransferMetaFlags metaFlag = kNoTransferFlags);

    template<class T>
    void TransferSTLStyleMap(T& data, ::TransferMetaFlags metaFlag = kNoTransferFlags);

    void TransferResourceImage(ActiveResourceImage targetResourceImage, const char* name, StreamingInfo& streamingInfo, void* buffer, UInt32 byteSize, InstanceID instanceID, const HuaHuo::Type* type) {}

    InstanceID GetNewInstanceIDforOldInstanceID(InstanceID oldInstanceID) { return m_GenerateIDFunctor->GenerateInstanceID(oldInstanceID, m_CachedMetaMaskStackTop); }
};

template<class T>
struct RemapPPtrTraits
{
    static bool IsManagedRefObj()                                       { return false; }
    static bool IsPPtr()                                                { return false; }
    static InstanceID GetInstanceIDFromPPtr(const T&)                   { AssertString("Never"); return InstanceID_None; }
    static void SetInstanceIDOfPPtr(T& , InstanceID)                    { AssertString("Never");  }
    static void* GetManagedReferenceObj(const T& data)                { AssertString("Never"); return 0; }
};

template<class T>
struct RemapPPtrTraits<PPtr<T> >
{
    static bool IsManagedRefObj()                                          { return false;}
    static bool IsPPtr()                                                   { return true; }
    static InstanceID GetInstanceIDFromPPtr(const PPtr<T>& data)           { return data.GetInstanceID(); }
    static void SetInstanceIDOfPPtr(PPtr<T>& data, InstanceID instanceID)  { data.SetInstanceID(instanceID);  }
    static void* GetManagedReferenceObj(const PPtr<T>& data)             { AssertString("Never"); return 0; }
};

template<class T>
struct RemapPPtrTraits<ImmediatePtr<T> >
{
    static bool IsManagedRefObj()                                                  { return false; }
    static bool IsPPtr()                                                           { return true; }
    static InstanceID GetInstanceIDFromPPtr(const ImmediatePtr<T>& data)           { return data.GetInstanceID(); }
    static void SetInstanceIDOfPPtr(ImmediatePtr<T>& data, InstanceID instanceID)  { data.SetInstanceID(instanceID);  }
    static void* GetManagedReferenceObj(const ImmediatePtr<T>& data)             { AssertString("Never"); return 0; }
};

template<class T>
void RemapPPtrTransfer::TransferBase(T& data, ::TransferMetaFlags metaFlag)
{
    Transfer(data, kTransferNameIdentifierBase, metaFlag);
}

template<class T>
void RemapPPtrTransfer::Transfer(T& data, const char*, ::TransferMetaFlags metaFlag)
{
#if UNITY_EDITOR
    if (AssetMetaDataOnly() && HasFlag(metaFlag, kIgnoreInMetaFiles))
        return;
#endif

    if (SerializeTraits<T>::MightContainPPtr())
    {
        if (metaFlag != 0)
            PushMetaFlag(metaFlag);

        if (RemapPPtrTraits<T>::IsPPtr())
        {
            Assert(m_GenerateIDFunctor != NULL);
            ANALYSIS_ASSUME(m_GenerateIDFunctor);
            InstanceID oldInstanceID = RemapPPtrTraits<T>::GetInstanceIDFromPPtr(data);
            InstanceID newInstanceID = GetNewInstanceIDforOldInstanceID(oldInstanceID);

            if (m_ReadPPtrs)
            {
                RemapPPtrTraits<T>::SetInstanceIDOfPPtr(data, newInstanceID);
            }
            else
            {
                Assert(oldInstanceID == newInstanceID);
            }
        }
//        else
//        {
//            if (m_ReferencedObjectFunctor != NULL && RemapPPtrTraits<T>::IsManagedRefObj())
//            {
//                void* ptr = RemapPPtrTraits<T>::GetManagedReferenceObj(data);
//                if (ptr)
//                {
//                    ScriptingObjectPtr anInst = *((ScriptingObjectPtr*)ptr);
//                    if (anInst != SCRIPTING_NULL)
//                        m_ReferencedObjectFunctor->ProcessReferencedObject(anInst, m_CachedMetaMaskStackTop);
//                }
//            }
//            SerializeTraits<T>::Transfer(data, *this);
//        }


        if (metaFlag != 0)
            PopMetaFlag();
    }
}

template<class T>
void RemapPPtrTransfer::TransferWithTypeString(T& data, const char*, const char*, ::TransferMetaFlags metaFlag)
{
    Transfer(data, NULL, metaFlag);
}

template<class T>
void RemapPPtrTransfer::TransferSTLStyleArray(T& data, ::TransferMetaFlags metaFlag)
{
    if (SerializeTraits<typename T::value_type>::MightContainPPtr())
    {
        typename T::iterator i = data.begin();
        typename T::iterator end = data.end();
        while (i != end)
        {
            Transfer(*i, "data", metaFlag);
            ++i;
        }
    }
}

template<class T>
void RemapPPtrTransfer::TransferSTLStyleMap(T& data, ::TransferMetaFlags metaFlag)
{
    typedef typename NonConstContainerValueType<T>::value_type NonConstT;
    if (SerializeTraits<NonConstT>::MightContainPPtr())
    {
        typename T::iterator i = data.begin();
        typename T::iterator end = data.end();

        // maps value_type is: pair<const First, Second>
        // So we have to write to maps non-const value type
        while (i != end)
        {
            NonConstT& p = (NonConstT&)(*i);
            Transfer(p, "data", metaFlag);
            i++;
        }
    }
}
