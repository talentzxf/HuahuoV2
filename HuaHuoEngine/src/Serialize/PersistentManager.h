//
// Created by VincentZhang on 5/1/2022.
//

#ifndef HUAHUOENGINE_PERSISTENTMANAGER_H
#define HUAHUOENGINE_PERSISTENTMANAGER_H
#include "Utilities/vector_map.h"
#include "TypeSystem/Object.h"
#include "SerializedFileLoadError.h"
#include <set>

class FileIdentifier;
class SerializedFile;

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

class PersistentManager {
    enum LockFlags
    {
        kLockFlagNone = 0,
        kMutexLock = 1 << 0,
        kIntegrationMutexLock = 1 << 1,
    };

public:
    Remapper*                               m_Remapper;

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
private:
    typedef std::vector<StreamNameSpace> StreamContainer;
    typedef vector_map<SInt32, SInt32, std::less<SInt32> /*, STL_ALLOCATOR(kMemSerialization, IntPair)*/> IDRemap;

    StreamContainer                         m_Streams;

    std::vector<IDRemap> m_GlobalToLocalNameSpace;
    std::vector<IDRemap> m_LocalToGlobalNameSpace;

    void SetActiveNameSpace(int activeNameSpace, ActiveNameSpaceType type = kReadingNameSpace);
    void ClearActiveNameSpace(ActiveNameSpaceType type = kReadingNameSpace);

    int  GetActiveNameSpace(ActiveNameSpaceType type = kReadingNameSpace) { Assert(m_ActiveNameSpace[type] != -1); return m_ActiveNameSpace[type]; }

    SInt32                                  m_ActiveNameSpace[kActiveNameSpaceCount]; // Protected by m_Mutex
};

PersistentManager& GetPersistentManager();
void CleanupPersistentManager();
void SetPersistentManager(PersistentManager* persistentManager);


#endif //HUAHUOENGINE_PERSISTENTMANAGER_H
