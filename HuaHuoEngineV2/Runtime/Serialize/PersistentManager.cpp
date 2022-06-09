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

bool StreamNameSpace::IsDestroyed(LocalIdentifierInFileType id)
{
    if (destroyedObjects == NULL)
        return false;

    return std::find(destroyedObjects->begin(), destroyedObjects->end(), id) != destroyedObjects->end();
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
    if (serializedFileIndex == -1)
        return kNoError;

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

    printf("%s, %d\n", __FILE__, __LINE__);
    // Remap globalPathID to localPathID
    int activeNameSpace = GetActiveNameSpace(kWritingNameSpace);

    printf("Active name space is:%d\n", activeNameSpace);

    IDRemap& globalToLocalNameSpace = m_GlobalToLocalNameSpace[activeNameSpace];
    IDRemap& localToGlobalNameSpace = m_LocalToGlobalNameSpace[activeNameSpace];

    printf("%s, %d\n", __FILE__, __LINE__);
    IDRemap::iterator found = globalToLocalNameSpace.find(globalIdentifier.serializedFileIndex);
    printf("%s, %d\n", __FILE__, __LINE__);
    if (found == globalToLocalNameSpace.end())
    {
        printf("%s, %d\n", __FILE__, __LINE__);
        // SET_ALLOC_OWNER(m_MemoryLabel);
        Assert(activeNameSpace < (int)m_Streams.size());
        Assert(m_Streams[activeNameSpace].stream != NULL);

        if(NULL == m_Streams[activeNameSpace].stream){
            printf("Stream is null!!\n");
        }

        printf("Get stream from active namespace:%d\n", activeNameSpace);

        SerializedFile& serialize = *m_Streams[activeNameSpace].stream;

        printf("%s, %d\n", __FILE__, __LINE__);

        FileIdentifier fileIdentifier = PathIDToFileIdentifierInternal(globalIdentifier.serializedFileIndex);

        printf("%s, %d\n", __FILE__, __LINE__);
        serialize.AddExternalRef(fileIdentifier);

        printf("%s, %d\n", __FILE__, __LINE__);

        // localIdentifierInFile mapping is not zero based. zero is reserved for mapping into the same file.
        localSerializedFileIndex = serialize.GetExternalRefs().size();

        printf("%s, %d\n", __FILE__, __LINE__);

        globalToLocalNameSpace[globalIdentifier.serializedFileIndex] = localSerializedFileIndex;
        printf("%s, %d\n", __FILE__, __LINE__);
        localToGlobalNameSpace[localSerializedFileIndex] = globalIdentifier.serializedFileIndex;
        printf("%s, %d\n", __FILE__, __LINE__);
    }
    else
        localSerializedFileIndex = found->second;
    printf("%s, %d\n", __FILE__, __LINE__);

    // Setup local identifier
    LocalSerializedObjectIdentifier localIdentifier;
    printf("%s, %d\n", __FILE__, __LINE__);
    localIdentifier.localSerializedFileIndex = localSerializedFileIndex;
    localIdentifier.localIdentifierInFile = localIdentifierInFile;
    printf("%s, %d\n", __FILE__, __LINE__);
    return localIdentifier;
}


void PersistentManager::InstanceIDToLocalSerializedObjectIdentifier(InstanceID instanceID, LocalSerializedObjectIdentifier& localIdentifier)
{
    // PERSISTENT_MANAGER_AUTOLOCK2(autoLock, HuaHuoEngine::kMutexLock, NULL, &gIDRemappingProfiler);
    printf("%s, %d\n", __FILE__, __LINE__);
    if (instanceID == InstanceID_None)
    {
        localIdentifier.localSerializedFileIndex = 0;
        localIdentifier.localIdentifierInFile = 0;
        return;
    }

    printf("%s, %d\n", __FILE__, __LINE__);
    SerializedObjectIdentifier globalIdentifier;
    printf("%s, %d\n", __FILE__, __LINE__);
    if (!m_Remapper->InstanceIDToSerializedObjectIdentifier(instanceID, globalIdentifier))
    {
        printf("%s, %d\n", __FILE__, __LINE__);
        localIdentifier.localSerializedFileIndex = 0;
        localIdentifier.localIdentifierInFile = 0;
        return;
    }

    printf("%s, %d\n", __FILE__, __LINE__);
    localIdentifier = GlobalToLocalSerializedFileIndex(globalIdentifier);
    printf("%s, %d\n", __FILE__, __LINE__);
}

#if HUAHUO_EDITOR
void PersistentManager::MakeObjectPersistent(InstanceID heapID, std::string pathName)
{
    printf("%s,%d\n", __FILE__, __LINE__);
    MakeObjectPersistentAtFileID(heapID, 0, pathName);
}

void PersistentManager::MakeObjectPersistentAtFileID(InstanceID heapID, LocalIdentifierInFileType fileID, std::string pathName)
{
    printf("%s,%d\n", __FILE__, __LINE__);
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
    printf("%s,%d\n", __FILE__, __LINE__);
    Assert(!pathName.empty());
    printf("%s,%d\n", __FILE__, __LINE__);
    if(this == NULL)
        printf("%s,%d\n", __FILE__, __LINE__);
    SInt32 globalNameSpace = InsertPathNameInternal(pathName, true);
    printf("%s,%d\n", __FILE__, __LINE__);
    StreamNameSpace* streamNameSpace = NULL;
    printf("%s,%d\n", __FILE__, __LINE__);
    for (int i = 0; i < size; i++)
    {
        printf("%s,%d\n", __FILE__, __LINE__);
        InstanceID heapID = heapIDs[i];
        LocalIdentifierInFileType fileID = fileIDs[i];
        printf("%s,%d\n", __FILE__, __LINE__);
        Object* o = Object::IDToPointer(heapID);
        printf("%s,%d\n", __FILE__, __LINE__);
        if ((options & kMakePersistentDontRequireToBeLoadedAndDontUnpersist) == 0)
        {
            printf("%s,%d\n", __FILE__, __LINE__);
            // Making an object that is not in memory persistent
            if (o == NULL)
            {
                ErrorString("Make Objects Persistent failed because the object can not be loaded");
                continue;
            }
            printf("%s,%d\n", __FILE__, __LINE__);
            // Make Object unpersistent first
            if (o->IsPersistent())
            {
                printf("%s,%d\n", __FILE__, __LINE__);
                SerializedObjectIdentifier identifier;
                InstanceIDToSerializedObjectIdentifier(heapID, identifier);
                Assert(identifier.serializedFileIndex != -1);

                printf("%s,%d\n", __FILE__, __LINE__);
                // Return if the file and serializedFileIndex is not going to change
                if (globalNameSpace == identifier.serializedFileIndex)
                {
                    if (fileID == 0 || identifier.localIdentifierInFile == fileID)
                        continue;
                }
                printf("%s,%d\n", __FILE__, __LINE__);
                MakeObjectUnpersistent(heapID, kDestroyFromFile);
            }
        }
        printf("%s,%d\n", __FILE__, __LINE__);
        if (streamNameSpace == NULL)
            streamNameSpace = &GetStreamNameSpaceInternal(globalNameSpace);
        printf("%s,%d\n", __FILE__, __LINE__);
        // Allocate an fileID for this object in the File
        if (fileID == 0)
        {
            printf("%s,%d\n", __FILE__, __LINE__);
            fileID = streamNameSpace->highestID;
            if (streamNameSpace->stream)
                fileID = std::max(streamNameSpace->highestID, streamNameSpace->stream->GetHighestID());
            fileID++;
        }
        printf("%s,%d\n", __FILE__, __LINE__);
        streamNameSpace->highestID = std::max(streamNameSpace->highestID, fileID);
        printf("%s,%d\n", __FILE__, __LINE__);
        SerializedObjectIdentifier identifier;
        identifier.serializedFileIndex = globalNameSpace;
        identifier.localIdentifierInFile = fileID;
        m_Remapper->SetupRemapping(heapID, identifier);
        fileIDs[i] = fileID;
        printf("%s,%d\n", __FILE__, __LINE__);
        if (o)
        {
            printf("%s,%d\n", __FILE__, __LINE__);
            // Assert(!(o->TestHideFlag(Object::kDontSaveInEditor) && (options & kAllowDontSaveObjectsToBePersistent) == 0));
            o->SetIsPersistent(true);
            printf("%s,%d\n", __FILE__, __LINE__);
            o->SetFileIDHint(fileID);
            o->SetDirty();
        }
    }
    printf("%s,%d\n", __FILE__, __LINE__);
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
#endif