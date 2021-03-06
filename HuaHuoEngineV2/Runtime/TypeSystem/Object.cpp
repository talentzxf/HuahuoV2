//
// Created by VincentZhang on 4/5/2022.
//

#include "Object.h"
#include "Type.h"
#include "Threads/AtomicOps.h"
#include "TypeManager.h"
#include <cstring>
#include "Utilities/RegisterRuntimeInitializeAndCleanup.h"
#include "Serialize/PersistentManager.h"
#include "BaseClasses/PPtr.h"

Object::IDToPointerMap*    Object::ms_IDToPointer = NULL;
Object::TypeToObjectSet*   Object::ms_TypeToObjectSet = NULL;

static Object::ObjectDestroyCallbackFunction* gDestroyedCallbackFunc = NULL;

static MemLabelRootId* gBaseObjectManagerContainer = NULL;

static volatile size_t gLowestInstanceID = -10;

namespace BaseObjectManager
{
    void StaticInitialize(void*)
    {
        gBaseObjectManagerContainer = HUAHUO_NEW_AS_ROOT(MemLabelRootId, kMemBaseObject, "Managers", "BaseObjectManager") ();
        TypeManager::InitializeGlobalInstance();
        Object::StaticInitialize();
    }

    void StaticDestroy(void*)
    {
        Object::StaticDestroy();
        TypeManager::CleanupGlobalInstance();
        HUAHUO_DELETE(gBaseObjectManagerContainer, kMemBaseObject);
    }
}

static ExecutionRestrictions gExecutionRestrictions = kNoRestriction;

ExecutionRestrictions GetExecutionRestrictions()
{
    return gExecutionRestrictions;
}

ExecutionRestrictions SetExecutionRestrictions(ExecutionRestrictions desiredRestrictions)
{
    // ASSERT_RUNNING_ON_MAIN_THREAD;

    ExecutionRestrictions originalRestrictions = gExecutionRestrictions;
    gExecutionRestrictions = desiredRestrictions;
    return originalRestrictions;
}

Object* ReadObjectFromPersistentManager(InstanceID instanceID)
{
    return NULL;
}

static RegisterRuntimeInitializeAndCleanup s_BaseObjectManagerCallbacks(BaseObjectManager::StaticInitialize, BaseObjectManager::StaticDestroy);

static int GetIdToPointerMapInitialBucketSize()
{
    // As of May 2018, 32k buckets occupy ~1.5MB on 64-bit systems
    return 32 * 1024;
}

Object::Object(MemLabelId label, ObjectCreationMode mode)
{
    Assert((long)GetLabelIdentifier(label) < (long)(1 << kMemLabelBits));
    m_MemLabelIdentifier = GetLabelIdentifier(label);

    m_InstanceID = InstanceID_None;
    m_CachedTypeIndex = INVALID_CACHED_TYPEINDEX;

    m_HideFlags = 0;
    m_TemporaryFlags = 0;
    m_IsPersistent = false;
}

void Object::StaticInitialize()
{
    // Object::ms_IDToPointer = HUAHUO_NEW(Object::IDToPointerMap, gBaseObjectManagerContainer->rootLabel) (GetIdToPointerMapInitialBucketSize(), gBaseObjectManagerContainer->rootLabel);
    Object::ms_IDToPointer = HUAHUO_NEW(Object::IDToPointerMap, kMemDefault);

    void* memory = HUAHUO_MALLOC(gBaseObjectManagerContainer->rootLabel, sizeof(Object::TypeToObjectSet) * RTTI::RuntimeTypeArray::MAX_RUNTIME_TYPES);
    Object::ms_TypeToObjectSet = AutoLabelConstructor<Object::TypeToObjectSet>::construct_n(memory, RTTI::RuntimeTypeArray::MAX_RUNTIME_TYPES, gBaseObjectManagerContainer->rootLabel);

    TypeRegistrationDesc desc;
    memset(&desc, 0, sizeof(TypeRegistrationDesc));

    desc.init.className = "Object";
    desc.init.classNamespace = "";
    desc.init.module = "Core";
    desc.init.persistentTypeID = 0;
    desc.init.size = sizeof(Object);
    desc.init.derivedFromInfo.typeIndex = RTTI::DefaultTypeIndex;
    desc.init.derivedFromInfo.descendantCount = RTTI::DefaultDescendentCount;
    desc.init.isAbstract = true;

    desc.type = &TypeContainer<Object>::rtti;

    TypeManager::Get().RegisterType(desc);
}

void Object::StaticDestroy() {
    delete Object::ms_IDToPointer;
    delete Object::ms_TypeToObjectSet;
}

InstanceID AllocateNextLowestInstanceID()
{
    // __FAKEABLE_FUNCTION__(AllocateNextLowestInstanceID, ());

    size_t newID = AtomicAdd(&gLowestInstanceID, -2);
    return InstanceID_Make((SInt32)newID);
}

void Object::InsertObjectInMap(Object* obj)
{
    // AssertObjectLockTaken(true);
    InstanceID instanceID = obj->GetInstanceID();

    bool inserted = ms_IDToPointer->insert(std::pair<InstanceID, Object*>(instanceID, obj)).second;
    // AssertMsg(inserted, "Unexpected: object already in ms_IDToPointer map.");

    inserted = ms_TypeToObjectSet[obj->m_CachedTypeIndex].insert(obj).second;
    // AssertMsg(inserted, "Unexpected: object already in ms_TypeToObjectSet.");

    // PROFILER_REGISTER_OBJECT(obj);
}

// TODO
void Object::RegisterInstanceID(Object* obj)
{
    RegisterInstanceIDNoLock(obj);
//    CHECK_IN_MAIN_THREAD;
//
//    SetObjectLockForWrite();
//    Assert(obj != NULL);
//    Assert(obj->m_InstanceID != InstanceID_None);
//    InsertObjectInMap(obj);
//
//    ReleaseObjectLock();
}

void Object::RegisterInstanceIDNoLock(Object* obj)
{
    // CHECK_IN_MAIN_THREAD;
    Assert(obj != NULL);
    Assert(obj->m_InstanceID != InstanceID_None);
    CalculateCachedTypeIndex(obj);
    InsertObjectInMap(obj);
}

Object::~Object() {
}

Object* Object::CalculateCachedTypeIndex(Object* obj)
{
    Assert(obj->GetTypeVirtualInternal()->GetRuntimeTypeIndex() < (1 << kCachedTypeIndexBits));
    obj->m_CachedTypeIndex = obj->GetTypeVirtualInternal()->GetRuntimeTypeIndex();
    return obj;
}

Object* Object::AllocateAndAssignInstanceID(Object* obj)
{
//    // CHECK_IN_MAIN_THREAD;
//    Assert(obj->m_InstanceID == InstanceID_None);
//
//    // SetObjectLockForWrite();
//
//    // Create a new unique instanceID for this Object.
//    // The created id will be negative beginning with -1
//    // Ids loaded from a file will be positive beginning with 1
//    obj->SetInstanceID(AllocateNextLowestInstanceID());
//    Assert((InstanceID_AsSInt32Ref(obj->GetInstanceID()) & 1) == 0);
//
//    CalculateCachedTypeIndex(obj);
//    InsertObjectInMap(obj);
//
//    ReleaseObjectLock();
//
//    obj->SetDirty();

    return AllocateAndAssignInstanceIDNoLock(obj);
}

Object* Object::AllocateAndAssignInstanceIDNoLock(Object* obj)
{
    // CHECK_IN_MAIN_THREAD;
    Assert(obj->m_InstanceID == InstanceID_None);

    // Create a new unique instanceID for this Object.
    // The created id will be negative beginning with -1
    // Ids loaded from a file will be positive beginning with 1
    obj->SetInstanceID(AllocateNextLowestInstanceID());
    Assert((InstanceID_AsSInt32Ref(obj->GetInstanceID()) & 1) == 0);

    CalculateCachedTypeIndex(obj);

    InsertObjectInMap(obj);

    obj->SetDirty();

    return obj;
}

// This must be executed on the main thread
void delete_object_internal_step1(Object* object)
{
//#if SHOW_DELETED_OBJECTS
//    if (object->GetType() != TypeOf<Mesh>())        // GUI creates lots of meshes during runtime, ignore those
//    {
//        const char* n = object->GetName();
//        if (n == NULL)
//            n = "<NULL>";
//        printf_console("Deleting %s (%s).\n", n, object->GetTypeName());
//    }
//#endif
//
//
//    PROFILER_UNREGISTER_OBJECT(object);
//
//#if !UNITY_RELEASE
//    // Explicitly allow destroying objects without Awake() having been called.
//    object->CheckCorrectAwakeUsage(false);
//#endif
//
//    // Send destroy message & clear event index
//    if (object->m_EventIndex != NULL)
//    {
//        GetEventManager().InvokeEvent(object->m_EventIndex, object, kWillDestroyEvent);
//        GetEventManager().RemoveEvent(object->m_EventIndex);
//        object->m_EventIndex = NULL;
//    }
//
//    SetObjectLockForWrite();
    // Remove this objects instanceID from the table.
    const InstanceID instanceID = object->GetInstanceID();
    bool erased = Object::ms_IDToPointer->erase(instanceID) > 0;
    AssertMsg(erased, "Failed to erase object from ms_IDToPointer. It was not part of the set.");

    erased = Object::ms_TypeToObjectSet[object->m_CachedTypeIndex].erase(object) > 0;
    AssertMsg(erased, "Failed to erase object from ms_TypeToObjectSet. It was not part of the set");
//    ReleaseObjectLock();
//
//    if (gDestroyedCallbackFunc)
//        gDestroyedCallbackFunc(instanceID);
//
//    object->MainThreadCleanup();
//
    object->m_InstanceID = InstanceID_None;
//
//    if (GetMonoManagerPtr() && object->GetGCHandle().HasTarget())
//        object->SetCachedScriptingObject(SCRIPTING_NULL);
}

void delete_object_internal(Object* p)
{
    if (!p)
        return;

    delete_object_internal_step1(p);
    delete_object_internal_step2(p);
}

// This can be execute on any thread.
void delete_object_internal_step2(Object* p)
{
    delete p;
}

void DestroySingleObject(Object* o)
{
    if (o == NULL)
        return;

    // Objects that were allocated with NEW_OBJECT_FROM_THREAD() do not have a valid instanceID.
    // They don't have tentacles in other systems (e.g. EventManager, HuaHuoEngine, MemoryProfilerStats),
    // so can just be deleted on any thread and skip deletion preparations.
    if (!o->IsInstanceIDCreated())
    {
        // o->SetMainThreadCleanupCalledInternal();
        delete_object_internal_step2(o);
        return;
    }

//    if (o->IsPersistent())
//        GetPersistentManager().MakeObjectUnpersistent(o->GetInstanceID(), kDestroyFromFile);

    delete_object_internal(o);
}

Object* Object::Produce(const HuaHuo::Type* targetCastType, const HuaHuo::Type* produceType, InstanceID instanceID, MemLabelId memLabel, ObjectCreationMode mode)
{
#if ENABLE_ASSERTIONS
    switch (mode)
    {
        case kCreateObjectDefault:
        case kCreateObjectDefaultNoLock:

        #if THREADED_LOADING
            AssertMsg(CurrentThread::EqualsID(GetPersistentManager().GetMainThreadID()),
                "Cannot create on non-main thread without kCreateObjectFromNonMainThread");
        #endif

            if (mode == kCreateObjectDefault)
                AssertMsg(IDToPointer(instanceID) == NULL, "Object is already loaded");

            break;

        case kCreateObjectFromNonMainThread:
            AssertMsg(instanceID != InstanceID_None, "Cannot create on non-main thread without an instanceID");
            break;

        default:
            AssertMsg(false, "Unknown case");
            break;
    }
#endif // ENABLE_ASSERTIONS

    // @joe says "not sure exactly why we did this. probably not necessary anymore but all our instanceID allocations are increment in even numbers." (scobi 12-jun-17)
    AssertMsg((InstanceID_AsSInt32Ref(instanceID) & 1) == 0, "Bit 0 of InstanceID is reserved");

    if (produceType == NULL)
        return NULL;

    HuaHuo::Type::FactoryFunction* factory = produceType->GetFactory();
    if (factory == NULL)
        return NULL;

    Object* newObject = factory(memLabel, mode);
    if (newObject == NULL)
        return NULL;

    if (instanceID != InstanceID_None)
    {
        newObject->SetInstanceID(instanceID);
        CalculateCachedTypeIndex(newObject);

        if (mode == kCreateObjectDefault)
        {
            RegisterInstanceID(newObject);
            newObject->SetDirty();
        }
        else if (mode == kCreateObjectDefaultNoLock)
        {
            RegisterInstanceIDNoLock(newObject);
            newObject->SetDirty();
        }
    }
    else
    {
        if (mode == kCreateObjectDefaultNoLock)
            AllocateAndAssignInstanceIDNoLock(newObject);
        else
            AllocateAndAssignInstanceID(newObject);
    }

    // we could do this targetCastType test against produceType, but then we'd run into problems with types
    // not existing (whether by stripping or other means, such as deprecation or fwd/back compat serialization)
    // and we'd be giving potential false positives with error messages. so instead we ask the produced
    // instance its actual type, and test against that.
    if (!newObject->Is(targetCastType))
    {
        AssertString(Format("Invalid conversion of runtime type %s to static type %s",
                            newObject->GetType()->GetName(), targetCastType->GetName()));

        newObject->Reset();
        DestroySingleObject(newObject);
        newObject = NULL;
    }

    return newObject;
}

template<class TransferFunction>
void Object::Transfer(TransferFunction& transfer)
{
#if UNITY_EDITOR
    if (!transfer.IsSerializingForGameRelease() && SerializePrefabIgnoreProperties(transfer) && !transfer.GetBuildUsage().strippedPrefabObject)
    {
        UInt32 flags = m_HideFlags;
        transfer.Transfer(flags, "m_ObjectHideFlags", kHideInEditorMask);
        m_HideFlags = flags;
    }

    if (transfer.GetFlags() & kSerializeDebugProperties)
    {
        InstanceID instanceID = GetInstanceID();
        transfer.Transfer(instanceID, "m_InstanceID");

        LocalIdentifierInFileType fileID;
        if (IsPersistent())
            fileID = GetPersistentManager().GetLocalFileID(instanceID);
        else
            fileID = GetFileIDHint();

        transfer.Transfer(fileID, "m_LocalIdentfierInFile");
    }
#endif
}


//void InstanceIDToLocalSerializedObjectIdentifier(InstanceID id, LocalSerializedObjectIdentifier& localIdentifier)
//{
//    HuaHuoEngine::getInstance()->InstanceIDToLocalSerializedObjectIdentifier(id, localIdentifier);
//}

void LocalSerializedObjectIdentifierToInstanceID(const LocalSerializedObjectIdentifier& fileID, InstanceID& memoryID)
{
    GetPersistentManager().LocalSerializedObjectIdentifierToInstanceID(fileID, memoryID);
}

Object* PreallocateObjectFromPersistentManager(InstanceID instanceID, bool threadedLoading)
{
    Object* obj = NULL;

    if (!threadedLoading)
    {
        obj = PPtr<Object>(instanceID);
    }
    else
    {
        obj = GetPersistentManager().PreallocateObjectThreaded(instanceID);
    }

    return obj;
}

void Object::SetPersistentDirtyIndex(UInt32 dirtyValue)
{
    m_DirtyIndex = dirtyValue;

//    if (gDiagObjectSetDirty && !TestHideFlag(kDontSaveInEditor))
//        LogStringMsg("Set Dirty(%d) : [%d] %s (%s)", m_DirtyIndex, GetInstanceID(), GetName(), GetTypeName());
}

UInt32 Object::GetPersistentDirtyIndex()
{
    return m_DirtyIndex;
}

void Object::IncrementPersistentDirtyIndex()
{
    // When we run out of dirty indices, make sure it stays at 1
    m_DirtyIndex++;
    if (m_DirtyIndex == 0)
        m_DirtyIndex = 1;

//    if (gDiagObjectSetDirty && !TestHideFlag(kDontSaveInEditor))
//        LogStringMsg("Increment Dirty(%d) : [%d] %s (%s)", m_DirtyIndex, GetInstanceID(), GetName(), GetTypeName());
}

void Object::SetDirty()
{
    IncrementPersistentDirtyIndex();

//    if (gSetDirtyCallbackFunc)
//    {
//        Object* objectsToDirty[1] = {this};
//        gSetDirtyCallbackFunc(objectsToDirty, 1);
//    }
//
//#if !UNITY_RELEASE
//    m_DEBUGPersistentTypeID = GetType()->GetPersistentTypeID();
//#endif
}

void SetObjectLockForWrite()
{
#if THREADED_LOADING
    if (gCreateObjectLockRecursionCount == 0)
    {
        gCreateObjectLock.WriteLock();
        gObjectLockedForWrite = true;
    }
    else
    {
        if (!gObjectLockedForWrite)
            ErrorStringMsg("object lock does not support promotion from readlock to writelock");
    }
    ++gCreateObjectLockRecursionCount;
#endif
}

void ReleaseObjectLock()
{
#if THREADED_LOADING
    --gCreateObjectLockRecursionCount;
    if (gCreateObjectLockRecursionCount == 0)
    {
        if (gObjectLockedForWrite)
        {
            gObjectLockedForWrite = false;
            gCreateObjectLock.WriteUnlock();
        }
        else if (!CurrentThread::IsMainThread())
            gCreateObjectLock.ReadUnlock();
    }
#endif
}

void UnloadObject(Object* p)
{
    if (!p)
        return;

    SetObjectLockForWrite();
    delete_object_internal(p);
    ReleaseObjectLock();
}

#if HUAHUO_EDITOR
InstanceIDResolveCallback* gInstanceIDResolveCallback = NULL;
const void* gInstanceIDResolveContext = NULL;

void SetInstanceIDResolveCallback(InstanceIDResolveCallback callback, const void* context)
{
    gInstanceIDResolveCallback = callback;
    gInstanceIDResolveContext = context;
}

#endif

void InstanceIDToLocalSerializedObjectIdentifier(InstanceID id, LocalSerializedObjectIdentifier& localIdentifier)
{
#if UNITY_EDITOR
    // Early out if referenced object is null
    if (id == InstanceID_None)
    {
        localIdentifier.localSerializedFileIndex = 0;
        localIdentifier.localIdentifierInFile = 0;
        return;
    }

    if (gInstanceIDResolveCallback == NULL)
    {
        GetPersistentManager().InstanceIDToLocalSerializedObjectIdentifier(id, localIdentifier);
        return;
    }
    else
    {
        gInstanceIDResolveCallback(id, localIdentifier, const_cast<void*>(gInstanceIDResolveContext));
    }
#else
    GetPersistentManager().InstanceIDToLocalSerializedObjectIdentifier(id, localIdentifier);
#endif
}

void Object::ClearPersistentDirty()
{
    m_DirtyIndex = 0;

//    if (gDiagObjectSetDirty && !TestHideFlag(kDontSaveInEditor))
//        LogStringMsg("Clear Dirty(%d) : [%d] %s (%s)", m_DirtyIndex, GetInstanceID(), GetName(), GetTypeName());
}

void Object::SetIsPersistent(bool p)
{
    // PROFILER_CHANGE_PERSISTENCE(GetInstanceID(), m_IsPersistent, p);
    m_IsPersistent = p;
}

INSTANTIATE_TEMPLATE_TRANSFER(Object);