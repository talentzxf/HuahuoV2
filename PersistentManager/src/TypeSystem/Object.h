//
// Created by VincentZhang on 4/5/2022.
//

#ifndef PERSISTENTMANAGER_OBJECT_H
#define PERSISTENTMANAGER_OBJECT_H

#include "RTTI.h"
#include "ObjectDefines.h"
#include "Type.h"
#include <unordered_map>
#include <unordered_set>

class Object {
public:
    typedef std::unordered_map<InstanceID, Object*> IDToPointerMap;

    template<typename T>
    static T* Produce(InstanceID instanceID = InstanceID_None, ObjectCreationMode mode = kCreateObjectDefault)
    {
        return static_cast<T*>(Produce(TypeOf<T>(), TypeOf<T>(), instanceID, mode));
    }

    template<typename T>
    static T* Produce(const HuaHuo::Type* type, InstanceID instanceID = InstanceID_None)
    {
        return static_cast<T*>(Produce(TypeOf<T>(), type, instanceID));
    }

    struct kTypeFlags {
        enum {
            value = kTypeIsAbstract
        };
    };
    typedef Object ThisType;

    static Object* CalculateCachedTypeIndex(Object* obj);

    InstanceID  GetInstanceID() const                                  { DebugAssert(m_InstanceID != InstanceID_None); return m_InstanceID; }

    // Static initialize and destroy for BaseObject
    static void StaticInitialize();
    static void StaticDestroy();

    /// Registers instance id with IDToPointerMap
    /// useful for thread loading with delayed activation from main thread
    /// Can only be called from main thead
    static void RegisterInstanceID(Object* obj);
    static void RegisterInstanceIDNoLock(Object* obj);

    /// Allocates new instanceID and registers it with IDToPointerMap
    /// Can only be called from main thead
    static Object* AllocateAndAssignInstanceID(Object* obj);
    static Object* AllocateAndAssignInstanceIDNoLock(Object* obj);

    void SetDirty() {}

    typedef std::unordered_set<Object*> TypeToObjectSet;
    bool Is(const HuaHuo::Type* type) const { return type->IsBaseOf(m_CachedTypeIndex); }

    virtual void Reset()
    {
        // SetResetCalledInternal();
    }

    bool        IsInstanceIDCreated() const                            { return m_InstanceID != InstanceID_None; }

    /// To destroy objects use delete_object instead of delete operator
    /// The default way to destroy objects is using the DestroyObject Function, which also destroys the object from it's file
    /// Must be protected by SetObjectLockForWrite  / ReleaseObjectLock
    friend void delete_object_internal_step1(Object* p);
    friend void delete_object_internal_step2(Object* p);

    Object(ObjectCreationMode mode);

    // Required by serialization
    virtual void VirtualRedirectTransfer(StreamedBinaryWrite&)  { AssertString(Format("Serialization not implemented for type %s", GetTypeName())); }
    virtual void VirtualRedirectTransfer(StreamedBinaryRead&)   { AssertString(Format("Serialization not implemented for type %s", GetTypeName())); }
//    virtual void VirtualRedirectTransfer(RemapPPtrTransfer&)           { AssertString(Format("Serialization not implemented for type %s", GetTypeName())); }
//    virtual void VirtualRedirectTransfer(GenerateTypeTreeTransfer&)    { AssertString(Format("Serialization not implemented for type %s", GetTypeName())); }
protected:
    virtual ~Object();

private:
    InstanceID              m_InstanceID;

    enum Bits
    {
        kMemLabelBits = 12,         // 12
        kTemporaryFlagsBits = 1,    // 13
        kHideFlagsBits = 7,         // 20
        kIsPersistentBits = 1,      // 21
        kCachedTypeIndexBits = 11   // 32
    };

    static const UInt32 INVALID_CACHED_TYPEINDEX = (1 << kCachedTypeIndexBits) - 1;

    // UInt32                m_MemLabelIdentifier : kMemLabelBits;
    UInt32                m_TemporaryFlags    : kTemporaryFlagsBits;
    UInt32                m_HideFlags         : kHideFlagsBits;
    UInt32                m_IsPersistent      : kIsPersistentBits;
    UInt32                m_CachedTypeIndex   : kCachedTypeIndexBits;

    virtual const HuaHuo::Type*const GetTypeVirtualInternal() const
    {
        AssertString("Object::GetTypeVirtualInternal called. GetTypeVirtualInternal should always be overriden in derived classes");
        return TypeOf<Object>();
    }
    static  IDToPointerMap* ms_IDToPointer;
    static  TypeToObjectSet* ms_TypeToObjectSet;
    void SetInstanceID(InstanceID inID)                { m_InstanceID = inID; }
    static void InsertObjectInMap(Object* obj);
    static Object* Produce(const HuaHuo::Type* targetCastType, const HuaHuo::Type* produceType, InstanceID instanceID, ObjectCreationMode mode);

};

InstanceID AllocateNextLowestInstanceID();

void delete_object_internal(Object* p);
void delete_object_internal_step1(Object* object);
void delete_object_internal_step2(Object* object);


#endif //PERSISTENTMANAGER_OBJECT_H
