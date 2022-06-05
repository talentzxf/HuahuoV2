//
// Created by VincentZhang on 5/1/2022.
//

#include "PersistentManager.h"
#include "Remapper.h"
#include "SerializedFile.h"
#include "Utilities/File.h"
#include "Serialize/SerializationCaching/FileCacherWrite.h"

#if DEBUGMODE
#define CheckedAssert(x) Assert(x)
#else
#define CheckedAssert(x)
#endif

#if HUAHUO_EDITOR || SUPPORT_RESOURCE_IMAGE_LOADING
static const char* kResourceImageExtensions[] = { "resG", "res", "resS" };
#endif

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

Object* PersistentManager::PreallocateObjectThreaded(InstanceID instanceID, LockFlags lockedFlags)
{
//    PERSISTENT_MANAGER_AUTOLOCK(autoLock, kMutexLock | kIntegrationMutexLock, lockedFlags, gLoadFromActivationQueueStall);
//    Object* obj = Object::IDToPointerThreadSafe(instanceID);
//
//    if (obj != NULL)
//        return obj;
//
//    Object* o = GetFromActivationQueue(instanceID, lockedFlags);
//    if (o != NULL)
//        return o;
//
//    // Find and load the right stream
//    SerializedObjectIdentifier identifier;
//    if (!m_Remapper->InstanceIDToSerializedObjectIdentifier(instanceID, identifier))
//        return NULL;
//
//    SerializedFile* stream = GetSerializedFileIfObjectAvailable(identifier.serializedFileIndex, identifier.localIdentifierInFile, lockedFlags);
//    if (stream == NULL)
//        return NULL;
//
//    ThreadedAwakeData* awakeData = CreateThreadActivationQueueEntry(*stream, identifier, instanceID, false, lockedFlags);
//    if (awakeData != NULL)
//        return awakeData->object;
//    else
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

static bool InitTempWriteFile(FileCacherWrite& writer, const std::string& path, unsigned cacheSize, bool shouldBeOnMemoryFileSystem)
{
    std::string tempWriteFileName = GenerateUniquePathSafe(std::string(shouldBeOnMemoryFileSystem ? "mem:/" + path : path));
    if (tempWriteFileName.empty())
        return false;

    return writer.InitWriteFile(shouldBeOnMemoryFileSystem ? "mem:/" + path : path, cacheSize);
}

void PersistentManager::BeginFileWriting(std::string path){
    int serializedFileIndex;
    serializedFileIndex = InsertPathNameInternal(path, false);

    // Create writable stream
    SerializedFile* tempSerialize = HUAHUO_NEW_AS_ROOT(SerializedFile, kMemSerialization, kSerializedFileArea, "") ();
#if UNITY_EDITOR
    tempSerialize->SetDebugPath(PathIDToPathNameInternal(serializedFileIndex, false));
#if ENABLE_MEM_PROFILER
    GetMemoryProfiler()->SetRootAllocationObjectName(tempSerialize, kMemSerialization, tempSerialize->GetDebugPath().c_str());
#endif
#endif
    // Create writing tools
    CachedWriter writer;
    FileCacherWrite serializedFileWriter;

    if (!InitTempWriteFile(serializedFileWriter,path, kCacheSize, false))
        return;
    writer.InitWrite(serializedFileWriter);

    tempSerialize->InitializeWrite(writer, /*target,*/ kReadWriteFromSerializedFile);
    m_Streams[serializedFileIndex].stream = tempSerialize;

    SetActiveNameSpace(serializedFileIndex, kWritingNameSpace);
}

void PersistentManager::BeginFileReading(std::string path){
    int serializedFileIndex;
    serializedFileIndex = InsertPathNameInternal(path, false);
    SetActiveNameSpace(serializedFileIndex, kReadingNameSpace);
}

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

    IDRemap& globalToLocalNameSpace = m_GlobalToLocalNameSpace[activeNameSpace];
    IDRemap& localToGlobalNameSpace = m_LocalToGlobalNameSpace[activeNameSpace];

    IDRemap::iterator found = globalToLocalNameSpace.find(globalIdentifier.serializedFileIndex);
    if (found == globalToLocalNameSpace.end())
    {
        // SET_ALLOC_OWNER(m_MemoryLabel);
        Assert(activeNameSpace < (int)m_Streams.size());
        Assert(m_Streams[activeNameSpace].stream != NULL);
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
#if !UNITY_EDITOR
        AssertString("PersistentManager: File '" + pathName + "' was expected to exist at absolute path '" + absolutePath + "'");
#endif
        return nameSpace;
    }

    // Is Builtin resource file?
    TransferInstructionFlags options = kNoTransferInstructionFlags;
//    if (IsPathBuiltinResourceFile(pathName))
//        options |= kIsBuiltinResourcesFile;

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

#endif