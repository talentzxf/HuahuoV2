//
// Created by VincentZhang on 5/1/2022.
//

#include "PersistentManager.h"
#include "Remapper.h"
#include "SerializedFile.h"

void PersistentManager::LocalSerializedObjectIdentifierToInstanceID(const LocalSerializedObjectIdentifier& localIdentifier, InstanceID& outInstanceID, LockFlags lockedFlags)
{
    LocalSerializedObjectIdentifierToInstanceID(-1, localIdentifier, outInstanceID, lockedFlags);
}

void PersistentManager::LocalSerializedObjectIdentifierToInstanceID(int activeNameSpace, const LocalSerializedObjectIdentifier& localIdentifier, InstanceID& outInstanceID, LockFlags lockedFlags)
{
//    LocalIdentifierInFileType localIdentifierInFile = localIdentifier.localIdentifierInFile;
//    int localSerializedFileIndex = localIdentifier.localSerializedFileIndex;
//
//    if (localIdentifierInFile == 0)
//    {
//        outInstanceID = InstanceID_None;
//        return;
//    }
//
//    //PERSISTENT_MANAGER_AUTOLOCK(autoLock, kMutexLock, lockedFlags, gIDRemappingProfiler);
//
//    if (activeNameSpace == -1)
//        activeNameSpace = GetActiveNameSpace();
//
//    Assert(localSerializedFileIndex != -1);
//
//    int globalFileIndex;
//    if (localSerializedFileIndex == 0)
//        globalFileIndex = activeNameSpace;
//    else
//    {
//        Assert(m_Streams[activeNameSpace].stream != NULL);
//
//        Assert(activeNameSpace < (int)m_LocalToGlobalNameSpace.size() && activeNameSpace >= 0);
//
//        IDRemap::iterator found = m_LocalToGlobalNameSpace[activeNameSpace].find(localSerializedFileIndex);
//
//        if (found != m_LocalToGlobalNameSpace[activeNameSpace].end())
//        {
//            globalFileIndex = found->second;
//        }
//        else
//        {
//            AssertString("illegal LocalPathID in persistentmanager");
//            outInstanceID = InstanceID_None;
//            return;
//        }
//    }
//
//    SerializedObjectIdentifier globalIdentifier;
//    globalIdentifier.serializedFileIndex = globalFileIndex;
//    globalIdentifier.localIdentifierInFile = localIdentifierInFile;
//
//#if SUPPORT_INSTANCE_ID_REMAP_ON_LOAD
//    ApplyInstanceIDRemapInternal(globalIdentifier);
//#endif
//
//    outInstanceID = m_Remapper->GetOrGenerateInstanceID(globalIdentifier);
//
//    // Preallocate all referenced objects right away this we we ensure that the loading code will actually load them.
//    if (m_ForcePreloadReferencedObjects && outInstanceID != InstanceID_None)
//    {
//        autoLock.Unlock(kMutexLock);
//        PreallocateObjectThreaded(outInstanceID, lockedFlags);
//    }
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
    DELETE(gPersistentManager);
    gPersistentManager = NULL;
}


void SetPersistentManager(PersistentManager* persistentManager)
{
    gPersistentManager = persistentManager;
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
