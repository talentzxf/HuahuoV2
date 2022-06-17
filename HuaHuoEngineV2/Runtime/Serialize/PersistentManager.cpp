//
// Created by VincentZhang on 5/1/2022.
//

#include "PersistentManager.h"
#include "Remapper.h"
#include "SerializedFile.h"
#include "Utilities/File.h"
#include "Serialize/SerializationCaching/FileCacherWrite.h"
#include "Serialize/SerializationCaching/MemoryCacheWriter.h"
#include "ObjectStore.h"
#include "Serialize/SerializationCaching/BlockMemoryCacheWriter.h"
#include "WriteData.h"
#include "Core/Finalizer.h"
#include "Serialize/SerializationCaching/FileCacherRead.h"
#include "AwakeFromLoadQueue.h"

#if DEBUGMODE
#define CheckedAssert(x) Assert(x)
#else
#define CheckedAssert(x)
#endif

#if HUAHUO_EDITOR || SUPPORT_RESOURCE_IMAGE_LOADING
static const char* kResourceImageExtensions[] = { "resG", "res", "resS" };
#endif



#if HUAHUO_EDITOR
class AutoResetInstanceIDResolver : NonCopyable
{
public:
    AutoResetInstanceIDResolver() : m_WasSet(false) {}
    ~AutoResetInstanceIDResolver()
    {
        if (m_WasSet)
            SetInstanceIDResolveCallback(NULL);
    }

    void Set(const InstanceIDResolver& resolver)
    {
        DebugAssert(!m_WasSet);
        SetInstanceIDResolveCallback(resolver.callback, resolver.context);
        m_WasSet = true;
    }

private:
    bool m_WasSet;
};

#endif

class AutoFileCacherReadOverride
{
public:
    AutoFileCacherReadOverride(SerializedFile *sf) : m_SerializedFile(sf)
    {
#if SUPPORT_THREADS && !PLATFORM_WEBGL
        // Because we won't be seeking around the file, we can use a larger cache and can also prefetch
        m_PrevReader = m_SerializedFile->GetCacheReader();
        m_OverrideReader = UNITY_NEW(FileCacherRead, kMemTempAlloc)(kMemTempAlloc, m_PrevReader->GetPathName(), 1 * 1024 * 1024, true);
        m_SerializedFile->SetCacheReader(m_OverrideReader);
#endif
    }

    ~AutoFileCacherReadOverride()
    {
#if SUPPORT_THREADS && !PLATFORM_WEBGL
        m_SerializedFile->SetCacheReader(m_PrevReader);
        UNITY_DELETE(m_OverrideReader, kMemTempAlloc);
#endif
    }

private:
    CacheReaderBase* m_PrevReader;
    FileCacherRead* m_OverrideReader;
    SerializedFile* m_SerializedFile;
};

struct ObjectLoadData
{
    SerializedObjectIdentifier identifier;
    InstanceID instanceID;

    bool operator<(const ObjectLoadData& other) const
    {
        return identifier < other.identifier;
    }
};
typedef std::vector<ObjectLoadData> ObjectLoadList;

// Prepare preallocated, but not yet loaded objects obtained from activation queue
// and sort them by SerializedObjectIdentifier if needed.
// When we load object with ReadObject or LoadObjectsThreaded and it has dependencies which were not yet loaded,
// we preallocate them and put to m_ThreadedObjectActivationQueue as not loaded objects.
// LoadRemainingPreallocatedObjects loads these objects.
// In order to read objects sequentially from disk, we sort them by offset
// which is indirectly defined by localIdentifierInFile in SerializedObjectIdentifier.
// SerializedObjectIdentifier is extracted and reused later.
static void PrepareLoadObjects(Remapper* remapper, ObjectLoadList& objectsToLoad, bool sortInstanceIDsForLoading)
{
    for (ObjectLoadList::iterator it = objectsToLoad.begin(); it != objectsToLoad.end(); ++it)
    {
        if (!remapper->InstanceIDToSerializedObjectIdentifier(it->instanceID, it->identifier))
            it->instanceID = InstanceID_None;
    }

    if (sortInstanceIDsForLoading)
        std::sort(objectsToLoad.begin(), objectsToLoad.end());
}

void PersistentManager::LocalSerializedObjectIdentifierToInstanceID(const LocalSerializedObjectIdentifier& localIdentifier, InstanceID& outInstanceID, LockFlags lockedFlags)
{
    LocalSerializedObjectIdentifierToInstanceID(-1, localIdentifier, outInstanceID, lockedFlags);
}

void PersistentManager::LocalSerializedObjectIdentifierToInstanceID(int activeNameSpace, const LocalSerializedObjectIdentifier& localIdentifier, InstanceID& outInstanceID, LockFlags lockedFlags)
{
    LocalIdentifierInFileType localIdentifierInFile = localIdentifier.localIdentifierInFile;
    int localSerializedFileIndex = localIdentifier.localSerializedFileIndex;

    if (localIdentifierInFile == 0)
    {
        outInstanceID = InstanceID_None;
        return;
    }

    //PERSISTENT_MANAGER_AUTOLOCK(autoLock, kMutexLock, lockedFlags, gIDRemappingProfiler);

    if (activeNameSpace == -1)
        activeNameSpace = GetActiveNameSpace();

    Assert(localSerializedFileIndex != -1);

    int globalFileIndex;
    if (localSerializedFileIndex == 0)
        globalFileIndex = activeNameSpace;
    else
    {
        Assert(m_Streams[activeNameSpace].stream != NULL);

        Assert(activeNameSpace < (int)m_LocalToGlobalNameSpace.size() && activeNameSpace >= 0);

        IDRemap::iterator found = m_LocalToGlobalNameSpace[activeNameSpace].find(localSerializedFileIndex);

        if (found != m_LocalToGlobalNameSpace[activeNameSpace].end())
        {
            globalFileIndex = found->second;
        }
        else
        {
            AssertString("illegal LocalPathID in persistentmanager");
            outInstanceID = InstanceID_None;
            return;
        }
    }

    SerializedObjectIdentifier globalIdentifier;
    globalIdentifier.serializedFileIndex = globalFileIndex;
    globalIdentifier.localIdentifierInFile = localIdentifierInFile;

#if SUPPORT_INSTANCE_ID_REMAP_ON_LOAD
    ApplyInstanceIDRemapInternal(globalIdentifier);
#endif

    outInstanceID = m_Remapper->GetOrGenerateInstanceID(globalIdentifier);

    // Preallocate all referenced objects right away this we we ensure that the loading code will actually load them.
    if (m_ForcePreloadReferencedObjects && outInstanceID != InstanceID_None)
    {
//        autoLock.Unlock(kMutexLock);
        PreallocateObjectThreaded(outInstanceID, lockedFlags);
    }
}

Object* PersistentManager::GetFromActivationQueue(InstanceID instanceID, LockFlags lockedFlags)
{
    // PROFILER_AUTO(gFindInActivationQueueProfiler);

    // AutoLock integrationAutoLock(*this, kIntegrationMutexLock, &lockedFlags);

    ThreadedObjectActivationQueue::iterator found = m_ThreadedObjectActivationQueue.find(instanceID);
    if (found != m_ThreadedObjectActivationQueue.end())
        return found->second.object;

    return NULL;
}

Object* PersistentManager::PreallocateObjectThreaded(InstanceID instanceID, LockFlags lockedFlags)
{
    // PERSISTENT_MANAGER_AUTOLOCK(autoLock, kMutexLock | kIntegrationMutexLock, lockedFlags, gLoadFromActivationQueueStall);
    Object* obj = Object::IDToPointerThreadSafe(instanceID);

    if (obj != NULL)
        return obj;

    Object* o = GetFromActivationQueue(instanceID, lockedFlags);
    if (o != NULL)
        return o;

    // Find and load the right stream
    SerializedObjectIdentifier identifier;
    if (!m_Remapper->InstanceIDToSerializedObjectIdentifier(instanceID, identifier))
        return NULL;

    SerializedFile* stream = GetSerializedFileIfObjectAvailable(identifier.serializedFileIndex, identifier.localIdentifierInFile, lockedFlags);
    if (stream == NULL)
        return NULL;

    ThreadedAwakeData* awakeData = CreateThreadActivationQueueEntry(*stream, identifier, instanceID, false, lockedFlags);
    if (awakeData != NULL)
        return awakeData->object;
    else
        return NULL;
}

static PersistentManager * gPersistentManager = NULL;
PersistentManager& GetPersistentManager()
{
    //__FAKEABLE_FUNCTION__(GetPersistentManager, ());

    Assert(gPersistentManager != NULL);
    return *gPersistentManager;
}

PersistentManager* GetPersistentManagerPtr()
{
    // __FAKEABLE_FUNCTION__(GetPersistentManagerPtr, ());
    return gPersistentManager;
}

void CleanupPersistentManager()
{
    HUAHUO_DELETE(gPersistentManager, kMemManager);
    gPersistentManager = NULL;
}

void SetPersistentManager(PersistentManager* persistentManager)
{
    gPersistentManager = persistentManager;
}

PersistentManager::PersistentManager(MemLabelId label)
        : m_MemoryLabel(label)
//        , m_Streams(label)
//        , m_ThreadedObjectActivationQueue(label)
//        , m_PreallocatedScriptingObjectCallback(NULL)
//        , m_PreallocatedScriptingObjectCallbackContext(NULL)
{
     m_ForcePreloadReferencedObjects = false;

    for (int i = 0; i < kActiveNameSpaceCount; i++)
        m_ActiveNameSpace[i] = -1;

    m_Remapper = HUAHUO_NEW_AS_ROOT(Remapper , kMemSerialization, kRemapperAllocArea, "Remapper") ();

#if DEBUGMODE
    m_PreventLoadingFromFile = -1;
    m_AllowLoadingFromDisk = true;
#endif

#if PERSISTENT_MANAGER_VERIFY_LOCK_ORDER
    m_MutexLockCount = 0;
    m_IntegrationMutexLockCount = 0;
    m_IntegrationMutexThreadID = 0;
#endif

    m_Abort = 0;
}

bool StreamNameSpace::IsDestroyed(LocalIdentifierInFileType id)
{
    if (destroyedObjects == NULL)
        return false;

    return std::find(destroyedObjects->begin(), destroyedObjects->end(), id) != destroyedObjects->end();
}

static inline void AddToAwakeQueue(const ThreadedAwakeData& awake, AwakeFromLoadQueue& awakeQueue)
{
    Assert(awake.loadStarted);

    if (awake.object)
    {
        AwakeFromLoadMode overrideAwakeFromLoadMode = kDefaultAwakeFromLoadInvalid;
#if UNITY_EDITOR
        overrideAwakeFromLoadMode = awake.overrideAwakeFromLoadMode;
#endif

        awakeQueue.Add(*awake.object/*, awake.oldType*/, awake.checkConsistency, overrideAwakeFromLoadMode);
    }
}

void PersistentManager::CopyToAwakeFromLoadQueueInternal(AwakeFromLoadQueue& awakeQueue)
{
    // Add to AwakeFromLoadQueue - this will take care of ensuring sort order
    awakeQueue.Reserve(m_ThreadedObjectActivationQueue.size());

    ThreadedObjectActivationQueue::iterator end = m_ThreadedObjectActivationQueue.end();
    for (ThreadedObjectActivationQueue::iterator i = m_ThreadedObjectActivationQueue.begin(); i != end; ++i)
        AddToAwakeQueue(i->second, awakeQueue);
}

void PersistentManager::ExtractAwakeFromLoadQueue(AwakeFromLoadQueue& awakeQueue)
{
    LockFlags lockedFlags = kLockFlagNone;
    // PERSISTENT_MANAGER_AUTOLOCK(integrationAutoLock, kIntegrationMutexLock, lockedFlags, gLoadFromActivationQueueStall);

    CopyToAwakeFromLoadQueueInternal(awakeQueue);
    m_ThreadedObjectActivationQueue.clear();
}

void PersistentManager::ExtractAwakeFromLoadQueue(const InstanceID* instanceIDs, size_t size, AwakeFromLoadQueue& awakeQueue, LockFlags lockedFlags)
{
    // PERSISTENT_MANAGER_AUTOLOCK(integrationAutoLock, kIntegrationMutexLock, lockedFlags, gLoadFromActivationQueueStall);

    // Add to AwakeFromLoadQueue - this will take care of ensuring sort order
    awakeQueue.Reserve(size);

    for (size_t i = 0; i < size; i++)
    {
        InstanceID instanceID = instanceIDs[i];
        ThreadedObjectActivationQueue::iterator found = m_ThreadedObjectActivationQueue.find(instanceID);
        if (found != m_ThreadedObjectActivationQueue.end())
        {
            AddToAwakeQueue(found->second, awakeQueue);
            m_ThreadedObjectActivationQueue.erase(found);
        }
    }
}

void PersistentManager::IntegrateAllThreadedObjects()
{
    // PROFILER_AUTO(kProfileIntegrateAllThreadedObjects);
    AwakeFromLoadQueue awakeQueue(kMemTempAlloc);

    ExtractAwakeFromLoadQueue(awakeQueue);
    awakeQueue.RegisterObjectInstanceIDs();
    awakeQueue.PersistentManagerAwakeFromLoad();
}

int PersistentManager::LoadFileCompletely(std::string& pathName)
{
////    PROFILER_AUTO(gLoadFileCompletely);
////    AutoLock autoLock(*this);
//
//    LoadProgress tempProgress;

    int result = LoadFileCompletelyThreaded(pathName, NULL, NULL, -1, PersistentManager::kLoadFlagNone/*, tempProgress*/);
    IntegrateAllThreadedObjects();

    return result;
}

SerializedFile* PersistentManager::GetSerializedFile(std::string& path, LockFlags lockedFlags)
{
    // __FAKEABLE_METHOD_OVERLOADED__(PersistentManager, GetSerializedFile, (path, lockedFlags), SerializedFile * (core::string_ref, LockFlags));

    // AutoLock autoLock(*this, kMutexLock, &lockedFlags);
    return GetSerializedFile(InsertPathNameInternal(path, true), lockedFlags);
}

SerializedFile* PersistentManager::GetSerializedFile(int serializedFileIndex, LockFlags lockedFlags)
{
    // __FAKEABLE_METHOD_OVERLOADED__(PersistentManager, GetSerializedFile, (serializedFileIndex, lockedFlags), SerializedFile * (int, LockFlags));

    if (serializedFileIndex == -1)
        return NULL;

    // AutoLock autoLock(*this, kMutexLock, &lockedFlags);
    StreamNameSpace& stream = GetStreamNameSpaceInternal(serializedFileIndex);
    return stream.stream;
}

SerializedFile* PersistentManager::GetSerializedFileIfObjectAvailable(int serializedFileIndex, LocalIdentifierInFileType id, LockFlags lockedFlags)
{
    if (serializedFileIndex == -1)
        return NULL;

    // AutoLock autoLock(*this, kMutexLock, &lockedFlags);
    StreamNameSpace& stream = GetStreamNameSpaceInternal(serializedFileIndex);

    if (stream.stream == NULL || !stream.stream->IsAvailable(id))
        return NULL;

    if (stream.IsDestroyed(id))
        return NULL;

    return stream.stream;
}


void PersistentManager::CheckInstanceIDsLoaded(InstanceID* heapIDs, int size, LockFlags lockedFlags)
{
//    if (size > 0)
//    {
//        AutoLock integrationAutoLock(*this, kIntegrationMutexLock, &lockedFlags);
//
//        // Search through threaded object activation queue and activate the object if necessary.
//        for (int j = 0; j < size; j++)
//        {
//            if (m_ThreadedObjectActivationQueue.count(heapIDs[j]) != 0)
//                heapIDs[j] = InstanceID_None;
//        }
//    }
//
//    // Check which objects are already loaded all at once to lock object creation only once for a short amount of time
//    // Since we have locked persistentmanager already no objects can be loaded in the mean time
//    SetObjectLockForRead();
//    Object::CheckInstanceIDsLoaded(heapIDs, size);
//    ReleaseObjectLock();
}

bool PersistentManager::ShouldAbort() const
{
    // return atomic_load_explicit(&m_Abort, ::memory_order_relaxed) != 0;
    return false;
}

Object* PersistentManager::ProduceObject(SerializedFile& file, SerializedObjectIdentifier identifier, InstanceID instanceID, ObjectCreationMode objectCreationMode, LockFlags lockedFlags)
{
    const HuaHuo::Type* type;
    LocalSerializedObjectIdentifier scriptReference;
    MemLabelId memLabel;
    if (!file.GetProduceData(identifier.localIdentifierInFile, type, scriptReference, memLabel))
        return NULL;

    Object* obj = Object::Produce(type, instanceID, memLabel, objectCreationMode);

    if (obj == NULL)
    {
        // We deprecate classes in the editor so no need for these errors.
        // In the player it's a different affair, usually something has gone terribly wrong.
        // Note: If class is available on Editor, but not available in player, be sure to check
        //       SlowIsTypeSupportedOnBuildTarget is correctly excluding the class for the target platform
#if !UNITY_EDITOR
        if (type == NULL)
        {
            ErrorStringMsg("Could not produce class with NULL type.");
            return NULL;
        }
#if SUPPORTS_GRANULAR_CLASS_REGISTRATION
        const char* error = "Could not produce class with ID %d.\nThis could be caused by a class being stripped from the build even though it is needed. Try disabling 'Strip Engine Code' in Player Settings.";
#else
        const char* error = "Could not produce class with ID %d.";
#endif
        ErrorStringMsg(error, static_cast<int>(type->GetPersistentTypeID()));
#endif

        return NULL;
    }
#if UNITY_EDITOR
    obj->SetFileIDHint(identifier.localIdentifierInFile);
#endif
//
//    // If we have a ObjectStoredSerializableManagedRef and a valid MonoScript, set the script
//    // on the Script host object and create the managed object instance.
//    //
//    // Note that it is possible for the script PPtr to be null or for the
//    // PPtr to reference an external file that no longer exists.
//    if (scriptReference.localIdentifierInFile != 0)
//    {
//        if (IManagedObjectHost::IsObjectsTypeAHost(obj))
//        {
//            // Resolve the script reference.  MonoScripts have to be loaded *before* objects referencing
//            // them are loaded.  We do *not* load them on-demand.
//            InstanceID scriptInstanceID = InstanceID_None;
//            LocalSerializedObjectIdentifierToInstanceID(identifier.serializedFileIndex, scriptReference, scriptInstanceID, lockedFlags);
//
//            // Try getting the script from the activation queue first since that's where scripts will usually be when we are
//            // loading scenes (we read them from one of the sharedassets files before reading the scene data itself).
//            MonoScript* script = dynamic_pptr_cast<MonoScript*>(GetFromActivationQueue(scriptInstanceID, lockedFlags));
//            if (!script)
//            {
//                // It's not in the load queue.  Assume it's a MonoScript that has already been fully loaded.
//                script = dynamic_pptr_cast<MonoScript*>(Object::IDToPointerThreadSafe(scriptInstanceID));
//            }
//
//            // set script.
//            SerializableManagedRef &scriptedObj = *IManagedObjectHost::GetManagedReference(obj);
//            SerializableManagedRefFriends::SetScriptInternal(scriptedObj, obj, PPtr<MonoScript>(scriptInstanceID));
//
//            // Set instance
//            ScriptingObjectPtr scriptingPtr = m_PreallocatedScriptingObjectCallback != NULL ? m_PreallocatedScriptingObjectCallback(m_PreallocatedScriptingObjectCallbackContext, instanceID, type, obj) : SCRIPTING_NULL;
//
//            // Ensure scriptingPtr type matches script type. If type doesn't match, don't reuse the scriptingPtr.
//            if (scriptingPtr != SCRIPTING_NULL)
//            {
//                if (!script || script->GetClass() != scripting_object_get_class(scriptingPtr))
//                    scriptingPtr = SCRIPTING_NULL;
//            }
//
//            SerializableManagedRefFriends::RebuildMonoInstance(scriptedObj, obj, script ? script->GetClass() : SCRIPTING_NULL, scriptingPtr, script);
//        }
//    }
//    else
//    {
//        ScriptingObjectPtr scriptingPtr = m_PreallocatedScriptingObjectCallback != NULL ? m_PreallocatedScriptingObjectCallback(m_PreallocatedScriptingObjectCallbackContext, instanceID, type, obj) : SCRIPTING_NULL;
//        if (scriptingPtr != SCRIPTING_NULL)
//            Scripting::ConnectScriptingWrapperToObject(scriptingPtr, obj);
//    }

    return obj;
}


ThreadedAwakeData* PersistentManager::CreateThreadActivationQueueEntry(SerializedFile& file, SerializedObjectIdentifier identifier, InstanceID instanceID, bool loadStarted, LockFlags lockedFlags)
{
    // DebugAssert(Object::IDToPointerThreadSafe(instanceID) == NULL);

    // AutoLock integrationAutoLock(*this, kIntegrationMutexLock, &lockedFlags);

    // Pop preallocated object from queue or create new object
    ThreadedObjectActivationQueue::iterator found = m_ThreadedObjectActivationQueue.find(instanceID);
    if (found != m_ThreadedObjectActivationQueue.end())
    {
        ThreadedAwakeData& awake = found->second;

        if (loadStarted)
            awake.loadStarted = true;

        Assert(awake.object != NULL);

        return &awake;
    }

    Object* obj = ProduceObject(file, identifier, instanceID, kCreateObjectFromNonMainThread, lockedFlags);

    // Object could not be created...
    if (obj == NULL)
        return NULL;

    ThreadedAwakeData awake;
    awake.instanceID = instanceID;
    awake.object = obj;
    awake.checkConsistency = false;
    awake.completedThreadAwake = false;
    awake.loadStarted = loadStarted;
    // awake.oldType = NULL;
#if UNITY_EDITOR
    awake.overrideAwakeFromLoadMode = kDefaultAwakeFromLoadInvalid;
#endif

    ThreadedAwakeData* result;

    result = &m_ThreadedObjectActivationQueue.insert(std::pair<InstanceID, ThreadedAwakeData>(instanceID, awake)).first->second;

    // DebugAssert(Object::IDToPointerThreadSafe(instanceID) == NULL);

    if(result){
        printf("New object inserted!%s,%d\n", __FILE__, __LINE__ );
    }else{
        printf("Can't create new object!%s,%d\n", __FILE__, __LINE__ );
    }
    return result;
}

void PersistentManager::PostReadActivationQueue(InstanceID instanceID/*, const TypeTree* oldType*/, bool didTypeTreeChange, LockFlags lockedFlags)
{
    // AutoLock integrationAutoLock(*this, kIntegrationMutexLock, &lockedFlags);

    ThreadedObjectActivationQueue::iterator found = m_ThreadedObjectActivationQueue.find(instanceID);
    Assert(found != m_ThreadedObjectActivationQueue.end());

    ThreadedAwakeData& awakeData = found->second;
    Object* obj = awakeData.object;
    {
        // PROFILER_AUTO(kProfileAwakeFromLoadThreaded, obj);
        obj->AwakeFromLoadThreaded();
    }
    // awakeData.oldType = oldType;
    awakeData.checkConsistency = didTypeTreeChange;
    awakeData.completedThreadAwake = true;
}

Object* PersistentManager::ReadAndActivateObjectThreaded(InstanceID instanceID, const SerializedObjectIdentifier& identifier, SerializedFile* stream, bool isPersistent, bool validateLoadingFromSceneFile, LockFlags lockedFlags)
{
    // PROFILER_AUTO_INSTANCE_ID(gReadObjectThreadedProfiler, instanceID);

    // DebugAssertMsg(Object::IDToPointerThreadSafe(instanceID) == NULL, "Object is already loaded!");

    if (stream == NULL)
    {
        // AutoLock autoLock(*this, kMutexLock, &lockedFlags);

        // Get the SerializedFile object if it wasn't provided
        stream = GetSerializedFileIfObjectAvailable(identifier.serializedFileIndex, identifier.localIdentifierInFile, lockedFlags);
        if (stream == NULL)
            return NULL;
    }

    if (validateLoadingFromSceneFile)
    {
        // Scene objects are loaded specially such that their in-memory objects are transient.
#if UNITY_EDITOR
        // Case 631608:
        // In this case, there is a missing MonoBehaviour referencing the SceneSettings
        // from another scene file. No idea, how could this happen.
        // If we don't return NULL, the SceneSettings will be loaded as persistent.
        // We'll crash later in RemoveDuplicateGameManagers().
        bool isInSceneFile = EndsWithCaseInsensitive(PathIDToPathNameInternal(identifier.serializedFileIndex, false), ".unity");
        if (isInSceneFile)
        {
            ErrorString("Do not use ReadObjectThreaded on scene objects!");
            return NULL;
        }
#endif
    }

    ThreadedAwakeData* awakeData = CreateThreadActivationQueueEntry(*stream, identifier, instanceID, true, lockedFlags);
    if (awakeData == NULL)
        return NULL;

    // AutoLock autoLock(*this, kMutexLock, &lockedFlags);

    // Find file id in stream and read the object
    SetActiveNameSpace(identifier.serializedFileIndex);
//    const TypeTree* oldType;
    bool didTypeTreeChange;
//
    Object& targetObject = *awakeData->object;
    stream->ReadObject(identifier.localIdentifierInFile, kCreateObjectFromNonMainThread, isPersistent, /*&oldType,*/ &didTypeTreeChange, targetObject);

    ClearActiveNameSpace();

    PostReadActivationQueue(instanceID/*, oldType*/, didTypeTreeChange, lockedFlags);

    return &targetObject;
}

void PersistentManager::LoadRemainingPreallocatedObjects(LockFlags lockedFlags)
{
    // PROFILER_AUTO(gLoadRemainingPreallocatedObjects);

    ObjectLoadList objectsToLoad; //(kMemTempAlloc);
    objectsToLoad.reserve(100);

//    // Make sure access to internals (m_Remapper, m_Streams) is exclusive.
//    AutoLock autoLock(*this, kMutexLock, &lockedFlags);

    while (!ShouldAbort())
    {
        objectsToLoad.resize(0);

        {
            // AutoLock integrationAutoLock(*this, kIntegrationMutexLock, &lockedFlags);

            for (ThreadedObjectActivationQueue::iterator i = m_ThreadedObjectActivationQueue.begin(); i != m_ThreadedObjectActivationQueue.end(); i++)
            {
                ThreadedAwakeData& awake = i->second;
                if (awake.loadStarted)
                    continue;

                ObjectLoadData& loadData = objectsToLoad.emplace_back();
                loadData.instanceID = awake.instanceID;
            }
        }

        if (objectsToLoad.empty())
            return;

        PrepareLoadObjects(m_Remapper, objectsToLoad, true);

        // At this point all SerializedFiles should be created, and all objects produces (because they are in m_ThreadedObjectActivationQueue)
        // and we can use default fallback label
        for (ObjectLoadList::const_iterator it = objectsToLoad.begin(), itEnd = objectsToLoad.end(); it != itEnd && !ShouldAbort(); ++it)
        {
            const InstanceID instanceID = it->instanceID;
            if (instanceID == InstanceID_None)
                continue;

            ReadAndActivateObjectThreaded(instanceID, it->identifier, NULL, true, true, lockedFlags);
        }
    }
}


int PersistentManager::LoadFileCompletelyThreaded(std::string& pathname, LocalIdentifierInFileType* fileIDs, InstanceID* instanceIDs, int size, LoadFlags flags/*, LoadProgress& loadProgress*/, LockFlags lockedFlags)
{

//    PROFILER_AUTO(kProfileLoadFileCompletelyThreaded);
//    AutoLock autoLock(*this, kMutexLock, &lockedFlags);
    bool savedForcePreloadReferencedObjects = m_ForcePreloadReferencedObjects;
    auto restorePreloadReferencedObjects = core::MakeFinalizer([&]()
                                                               {
                                                                   m_ForcePreloadReferencedObjects = savedForcePreloadReferencedObjects;
                                                               });


    if (HasFlag(flags, kForcePreloadReferencedObjects))
        m_ForcePreloadReferencedObjects = true;


    // DebugAssert(!HasPreallocatedObjects());



if(this == NULL){

}

    // Find Stream
    int serializedFileIndex = InsertPathNameInternal(pathname, true);
    SerializedFile* serializedFile = GetSerializedFile(serializedFileIndex, lockedFlags);
    if (serializedFile == NULL)
        return kFileCouldNotBeRead;


    Assert(!(fileIDs != NULL && size == -1));
    Assert(!(instanceIDs != NULL && size == -1));

    // Because we won't be seeking around the file, we can use a larger cache and can also prefetch
    AutoFileCacherReadOverride autoCacherReaderResize(serializedFile);

    // Get all file IDs we want to load and generate instance ids
    std::vector<LocalIdentifierInFileType> fileIDsVector; //(kMemTempAlloc);
    std::vector<InstanceID> instanceIDsVector; //(kMemTempAlloc);
    if (size == -1)
    {

        GetAllFileIDs(pathname, fileIDsVector);

        fileIDs = fileIDsVector.begin().base();

        size = fileIDsVector.size();

        // loadProgress.AddTotalItemCount(size);
        instanceIDsVector.resize(size, InstanceID_None);

        instanceIDs = instanceIDsVector.begin().base();

    }


    // In the editor we can not use preallocate ranges since fileID's might be completely arbitrary ranges
    bool loadScene = HasFlag(flags, kSceneLoad);
    if (loadScene /*&& !UNITY_EDITOR*/)
    {
        LocalIdentifierInFileType highestFileID = 0;
        for (int i = 0; i < size; i++)
        {
            Assert(fileIDs[i] >= 0);
            highestFileID = std::max(highestFileID, fileIDs[i]);
        }

        InstanceID firstPreallocatedID, lastPreallocatedID;
        m_Remapper->PreallocateIDs(highestFileID, serializedFileIndex, firstPreallocatedID, lastPreallocatedID);

        for (int i = 0; i < size; i++)
        {
            LocalIdentifierInFileType fileID = fileIDs[i];
            Assert(!m_Remapper->IsSerializedObjectIdentifierMappedToAnything(SerializedObjectIdentifier(serializedFileIndex, fileID)));
            instanceIDs[i] = InstanceID_Make(InstanceID_AsSInt32Ref(firstPreallocatedID) + fileID * 2);
        }

#if DEBUGMODE
        CheckInstanceIDsLoaded(&instanceIDs[0], size, lockedFlags);
        for (int i = 0; i < size; i++)
        {
            Assert(instanceIDs[i] != InstanceID_None);
        }
#endif
    }
    else
    {

        for (int i = 0; i < size; i++)
        {

            LocalIdentifierInFileType fileID = fileIDs[i];
            InstanceID heapID = m_Remapper->GetOrGenerateInstanceID(SerializedObjectIdentifier(serializedFileIndex, fileID));

            if (heapID == InstanceID_None)
            {
                AssertString("Loading an object that was made unpersistent but wasn't destroyed before reloading it");
            }

            instanceIDs[i] = heapID;
        }

        // - Figure out which ones are already loaded
        CheckInstanceIDsLoaded(&instanceIDs[0], size, lockedFlags);

    }

    // Load all objects
    for (int i = 0; i < size && !ShouldAbort(); i++)
    {

        // loadProgress.BeginProcessItem();

        const InstanceID instanceID = instanceIDs[i];
        if (instanceID == InstanceID_None)
            continue;

        SerializedObjectIdentifier identifier(serializedFileIndex, fileIDs[i]);
        Object* object = ReadAndActivateObjectThreaded(instanceID, identifier, serializedFile, !loadScene, false, lockedFlags);
        if (object == NULL)
            continue;

        // loadProgress.DidReadObject(*object);
    }

    LoadRemainingPreallocatedObjects(lockedFlags);

    if (loadScene)
    {
#if UNITY_EDITOR
        for (int i = 0; i < size; i++)
            m_Remapper->Remove(instanceIDs[i]);
#else
        m_Remapper->ClearPreallocateIDs();
#endif
    }

    return kNoError;
}

static bool InitTempWriteFile(FileCacherWrite& writer, const std::string& path, unsigned cacheSize, bool shouldBeOnMemoryFileSystem)
{
//    std::string tempWriteFileName = GenerateUniquePathSafe(std::string(shouldBeOnMemoryFileSystem ? "mem:/" + path : path));
//    if (tempWriteFileName.empty())
//        return false;
//
//    return writer.InitWriteFile(shouldBeOnMemoryFileSystem ? "mem:/" + path : path, cacheSize);
    return writer.InitWriteFile(path, cacheSize);
}

int PersistentManager::WriteFile(std::string& path, BuildTargetSelection target /*= BuildTargetSelection::NoTarget()*/, TransferInstructionFlags options /*= 0*/)
{
    // PROFILER_AUTO(gWriteFileProfiler);

//    LockFlags lockedFlags = kLockFlagNone;
//    AutoLock autoLock(*this, kMutexLock, &lockedFlags);


    int serializedFileIndex;

    serializedFileIndex = InsertPathNameInternal(path, false);

    if (serializedFileIndex == -1){

        return kNoError;
    }


//    bool needsWrite = TestNeedWriteFile(serializedFileIndex);
//
//    // Early out
//    if (!needsWrite)
//    {
//        // @TODO: THIS SHOULD NOT BE HACKED IN HERE. Make test coverage against increased leaking then remove this and call CleanupStream explicitly.
//        // We have already determined that the stream does not have any destroyed objects so it is safe so call this function with the 'false' argument
//        // as there is nothing to clean up anyway
//        CleanupStreamAndNameSpaceMapping(serializedFileIndex, false);
//        return kNoError;
//    }


    ObjectIDs writeObjects; //(kMemTempAlloc);
    if (options & kDontReadObjectsFromDiskBeforeWriting)
    {

        GetLoadedInstanceIDsAtPath(path, &writeObjects);

        Assert(!writeObjects.empty());
    }
    else
    {

        // Load all writeobjects into memory
        // (dont use LoadFileCompletely, since that reads all objects
        // even those that might have been changed in memory)
        GetInstanceIDsAtPath(path, &writeObjects);
    }

    WriteDataArray writeData;


    for (const auto instanceID : writeObjects)
    {

        // Force load object from disk.
        Object* o = dynamic_instanceID_cast<Object*>(instanceID);


        if (o == NULL)
            continue;

        Assert(!(o != NULL && !o->IsPersistent()));


        SerializedObjectIdentifier identifier;

        m_Remapper->InstanceIDToSerializedObjectIdentifier(instanceID, identifier);


        Assert(identifier.serializedFileIndex == serializedFileIndex);

        DebugAssert(o->IsPersistent());
        DebugAssert(m_Remapper->GetSerializedFileIndex(instanceID) == serializedFileIndex);
        DebugAssert(m_Remapper->IsSerializedObjectIdentifierMappedToAnything(identifier));

        writeData.push_back(WriteData(identifier.localIdentifierInFile, instanceID/*, BuildUsageTag()*/));

    }

    std::sort(writeData.begin(), writeData.end());


    int result = WriteFile(path, serializedFileIndex, writeData.begin().base(), writeData.size()/*, GlobalBuildData()*/, NULL, target, options, NULL/*, lockedFlags*/);

    if (result != kNoError && options & kAllowTextSerialization)
        // Try binary serialization as a fallback.
        result = WriteFile(path, serializedFileIndex, writeData.begin().base(), writeData.size()/*, GlobalBuildData()*/, NULL, target, options & ~kAllowTextSerialization, NULL/*, lockedFlags*/);

    return result;
}


//static bool InitTempWriteFile(FileCacherWrite& writer, unsigned cacheSize, bool shouldBeOnMemoryFileSystem)
//{
//    core::string tempWriteFileName = shouldBeOnMemoryFileSystem ? GetUniqueTempPath("mem:/Temp/UnityTempFile-") : GetUniqueTempPathInProject();
//    if (tempWriteFileName.empty())
//        return false;
//
//    return writer.InitWriteFile(tempWriteFileName, cacheSize);
//}

//int PersistentManager::BeginFileWriting(std::string& path){
//    int serializedFileIndex;
//    serializedFileIndex = InsertPathNameInternal(path, false);
//
//    if(m_Streams[serializedFileIndex].stream == NULL){
//        // Create writable stream
//        SerializedFile* tempSerialize = HUAHUO_NEW_AS_ROOT(SerializedFile, kMemSerialization, kSerializedFileArea, "") ();
//#if UNITY_EDITOR
//        tempSerialize->SetDebugPath(PathIDToPathNameInternal(serializedFileIndex, false));
//#if ENABLE_MEM_PROFILER
//    GetMemoryProfiler()->SetRootAllocationObjectName(tempSerialize, kMemSerialization, tempSerialize->GetDebugPath().c_str());
//#endif
//#endif
//        // Create writing tools
//        CachedWriter writer;
//        FileCacherWrite serializedFileWriter;
//
////    if (!InitTempWriteFile(serializedFileWriter,path, kCacheSize, false))
////        return;
//        serializedFileWriter.InitWriteFile(path, kCacheSize);
//        writer.InitWrite(serializedFileWriter);
//
//        tempSerialize->InitializeWrite(writer, /*target,*/ kReadWriteFromSerializedFile);
//        m_Streams[serializedFileIndex].stream = tempSerialize;
//    }
//    SetActiveNameSpace(serializedFileIndex, kWritingNameSpace);
//
//    return serializedFileIndex;
//}
//
//void PersistentManager::BeginFileReading(std::string& path){
//    int serializedFileIndex;
//    serializedFileIndex = InsertPathNameInternal(path, false);
//    SetActiveNameSpace(serializedFileIndex, kReadingNameSpace);
//}

void PersistentManager::SetActiveNameSpace(int activeNameSpace, ActiveNameSpaceType type)
{
    // Recursive serialization is not allowed for threaded serialization.
    // If this assert triggers we have to fix the calling code to not do any recursive serialization.
    Assert(m_ActiveNameSpace[type] == -1);
    m_ActiveNameSpace[type] = activeNameSpace;
}

void PersistentManager::ClearActiveNameSpace(ActiveNameSpaceType type)
{
    Assert(m_ActiveNameSpace[type] != -1);
    m_ActiveNameSpace[type] = -1;
}

LocalSerializedObjectIdentifier PersistentManager::GlobalToLocalSerializedFileIndex(const SerializedObjectIdentifier& globalIdentifier)
{
    // AutoLock autoLock(*this);
    LocalIdentifierInFileType localIdentifierInFile = globalIdentifier.localIdentifierInFile;
    int localSerializedFileIndex;


    // Remap globalPathID to localPathID
    int activeNameSpace = GetActiveNameSpace(kWritingNameSpace);

    printf("Active name space is:%d\n", activeNameSpace);

    IDRemap& globalToLocalNameSpace = m_GlobalToLocalNameSpace[activeNameSpace];
    IDRemap& localToGlobalNameSpace = m_LocalToGlobalNameSpace[activeNameSpace];


    IDRemap::iterator found = globalToLocalNameSpace.find(globalIdentifier.serializedFileIndex);

    if (found == globalToLocalNameSpace.end())
    {

        // SET_ALLOC_OWNER(m_MemoryLabel);
        Assert(activeNameSpace < (int)m_Streams.size());
        Assert(m_Streams[activeNameSpace].stream != NULL);

        if(NULL == m_Streams[activeNameSpace].stream){
            printf("Stream is null!!\n");
        }

        printf("Get stream from active namespace:%d\n", activeNameSpace);

        SerializedFile& serialize = *m_Streams[activeNameSpace].stream;



        FileIdentifier fileIdentifier = PathIDToFileIdentifierInternal(globalIdentifier.serializedFileIndex);


        serialize.AddExternalRef(fileIdentifier);



        // localIdentifierInFile mapping is not zero based. zero is reserved for mapping into the same file.
        localSerializedFileIndex = serialize.GetExternalRefs().size();



        globalToLocalNameSpace[globalIdentifier.serializedFileIndex] = localSerializedFileIndex;

        localToGlobalNameSpace[localSerializedFileIndex] = globalIdentifier.serializedFileIndex;

    }
    else
        localSerializedFileIndex = found->second;


    // Setup local identifier
    LocalSerializedObjectIdentifier localIdentifier;

    localIdentifier.localSerializedFileIndex = localSerializedFileIndex;
    localIdentifier.localIdentifierInFile = localIdentifierInFile;

    return localIdentifier;
}


void PersistentManager::InstanceIDToLocalSerializedObjectIdentifier(InstanceID instanceID, LocalSerializedObjectIdentifier& localIdentifier)
{
    // PERSISTENT_MANAGER_AUTOLOCK2(autoLock, HuaHuoEngine::kMutexLock, NULL, &gIDRemappingProfiler);

    if (instanceID == InstanceID_None)
    {
        localIdentifier.localSerializedFileIndex = 0;
        localIdentifier.localIdentifierInFile = 0;
        return;
    }


    SerializedObjectIdentifier globalIdentifier;

    if (!m_Remapper->InstanceIDToSerializedObjectIdentifier(instanceID, globalIdentifier))
    {

        localIdentifier.localSerializedFileIndex = 0;
        localIdentifier.localIdentifierInFile = 0;
        return;
    }


    localIdentifier = GlobalToLocalSerializedFileIndex(globalIdentifier);

}

#if HUAHUO_EDITOR
void PersistentManager::MakeObjectPersistent(InstanceID heapID, std::string pathName)
{

    MakeObjectPersistentAtFileID(heapID, 0, pathName);
}

void PersistentManager::MakeObjectPersistentAtFileID(InstanceID heapID, LocalIdentifierInFileType fileID, std::string pathName)
{

    MakeObjectsPersistent(&heapID, &fileID, 1, pathName);
}

void PersistentManager::AddStream()
{
    // AutoLock autoLock(*this);
    m_Streams.push_back(StreamNameSpace());
    m_GlobalToLocalNameSpace.push_back(IDRemap());
    m_LocalToGlobalNameSpace.push_back(IDRemap());
}

bool PersistentManager::InstanceIDToSerializedObjectIdentifier(InstanceID instanceID, SerializedObjectIdentifier& identifier)
{
    // PERSISTENT_MANAGER_AUTOLOCK2(autoLock, PersistentManager::kMutexLock, NULL, &gIDRemappingProfiler);

    return m_Remapper->InstanceIDToSerializedObjectIdentifier(instanceID, identifier);
}

StreamNameSpace* PersistentManager::GetStreamNameSpaceInternal(std::string path)
{
    // Must lock m_Mutex BEFORE calling this function since it returns a pointer to an element in m_Streams

    int serializedFileIndex = InsertPathNameInternal(path, true);
    if (serializedFileIndex == -1)
        return NULL;

    StreamNameSpace& streamNameSpace = GetStreamNameSpaceInternal(serializedFileIndex);

    return &streamNameSpace;
}

//This public call will always pass false to trackNativeLoadedAsset
//as the PersistentManager (and subclasses) should be responsible
//for saying when something should be loaded, and not externally
std::string PersistentManager::PathIDToPathNameInternal(int pathID)
{
    return PathIDToPathNameInternal(pathID, false);
}

std::string PersistentManager::RemapToAbsolutePath(const std::string& path)
{
    // AutoLock autoLock(*this);

    UserPathRemap::iterator found = m_UserPathRemap.find(path);
    if (found != m_UserPathRemap.end())
        return std::string(found->second);

    return PathToAbsolutePath(path);
}

static void CleanupStream(StreamNameSpace& stream, bool cleanupDestroyedList)
{
    if (cleanupDestroyedList)
    {
        std::vector<LocalIdentifierInFileType>* destroyedObjects = stream.destroyedObjects;
        stream.destroyedObjects = NULL;

        HUAHUO_DELETE(destroyedObjects, kMemSerialization);
    }

    if (stream.stream)
    {
        stream.stream->Release();
        stream.stream = NULL;
    }
}

StreamNameSpace& PersistentManager::GetStreamNameSpaceInternal(int nameSpaceID)
{
    // Must lock m_Mutex BEFORE calling this function since it returns a reference to an element in m_Streams

    CheckedAssert(m_PreventLoadingFromFile != nameSpaceID);

    StreamNameSpace& nameSpace = m_Streams[nameSpaceID];

    // Stream already loaded
    if (nameSpace.stream)
        return nameSpace;

    // PROFILER_AUTO(gLoadStreamNameSpaceProfiler);

    // Load Stream
    std::string pathName = PathIDToPathNameInternal(nameSpaceID, true);
    if (pathName.empty())
        return nameSpace;

    // File not found
    std::string absolutePath = RemapToAbsolutePath(pathName);
#if UNITY_EDITOR
    if (absolutePath.ends_with(GetNonExistingContentPath())) // It's much faster to check this than to let the file system try and open a file using this name
        return nameSpace;
#endif
#if UNITY_EDITOR && PLATFORM_LINUX
    absolutePath = GetActualPathSlow(absolutePath);
#endif
    if (!IsFileCreated(absolutePath))
    {
        printf("File can't be created!!!!!!!!!! Path:%s\n", absolutePath.c_str());
#if !UNITY_EDITOR
        AssertString("PersistentManager: File '" + pathName + "' was expected to exist at absolute path '" + absolutePath + "'");
#endif
        // return nameSpace;
        CreateFile(absolutePath);
    }

    printf("File has been created!!!!!!!! Namespace:%d\n", nameSpaceID);

    // Is Builtin resource file?
    TransferInstructionFlags options = kNoTransferInstructionFlags;
//    if (IsPathBuiltinResourceFile(pathName))
//        options |= kIsBuiltinResourcesFile;

    printf("Creating stream here\n");
    nameSpace.stream = HUAHUO_NEW_AS_ROOT(SerializedFile, kMemSerialization, kSerializedFileArea, pathName.c_str()) ();
            SET_ALLOC_OWNER(CreateMemLabel(kMemSerialization, nameSpace.stream));
#if UNITY_EDITOR
    nameSpace.stream->SetDebugPath(pathName);
#if ENABLE_MEM_PROFILER
    GetMemoryProfiler()->SetRootAllocationObjectName(nameSpace.stream, kMemSerialization, nameSpace.stream->GetDebugPath().c_str());
#endif
#endif

    // Resource image loading is only supported in the player!
    ResourceImageGroup group;
#if SUPPORT_RESOURCE_IMAGE_LOADING
    for (int i = 0; i < kNbResourceImages; i++)
    {
        std::string resourceImagePath = AppendPathNameExtension(absolutePath, kResourceImageExtensions[i]);
        if (IsFileCreated(resourceImagePath) && i != kStreamingResourceImage)
            group.AddResourceImage(i, resourceImagePath);
    }
#endif

    nameSpace.loadError = nameSpace.stream->InitializeRead(absolutePath, group, kCacheSize, false, options);

    if (nameSpace.loadError != kSerializedFileLoadError_None)
    {
        Assert(nameSpace.GetDestroyedObjectsPtr() == NULL);
        CleanupStream(nameSpace, false);
        return nameSpace;
    }

    PostLoadStreamNameSpaceInternal(nameSpace, nameSpaceID);

    return m_Streams[nameSpaceID];
}

void PersistentManager::PostLoadStreamNameSpaceInternal(StreamNameSpace& nameSpace, int nameSpaceID)
{
    // Calling function must lock m_Mutex since this accesses namespace arrays

    nameSpace.highestID = std::max(nameSpace.highestID, nameSpace.stream->GetHighestID());

            SET_ALLOC_OWNER(m_MemoryLabel);
    const SerializedFile::FileIdentifierArray& externalRefs = nameSpace.stream->GetExternalRefs();

    // Read all local pathnames and generate global<->localnamespace mapping
    // localIdentifierInFile mapping is not zero based. zero is reserved for mapping into the same file.
    for (unsigned int i = 0; i != externalRefs.size(); i++)
    {
        int serializedFileIndex = InsertFileIdentifierInternal(externalRefs[i], FileIdentifier::kCreate | FileIdentifier::kAllowRemap);
        m_GlobalToLocalNameSpace[nameSpaceID][serializedFileIndex] = i + 1;
        m_LocalToGlobalNameSpace[nameSpaceID][i + 1] = serializedFileIndex;
    }

    // Setup global to self namespace mapping
    m_GlobalToLocalNameSpace[nameSpaceID][nameSpaceID] = 0;
    m_LocalToGlobalNameSpace[nameSpaceID][0] = nameSpaceID;
}


void PersistentManager::MakeObjectsPersistent(const InstanceID* heapIDs, LocalIdentifierInFileType* fileIDs, int size, std::string pathName, int options)
{
    // PROFILER_AUTO(gMakeObjectPersistentProfiler);

    CheckedAssert(m_AllowLoadingFromDisk);
    // ASSERT_RUNNING_ON_MAIN_THREAD;
    // AutoLock autoLock(*this);
    Assert(!pathName.empty());
    SInt32 globalNameSpace = InsertPathNameInternal(pathName, true);
    StreamNameSpace* streamNameSpace = NULL;
    for (int i = 0; i < size; i++)
    {
        InstanceID heapID = heapIDs[i];
        LocalIdentifierInFileType fileID = fileIDs[i];
        Object* o = Object::IDToPointer(heapID);
        if ((options & kMakePersistentDontRequireToBeLoadedAndDontUnpersist) == 0)
        {

            // Making an object that is not in memory persistent
            if (o == NULL)
            {
                ErrorString("Make Objects Persistent failed because the object can not be loaded");
                continue;
            }

            // Make Object unpersistent first
            if (o->IsPersistent())
            {

                SerializedObjectIdentifier identifier;
                InstanceIDToSerializedObjectIdentifier(heapID, identifier);
                Assert(identifier.serializedFileIndex != -1);


                // Return if the file and serializedFileIndex is not going to change
                if (globalNameSpace == identifier.serializedFileIndex)
                {
                    if (fileID == 0 || identifier.localIdentifierInFile == fileID)
                        continue;
                }

                MakeObjectUnpersistent(heapID, kDestroyFromFile);
            }
        }

        if (streamNameSpace == NULL)
            streamNameSpace = &GetStreamNameSpaceInternal(globalNameSpace);

        // Allocate an fileID for this object in the File
        if (fileID == 0)
        {

            fileID = streamNameSpace->highestID;
            if (streamNameSpace->stream)
                fileID = std::max(streamNameSpace->highestID, streamNameSpace->stream->GetHighestID());
            fileID++;
        }

        streamNameSpace->highestID = std::max(streamNameSpace->highestID, fileID);

        SerializedObjectIdentifier identifier;
        identifier.serializedFileIndex = globalNameSpace;
        identifier.localIdentifierInFile = fileID;
        m_Remapper->SetupRemapping(heapID, identifier);
        fileIDs[i] = fileID;

        if (o)
        {

            // Assert(!(o->TestHideFlag(Object::kDontSaveInEditor) && (options & kAllowDontSaveObjectsToBePersistent) == 0));
            o->SetIsPersistent(true);

            o->SetFileIDHint(fileID);
            o->SetDirty();
        }
    }

}

static const char* kSerializedFileArea = "SerializedFile";
static const char* kRemapperAllocArea = "PersistentManager.Remapper";

static void AddDetroyedObject(StreamNameSpace& streamNameSpace, LocalIdentifierInFileType id)
{
    if (streamNameSpace.destroyedObjects == NULL){
        // streamNameSpace.destroyedObjects = HUAHUO_NEW_AS_ROOT(std::vector<LocalIdentifierInFileType>, kMemSerialization, kSerializedFileArea, "DestroyedObjects") ();
        streamNameSpace.destroyedObjects = new std::vector<LocalIdentifierInFileType>;
    }


    streamNameSpace.destroyedObjects->push_back(id);
}

void PersistentManager::DestroyFromFile(InstanceID memoryID)
{
//    AutoLock autoLock(*this);

    SerializedObjectIdentifier identifier;
    m_Remapper->InstanceIDToSerializedObjectIdentifier(memoryID, identifier);

    if (identifier.serializedFileIndex == -1)
        return;

    StreamNameSpace& streamNameSpace = GetStreamNameSpaceInternal(identifier.serializedFileIndex);
    SerializedFile* serialize = streamNameSpace.stream;
    if (serialize == NULL)
        return;

    AddDetroyedObject(streamNameSpace, identifier.localIdentifierInFile);
}

void PersistentManager::MakeObjectUnpersistent(InstanceID memoryID, UnpersistMode mode)
{
    // PROFILER_AUTO_INSTANCE_ID(gMakeObjectUnpersistentProfiler, memoryID);

    CheckedAssert(m_AllowLoadingFromDisk);
//    ASSERT_RUNNING_ON_MAIN_THREAD;
//    AutoLock autoLock(*this);

    Object* o = Object::IDToPointer(memoryID);
    if (o && !o->IsPersistent())
        return;

    if (mode == kDestroyFromFile)
        DestroyFromFile(memoryID);

    m_Remapper->Remove(memoryID);

    if (o)
        o->SetIsPersistent(false);
}

// Returns fileIDs from path.
void PersistentManager::GetAllFileIDs(std::string& pathName, std::vector<LocalIdentifierInFileType>& fileIDs){
    // AutoLock autoLock(*this);

    StreamNameSpace* streamNameSpace = GetStreamNameSpaceInternal(pathName);
    if (streamNameSpace == NULL || streamNameSpace->stream == NULL)
        return;

    streamNameSpace->stream->GetAllFileIDs(fileIDs);

    for (std::vector<LocalIdentifierInFileType>::iterator it = fileIDs.begin(); it != fileIDs.end();)
    {
        // If it's destroyed.
        if (streamNameSpace->IsDestroyed(*it))
        {
            it = fileIDs.erase(it);
            continue;
        }

        ++it;
    }
}

void PersistentManager::GetInstanceIDsAtPath(std::string& pathName, ObjectIDs* objects)
{
    // AutoLock autoLock(*this);

    Assert(objects != NULL);

    int serializedFileIndex = InsertPathNameInternal(pathName, true);
    if (serializedFileIndex == -1)
        return;

    std::vector<LocalIdentifierInFileType> fileIDs;//(kMemTempAlloc);
    GetAllFileIDs(pathName, fileIDs);

    for (std::vector<LocalIdentifierInFileType>::iterator i = fileIDs.begin(); i != fileIDs.end(); ++i)
    {
        SerializedObjectIdentifier identifier(serializedFileIndex, *i);
        InstanceID memoryID = m_Remapper->GetOrGenerateInstanceID(identifier);

        if (memoryID != InstanceID_None)
            objects->emplace(memoryID);
    }

    // Get all objects that were made persistent but might not already be written to the file
    m_Remapper->GetAllLoadedObjectsForSerializedFileIndex(serializedFileIndex, objects);
}

void PersistentManager::GetLoadedInstanceIDsAtPath(std::string& pathName, ObjectIDs* objects)
{
    // AutoLock autoLock(*this);

    Assert(objects != NULL);

    int serializedFileIndex = InsertPathNameInternal(pathName, false);
    if (serializedFileIndex != -1)
    {
        // Get all objects that were made persistent but might not already be written to the file
        m_Remapper->GetAllLoadedObjectsForSerializedFileIndex(serializedFileIndex, objects);
    }
}

PersistentManager* PersistentManager::GetPersistentManager() {
    return ::GetPersistentManagerPtr();
}

int PersistentManager::WriteFile(std::string& path, int serializedFileIndex, const WriteData* writeData, int size, /*const GlobalBuildData& globalBuildData,*/ VerifyWriteObjectCallback* verifyCallback, BuildTargetSelection target, TransferInstructionFlags options, const InstanceIDResolver* instanceIDResolver, LockFlags lockedFlags, ReportWriteObjectCallback* reportCallback, void* reportCallbackUserData)
{
    
    WriteInformation writeInfo;
    return WriteFile(path, serializedFileIndex, writeData, size, /*globalBuildData,*/ verifyCallback, target, options, writeInfo, instanceIDResolver, lockedFlags, reportCallback, reportCallbackUserData);
}

void PersistentManager::CleanupStreamAndNameSpaceMapping(unsigned serializedFileIndex, bool cleanupDestroyedList)
{
    // Unload the file any way
    // This saves memory - especially when reimporting lots of assets like when rebuilding the library
    CleanupStream(m_Streams[serializedFileIndex], cleanupDestroyedList);

    m_GlobalToLocalNameSpace[serializedFileIndex].clear();
    m_LocalToGlobalNameSpace[serializedFileIndex].clear();
}

int PersistentManager::WriteFile(std::string& path, int serializedFileIndex, const WriteData* writeData, int size, /*const GlobalBuildData& globalBuildData,*/ VerifyWriteObjectCallback* verifyCallback, BuildTargetSelection target, TransferInstructionFlags options, WriteInformation& writeInfo, const InstanceIDResolver* instanceIDResolver, LockFlags lockedFlags, ReportWriteObjectCallback* reportCallback, void* reportCallbackUserData)
{
#if UNITY_EDITOR
    if (options & kAllowTextSerialization)
    {
        for (int i = 0; i < size; i++)
        {
            Object* o = writeData[i].objectPtr;
            if (o == nullptr)
            {
                o = dynamic_instanceID_cast<Object*>(writeData[i].instanceID);
                if (o == nullptr)
                    continue;
            }

            if (IsTypeNonTextSerialized(o->GetType()) || IsObjectNonTextSerialized(o))
                options &= ~kAllowTextSerialization;
        }
    }
#endif

//    internal::ScopedWriteFileBuildStep writeFileStep(reportCallback, reportCallbackUserData, core::Join("Write file:", path));

//    AutoLock autoLock(*this, kMutexLock, &lockedFlags);
    //printf_console("Writing file %s\n", pathName.c_str());

    AutoResetInstanceIDResolver autoResetIDResolver;
    if (instanceIDResolver)
        autoResetIDResolver.Set(*instanceIDResolver);

    // Create writing tools
    CachedWriter writer;

    FileCacherWrite serializedFileWriter;
    // FileCacherWrite resourceImageWriters[kNbResourceImages];

    bool isTempFileOnMemoryFileSystem = options & kTempFileOnMemoryFileSystem;

    // ScopedMemoryMount scopedMemoryMount(isTempFileOnMemoryFileSystem);

    if (!InitTempWriteFile(serializedFileWriter, path, kCacheSize, isTempFileOnMemoryFileSystem))
        return kFileCouldNotBeWritten;
    writer.InitWrite(serializedFileWriter);

//    if (options & kBuildResourceImage)
//    {
//        for (int i = 0; i < kNbResourceImages; i++)
//        {
//            core::string tempPath = AppendPathNameExtension("Temp/tempFile", kResourceImageExtensions[i]);
//            if (!InitTempWriteFile(resourceImageWriters[i], tempPath, kCacheSize, isTempFileOnMemoryFileSystem))
//                return kFileCouldNotBeWritten;
//
//            core::string dstResourcePath = AppendPathNameExtension(path, kResourceImageExtensions[i]);
//            writer.InitResourceImage((ActiveResourceImage)i, resourceImageWriters[i], dstResourcePath);
//        }
//    }

//    if (options & kResolveStreamedResourceSources)
//        writer.SetStreamedResourceSource(core::Join(path, ".resource"));

    // Cleanup old stream and mapping
    // We are about to write the file so it is safe to tell the function to cleanup the list of destroyed objects
    CleanupStreamAndNameSpaceMapping(serializedFileIndex, true);

    // Setup global to self namespace mapping
    m_GlobalToLocalNameSpace[serializedFileIndex][serializedFileIndex] = 0;
    m_LocalToGlobalNameSpace[serializedFileIndex][0] = serializedFileIndex;

    // Create writable stream
    SerializedFile* tempSerialize = HUAHUO_NEW_AS_ROOT(SerializedFile, kMemSerialization, kSerializedFileArea, "") ();
#if UNITY_EDITOR
    tempSerialize->SetDebugPath(PathIDToPathNameInternal(serializedFileIndex, false));
#if ENABLE_MEM_PROFILER
    GetMemoryProfiler()->SetRootAllocationObjectName(tempSerialize, kMemSerialization, tempSerialize->GetDebugPath().c_str());
#endif
#endif

    tempSerialize->InitializeWrite(writer/*, target*/, options);
    m_Streams[serializedFileIndex].stream = tempSerialize;

    // Generate all uniqueScriptTypeReferences
    // This way we can produce monobehaviours and their C# class without reading the actual data.
    // (See PreallocatObjectThreaded)
    SetActiveNameSpace(serializedFileIndex, kWritingNameSpace);

    std::vector<LocalSerializedObjectIdentifier> scriptTypeReferences;
    LocalSerializedObjectIdentifier scriptLocalIdentifier;
    for (int i = 0; i < size; i++)
    {

        Object* o = writeData[i].objectPtr;
        if (o == NULL)
            o = Object::IDToPointer(writeData[i].instanceID);

//        scriptLocalIdentifier = GetScriptLocalIdentifier(o);
//        if (scriptLocalIdentifier.localIdentifierInFile != 0)
//            scriptTypeReferences.push_back(scriptLocalIdentifier);
    }
//    vector_set<LocalSerializedObjectIdentifier> uniqueScriptTypeReferences;
//    uniqueScriptTypeReferences.assign_clear_duplicates(scriptTypeReferences.begin(), scriptTypeReferences.end());

    bool reportObjectNames = false;
    if (reportCallback)
        reportObjectNames = reportCallback(PersistentManager::ReportWriteObjectStep_CheckReportability, 0, "", 0, reportCallbackUserData);

    bool writeSuccess = true;
    writeInfo.locations.resize(size);
    // Write Objects in fileID order
    for (int i = 0; i < size; i++)
    {
        LocalIdentifierInFileType localIdentifierInFile = writeData[i].localIdentifierInFile;
        InstanceID instanceID = writeData[i].instanceID;

        SerializedObjectIdentifier identifier(serializedFileIndex, localIdentifierInFile);

        bool shouldUnloadImmediately = false;

        // internal::ScopedWriteFileBuildStep writeObjectStep(reportCallback, reportCallbackUserData, "Write object", instanceID);

        Object* o = writeData[i].objectPtr;
        if (o == NULL)
            o = Object::IDToPointer(instanceID);

        if (o == NULL)
        {
//            if (options & kLoadAndUnloadAssetsDuringBuild)
//            {
//                internal::ScopedWriteFileBuildStep loadStep(reportCallback, reportCallbackUserData, "Load object", instanceID);
//
//                o = ReadObject(instanceID, kPersistentManagerAwakeFromLoadMode | kWillUnloadAfterWritingBuildData);
//                shouldUnloadImmediately = true;
//            }

            // Object can not be loaded, don't write it
            if (o == NULL)
            {
                continue;
            }
        }

        if (verifyCallback != NULL)
        {
            VerifyWriteObjectResult result = verifyCallback(o, target.platform);
            if (result == kFailBuild)
                writeSuccess = false;
            if (result == kSkipObject)
                continue;
        }

//        // Extract script type index for monobehaviours
//        SInt16 scriptTypeIndex = -1;
//        scriptLocalIdentifier = GetScriptLocalIdentifier(o);
//        if (scriptLocalIdentifier.localIdentifierInFile != 0)
//        {
//            vector_set<LocalSerializedObjectIdentifier>::iterator found = uniqueScriptTypeReferences.find(scriptLocalIdentifier);
//            if (found == uniqueScriptTypeReferences.end())
//            {
//                ErrorString("Failed to write script type references");
//                writeSuccess = false;
//            }
//
//            scriptTypeIndex = distance(uniqueScriptTypeReferences.begin(), found);
//        }

#if UNITY_EDITOR
        if ((options & kHandleDrivenProperties) && GetDrivenPropertyManager().HasDrivenProperty(o))
        {
            dynamic_array<UInt8> buffer(kMemTempAlloc);
            WriteObjectToVector(*o, buffer, kSerializeForPrefabSystem);

            dynamic_array<UInt8> patchedBuffer(buffer);
            TypeTree typeTree;
            TypeTreeCache::GetTypeTree(o, kSerializeForPrefabSystem, typeTree);
            GetDrivenPropertyManager().PatchSerializedDataWithOriginalValues(o, typeTree, patchedBuffer);
            ReadObjectFromVector(*o, patchedBuffer, kSerializeForPrefabSystem);

            // Write
            tempSerialize->WriteObject(*o, localIdentifierInFile, scriptTypeIndex, writeData[i].buildUsage, globalBuildData);

            ReadObjectFromVector(*o, buffer, kSerializeForPrefabSystem);
        }
        else
#endif
        {
            tempSerialize->WriteObject(*o, localIdentifierInFile); //, scriptTypeIndex, writeData[i].buildUsage, globalBuildData);
        }

        writeInfo.locations[i].offset = tempSerialize->GetByteStart(localIdentifierInFile);
        writeInfo.locations[i].size = (UInt64)tempSerialize->GetByteSize(localIdentifierInFile);

        o->ClearPersistentDirty();

//        if (reportObjectNames)
//        {
//            std::string objectName = "[unnamed]";
//            NamedObject* namedObject = dynamic_pptr_cast<NamedObject*>(o);
//            if (namedObject)
//            {
//                char const* tmpObjectName = namedObject->GetName();
//                if (tmpObjectName && tmpObjectName[0] != '\0')
//                    objectName = tmpObjectName;
//            }
//            TempString objectTypeName = "";
//            const char* tmpTypeName = o->GetTypeName();
//            if (tmpTypeName && tmpTypeName[0] != '\0')
//                objectTypeName = core::Join(" (", tmpTypeName, ")");
//            writeObjectStep.UpdateStepName(core::Join("Write object - ", objectName, objectTypeName), instanceID);
//        }

        // Unload
        if (shouldUnloadImmediately)
        {
            // internal::ScopedWriteFileBuildStep unloadStep(reportCallback, reportCallbackUserData, "Unload object", instanceID);
            UnloadObject(o);
        }
    }

    ClearActiveNameSpace(kWritingNameSpace);

    // tempSerialize->SetScriptTypeReferences(&uniqueScriptTypeReferences.get_vector()[0], uniqueScriptTypeReferences.size());

    {
        // internal::ScopedWriteFileBuildStep finishWritingStep(reportCallback, reportCallbackUserData, "Finish writing");
        writeSuccess = writeSuccess && tempSerialize->FinishWriting(&writeInfo.headerOffset) ; //&& !tempSerialize->HasErrors();
    }


    // Delete temp stream
    if (m_Streams[serializedFileIndex].stream != tempSerialize)
    {
        writeSuccess = false;
        if (tempSerialize)
            tempSerialize->Release();
        tempSerialize = NULL;
    }

    // Delete mappings
    // There should not be any destroyed object on the stream at this point, so there is no point in telling the function to clean them up
    CleanupStreamAndNameSpaceMapping(serializedFileIndex, false);

    if (!writeSuccess)
    {
        //      ErrorString ("Writing file: " + path + " failed. The temporary file " + serializedFileWriter.GetPathName() + " couldn't be written.");
        return kFileCouldNotBeWritten;
    }

    // Atomically move the serialized file into the target location
    std::string actualNewPathName = RemapToAbsolutePath(path);

//    // make sure the file is not in the async read manager cache
//    AsyncReadForceCloseAllFiles();

//    if (!MoveReplaceFile(serializedFileWriter.GetPathName(), actualNewPathName))
//    {
//        ErrorString("File " + path + " couldn't be written. Because moving " + serializedFileWriter.GetPathName() + " to " + actualNewPathName + " failed.");
//        return kFileCouldNotBeWritten;
//    }
//    SetFileFlags(actualNewPathName, kFileFlagTemporary, 0);


//    if (options & kBuildResourceImage)
//    {
//        // Move the resource images into the target location
//        for (int i = 0; i < kNbResourceImages; i++)
//        {
//            std::string targetPath = AppendPathNameExtension(actualNewPathName, kResourceImageExtensions[i]);
//            core::string tempWriteFileName(resourceImageWriters[i].GetPathName(), kMemTempAlloc);
//
//            if (!GetFileLength(tempWriteFileName).IsZero())
//            {
//                if (!MoveReplaceFile(tempWriteFileName, targetPath))
//                {
//                    ErrorString("File " + path + " couldn't be written. Because moving " + tempWriteFileName + " to " + actualNewPathName + " failed.");
//                    return kFileCouldNotBeWritten;
//                }
//                SetFileFlags(targetPath, kFileFlagTemporary, 0);
//            }
//            else
//                ::DeleteFile(targetPath);
//        }
//    }

    return kNoError;
}

#if WEB_ENV

size_t LoadFileCompletely(std::string fName){
    return GetPersistentManager().LoadFileCompletely(fName);
}

EMSCRIPTEN_BINDINGS(HuaHuoEngineV2) {
    emscripten::function("LoadFileCompletely", &LoadFileCompletely);
}

#endif
#endif