//
// Created by VincentZhang on 4/5/2022.
//

#ifndef PERSISTENTMANAGER_OBJECT_H
#define PERSISTENTMANAGER_OBJECT_H

#include "RTTI.h"
#include "ObjectDefines.h"
#include "Type.h"
#include "Memory/AllocatorLabels.h"
#include <unordered_map>
#include <unordered_set>

enum AwakeFromLoadMode
{
    // This is the default, usually called from the inspector or various serialization methods
    kDefaultAwakeFromLoad = 0,
    // The object was loaded from disk
    kDidLoadFromDisk = 1 << 0,
    // The object was loaded from a loading thread (in almost all cases through loading from disk asynchronously)
    kDidLoadThreaded = 1 << 1,
    // Object was instantiated and is now getting its first Awake function or it was created from code and gets the Awake function called
    kInstantiateOrCreateFromCodeAwakeFromLoad = 1 << 2,
    // GameObject was made active or a component was added to an active game object
    kActivateAwakeFromLoad = 1 << 3,
    // The object serialized values have been modified by the animation system.
    kAnimationAwakeFromLoad = 1 << 4,

    // This object is only loaded temporarily for the purpose of building data for assetbundles/player.
    // The object will be unloaded immediately afterwards.
    // When loading an asset for build the full maks will be: kPersistentManagerAwakeFromLoadMode | kWillUnloadAfterWritingBuildData
    kWillUnloadAfterWritingBuildData = 1 << 5,


    // This is the combined flags which are used to load objects from the disk (PersistentManager)
    kPersistentManagerAwakeFromLoadMode = kDidLoadFromDisk | kDidLoadThreaded,
    kDefaultAwakeFromLoadInvalid = -1
};
ENUM_FLAGS(AwakeFromLoadMode);

typedef SInt64 LocalIdentifierInFileType;

struct LocalSerializedObjectIdentifier
{
    SInt32 localSerializedFileIndex;
    LocalIdentifierInFileType localIdentifierInFile;

    LocalSerializedObjectIdentifier()
    {
        localIdentifierInFile = 0;
        localSerializedFileIndex = 0;
    }

    friend bool operator<(const LocalSerializedObjectIdentifier& lhs, const LocalSerializedObjectIdentifier& rhs)
    {
        if (lhs.localSerializedFileIndex != rhs.localSerializedFileIndex)
            return lhs.localSerializedFileIndex < rhs.localSerializedFileIndex;
        else
            return lhs.localIdentifierInFile < rhs.localIdentifierInFile;
    }

    friend bool operator==(const LocalSerializedObjectIdentifier& lhs, const LocalSerializedObjectIdentifier& rhs)
    {
        return lhs.localIdentifierInFile == rhs.localIdentifierInFile && lhs.localSerializedFileIndex == rhs.localSerializedFileIndex;
    }
};


void InstanceIDToLocalSerializedObjectIdentifier(InstanceID id, LocalSerializedObjectIdentifier& localIdentifier);
void LocalSerializedObjectIdentifierToInstanceID(const LocalSerializedObjectIdentifier& fileID, InstanceID& memoryID);

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

    const HuaHuo::Type* GetType() const { return HuaHuo::Type::GetTypeByRuntimeTypeIndex(m_CachedTypeIndex); }

    // Helper method to get the type name of the class
    const char* GetTypeName() const { return GetType()->GetName(); }

    // Required by serialization
    virtual void VirtualRedirectTransfer(StreamedBinaryWrite&)  { AssertString(Format("Serialization not implemented for type %s", GetTypeName())); }
    virtual void VirtualRedirectTransfer(StreamedBinaryRead&)   { AssertString(Format("Serialization not implemented for type %s", GetTypeName())); }
//    virtual void VirtualRedirectTransfer(RemapPPtrTransfer&)           { AssertString(Format("Serialization not implemented for type %s", GetTypeName())); }
//    virtual void VirtualRedirectTransfer(GenerateTypeTreeTransfer&)    { AssertString(Format("Serialization not implemented for type %s", GetTypeName())); }

    /// Get and set the name
    virtual char const* GetName() const { return ""; }
    virtual void SetName(char const* /*name*/) {}

    enum HideFlags
    {
        kHideFlagsNone = 0,

        // The object will not appear in the hierarchy and will not show up in the project view if it is stored in an asset.
        kHideInHierarchy = 1 << 0,

        // The object will be hidden in the inspector
        kHideInspector = 1 << 1,

        // The object will not be saved when saving a scene in the editor
        kDontSaveInEditor = 1 << 2,

        // The object is not editable in the inspector
        kNotEditable = 1 << 3,

        // The object will not be saved when building a player
        kDontSaveInBuild = 1 << 4,

        // The object will not be unloaded by UnloadUnusedAssets
        kDontUnloadUnusedAsset = 1 << 5,

        // The object will not be destroyed by Destroy or DestroyImmediate
        kDontAllowDestruction = 1 << 6,

        // Cannot add flags without allocating bits and updating kHideFlagsBits.

        // Common mode that is used for objects that are "owned" by a manager and are not stored anywhere.
        kHideAndDontSave                    = kDontUnloadUnusedAsset | kHideInHierarchy | kNotEditable | kDontSaveInEditor | kDontSaveInBuild,

        // A common mode used by incremental baking processes, where the data should not be stored in the scene on disk
        // But should be included in a build.
        kHideAndDontSaveButIncludeInBuild   = kDontUnloadUnusedAsset | kHideInHierarchy | kNotEditable | kDontSaveInEditor
    };
    ENUM_FLAGS_AS_MEMBER(HideFlags);

    HideFlags GetHideFlags() const { return static_cast<HideFlags>(m_HideFlags); }
    virtual void SetHideFlags(HideFlags flags) { m_HideFlags = flags; }

    /// Is this object persistent?
    bool IsPersistent() const { return m_IsPersistent; }

#if ENABLE_MEM_PROFILER
    MemLabelId GetMemoryLabel() const { return m_FullMemoryLabel; }
#else
    MemLabelId GetMemoryLabel() const { return CreateMemLabel((MemLabelIdentifier)m_MemLabelIdentifier); }
#endif
protected:
    virtual ~Object();
    template<class TransferFunction>
    void Transfer(TransferFunction& transfer);
    /// AwakeFromLoad is called on the main thread, after the data of the object has been modified from outside in a generic way.
    /// For example:
    /// * after reading from disk
    /// * reading from a binary serialized array (eg. SerializedProperty / Prefabs)
    /// * Also the animation system might patch float and bool member variables of the object directly.
    /// * After an object has been created from code
    /// * When a game object is being activated

    /// AwakeFromLoadMode can be used to differentiate the different actions.

    /// AwakeFromLoad is effectively the call where you get to sync up any cached state that can go out of sync when the data is changed underneath you.
    /// AwakeFromLoad is always called on the main thread.
    virtual void AwakeFromLoad(AwakeFromLoadMode awakeMode)
    {
        // SetAwakeCalledInternal();
        //if (awakeMode & kDidLoadThreaded)
        //SetAwakeDidLoadThreadedCalledInternal();
    }

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

    UInt32                m_MemLabelIdentifier : kMemLabelBits;
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
