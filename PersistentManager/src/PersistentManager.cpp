//
// Created by VincentZhang on 4/1/2022.
//

#include "PersistentManager.h"
#include "BaseClasses/ClassRegistration.h"
#include "Utilities/RegisterRuntimeInitializeAndCleanup.h"
#include "TypeSystem/TypeManager.h"

void PersistentManager::InitEngine() {
    RegisterRuntimeInitializeAndCleanup::ExecuteInitializations();
    RegisterAllClasses();

    TypeManager::Get().InitializeAllTypes();
}

PersistentManager *PersistentManager::gInstance = new PersistentManager();

PersistentManager::PersistentManager() {
    // TODO: Delete this when PersistentManager is destructed.
    this->pByteArray = new ByteArray();

    this->writeHeader();
}

void PersistentManager::writeHeader() {
    HHHeader hhHeader;
    auto const headerPtr = reinterpret_cast<UInt8 *>(&hhHeader);
    this->pByteArray->write(headerPtr, sizeof(HHHeader));

}

ByteArray::ByteArray() {
    this->mpArray = pBuffer.data();
}

UInt8 ByteArray::getByte(int idx) {
    return this->mpArray[idx];
}

void ByteArray::write(UInt8 *pBuffer, int size) {
    int curPos = this->pBuffer.size();
    this->pBuffer.resize(sizeof(HHHeader));
    std::copy(pBuffer, pBuffer + size, this->pBuffer.begin() + curPos);
}

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
