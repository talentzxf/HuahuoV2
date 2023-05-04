#include "SafeBinaryRead.h"
#include "Utilities/Align.h"
#include "Utilities/StringComparison.h"
#include "GfxDevice/utilities/GfxDoubleCache.h"
#include "Serialize/TypeTreeQueries.h"
#include "Utilities/CRC.h"
#include "TransferNameConversions.h"
#include "Serialize/SerializeReferenceLabels.h"

#define LOG_MISSING_VARIBALES 0

// #if SUPPORT_SERIALIZED_TYPETREES

typedef std::pair<char*, char*> CharPtrPair;
typedef UNITY_MAP_CMP (kMemSerialization, CharPtrPair, ConversionFunction*,  smaller_cstring_pair) ConverterFunctions;
// static RuntimeStatic<ConverterFunctions> gConverterFunctions(kMemSerialization);
static ConverterFunctions gConverterFunctions;

ConversionFunction* FindConverter(const char* oldType, const char* newTypeName)
{
    std::pair<char*, char*> arg = std::make_pair(const_cast<char*>(oldType), const_cast<char*>(newTypeName));

    ConverterFunctions::iterator found = gConverterFunctions.find(arg);
    if (found == gConverterFunctions.end())
        return NULL;

    return found->second;
}

void SafeBinaryRead::RegisterConverter(const char* oldType, const char* newType, ConversionFunction* converter)
{
    std::pair<char*, char*> arg = std::make_pair(const_cast<char*>(oldType), const_cast<char*>(newType));
    AssertMsg(!gConverterFunctions.count(arg), "Duplicate conversion registered");
    gConverterFunctions[arg] = converter;
}

void SafeBinaryRead::CleanupConverterTable()
{
    gConverterFunctions.clear();
}

static void Walk(const TypeTree& typeTree, CachedReader& cache, SInt32* bytePosition, bool endianSwap, TypeTreeNode::ETypeFlags terminateWalkMask);

static inline UInt32 CRCString(const char* str) { return CRCFeed(0, reinterpret_cast<const UInt8*>(str), strlen(str)); }


SafeBinaryRead::SafeBinaryRead()
    :
    m_AllowNameConversions(nullptr)
{
    m_PositionInArray.reserve(64);
    m_StackInfo.reserve(64);
    m_UserData = NULL;
    m_DidReadLastProperty = false;
}

SafeBinaryRead::~SafeBinaryRead()
{
    Assert(m_StackInfo.empty());
    Assert(m_PositionInArray.empty());
}

CachedReader& SafeBinaryRead::Init(
    const TypeTreeIterator& oldBase,
    size_t bytePosition,
    SInt64 byteSize,
    TransferInstructionFlags flags,
    void * managedIdsReferenceToReuse)
{
    Assert(m_StackInfo.empty());
    // Assert(bytePosition.IsCastPossible<size_t>()); // because "CacheReader" is 'size_t' bound.
    m_OldBaseType = oldBase;
    m_BaseBytePosition = bytePosition;
    Assert(m_BaseBytePosition >= 0);
    m_BaseByteSize = byteSize;
    m_Flags = flags;
    m_AllowNameConversions = NULL;
    m_ReferenceFromIDCache = managedIdsReferenceToReuse;
    return m_Cache;
}

CachedReader& SafeBinaryRead::Init(SafeBinaryRead& transfer)
{
    size_t newBasePosition = transfer.m_StackInfo.back().bytePosition;
    size_t size = transfer.m_BaseByteSize - (newBasePosition - transfer.m_BaseBytePosition);
    Init(transfer.m_StackInfo.back().type, newBasePosition, size, transfer.m_Flags);
    m_Cache.InitRead(*transfer.m_Cache.GetCacher(), transfer.m_StackInfo.back().bytePosition, size);
    m_AllowNameConversions = transfer.m_AllowNameConversions;
    m_ReferenceFromIDCache = transfer.m_ReferenceFromIDCache;

    return m_Cache;
}

static void Walk(const TypeTreeIterator& type, CachedReader& cache, size_t* bytePosition, bool endianSwap, TypeTreeNode::ETypeFlags & terminateWalkMask)
{
    Assert(bytePosition != NULL);

    Assert(type->m_ByteSize == -1
        || (type->m_MetaFlag & kAnyChildUsesAlignBytesFlag) == 0
        || !type.Children().IsNull());

    TypeTreeNode::ETypeFlags orgMask = terminateWalkMask;

    if (type->m_ByteSize != -1 && (type->m_MetaFlag & kAnyChildUsesAlignBytesFlag) == 0)
    {
        *bytePosition += (UInt64)type->m_ByteSize;
    }
    else if (type->IsArray())
    {
        // First child in an array is the size
        // Second child is the homogenous type of the array
        Assert(TypeTreeQueries::GetTypeChildrenCount(type) == 2);
        Assert(type.Children().Type() == SerializeTraits<SInt32>::GetTypeString(NULL));
        Assert(type.Children().Name() == CommonString(size));

        SInt32 arraySize, i;
        cache.Read(arraySize, *bytePosition);
        if (endianSwap)
            SwapEndianBytes(arraySize);

        *bytePosition += (UInt64)sizeof(arraySize);

        TypeTreeIterator elementTypeTree = type.Children().Next();

        // If the bytesize is known we can simply skip the recursive loop
        if (elementTypeTree->m_ByteSize != -1 && (elementTypeTree->m_MetaFlag & (kAnyChildUsesAlignBytesFlag | kAlignBytesFlag)) == 0)
            *bytePosition += (UInt64)(arraySize * elementTypeTree->m_ByteSize);
        // Otherwise recursively Walk element typetree
        else
        {
            for (i = 0; i < arraySize; ++i)
            {
                Walk(elementTypeTree, cache, bytePosition, endianSwap, terminateWalkMask);
                if (terminateWalkMask != orgMask)
                    return;
            }
        }
    }
    else
    {
        if (type->m_TypeFlags & terminateWalkMask)
        {
            terminateWalkMask = TypeTreeNode::kFlagNone;
            return;
        }

        for (TypeTreeIterator i = type.Children(); !i.IsNull(); i = i.Next())
        {
            Walk(i, cache, bytePosition, endianSwap, terminateWalkMask);
            if (terminateWalkMask != orgMask)
                return;
        }
    }

    if (type->m_MetaFlag & kAlignBytesFlag)
    {
        *bytePosition = AlignToPowerOfTwo<UInt64>((UInt64)bytePosition, 4);
    }
}

// Walk through typetree and data to find the bytePosition
void SafeBinaryRead::Walk(const TypeTreeIterator& type, size_t* bytePosition, TypeTreeNode::ETypeFlags terminateWalkMask)
{
    ::Walk(type, m_Cache, bytePosition, ConvertEndianess(), terminateWalkMask);
}

void SafeBinaryRead::OverrideRootTypeName(const char* typeString)
{
    Assert(m_StackInfo.size() == 1);
    m_StackInfo.back().currentTypeName = typeString;
    #if !UNITY_RELEASE
    m_StackInfo.back().currentTypeNameCheck = CRCString(typeString);
    #endif
}

int SafeBinaryRead::BeginTransfer(const char* name, const char* typeString, ConversionFunction** converter, bool allowTypeConversion)
{
    if (converter != NULL)
        *converter = NULL;

    m_DidReadLastProperty = false;

    // For the first Transfer only setup the stack to the base parameters
    if (m_StackInfo.empty())
    {
        ErrorIf(name != m_OldBaseType.Name());

        StackedInfo& newInfo = m_StackInfo.emplace_back();
        newInfo.type = m_OldBaseType;
        newInfo.bytePosition = m_BaseBytePosition;
        newInfo.version = 1;
        newInfo.currentTypeName = typeString;
        #if !UNITY_RELEASE
        newInfo.currentTypeNameCheck = CRCString(typeString);
        #endif
        newInfo.cachedIterator = newInfo.type.Children();
        newInfo.cachedBytePosition = m_BaseBytePosition;

        m_CurrentStackInfo = &newInfo;

        return kMatchesType;
    }

    TypeTreeIterator c;

    StackedInfo& info = *m_CurrentStackInfo;

    TypeTreeIterator children = info.type.Children();

    // Start searching at the cached position
    size_t newBytePosition = info.cachedBytePosition;
    int count = 0;
    for (c = info.cachedIterator; !c.IsNull(); c = c.Next())
    {
        if (c.Name() == name)
            break;

        // Walk through old typetree, updating position
        Walk(c, &newBytePosition, TypeTreeNode::ETypeFlags::kFlagIsManagedReferenceRegistry);
        count++;
    }

    // Didn't find it, try again starting at the first child
    if (c.IsNull())
    {
        // Find name conversion lookup for this type
    #if !UNITY_RELEASE
        DebugAssert(CRCString(info.currentTypeName) == info.currentTypeNameCheck);
    #endif
        const OldTransferNames* nameConversionGlobal = GetAllowNameConversions(GetGlobalAllowNameConversions(), info.currentTypeName, name);
        const OldTransferNames* nameConversionLocal = GetAllowNameConversions(m_AllowNameConversions, info.currentTypeName, name);

        newBytePosition = info.bytePosition;
        for (c = children; !c.IsNull(); c = c.Next())
        {
            if (c.Name() == name)
                break;
            if (IsNameConversionAllowed(nameConversionLocal, c.Name().c_str()))
                break;
            if (IsNameConversionAllowed(nameConversionGlobal, c.Name().c_str()))
                break;

            // Walk through old typetree, updating position
            Walk(c, &newBytePosition, TypeTreeNode::ETypeFlags::kFlagIsManagedReferenceRegistry);
        }

        // No child with name was found?
        if (c.IsNull())
        {
            #if LOG_MISSING_VARIBALES
            core::string s("Variable not found in old file ");
            GetTypePath(m_StackInfo.back().type, s);
            s = s + " new name and type: " + name;
            s = s + '(' + typeString + ")\n";
            m_OldBaseType.DebugPrint(s, 0);
            LogString(s);
            #endif

            return kNotFound;
        }
    }

    info.cachedIterator = c;
    info.cachedBytePosition = newBytePosition;

    // Walk trough the already iterated array elements
    if (info.type->IsArray() && c != children)
    {
        SInt32 arrayPosition = *m_CurrentPositionInArray;

        // There are no arrays in the subtree so
        // we can simply use the cached bytesize
        // Alignment cuts across this so use the slow path in that case
        if (c->m_ByteSize != -1 && (c->m_MetaFlag & (kAnyChildUsesAlignBytesFlag | kAlignBytesFlag)) == 0)
        {
            newBytePosition += (UInt64)(c->m_ByteSize * arrayPosition);
        }
        // Walk through old typetree, updating position
        else
        {
            ArrayPositionInfo& arrayInfo = m_PositionInArray.back();
            SInt32 cachedArrayPosition = 0;
            if (arrayInfo.cachedArrayPosition <= arrayPosition) // uses fact that cachedArrayPosition being initialized to max<value>
            {
                newBytePosition = arrayInfo.cachedBytePosition;
                cachedArrayPosition = arrayInfo.cachedArrayPosition;
            }

            for (SInt32 i = cachedArrayPosition; i < arrayPosition; i++)
                Walk(c, &newBytePosition, TypeTreeNode::kFlagNone);

            arrayInfo.cachedArrayPosition = arrayPosition;
            arrayInfo.cachedBytePosition = newBytePosition;
        }

        (*m_CurrentPositionInArray)++;
    }

    StackedInfo& newInfo = m_StackInfo.emplace_back();
    newInfo.type = c;
    newInfo.bytePosition = newBytePosition;
    newInfo.version = 1;
    newInfo.cachedIterator = newInfo.type.Children();
    newInfo.cachedBytePosition = newBytePosition;
    newInfo.currentTypeName = typeString;
    #if !UNITY_RELEASE
    newInfo.currentTypeNameCheck = CRCString(typeString);
    #endif

    m_CurrentStackInfo = &newInfo;

    int conversion = kNeedConversion;

    // Does the type match (compare type string)
    // The root type should get a transfer in any case because the type might change
    // Eg. TransformComponent renamed to Transform (Typename mismatch but we still want to serialize)
    if (c.Type() == typeString || allowTypeConversion || m_StackInfo.size() == 1)
    {
        // Switching between referenced and in-place forces conversion.
        if (!(c->IsManagedReference() == (typeString == SerializeReferenceLabels::kManagedReferenceLabel || typeString == SerializeReferenceLabels::kManagedRefArrayItemTypeLabel)))
            return kNeedConversion;

        conversion = kMatchesType;
        if (c.ByteSize() != -1 && (c.MetaFlags() & (kAnyChildUsesAlignBytesFlag | kAlignBytesFlag)) == 0)
        {
            conversion = kFastPathKnownByteSizeArrayType;
        }
    }
    else if (conversion == kNeedConversion && converter != NULL)
        *converter = FindConverter(c.Type().c_str(), typeString);

    return conversion;
}

void SafeBinaryRead::SetVersion(int version)
{
    m_CurrentStackInfo->version = version;
}

void SafeBinaryRead::EndTransfer()
{
    m_StackInfo.pop_back();
    if (!m_StackInfo.empty())
    {
        m_CurrentStackInfo = &m_StackInfo.back();
    }
    else
        m_CurrentStackInfo = NULL;

    m_DidReadLastProperty = true;
}

bool SafeBinaryRead::BeginArrayTransfer(const char* name, const char* typeString, SInt32& size)
{
    if (BeginTransfer(name, typeString, NULL, false) == kNotFound)
        return false;

    Transfer(size, "size");
    ArrayPositionInfo info;
    info.arrayPosition = 0;
    info.cachedBytePosition = (UInt64)0;  // This value is NEVER used.
    info.cachedArrayPosition = std::numeric_limits<SInt32>::max(); // This value stand for: "not set yet"
    m_PositionInArray.push_back(info);
    m_CurrentPositionInArray = &m_PositionInArray.back().arrayPosition;

    Assert(!GetActiveOldTypeTreeIterator().Children().IsNull() && GetActiveOldTypeTreeIterator().Children().Name() == CommonString(size));

    return true;
}

void SafeBinaryRead::EndArrayTransfer()
{
    m_PositionInArray.pop_back();
    if (!m_PositionInArray.empty())
        m_CurrentPositionInArray = &m_PositionInArray.back().arrayPosition;
    else
        m_CurrentPositionInArray = NULL;

    EndTransfer();
}

bool SafeBinaryRead::IsCurrentVersion()
{
    return m_CurrentStackInfo->version == m_CurrentStackInfo->type->m_Version;
}

bool SafeBinaryRead::IsOldVersion(int version)
{
    return m_CurrentStackInfo->type->m_Version == version;
}

bool SafeBinaryRead::IsVersionSmallerOrEqual(int version)
{
    return m_CurrentStackInfo->type->m_Version <= version;
}

void SafeBinaryRead::TransferTypeless(UInt32* byteSize, const char* name, TransferMetaFlags metaflag)
{
    SInt32 size;
    if (!BeginArrayTransfer(name, "TypelessData", size))
    {
        *byteSize = 0;
        return;
    }
    // We can only transfer the array if the size was transferred as well
    Assert(!GetActiveOldTypeTreeIterator().Children().IsNull() && GetActiveOldTypeTreeIterator().Children().Name() == CommonString(size));

    *byteSize = size;

    EndArrayTransfer();
}

void SafeBinaryRead::TransferTypelessData(UInt32 byteSize, void* copyData, int metaData)
{
    if (copyData == NULL || byteSize == 0)
        return;

    m_Cache.Read(copyData, byteSize);
}

void SafeBinaryRead::TransferResourceImage(ActiveResourceImage targetResourceImage, const char* name, StreamingInfo& streamingInfo, void* buffer, UInt32 byteSize, InstanceID instanceID, const HuaHuo::Type* type)
{
    Transfer(streamingInfo, name);
}

// #endif // SUPPORT_SERIALIZED_TYPETREES
