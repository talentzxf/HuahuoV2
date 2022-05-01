#pragma once

#include "PersistentManager.h"
#include <map>

// The Remapper is responsible for tracking the relationships between objects in memory (by heapID) and their representations on disk (fileID)
// All instance IDs that the Remapper generates are positive (compared to instance IDs for objects created directly in memory, which are negative)
// However, note that we might create an object in memory and then associate it with a file ID, so the instanceID maps here may contain negative values

class Remapper
{
public:
    static const char *kHighestInstanceIDOverflowErrorMessage;

    Remapper(/*MemLabelRef label = kMemSerialization*/);

    // Allocate - or retrieve, if we already allocated - the in-memory instance ID for the given serialized object reference.
    InstanceID GetOrGenerateInstanceID(const SerializedObjectIdentifier& identifier);

    // Install a mapping between the given instance ID and serialized object ID, replacing any existing mappings for either identifier
    void SetupRemapping(InstanceID instanceID, const SerializedObjectIdentifier& identifier);

    // Reserve a range of instance IDs for objects in the given file, all in one batch. These IDs won't be set up with full mappings - but we need to
    // ensure that the instance IDs are reserved so that they're not allocated to anything else.
    // NOTE that the first parameter is NOT 'the number of IDs to allocate', but the highest fileID, so the total allocated will be this+1!
    void PreallocateIDs(LocalIdentifierInFileType highestFileID, int serializedFileIndex, InstanceID& firstPreallocatedID, InstanceID& lastPreallocatedID);

    // Check if the given instance ID is within the latest preallocated range
    bool IsPreallocatedID(InstanceID instanceID);

    // Clear the information about the preallocated IDs
    void ClearPreallocateIDs();

    // Remove the mapping for the object with the given in-memory instance ID
    void Remove(InstanceID instanceID);

    // Remove the mapping information for all objects associated with the given serialized file, and return the instance IDs of the affected objects
    void RemoveCompleteSerializedFileIndex(int serializedFileIndex, std::vector<InstanceID>& objects);

    // Retrieve the serialized object reference for the given in-memory instance ID
    bool InstanceIDToSerializedObjectIdentifier(InstanceID instanceID, SerializedObjectIdentifier& identifier);

    // Get instance IDs of all objects for the given pathID which actually exist in memory, instead of only having registered IDs
    void GetAllLoadedObjectsForSerializedFileIndex(int serializedFileIndex, PersistentManager::ObjectIDs* objects);

    // Retrieve only the serialized file index for the given in-memory instance ID (throw away the index-within-the-file information)
    inline int GetSerializedFileIndex(InstanceID instanceID)
    {
        SerializedObjectIdentifier identifier;
        InstanceIDToSerializedObjectIdentifier(instanceID, identifier);
        return identifier.serializedFileIndex;
    }

    // Check if the given instance ID has any associated serialized object ID
    inline bool IsInstanceIDMappedToAnything(InstanceID instanceID)
    {
        Assert(m_ActivePreallocatedSerializedFileIndex == -1);
        return m_InstanceIDToSerializedObject.count(instanceID);
    }

    // Check if the given serialized object ID has any associated instance ID
    inline bool IsSerializedObjectIdentifierMappedToAnything(const SerializedObjectIdentifier& identifier)
    {
        return m_SerializedObjectToInstanceID.find(identifier) != m_SerializedObjectToInstanceID.end();
    }

    // Return the number of instanceID/fileID mappings that have been configured
    inline size_t GetNumMappings()
    {
        Assert(m_InstanceIDToSerializedObject.size() == m_SerializedObjectToInstanceID.size());
        return m_InstanceIDToSerializedObject.size();
    }

private:
    // Define a struct with the exact same layout as SerializedObjectToInstanceIDMap::_Tree_nod::_Node.
    // As on different platforms, stl implementations might have different name of the internal structure definition, we use this structure to get the size of the tree node.
    struct _Map_Node
    {
        void* _Left;
        void* _Parent;
        void* _Right;
        std::pair<SerializedObjectIdentifier, InstanceID> _Myval;
        char _Color;
        char _Isnil;
    };

#if ENABLE_CUSTOM_ALLOCATORS_FOR_STDMAP
    MemoryPool      m_SerializedObjectIdentifierPool;
    typedef std::map<SerializedObjectIdentifier, InstanceID, std::less<SerializedObjectIdentifier>, memory_pool_explicit<std::pair<const SerializedObjectIdentifier, InstanceID> > > SerializedObjectToInstanceIDMap;
#else
    typedef std::map<SerializedObjectIdentifier, InstanceID, std::less<SerializedObjectIdentifier> > SerializedObjectToInstanceIDMap;
#endif
    typedef std::unordered_map<InstanceID, SerializedObjectIdentifier> InstanceIDToSerializedObjectMap;

    // Performs the increment operation, but forces the application to crash is the increment fails
    // due to overflow. The increment operation is always performed.
    void IncreaseHighestInstanceIDAndCrashInCaseOfOverflow(int increment);

    typedef SerializedObjectToInstanceIDMap::iterator SerializedObjectToInstanceIDIterator;
    typedef InstanceIDToSerializedObjectMap::iterator InstanceIDToSerializedObjectIterator;

    SerializedObjectToInstanceIDMap m_SerializedObjectToInstanceID;
    InstanceIDToSerializedObjectMap m_InstanceIDToSerializedObject;

    // Instance ID's are simply allocated in an increasing index
    int                                             m_HighestInstanceID;

    // When loading scenes we can fast path because objects are not kept persistent / unloaded / loaded again etc.
    // So we just preallocate a bunch of id's and use those without going through a lot of table lookups.
    int                                             m_ActivePreallocatedIDBase;
    int                                             m_ActivePreallocatedIDEnd;
    int                                             m_ActivePreallocatedSerializedFileIndex;
};
