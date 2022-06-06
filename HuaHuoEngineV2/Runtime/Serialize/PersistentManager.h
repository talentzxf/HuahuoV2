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
#include <set>

class SerializedFile;

struct HuahuoHeader{
    char magic[6];
    int version;
    size_t dataOffset = 0;
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

    // VZ: Created for HuaHuo Store.
    void BeginFileWriting(std::string path);
    void BeginFileReading(std::string path);

    size_t WriteStoreFileInMemory();


    static PersistentManager* GetPersistentManager();

    UInt8* GetBufferPtr(){
        return m_Buffer.data();
    }
protected:
    ///  maps a pathID to a pathname/file guid/fileidentifier.
    /// (pathID can be assumed to be allocated before with InsertPathName)
    virtual std::string PathIDToPathNameInternal(int pathID, bool trackNativeLoadedAsset) const = 0;

    MemLabelId          GetMemoryLabel() const { return m_MemoryLabel; }

    size_t WriteStoreFileInMemoryInternal(std::vector<UInt8>& memory, Object* managerObj);

private:
    std::vector<UInt8> m_Buffer;

    StreamContainer                         m_Streams;
    bool m_ForcePreloadReferencedObjects;

    std::vector<IDRemap> m_GlobalToLocalNameSpace;
    std::vector<IDRemap> m_LocalToGlobalNameSpace;

    void SetActiveNameSpace(int activeNameSpace, ActiveNameSpaceType type = kReadingNameSpace);
    void ClearActiveNameSpace(ActiveNameSpaceType type = kReadingNameSpace);

    int  GetActiveNameSpace(ActiveNameSpaceType type = kReadingNameSpace) { Assert(m_ActiveNameSpace[type] != -1); return m_ActiveNameSpace[type]; }

    StreamNameSpace& GetStreamNameSpaceInternal(int nameSpaceID);
    StreamNameSpace* GetStreamNameSpaceInternal(std::string path);

    void PostLoadStreamNameSpaceInternal(StreamNameSpace& nameSpace, int namespaceID);

    UserPathRemap                           m_UserPathRemap;

    SInt32                                  m_ActiveNameSpace[kActiveNameSpaceCount]; // Protected by m_Mutex
    MemLabelId                              m_MemoryLabel;
    int                                     m_Abort;
};

PersistentManager& GetPersistentManager();
PersistentManager* GetPersistentManagerPtr();
void CleanupPersistentManager();
void SetPersistentManager(PersistentManager* persistentManager);

#endif //HUAHUOENGINE_PERSISTENTMANAGER_H
