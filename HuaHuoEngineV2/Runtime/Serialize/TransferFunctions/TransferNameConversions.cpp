#include "Configuration/IntegerDefinitions.h"
#include "TransferNameConversions.h"
#include "Utilities/RegisterRuntimeInitializeAndCleanup.h"
#include "Utilities/StringComparison.h"
#include "TypeSystem/Type.h"
#include "Memory/STLAllocator.h"
#include <map>

static AllowNameConversions* gAllowNameConversion = NULL;

typedef std::pair<char*, char*> CharPtrPair;

class AllowNameConversions : public UNITY_MAP_CMP(kMemSerialization, CharPtrPair, OldTransferNames, smaller_cstring_pair)
{
public:
    AllowNameConversions(bool inCopyOldName)
    {
        copyOldName = inCopyOldName;
    }

    //When true it will be this container responsibility to manage the memory for the oldName.
    bool copyOldName;
};

const OldTransferNames* GetAllowNameConversions(const AllowNameConversions* AllowNameConversions, const char* type, const char* name)
{
    if (AllowNameConversions == NULL)
        return NULL;

    AllowNameConversions::const_iterator foundNameConversion = AllowNameConversions->find(std::make_pair(const_cast<char*>(type), const_cast<char*>(name)));
    if (foundNameConversion != AllowNameConversions->end())
        return &foundNameConversion->second;
    else
        return NULL;
}

void RegisterAllowNameConversion(AllowNameConversions& nameConversions, const char* type, const char* oldName, const char* newName)
{
    if (nameConversions.copyOldName)
    {
        oldName = StrDup(kMemSerialization, oldName);
    }
    AllowNameConversions::mapped_type& allowed = nameConversions[std::make_pair(const_cast<char*>(type), const_cast<char*>(newName))];
    bool didInsert = allowed.insert(const_cast<char*>(oldName)).second;
    if (!didInsert && nameConversions.copyOldName)
    {
        //TODO: This code path should be never hit. in 4.6 we will keep like this, but in 5.x we should make sure we only ever build the localNameConversion once.
        //ErrorStringMsg("Trying to insert a conversion that a;ready exists. Type: %s oldName: %s, newName: %s", type, oldName, newName);
        HUAHUO_FREE(kMemSerialization, const_cast<char*>(oldName));
    }
}

void RegisterAllowNameConversion(const char* type, const char* oldName, const char* newName)
{
    RegisterAllowNameConversion(*gAllowNameConversion, type, oldName, newName);
}

#if !UNITY_EXTERNAL_TOOL
void RegisterAllowNameConversionInDerivedTypes(const char* typeName, const char* oldName, const char* newName)
{
    std::vector<const HuaHuo::Type*> types;
    const HuaHuo::Type* type = HuaHuo::Type::FindTypeByName(typeName);
    type->FindAllDerivedClasses(types, HuaHuo::Type::kOnlyNonAbstract);
    for (size_t i = 0; i < types.size(); i++)
    {
        RegisterAllowNameConversion(types[i]->GetName(), oldName, newName);
    }
}

#endif

AllowNameConversions* CreateAllowNameConversions()
{
    return HUAHUO_NEW(AllowNameConversions, kMemSerialization)(true);
}

void DestroyAllowNameConversions(AllowNameConversions* conversions)
{
    if (conversions->copyOldName)
    {
        for (AllowNameConversions::iterator i = conversions->begin(); i != conversions->end(); i++)
        {
            for (OldTransferNames::iterator o = i->second.begin(); o != i->second.end(); o++)
                HUAHUO_FREE(kMemSerialization, *o);
        }
    }

    HUAHUO_DELETE(conversions, kMemSerialization);
}

static void InitializeGlobalNameConversion(void*)
{
    gAllowNameConversion = HUAHUO_NEW_AS_ROOT_NO_LABEL(AllowNameConversions(false), kMemSerialization, "Managers", "SerializationBackwardsCompatibility");
}

static void CleanupGlobalNameConversion(void*)
{
    HUAHUO_DELETE(gAllowNameConversion, kMemSerialization);
}

const AllowNameConversions* GetGlobalAllowNameConversions()
{
    return gAllowNameConversion;
}

static RegisterRuntimeInitializeAndCleanup s_TranferNameConversionsManagerCallbacks(InitializeGlobalNameConversion, CleanupGlobalNameConversion);
