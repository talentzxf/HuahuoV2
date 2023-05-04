//
// Created by VincentZhang on 4/6/2022.
//

#include "TypeManager.h"
#include "Object.h"
#include "Utilities/ArrayUtility.h"
#include "Utilities/Word.h"

void TypeManager::InitializeGlobalInstance()
{
    AssertMsg(ms_Instance == NULL, "Global TypeManager instance is already initialized");
    ms_Instance = HUAHUO_NEW_AS_ROOT_NO_LABEL(TypeManager(RTTI::GetRuntimeTypes()), kMemBaseObject, "Managers", "RTTI");
}

void TypeManager::CleanupGlobalInstance()
{
    AssertMsg(ms_Instance != NULL, "Global TypeManager instance is not initialized");
    HUAHUO_DELETE(ms_Instance, kMemBaseObject);
}

static TypeRegistrationDesc InitializeTypeRegistrationDesc(PersistentTypeID typeID, RTTI* type, const char* name, const char* nameSpace, int size, bool isStripped)
{
    TypeRegistrationDesc desc = TYPEREGISTRATIONDESC_DEFAULT_INITIALIZER_LIST;
    desc.init.persistentTypeID = typeID;
    desc.init.className = name;
    desc.init.classNamespace = nameSpace;
    desc.init.isStripped = isStripped;
    desc.init.size = size;
    desc.type = type;
    return desc;
}

TypeManager* TypeManager::ms_Instance = NULL;

void GlobalRegisterType(const TypeRegistrationDesc& desc)
{
    TypeManager::Get().RegisterType(desc);
}

TypeManager::TypeManager(RTTI::RuntimeTypeArray& runtimeTypeArray)
        : m_RuntimeTypes(runtimeTypeArray)
{
    m_RuntimeTypes.Count = 0;
}

TypeManager::~TypeManager()
{
    DeserializationStubMap::iterator it = m_DeserializationStubMap.begin();
    for (; m_DeserializationStubMap.end() != it; ++it)
        HUAHUO_DELETE(it->second, kMemBaseObject);
    m_DeserializationStubMap.clear();
    m_RuntimeTypes.Count = 0;
}

const RTTI* TypeManager::GetDeserializationRTTIStubForPersistentTypeID(PersistentTypeID typeID) {
    {
        // ReadWriteLock::AutoReadLock readLock(m_DeserializationStubMapLock);
        const DeserializationStubMap &map = m_DeserializationStubMap;
        DeserializationStubMap::const_iterator it = map.find(typeID);
        if (it != map.end())
            return it->second;
    }
    {
        // ReadWriteLock::AutoWriteLock writeLock(m_DeserializationStubMapLock);
        RTTI *newStub = HUAHUO_NEW(RTTI, kMemBaseObject);
        RTTI defaultRTTI = RTTI_DEFAULT_INITIALIZER_LIST;
        *newStub = defaultRTTI;
        newStub->persistentTypeID = typeID;
        std::pair<DeserializationStubMap::iterator, bool> inserted = m_DeserializationStubMap.insert(std::pair<PersistentTypeID, RTTI*>(typeID, newStub));
        if (!inserted.second)
            HUAHUO_DELETE(newStub, kMemBaseObject);

        return inserted.first->second;
    }
}

void TypeManager::CallInitializeTypes()
{
    // Call the IntializeClass function for classes that have it
    // This is done after all classes are registered and the rtti setup
    // so that the rtti system can be used insie InitializeClass()
    for (TypeCallbacks::iterator i = m_TypeCallbacks.begin(); i != m_TypeCallbacks.end(); ++i)
    {
        if (i->second.initType)
            i->second.initType();
    }
}

void TypeManager::CallPostInitializeTypes()
{
    // Call the PostIntializeClass function for classes that have it
    // This is done after all classes are registered and the rtti setup
    // so that the rtti system can be used inside PostInitializeClass()
    for (TypeCallbacks::iterator i = m_TypeCallbacks.begin(); i != m_TypeCallbacks.end(); ++i)
    {
        if (i->second.postInitType)
            i->second.postInitType();
    }
}

void TypeManager::CleanupAllTypes()
{
    // this needs to be the reverse of construction order (just like c++ ctor/dtor). otherwise dependencies
    // assumed in initialization cannot be relied upon during shutdown.
    for (TypeCallbacks::reverse_iterator i = m_TypeCallbacks.rbegin(); i != m_TypeCallbacks.rend(); ++i)
    {
        if (i->second.cleanupType)
            i->second.cleanupType();
    }
}

void TypeManager::FatalErrorOnPersistentTypeIDConflict(PersistentTypeID typeID, const char* name)
{
//    RTTIMap::const_iterator iRTTI = m_RTTI.find(typeID);
//    if (iRTTI != m_RTTI.end())
//        FatalErrorString(Format("ClassID %d (%s) conflicts with that of another class (%s). Please resolve the conflict.", typeID, name, iRTTI->second->className));
//
//    ReservedTypeIDMap::const_iterator iReserved = m_ReservedTypeIDs.find(typeID);
//    if (iReserved != m_ReservedTypeIDs.end())
//        FatalErrorString(Format("ClassID %d (%s) conflicts with that of another class (%s). Please resolve the conflict.", typeID, name, iReserved->second));
}

void TypeManager::RegisterType(const TypeRegistrationDesc& desc)
{
    Assert(desc.init.persistentTypeID != RTTI::UndefinedPersistentTypeID);
    FatalErrorOnPersistentTypeIDConflict(desc.init.persistentTypeID, desc.init.className);

    // Copy info from desc to rtti
    RTTI& destinationRTTI = *desc.type;
    destinationRTTI = desc.init;

    m_RTTI[destinationRTTI.persistentTypeID] = &destinationRTTI;

    // Store callbacks
    if (desc.initCallback || desc.postInitCallback || desc.cleanupCallback)
    {
        TypeCallbackStruct& callback = m_TypeCallbacks[destinationRTTI.persistentTypeID];
        callback.initType = desc.initCallback;
        callback.postInitType = desc.postInitCallback;
        callback.cleanupType = desc.cleanupCallback;
    }

    // Store String -> persistentTypeID
    if (!destinationRTTI.isStripped)
    {
        Assert(m_StringToType.find(destinationRTTI.className) == m_StringToType.end());
        m_StringToType[destinationRTTI.className] = &destinationRTTI;
    }

#if ENABLE_ASSERTIONS
    core::hash_map<const Unity::Type*, bool> uniqueCheck;
    for (size_t i = 0; i < desc.init.attributeCount; ++i)
    {
        const Unity::Type* attrType = desc.init.attributes[i].GetType();
        AssertFormatMsg(
            uniqueCheck.insert(attrType, true).second,
            "Only a single instance of a given attribute (%s) is permitted to be registered to a type (%s)",
            attrType->GetFullName().c_str(), destinationRTTI.GetFullName().c_str());
    }
#endif
}

void TypeManager::RegisterNonObjectType(PersistentTypeID typeID, RTTI* type, const char* name, const char* nameSpace)
{
    RegisterType(InitializeTypeRegistrationDesc(typeID, type, name, nameSpace, 0, false));
}

class TypeManager::Builder
{
    struct Node
    {
        RTTI* pRTTI;
        SInt32 firstChild;
        SInt32 nextSibling;
    };

public:
    Builder()
    {
    }

    UInt32 Build(const RTTIMap& rttiMap)
    {
        LookupOrAdd(&TypeContainer<Object>::rtti); // Adding Object first ensures it gets type index 0
        for (RTTIMap::const_iterator iRTTI = rttiMap.begin(); iRTTI != rttiMap.end(); ++iRTTI)
        {
            if (!iRTTI->second->isStripped)
                LookupOrAdd(iRTTI->second);
        }

        for (RTTIMap::const_iterator iRTTI = rttiMap.begin(); iRTTI != rttiMap.end(); ++iRTTI)
        {
            DebugAssertMsg(iRTTI->second->isStripped || iRTTI->second->derivedFromInfo.typeIndex != RTTI::DefaultTypeIndex, "(Internal type system error) Type was not added");
            iRTTI->second->derivedFromInfo.typeIndex = RTTI::DefaultTypeIndex;
        }

        const UInt32 nodeCount = nodes.size();
        UInt32 nextTypeIndex = 0;
        for (UInt32 iNode = 0; iNode < nodeCount; ++iNode)
        {
            const Node& node = nodes[iNode];
            if (node.pRTTI->isStripped)
                continue;

            if (node.pRTTI->derivedFromInfo.typeIndex == RTTI::DefaultTypeIndex)
                nextTypeIndex += TraverseDepthFirst(node, nextTypeIndex);
        }

        return nextTypeIndex;
    }

    UInt32 TraverseDepthFirst(const Node& node, UInt32 typeIndex)
    {
        UInt32 descendantCount = 1;
        for (SInt32 child = node.firstChild; child != -1; child = nodes[child].nextSibling)
            descendantCount += TraverseDepthFirst(nodes[child], typeIndex + descendantCount);

        RTTI::DerivedFromInfo& derivedFromInfo = node.pRTTI->derivedFromInfo;
        DebugAssertMsg((derivedFromInfo.typeIndex == RTTI::DefaultTypeIndex) &&
                       (derivedFromInfo.descendantCount == RTTI::DefaultDescendentCount), "Type is already initialized?");
        derivedFromInfo.typeIndex = typeIndex;
        derivedFromInfo.descendantCount = descendantCount;
        return descendantCount;
    }

private:

    SInt32 Add(RTTI* pRTTI)
    {
        Assert(pRTTI->derivedFromInfo.typeIndex == RTTI::DefaultTypeIndex);

        RTTI* pBaseRTTI = const_cast<RTTI*>(pRTTI->base);
        SInt32 baseID = pBaseRTTI ? LookupOrAdd(pBaseRTTI) : -1;

        SInt32 newNodeID = nodes.size();
        Node& newNode = nodes.emplace_back();
        newNode.pRTTI = pRTTI;
        newNode.firstChild = -1;

        pRTTI->derivedFromInfo.typeIndex = newNodeID;

        if (pBaseRTTI == NULL)
        {
            newNode.nextSibling = -1;
        }
        else
        {
            SInt32* prevNodeNext = &nodes[baseID].firstChild;
            while ((*prevNodeNext != -1) && (strcmp(nodes[*prevNodeNext].pRTTI->className, pRTTI->className) < 0))
            {
                prevNodeNext = &nodes[*prevNodeNext].nextSibling;
            }
            newNode.nextSibling = *prevNodeNext;
            *prevNodeNext = newNodeID;
        }

        return newNodeID;
    }

    SInt32 LookupOrAdd(RTTI* pRTTI)
    {
        SInt32 newNodeId = pRTTI->derivedFromInfo.typeIndex;
        if (newNodeId == RTTI::DefaultTypeIndex)
            newNodeId = Add(pRTTI);
        return newNodeId;
    }

    std::vector<Node> nodes;
};

void TypeManager::InitializeAllTypes() {
    Builder builder;
    m_RuntimeTypes.Count = builder.Build(m_RTTI);
    AssertMsg(m_RuntimeTypes.Count < m_RuntimeTypes.MAX_RUNTIME_TYPES, "Too many runtime types! Need to bump the MAX_RUNTIME_TYPES");

    // AttributeLookupMap attributeLookupMap = CreateAttributeLookupMap();

    for (RTTIMap::iterator i1 = m_RTTI.begin(); i1 != m_RTTI.end(); ++i1)
    {
        if (i1->second->isStripped)
            continue;

        RTTI::DerivedFromInfo& rtti_info = i1->second->derivedFromInfo;
        AssertFormatMsg(
                rtti_info.typeIndex < ARRAY_SIZE(m_RuntimeTypes.Types),
                "Invalid type index of %d (0x%08X); if it is 0x%08X [DefaultTypeIndex], then the problem is that the type has not been registered",
                rtti_info.typeIndex, rtti_info.typeIndex, RTTI::DefaultTypeIndex);
        m_RuntimeTypes.Types[rtti_info.typeIndex] = i1->second;
        // RegisterTypeInGlobalAttributeMap(*i1->second, attributeLookupMap);
    }
}


const RTTI* TypeManager::ClassNameToRTTI(const char* name, bool caseInsensitive) const
{
    if (!caseInsensitive)
    {
        StringToTypeMap::const_iterator i = m_StringToType.find(name);
        return i != m_StringToType.end() ? i->second : NULL;
    }
    else
    {
        for (StringToTypeMap::const_iterator i = m_StringToType.begin(); i != m_StringToType.end(); ++i)
        {
            if (StrIEquals(name, i->first))
                return i->second;
        }
        return NULL;
    }
}

const RTTI* TypeManager::PersistentTypeIDToRTTI(PersistentTypeID id) const
{
    RTTIMap::const_iterator i = m_RTTI.find(id);
    if (i == m_RTTI.end())
        return NULL;
    return i->second;
}

void TypeManager::FindAllRTTIDerivedTypes(const RTTI* baseType, std::vector<const RTTI*>& derivedTypes, bool onlyNonAbstract) const
{
    const UInt32 typeIndexBegin = baseType->derivedFromInfo.typeIndex;
    const UInt32 typeIndexEnd = baseType->derivedFromInfo.typeIndex + baseType->derivedFromInfo.descendantCount;

    derivedTypes.reserve(baseType->derivedFromInfo.descendantCount);
    if (onlyNonAbstract)
    {
        for (UInt32 typeIndex = typeIndexBegin; typeIndex < typeIndexEnd; ++typeIndex)
        {
            const RTTI* type = m_RuntimeTypes.Types[typeIndex];
            if (!type->isAbstract)
                derivedTypes.push_back(m_RuntimeTypes.Types[typeIndex]);
        }
    }
    else
    {
        for (UInt32 typeIndex = typeIndexBegin; typeIndex < typeIndexEnd; ++typeIndex)
            derivedTypes.push_back(m_RuntimeTypes.Types[typeIndex]);
    }
}