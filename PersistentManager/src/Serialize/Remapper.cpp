#include "Remapper.h"
#include <limits>

const char *Remapper::kHighestInstanceIDOverflowErrorMessage = "The highest instance ID in the Remapper has overflown. The application will now exit.";

Remapper::Remapper(/*MemLabelRef label*/)
    : m_HighestInstanceID(0), m_ActivePreallocatedIDBase(0), m_ActivePreallocatedIDEnd(0), m_ActivePreallocatedSerializedFileIndex(-1)
    // map node contains 3 pointers (left, right, parent)
#if ENABLE_CUSTOM_ALLOCATORS_FOR_STDMAP
    , m_SerializedObjectIdentifierPool(label, false, "Remapper pool", sizeof(_Map_Node), 16 * 1024)
    , m_SerializedObjectToInstanceID(std::less<SerializedObjectIdentifier>(), m_SerializedObjectIdentifierPool)
#endif
    // , m_InstanceIDToSerializedObject(label)
{
}

void Remapper::PreallocateIDs(LocalIdentifierInFileType highestFileID, int serializedFileIndex, InstanceID& firstPreallocatedID, InstanceID& lastPreallocatedID)
{
    Assert(m_ActivePreallocatedSerializedFileIndex == -1);
    Assert(serializedFileIndex != -1);

    IncreaseHighestInstanceIDAndCrashInCaseOfOverflow(2);
    InstanceID_AsSInt32Ref(firstPreallocatedID) = m_ActivePreallocatedIDBase = m_HighestInstanceID;

    // Since this method performs arithmetic operations that involve highestFileID (which is a LocalIdentifierInFileType -or SInt64-)
    // and m_HighestInstanceID (which is an int), make sure that highestFileID can be safely cast to an int and that
    // when multiplied by two it doesn't overflow.
    Assert(highestFileID < std::numeric_limits<int>::max() / 2 - 1);

    int highestFileIDAsInt = (int)highestFileID;
    IncreaseHighestInstanceIDAndCrashInCaseOfOverflow(highestFileIDAsInt * 2);
    InstanceID_AsSInt32Ref(lastPreallocatedID) = m_ActivePreallocatedIDEnd = m_HighestInstanceID;
    //printf_console("Preallocating %d .. %d\n", m_ActivePreallocatedIDBase, m_ActivePreallocatedIDEnd);
    m_ActivePreallocatedSerializedFileIndex = serializedFileIndex;
}

void Remapper::ClearPreallocateIDs()
{
    Assert(m_ActivePreallocatedSerializedFileIndex != -1);
    m_ActivePreallocatedIDBase = 0;
    m_ActivePreallocatedIDEnd = 0;
    m_ActivePreallocatedSerializedFileIndex = -1;
}

void Remapper::Remove(InstanceID instanceID)
{
    Assert(m_ActivePreallocatedSerializedFileIndex == -1);

    InstanceIDToSerializedObjectIterator i = m_InstanceIDToSerializedObject.find(instanceID);
    if (i == m_InstanceIDToSerializedObject.end())
        return;

    SerializedObjectToInstanceIDIterator j = m_SerializedObjectToInstanceID.find(i->second);
    Assert(j != m_SerializedObjectToInstanceID.end());
    SerializedObjectIdentifier bug = j->first;

    m_InstanceIDToSerializedObject.erase(i);
    m_SerializedObjectToInstanceID.erase(j);
    Assert(m_SerializedObjectToInstanceID.find(bug) == m_SerializedObjectToInstanceID.end());
}

void Remapper::RemoveCompleteSerializedFileIndex(int serializedFileIndex, std::vector<InstanceID>& objects)
{
    Assert(m_ActivePreallocatedSerializedFileIndex == -1);

    SerializedObjectIdentifier proxy;
    proxy.serializedFileIndex = serializedFileIndex;
    proxy.localIdentifierInFile = std::numeric_limits<LocalIdentifierInFileType>::min();

    SerializedObjectToInstanceIDIterator begin = m_SerializedObjectToInstanceID.lower_bound(proxy);
    proxy.localIdentifierInFile = std::numeric_limits<LocalIdentifierInFileType>::max();
    SerializedObjectToInstanceIDIterator end = m_SerializedObjectToInstanceID.upper_bound(proxy);
    for (SerializedObjectToInstanceIDIterator i = begin; i != end; i++)
    {
        ErrorIf(i->first.serializedFileIndex != serializedFileIndex);
        m_InstanceIDToSerializedObject.erase(m_InstanceIDToSerializedObject.find(i->second));
        objects.push_back(i->second);
    }
    m_SerializedObjectToInstanceID.erase(begin, end);
}

bool Remapper::IsPreallocatedID(InstanceID instanceID)
{
    return m_ActivePreallocatedSerializedFileIndex != -1
        && InstanceID_AsSInt32Ref(instanceID) >= m_ActivePreallocatedIDBase
        && InstanceID_AsSInt32Ref(instanceID) <= m_ActivePreallocatedIDEnd;
}

bool Remapper::InstanceIDToSerializedObjectIdentifier(InstanceID instanceID, SerializedObjectIdentifier& identifier)
{
    // __FAKEABLE_METHOD__(Remapper, InstanceIDToSerializedObjectIdentifier, (instanceID, identifier));

    if (IsPreallocatedID(instanceID))
    {
        identifier.serializedFileIndex = m_ActivePreallocatedSerializedFileIndex;
        identifier.localIdentifierInFile = (InstanceID_AsSInt32Ref(instanceID) - m_ActivePreallocatedIDBase) / 2;
        return true;
    }

    InstanceIDToSerializedObjectIterator i = m_InstanceIDToSerializedObject.find(instanceID);
    if (i == m_InstanceIDToSerializedObject.end())
    {
        identifier.serializedFileIndex = -1;
        identifier.localIdentifierInFile = 0;
        return false;
    }
    identifier = i->second;

    return true;
}

InstanceID Remapper::GetOrGenerateInstanceID(const SerializedObjectIdentifier& identifier)
{
    if (identifier.serializedFileIndex == -1)
        return InstanceID_None;

    if (m_ActivePreallocatedSerializedFileIndex != -1 && m_ActivePreallocatedSerializedFileIndex == identifier.serializedFileIndex)
    {
        return InstanceID_Make(identifier.localIdentifierInFile * 2 + m_ActivePreallocatedIDBase);
    }

    std::pair<SerializedObjectToInstanceIDIterator, bool> inserted = m_SerializedObjectToInstanceID.insert(std::make_pair<const SerializedObjectIdentifier&, InstanceID>(identifier, InstanceID_None));
    if (inserted.second)
    {
        InstanceID instanceID = InstanceID_None;

        IncreaseHighestInstanceIDAndCrashInCaseOfOverflow(2);
        instanceID = InstanceID_Make(m_HighestInstanceID);

        inserted.first->second = instanceID;

        Assert(m_InstanceIDToSerializedObject.find(instanceID) == m_InstanceIDToSerializedObject.end());
        m_InstanceIDToSerializedObject.insert_or_assign(instanceID, identifier);

        return instanceID;
    }
    else
        return inserted.first->second;
}

void Remapper::SetupRemapping(InstanceID instanceID, const SerializedObjectIdentifier& identifier)
{
    Assert(m_ActivePreallocatedSerializedFileIndex == -1);

    if (m_InstanceIDToSerializedObject.find(instanceID) != m_InstanceIDToSerializedObject.end())
    {
        m_SerializedObjectToInstanceID.erase(m_InstanceIDToSerializedObject.find(instanceID)->second);
        m_InstanceIDToSerializedObject.erase(instanceID);
    }

    if (m_SerializedObjectToInstanceID.find(identifier) != m_SerializedObjectToInstanceID.end())
    {
        m_InstanceIDToSerializedObject.erase(m_SerializedObjectToInstanceID.find(identifier)->second);
        m_SerializedObjectToInstanceID.erase(identifier);
    }

    m_InstanceIDToSerializedObject[instanceID] = identifier;
    m_SerializedObjectToInstanceID[identifier] = instanceID;

    /*
    //      This code asserts more when something goes wrong but also in edge cases that are allowed.
    SerializedObjectIdentifier id;
    id.fileID = fileID;
    id.pathID = pathID;

    HeapIDToFileIterator inserted;
    inserted = m_HeapIDToFile.insert (std::make_pair (memoryID, id)).first;
    Assert (inserted->second == id);
    inserted->second = id;

    FileToHeapIDIterator inserted2;
    #if DEBUGMODE
    inserted2 = m_FileToHeapID.find (id);
    Assert (!(inserted2 != m_FileToHeapID.end () && inserted2->second != memoryID));
    #endif

    inserted2 = m_FileToHeapID.insert (std::make_pair (id, memoryID)).first;
    Assert (!(inserted2->second != memoryID));
    inserted2->second = memoryID;
    */
}

//void Remapper::GetAllLoadedObjectsForSerializedFileIndex(int serializedFileIndex, PersistentManager::ObjectIDs* objects)
//{
//    Assert(m_ActivePreallocatedSerializedFileIndex == -1);
//    Assert(objects != NULL);
//
//    SerializedObjectIdentifier proxy;
//    proxy.localIdentifierInFile = std::numeric_limits<LocalIdentifierInFileType>::min();
//    proxy.serializedFileIndex = serializedFileIndex;
//    SerializedObjectToInstanceIDIterator begin = m_SerializedObjectToInstanceID.lower_bound(proxy);
//    proxy.localIdentifierInFile = std::numeric_limits<LocalIdentifierInFileType>::max();
//    SerializedObjectToInstanceIDIterator end = m_SerializedObjectToInstanceID.upper_bound(proxy);
//
//    SetObjectLockForRead();
//    for (auto i = begin; i != end; ++i)
//    {
//        InstanceID instanceID = i->second;
//        Object* o = Object::IDToPointerLockTaken(instanceID);
//        if (o)
//            objects->emplace_back_unsorted(instanceID);
//    }
//    ReleaseObjectLock();
//
//    objects->sort_and_remove_duplicates();
//}

void Remapper::IncreaseHighestInstanceIDAndCrashInCaseOfOverflow(int increment)
{
    if (!DoesAdditionOverflow(m_HighestInstanceID, increment))
    {
        m_HighestInstanceID += increment;
    }
    else
    {
        ErrorString(kHighestInstanceIDOverflowErrorMessage);
        // DiagnosticsUtils_Bindings::ForceCrash(DiagnosticsUtils_Bindings::kAbort, NULL);
    }
}
