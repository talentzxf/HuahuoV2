//
// Created by VincentZhang on 4/5/2022.
//

#ifndef HUAHUOENGINE_OBJECT_H
#define HUAHUOENGINE_OBJECT_H

#include "Internal/PlatformEnvironment.h"
#include "RTTI.h"
#include "ObjectDefines.h"
#include "Type.h"
#include "Memory/AllocatorLabels.h"
#include <unordered_map>
#include <unordered_set>
#include "Internal/CoreMacros.h"
#include "Serialize/TransferFunctions/RemapPPtrTransfer.h"
#include "Serialize/TransferFunctions/GenerateTypeTreeTransfer.h"

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


    // This is the combined flags which are used to load objects from the disk (HuaHuoEngine)
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

enum ExecutionRestrictions
{
    kNoRestriction                  = 0,
    kDisableImmediateDestruction    = 1 << 0,
    kDisableSendMessage             = 1 << 1,
    kDisableRendering               = 1 << 2,
};

void InstanceIDToLocalSerializedObjectIdentifier(InstanceID id, LocalSerializedObjectIdentifier& localIdentifier);
void LocalSerializedObjectIdentifierToInstanceID(const LocalSerializedObjectIdentifier& fileID, InstanceID& memoryID);

EXPORT_COREMODULE Object* ReadObjectFromPersistentManager(InstanceID instanceID);

ExecutionRestrictions EXPORT_COREMODULE GetExecutionRestrictions();
ExecutionRestrictions EXPORT_COREMODULE SetExecutionRestrictions(ExecutionRestrictions desiredRestrictions);

FORCE_INLINE bool GetDisableImmediateDestruction() { return (GetExecutionRestrictions() & kDisableImmediateDestruction) != 0; }
FORCE_INLINE bool GetDisableSendMessage() { return (GetExecutionRestrictions() & kDisableSendMessage) != 0; }
FORCE_INLINE bool GetDisableRendering() { return (GetExecutionRestrictions() & kDisableRendering) != 0; }

class Object {
public:
    typedef std::unordered_map<InstanceID, Object*> IDToPointerMap;

private:

    static Object* Produce(const HuaHuo::Type* targetCastType, const HuaHuo::Type* produceType, InstanceID instanceID, MemLabelId memLabel, ObjectCreationMode mode);

public:

    static Object* Produce(const HuaHuo::Type* type, InstanceID instanceID = InstanceID_None, MemLabelId memLabel = kMemBaseObject, ObjectCreationMode mode = kCreateObjectDefault)
    {
        return Produce(TypeOf<Object>(), type, instanceID, memLabel, mode);
    }

    template<typename T>
    static T* Produce(const HuaHuo::Type* type, InstanceID instanceID = InstanceID_None, MemLabelId memLabel = kMemBaseObject, ObjectCreationMode mode = kCreateObjectDefault)
    {
        return static_cast<T*>(Produce(TypeOf<T>(), type, instanceID, memLabel, mode));
    }

    template<typename T>
    static T* Produce(InstanceID instanceID = InstanceID_None, MemLabelId memLabel = kMemBaseObject, ObjectCreationMode mode = kCreateObjectDefault)
    {
        return static_cast<T*>(Produce(TypeOf<T>(), TypeOf<T>(), instanceID, memLabel, mode));
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

#if HUAHUO_EDITOR
    /// Has this object been synced with the PersistentManager
    bool IsPersistentDirty() const                     { return m_DirtyIndex != 0; }

    virtual void SetPersistentDirtyIndex(UInt32 dirtyIndex);
    void IncrementPersistentDirtyIndex();
    virtual UInt32 GetPersistentDirtyIndex();

    /// @TODO: Rename this to SetPersistentDirty
    /// Whenever variables that are being serialized in Transfer change, SetDirty () should be called
    /// This will allow tracking of objects that have changed since the last saving to disk or over the network
    void SetDirty();

    /// This method can be called if you need to unload an object from memory even if it's dirty. Editor-only!
    virtual void ClearPersistentDirty();

    // Callback support for callbacks when SetDirty is called
    typedef void ObjectDirtyCallbackFunction (Object * const * objectsToDirty, size_t count);
    static void RegisterDirtyCallback(ObjectDirtyCallbackFunction* callback);
    static ObjectDirtyCallbackFunction* GetDirtyCallback();
    static void BatchSetPersistentDirty(Object * const * objectsToDirty, size_t count);

    void SetFileIDHint(LocalIdentifierInFileType hint) { m_FileIDHint = hint; }
    LocalIdentifierInFileType GetFileIDHint() const { return m_FileIDHint; }

    virtual void SetIsPreviewSceneObject(bool isPreviewSceneObject) { m_IsPreviewSceneObject = isPreviewSceneObject; }
    bool IsPreviewSceneObject() const { return m_IsPreviewSceneObject; }

#else
    void SetDirty() {}
#endif

    typedef std::unordered_set<Object*> TypeToObjectSet;
    bool Is(const HuaHuo::Type* type) const { return type->IsBaseOf(m_CachedTypeIndex); }
    template<typename T> bool Is() const { return TypeOf<T>()->IsBaseOf(m_CachedTypeIndex); }

    virtual void Reset()
    {
        SetResetCalledInternal();
    }

    /// Smart Reset is called when "Reset" context menu is selected, when AddComponent is called and a new ScriptableObject is created,
    /// or when an AssetImporter is created from scratch.
    /// For default initialization/resetting, use Reset() instead.
    ///
    /// It is most useful for calculating values that depend o a global view, eg. adjust a collider box size by the renderers mesh bounding volume.
    /// You can traverse the hierarchy and the object is guaranteed to already be in a valid state.
    /// Assumptions you can make:
    /// * If it is a component, then it has been added to a fully initialized gameobject
    /// * All managers have been fully initialized
    /// * Always called on the main thread
    ///
    /// In SmartReset you can access others objects. The values you initialize can be indeterministic and usually depend on other objects in the scene.
    /// eg. m_HitPoints = QueryComponent(Orc) != NULL ? 100 : 20;
    virtual void SmartReset()
    {
        SetResetCalledInternal();
//#if !UNITY_RELEASE
//        m_AwakeCalled                   = 0;
//        m_AwakeThreadedCalled           = 0;
//        m_AwakeDidLoadThreadedCalled    = 0;
//#endif
    }

    bool        IsInstanceIDCreated() const                            { return m_InstanceID != InstanceID_None; }

    /// To destroy objects use delete_object instead of delete operator
    /// The default way to destroy objects is using the DestroyObject Function, which also destroys the object from it's file
    /// Must be protected by SetObjectLockForWrite  / ReleaseObjectLock
    friend void delete_object_internal_step1(Object* p);
    friend void delete_object_internal_step2(Object* p);

    Object(MemLabelId label, ObjectCreationMode mode);

    const HuaHuo::Type* GetType() const { return HuaHuo::Type::GetTypeByRuntimeTypeIndex(m_CachedTypeIndex); }

    // Helper method to get the type name of the class
    const char* GetTypeName() const { return GetType()->GetName(); }

    // Required by serialization
    virtual void VirtualRedirectTransfer(StreamedBinaryWrite&)  { AssertString(Format("Serialization not implemented for type %s", GetTypeName())); }
    virtual void VirtualRedirectTransfer(StreamedBinaryRead&)   { AssertString(Format("Serialization not implemented for type %s", GetTypeName())); }
    virtual void VirtualRedirectTransfer(RemapPPtrTransfer&)           { AssertString(Format("Serialization not implemented for type %s", GetTypeName())); }
    virtual void VirtualRedirectTransfer(GenerateTypeTreeTransfer&)    { AssertString(Format("Serialization not implemented for type %s", GetTypeName())); }

    /// Get and set the name
    virtual char const* GetName() const { return ""; }
    virtual void SetName(char const* /*name*/) {}

    inline void SetResetCalledInternal() {}
    inline void SetAwakeCalledInternal() {}
    inline void SetAwakeDidLoadThreadedCalledInternal() {}
    inline void SetMainThreadCleanupCalledInternal() {}

    /// AwakeFromLoadThreaded is called immediately after deserialization when an object is loaded from disk.
    /// It is called on the loading thread, thus NOT guaranteed to always be called.
    /// Objects created from code do not call this function, it is only called when loading from disk.
    /// Thus it is not safe to access other objects or any global state from AwakeFromLoadThreaded unless the code is thread safe
    /// A common usage case is to compute expensive data on the loading thread in order to not incur any cost on the main thread.
    virtual void AwakeFromLoadThreaded()
    {
#if !UNITY_RELEASE
        m_AwakeThreadedCalled           = 1;
        m_AwakeCalled                   = 0;
        m_AwakeDidLoadThreadedCalled    = 0;
#endif
    }

    /// Some classes need to deallocate resources on the main thread or deregister themselves from other objects,
    /// for those things you want to override MainThreadCleanup.
    /// Unless the code is thread safe, that work must be done in MainThreadCleanup as there are no guarantees about when on the other thread the destructor will be called.
    ///
    /// Every class implementing MainThreadCleanup must call Super::MainThreadCleanup().
    /// void MyClass::MainThreadCleanup()
    /// {
    ///     Super::MainThreadCleanup();
    ///     RemoveMeFromSomeGlobalLinkedList ();
    /// }
    virtual void MainThreadCleanup()
    {
        SetMainThreadCleanupCalledInternal();
    }

    /// ThreadedCleanup is called just before the object is destroyed.
    /// ThreadedCleanup may be called on the destruction thread, or in some cases on the main thread, but there is no guarantee.
    /// This functions exists to let you to do any type of cleanup that does not touch global state or other objects.
    /// It is not safe to touch global state or access other objects from ThreadedCleanup.
    /// For this reason GetInstanceID () returns 0 and asserts on access when it is accessed after MainThreadCleanup has been called.
    /// See BatchDeleteObjects.h
    /// NOTE: This function is not virtual the destructor at each level thus you shall not call the super classes ThreadedCleanup.
    void ThreadedCleanup() {}

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
         SetAwakeCalledInternal();
        if (awakeMode & kDidLoadThreaded)
        SetAwakeDidLoadThreadedCalledInternal();
    }

    // Finds the pointer to the object referenced by instanceID (NULL if none found in memory)
    /// This function may only be called from the main thread
    static Object* IDToPointer(InstanceID inInstanceID);

    /// Callback support for callbacks when an object is destroyed
    typedef void ObjectDestroyCallbackFunction (InstanceID instanceID);

    void SetIsPersistent(bool p);

    /// This function may not be called unless you use SetObjectLockForRead  / ReleaseObjectLock
    /// or SetObjectLookupReadOnly/ReleaseObjectLookupReadOnly on main thread while the job is running
    /// If you don't know 100% what you are doing use: IDToPointerThreadSafe instead
    static Object* IDToPointerLockTaken(InstanceID inInstanceID);

    /// Returns whether or not the class needs one typetree per object, not per persistentTypeID
    /// Having a per object typetree makes serialization considerably slower because safeBinaryTransfer is always used
    /// Since no TypeTree can be generated before reading the object.
    /// The File size will also increase because the typetree is not shared among the same classes.
    /// It is used for example in PythonBehaviour
    /// Also for one class you have to always returns true or always false.
    virtual bool GetNeedsPerObjectTypeTree() const { return false; }

    /// Threadsafe version that calls Lock/Unlock-ObjectCreation
    static Object* IDToPointerThreadSafe(InstanceID inInstanceID);

protected:
    virtual ~Object();
    template<class TransferFunction>
    void Transfer(TransferFunction& transfer);


private:

#if HUAHUO_EDITOR
    UInt32                    m_DirtyIndex;
    LocalIdentifierInFileType m_FileIDHint;
    bool                      m_IsPreviewSceneObject;
#endif

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

    static Object* IDToPointerInternal(InstanceID inInstanceID);

#if !UNITY_RELEASE
    PersistentTypeID      m_DEBUGPersistentTypeID;
    UInt32                m_AwakeCalled : 1;
    UInt32                m_ResetCalled : 1;
    UInt32                m_AwakeThreadedCalled : 1;
    UInt32                m_AwakeDidLoadThreadedCalled : 1;
    UInt32                m_MainThreadCleanupCalled : 1;
#endif

};

typedef void InstanceIDResolveCallback (InstanceID id, LocalSerializedObjectIdentifier& localIdentifier, void* context);
void SetInstanceIDResolveCallback(InstanceIDResolveCallback* callback, const void* context = NULL);

//Implementation
FORCE_INLINE Object* Object::IDToPointer(InstanceID instanceID)
{
#if THREADED_LOADING
DebugAssert(CurrentThread::IsMainThread());
#endif
return IDToPointerInternal(instanceID);
}

FORCE_INLINE Object* Object::IDToPointerInternal(InstanceID instanceID)
{
    if (!ms_IDToPointer)
        return NULL;

    IDToPointerMap::const_iterator i = ms_IDToPointer->find(instanceID);
    if (i != ms_IDToPointer->end())
    {
#if ENABLE_ASSET_IMPORT_REPORTING
        if (m_ObjectAccessCallback != NULL)
            m_ObjectAccessCallback(i->second);
#endif
        return i->second;
    }

    return NULL;
}

FORCE_INLINE Object* Object::IDToPointerLockTaken(InstanceID instanceID)
{
    //@TODO: Assert against this...
    //@TODO: handle 0 case specifically?
    // AssertObjectLockTaken(false);
    return IDToPointerInternal(instanceID);
}

FORCE_INLINE Object* Object::IDToPointerThreadSafe(InstanceID instanceID)
{
    // SetObjectLockForRead();
    Object* obj = IDToPointerInternal(instanceID);
    // ReleaseObjectLock();
    return obj;
}

// Destroys a Object removing from memory and disk when needed.
// Might load the object as part of destruction which is probably unwanted.
// @TODO: Refactor code to not do that
void EXPORT_COREMODULE DestroySingleObject(Object* o);
void UnloadObject(Object* o);

InstanceID AllocateNextLowestInstanceID();

void delete_object_internal(Object* p);
void delete_object_internal_step1(Object* object);
void delete_object_internal_step2(Object* object);

// Global helpers to retrieve instance IDs - see InstanceID.h for details
inline InstanceID GetInstanceIDFrom(const Object* object) { return object != NULL ? object->GetInstanceID() : InstanceID_None; }
inline InstanceID GetInstanceIDFrom(const Object& object) { return object.GetInstanceID(); }

void InstanceIDToLocalSerializedObjectIdentifier(InstanceID id, LocalSerializedObjectIdentifier& localIdentifier);

Object* PreallocateObjectFromPersistentManager(InstanceID instanceID, bool threadedLoading);
#endif //HUAHUOENGINE_OBJECT_H
