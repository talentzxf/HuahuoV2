//
// Created by VincentZhang on 5/1/2022.
//

#ifndef HUAHUOENGINE_PERSISTENTMANAGER_H
#define HUAHUOENGINE_PERSISTENTMANAGER_H
#include "Utilities/vector_map.h"
#include "TypeSystem/Object.h"
#include "SerializedFileLoadError.h"
#include "SerializedFile.h"
#include "Utilities/StringComparison.h"
#include "BuildTarget.h"
#include "AwakeFromLoadQueue.h"
#include <set>

class SerializedFile;

struct HuahuoHeader{
    char magic[6];
    int version;
    size_t dataOffset = 0;
    size_t totalObjects = 0;
};

struct StreamNameSpace
{
    SerializedFile* stream;
    LocalIdentifierInFileType highestID;
    std::vector<LocalIdentifierInFileType>* destroyedObjects;
    SerializedFileLoadError loadError;

    StreamNameSpace()
            : stream(NULL)
            , highestID(0)
            , destroyedObjects(NULL)
            , loadError(kSerializedFileLoadError_None)
    {}

    std::vector<LocalIdentifierInFileType>* GetDestroyedObjectsPtr()
    {
        return destroyedObjects;
    }

    bool IsDestroyed(LocalIdentifierInFileType id);
};

class Remapper;
struct SerializedObjectIdentifier
{
    SInt32 serializedFileIndex;
    LocalIdentifierInFileType localIdentifierInFile;

    SerializedObjectIdentifier(SInt32 inSerializedFileIndex, LocalIdentifierInFileType inLocalIdentifierInFile)
            : serializedFileIndex(inSerializedFileIndex)
            , localIdentifierInFile(inLocalIdentifierInFile)
    {}

    SerializedObjectIdentifier()
            : serializedFileIndex(0)
            , localIdentifierInFile(0)
    {}


    friend bool operator<(const SerializedObjectIdentifier& lhs, const SerializedObjectIdentifier& rhs)
    {
        if (lhs.serializedFileIndex < rhs.serializedFileIndex)
            return true;
        else if (lhs.serializedFileIndex > rhs.serializedFileIndex)
            return false;
        else
            return lhs.localIdentifierInFile < rhs.localIdentifierInFile;
    }

    friend bool operator!=(const SerializedObjectIdentifier& lhs, const SerializedObjectIdentifier& rhs)
    {
        return lhs.serializedFileIndex != rhs.serializedFileIndex || lhs.localIdentifierInFile != rhs.localIdentifierInFile;
    }

    friend bool operator==(const SerializedObjectIdentifier& lhs, const SerializedObjectIdentifier& rhs)
    {
        return lhs.serializedFileIndex == rhs.serializedFileIndex && lhs.localIdentifierInFile == rhs.localIdentifierInFile;
    }
};

enum UnpersistMode
{
    kDontDestroyFromFile = 0,
    kDestroyFromFile = 1
};

enum
{
    kNoError = 0,
    kFileCouldNotBeRead = 1,
    kTypeTreeIsDifferent = 2,
    kFileCouldNotBeWritten = 3
};

enum VerifyWriteObjectResult
{
    kWriteObject,
    kSkipObject,
    kFailBuild
};

struct ThreadedAwakeData
{
    InstanceID      instanceID;
    // const TypeTree* oldType;
    Object*         object;
    bool            checkConsistency; ///< Refactor to safeLoaded

    // Has the object been fully loaded with AwakeFromLoadThreaded.
    // We have to make sure the Object* is available already, so that recursive PPtr's to each other from Mono can correctly be resolved.
    // In this case, neither object has been fully created, but we can setup pointers between them already.
    bool            completedThreadAwake;

    bool            loadStarted;      ///< Indicates, that object is being loaded.

#if UNITY_EDITOR
    AwakeFromLoadMode overrideAwakeFromLoadMode;
#endif
};

#if HUAHUO_EDITOR
struct InstanceIDResolver
{
    InstanceIDResolveCallback* callback;
    const void* context;
};
#endif

struct WriteData;
struct WriteInformation;

class PersistentManager {
    enum LockFlags
    {
        kLockFlagNone = 0,
        kMutexLock = 1 << 0,
        kIntegrationMutexLock = 1 << 1,
    };

    enum
    {
#if PLATFORM_WINRT
        kCacheSize = 1024 * 64
#elif PLATFORM_PLAYSTATION
        kCacheSize = 1024 * 64  // Important this matches the psarc block size (64KB)
#elif UNITY_SWITCH
        kCacheSize = 1024 * 64
#else
        kCacheSize = 1024 * 7
#endif
    };

public:
    PersistentManager(MemLabelId label);
    virtual ~PersistentManager(){};
    Remapper*                               m_Remapper;

    typedef std::pair<SInt32, SInt32> IntPair;
    typedef std::pair<std::string, std::string> StringPair;
    typedef std::vector<StreamNameSpace> StreamContainer;
    typedef vector_map<SInt32, SInt32, std::less<SInt32>, STL_ALLOCATOR(kMemSerialization, IntPair)> IDRemap;
    typedef vector_map<std::string, std::string, compare_string_insensitive, STL_ALLOCATOR(kMemSerialization, StringPair)> UserPathRemap;

    std::string RemapToAbsolutePath(const std::string& path);

    // On return: objects are the instanceIDs of all objects resident in the file referenced by pathName
    typedef std::set<InstanceID> ObjectIDs;

    // Computes the memoryID (object->GetInstanceID ()) from fileID
// fileID is relative to the file we are currently writing/reading from.
// It can only be called when reading/writing objects in order to
// convert ptrs from file space to global space
    void LocalSerializedObjectIdentifierToInstanceID(const LocalSerializedObjectIdentifier& identifier, InstanceID& memoryID, LockFlags lockedFlags = kLockFlagNone);
    void LocalSerializedObjectIdentifierToInstanceID(int activeNameSpace, const LocalSerializedObjectIdentifier& localIdentifier, InstanceID& outInstanceID, LockFlags lockedFlags = kLockFlagNone);

    // fileID from memory ID (object->GetInstanceID ())
    // It can only be called when reading/writing objects in order
    // to convert ptrs from global space to file space
    void InstanceIDToLocalSerializedObjectIdentifier(InstanceID memoryID, LocalSerializedObjectIdentifier& identifier);
//void LocalSerializedObjectIdentifierToInstanceID(const FileIdentifier& fileIdentifier, LocalIdentifierInFileType localID, InstanceID& outInstanceID, LockFlags lockedFlags = kLockFlagNone);

    // Translates globalIdentifier.serializedFileIndex from a global index into the local file index based on what file we are currently writing.
    // It can only be called when reading/writing objects in order
    // to convert ptrs from global space to file space
    LocalSerializedObjectIdentifier GlobalToLocalSerializedFileIndex(const SerializedObjectIdentifier& globalIdentifier);

    enum ActiveNameSpaceType { kReadingNameSpace = 0, kWritingNameSpace = 1, kActiveNameSpaceCount = 2 };

    virtual FileIdentifier PathIDToFileIdentifierInternal(int pathID) const = 0;

    // Creates the object at instanceID or returns the pointer to one that has already been created.
    // Can be called during serialization, it will not read any data but all read functions will make sure that all PreallocatedObjects will be fully read before returning.
    // (Must be surrounded by SetObjectLockForRead()  / UnlockObjectCreation)
    Object* PreallocateObjectThreaded(InstanceID instanceID, LockFlags lockedFlags = kLockFlagNone);

#if HUAHUO_EDITOR
    // Makes an object persistent and generates a unique fileID in pathName
    // The object can now be referenced by other objects that write to disk
    void MakeObjectPersistent(InstanceID heapID, std::string pathName);
    // Makes an object persistent if fileID == 0 a new unique fileID in pathName will be generated
    // The object can now be referenced by other objects that write to disk
    // If the object is already persistent in another file or another fileID it will be destroyed from that file.
    void MakeObjectPersistentAtFileID(InstanceID heapID, LocalIdentifierInFileType fileID, std::string pathName);

    /// Batch multiple heapID's and fileID's into one path name.
    /// on return fileID's will contain the file id's that were generated (if fileIds[i] is non-zero that fileID will be used instead)
    enum { kMakePersistentDontRequireToBeLoadedAndDontUnpersist = 1 << 0, kAllowDontSaveObjectsToBePersistent = 1 << 1  };
    void MakeObjectsPersistent(const InstanceID* heapIDs, LocalIdentifierInFileType* fileIDs, int size, std::string pathName, int options = 0);
#endif
    // Makes an object unpersistent
    void MakeObjectUnpersistent(InstanceID memoryID, UnpersistMode unpersistMode);

    //// Subclasses have to override these methods which map from PathIDs to FileIdentifier
    /// Maps a pathname/fileidentifier to a pathID. If the pathname is not yet known, you have to call AddStream ().
    /// The pathIDs start at 0 and increment by 1
    virtual int InsertFileIdentifierInternal(FileIdentifier file, FileIdentifier::InsertMode mode) = 0;
    virtual int InsertPathNameInternal(std::string pathname, bool create) = 0;

    /// Adds a new empty stream. Used by subclasses inside InsertPathName when a new pathID has to be added
    void AddStream();

    bool InstanceIDToSerializedObjectIdentifier(InstanceID instanceID, SerializedObjectIdentifier& identifier);

    void DestroyFromFile(InstanceID memoryID);

    ///  maps a pathID to a pathname/file guid/fileidentifier.
    /// (pathID can be assumed to be allocated before with InsertPathName)
    std::string PathIDToPathNameInternal(int pathID);

    void GetInstanceIDsAtPath(std::string& pathName, ObjectIDs* objects);

    // Returns fileIDs from path.
    void GetAllFileIDs(std::string& pathName, std::vector<LocalIdentifierInFileType>& fileIDs);

    void GetLoadedInstanceIDsAtPath(std::string& pathName, ObjectIDs* objects);

    // VZ: Created for HuaHuo Store.
    static PersistentManager* GetPersistentManager();

    typedef VerifyWriteObjectResult VerifyWriteObjectCallback (Object* verifyDeployment, BuildTargetPlatform target);

    enum ReportWriteObjectStep
    {
        ReportWriteObjectStep_CheckReportability,  // when called with this ReportWriteObjectStep type, ReportWriteObjectCallback should return a value specifying if it accepts (1) further reports for this step, or not (0)
        ReportWriteObjectStep_Begin,
        ReportWriteObjectStep_UpdateName,
        ReportWriteObjectStep_End,
    };
    typedef int ReportWriteObjectCallback (ReportWriteObjectStep reportType, int reportStep, const std::string& stepName, InstanceID assetInstanceID, void* userData);

    // Writes all persistent objects in memory that are made persistent at pathname to the file
    // And completes all write operation (including writing the header)
    // Returns the error (kNoError)
    // options: kSerializeGameRelease, kSwapEndianess, kBuildPlayerOnlySerializeBuildProperties
    int WriteFile(std::string& path, BuildTargetSelection target = BuildTargetSelection::NoTarget(), TransferInstructionFlags options = kNoTransferInstructionFlags);

    int WriteFile(std::string& path, int serializedFileIndex, const WriteData* writeData, int size, /*const GlobalBuildData& globalBuildData,*/ VerifyWriteObjectCallback* verifyCallback, BuildTargetSelection target, TransferInstructionFlags options, const InstanceIDResolver* instanceIDResolver = NULL, LockFlags lockedFlags = kLockFlagNone, ReportWriteObjectCallback* reportCallback = NULL, void* reportCallbackUserData = NULL);
    int WriteFile(std::string& path, int serializedFileIndex, const WriteData* writeData, int size, /*const GlobalBuildData& globalBuildData,*/ VerifyWriteObjectCallback* verifyCallback, BuildTargetSelection target, TransferInstructionFlags options, WriteInformation& writeInfo, const InstanceIDResolver* instanceIDResolver = NULL, LockFlags lockedFlags = kLockFlagNone, ReportWriteObjectCallback* reportCallback = NULL, void* reportCallbackUserData = NULL);

    int LoadFileCompletely(std::string& pathName);

    enum LoadFlags
    {
        kLoadFlagNone = 0,
        kSceneLoad = 1 << 0,
        kForcePreloadReferencedObjects = 1 << 1,
    };
    ENUM_FLAGS_AS_MEMBER(LoadFlags);
    /// Load the entire file from a different thread
    int LoadFileCompletelyThreaded(std::string& pathname, LocalIdentifierInFileType* fileIDs, InstanceID* instanceIDs, int size, LoadFlags flags,/* LoadProgress& loadProgress,*/ LockFlags lockedFlags = kLockFlagNone);

    int LoadFile(const char* pathName)
    {
        std::string pathNameStr(pathName);
        return LoadFileCompletely(pathNameStr);
    }

    /// Thread locking must be performed from outside using Lock/Unlock
    SerializedFile* GetSerializedFile(std::string& path, LockFlags lockedFlags = kLockFlagNone);
    SerializedFile* GetSerializedFile(int serializedFileIndex, LockFlags lockedFlags = kLockFlagNone);
    SerializedFile* GetSerializedFileIfObjectAvailable(int serializedFileIndex, LocalIdentifierInFileType id, LockFlags lockedFlags = kLockFlagNone);

    // Extract all or the specified Objects that have been read but not yet integrated.
    // (Main thread work like AwakeFromLoad and RegisterInstanceID has not yet been called on them)
    // ALl objects can then be integrated with IntegrateAwakeFromLoadQueue.
    void ExtractAwakeFromLoadQueue(AwakeFromLoadQueue& awakeQueue);
    void ExtractAwakeFromLoadQueue(const InstanceID* instanceIDs, size_t size, AwakeFromLoadQueue& awakeQueue, LockFlags lockedFlags = kLockFlagNone);
protected:
    ///  maps a pathID to a pathname/file guid/fileidentifier.
    /// (pathID can be assumed to be allocated before with InsertPathName)
    virtual std::string PathIDToPathNameInternal(int pathID, bool trackNativeLoadedAsset) const = 0;

    MemLabelId          GetMemoryLabel() const { return m_MemoryLabel; }
    void                CleanupStreamAndNameSpaceMapping(unsigned serializedFileIndex, bool cleanupDestroyedList);
private:
    StreamContainer                         m_Streams;
    bool m_ForcePreloadReferencedObjects;

    std::vector<IDRemap> m_GlobalToLocalNameSpace;
    std::vector<IDRemap> m_LocalToGlobalNameSpace;
    typedef std::map<InstanceID, ThreadedAwakeData>    ThreadedObjectActivationQueue;

    ThreadedObjectActivationQueue           m_ThreadedObjectActivationQueue; // Protected by m_IntegrationMutex

    void LoadRemainingPreallocatedObjects(LockFlags lockedFlags);
    ThreadedAwakeData*  CreateThreadActivationQueueEntry(SerializedFile& file, SerializedObjectIdentifier localIdentifier, InstanceID instanceID, bool loadStarted, LockFlags lockedFlags = kLockFlagNone);
    void                PostReadActivationQueue(InstanceID instanceID, /*const TypeTree* oldType,*/ bool didTypeTreeChange, LockFlags lockedFlags = kLockFlagNone);
    Object*             ProduceObject(SerializedFile& file, SerializedObjectIdentifier identifier, InstanceID instanceID, ObjectCreationMode objectCreationMode, LockFlags lockedFlags = kLockFlagNone);
    void SetActiveNameSpace(int activeNameSpace, ActiveNameSpaceType type = kReadingNameSpace);
    void ClearActiveNameSpace(ActiveNameSpaceType type = kReadingNameSpace);
    /// Goes through object activation queue and calls AwakeFromLoad if it has been serialized already but not AwakeFromLoad called.
    Object* GetFromActivationQueue(InstanceID instanceID, LockFlags lockedFlags = kLockFlagNone);
    void IntegrateAllThreadedObjects();
    int  GetActiveNameSpace(ActiveNameSpaceType type = kReadingNameSpace) { Assert(m_ActiveNameSpace[type] != -1); return m_ActiveNameSpace[type]; }
    void CopyToAwakeFromLoadQueueInternal(AwakeFromLoadQueue& awakeQueue);

    StreamNameSpace& GetStreamNameSpaceInternal(int nameSpaceID);
    StreamNameSpace* GetStreamNameSpaceInternal(std::string path);

    void PostLoadStreamNameSpaceInternal(StreamNameSpace& nameSpace, int namespaceID);

    void CheckInstanceIDsLoaded(InstanceID* heapIDs, int size, LockFlags lockedFlags = kLockFlagNone);
    Object*             ReadAndActivateObjectThreaded(InstanceID instanceID, const SerializedObjectIdentifier& identifier, SerializedFile* stream, bool isPersistent, bool validateLoadingFromSceneFile, LockFlags lockedFlags = kLockFlagNone);

    bool ShouldAbort() const;

    UserPathRemap                           m_UserPathRemap;

    SInt32                                  m_ActiveNameSpace[kActiveNameSpaceCount]; // Protected by m_Mutex
    MemLabelId                              m_MemoryLabel;
    int                                     m_Abort;
};

PersistentManager& GetPersistentManager();
PersistentManager* GetPersistentManagerPtr();
void CleanupPersistentManager();
void SetPersistentManager(PersistentManager* persistentManager);

#if WEB_ENV
#include <emscripten/bind.h>
size_t LoadFileCompletely(std::string fName);

#endif
#endif //HUAHUOENGINE_PERSISTENTMANAGER_H
