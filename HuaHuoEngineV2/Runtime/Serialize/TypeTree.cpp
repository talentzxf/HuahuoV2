//
// Created by VincentZhang on 2023-04-27.
//

#include "TypeTree.h"
#include "Serialize/SwapEndianBytes.h"
#include "Utilities/Word.h"
#include "Containers/CommonString.h"


#if ENABLE_PERFORMANCE_TESTS
void TypeTreePerformanceTestTouchValue(void * value)
{
    // does nothing: helper to prevent performance test from being optimized away by the compiler
}

#endif

#if ENABLE_SECURITY
#define TEST_LEN(x) if (iterator + sizeof(x) > end) \
{\
    return false; \
}
#else
#define TEST_LEN(x)
#endif

namespace TypeTreeQueries
{
    bool GetTypeTreeFromReferencedType(const TypeTreeIterator& type, const UInt8* data, int* bytePosition, TypeTree & refedTypeTree);
}

//----------------------------------------------------------------------------------------------------------------------
// What is this: Opens in a controled manor, access to "internal" methods of TypeTreeIterator.
//               It's just a way to work arround language expressivety limitations.
// Motivation  : Certain methods of TypeTreeIterator are for internal use by the coupled class TypeTree. We want to expose
//               these methods to TypeTree only without resorting to making it a friend class, to limit encapsulation violations,
//               while keeping those methods unavailable from the public API.
// Note:
// - This class is able to violate encapsulation, but does it in such a blatant manour that it sticks out as a sore thumb and
//    should trigger questioning in the mind of whoever is thinking of adding to it.
// - This is not for "ease of use", quite the contrary: it's to restrict access while allowing/controling exceptions.
//----------------------------------------------------------------------------------------------------------------------
class TypeTreeIteratorVIP
{
    friend class TypeTree;
    friend bool TypeTreeQueries::GetTypeTreeFromReferencedType(const TypeTreeIterator& type, const UInt8* data, int* bytePosition, TypeTree & refedTypeTree);

    static inline size_t GetNodeIndex(const TypeTreeIterator & iterator)               { return iterator.m_NodeIndex; }
    static inline bool IsIteratorOnTypeTree(const TypeTreeIterator& i, const TypeTree* type) { return i.m_LinkedTypeTree == type;  }
    static inline TypeTreeShareableData * GetTypeTreeData(const TypeTreeIterator& i)
    {
        return const_cast<TypeTreeShareableData*>(i.m_TypeTreeData);
    }

    static inline void Copy(TypeTreeIterator & node, const TypeTreeIterator & reference, size_t nodeIndex)
    {
        node.m_LinkedTypeTree = reference.m_LinkedTypeTree;
        node.m_TypeTreeData = reference.m_TypeTreeData;
        node.m_NodeIndex = nodeIndex;
    }
};

void TypeTreeIterator::DebugPrint(std::string& buffer, int level,  DebugPrintInstructions instructions) const
{
    const TypeTreeNode* node = GetNode();
    for (int i = 0; i < level; ++i)
        buffer += "\t";
    if (::HasFlag(instructions, DebugPrintInstructions::kName))
        buffer += Name().c_str();
    if (::HasFlag(instructions, DebugPrintInstructions::kType))
    {
        buffer += " Type:";
        buffer += Type().c_str();
    }
    if (::HasFlag(instructions, DebugPrintInstructions::kSize))
        buffer += " ByteSize:" + IntToString(node->m_ByteSize);
    if (::HasFlag(instructions, DebugPrintInstructions::kMetaFags))
        buffer += " MetaFlag:" + IntToString(node->m_MetaFlag);
    if (::HasFlag(instructions, DebugPrintInstructions::kOffsets))
    {
        if (ByteOffset() != -1 && ByteOffset() != TypeTree::kByteOffsetUnset)
            buffer += " Offset:" + IntToString(ByteOffset() & TypeTree::kByteOffsetMask);
    }
    if (::HasFlag(instructions, DebugPrintInstructions::kTypeFlags))
    {
        if (node->IsArray())
            buffer += " IsArray";
        if (node->IsManagedReference())
            buffer += " IsRef";
        if (node->IsManagedReferenceRegistry())
            buffer += " IsRegistry";
        if (node->IsArrayOfRefs())
            buffer += " IsArrayOfRefs";
    }
    if (::HasFlag(instructions, DebugPrintInstructions::kIndex))
    {
        buffer += " (node index: ";
        buffer += IntToString(m_NodeIndex);
        buffer += ")";
    }
    buffer += "\n";
    for (TypeTreeIterator i = Children(); !i.IsNull(); i = i.Next())
        i.DebugPrint(buffer, level + 1, instructions);
}

// Search towards the head of the node array to find the first node that has the level equal to the current node's level - 1.
TypeTreeIterator TypeTreeIterator::Father() const
{
    Assert(!IsNull());
    const TypeTreeNode* node = GetNode();
    const UInt32 fatherLevel = node->m_Level - 1;

    for (const TypeTreeNode* p = node - 1; p >= m_TypeTreeData->Nodes().data(); --p)
    {
        if (p->m_Level == fatherLevel)
            return TypeTreeIterator(*this, p - m_TypeTreeData->Nodes().data());
    }

    return TypeTreeIterator();
}

// The next node is current node's first child if the next node is within the node array range and has the level equal to the current
// node's level + 1.
TypeTreeIterator TypeTreeIterator::Children() const
{
    Assert(!IsNull());
    const TypeTreeNode* node = GetNode();
    const TypeTreeNode* end = m_TypeTreeData->Nodes().data() + m_TypeTreeData->NodeCount();
    return node + 1 < end && (node + 1)->m_Level == node->m_Level + 1
           ? TypeTreeIterator(*this, m_NodeIndex + 1)
           : TypeTreeIterator();
}

// Search towards the tail of the node array to find the first node that has a level equal to the current node's level. It a node with a lower
// level is encountered first, no sibling is found.
TypeTreeIterator TypeTreeIterator::Next() const
{
    Assert(!IsNull());
    const TypeTreeNode* node = GetNode();
    const UInt32 siblingLevel = node->m_Level;
    const TypeTreeNode* end = m_TypeTreeData->Nodes().data() + m_TypeTreeData->Nodes().size();
    for (const TypeTreeNode* p = node + 1; p < end; ++p)
    {
        if (p->m_Level > siblingLevel)
            continue;
        return p->m_Level == siblingLevel
               ? TypeTreeIterator(*this, p - m_TypeTreeData->Nodes().data())
               : TypeTreeIterator();
    }
    return TypeTreeIterator();
}

// Get the last sibling.
TypeTreeIterator TypeTreeIterator::Last() const
{
    Assert(!IsNull());
    TypeTreeIterator i = *this;
    TypeTreeIterator next;
    while (true)
    {
        next = i.Next();
        if (next.IsNull())
            break;
        i = next;
    }
    return i;
}

TypeTreeIterator TypeTreeIterator::FindChild(std::string name) const
{
    Assert(!IsNull());
    for (TypeTreeIterator i = Children(); !i.IsNull(); i = i.Next())
        if (name == i.Name().c_str())
            return i;
    return TypeTreeIterator();
}

// Extract the actual string pointer from an offset value
static const char* CalculateString(UInt32 offset, const char* stringBuffer)
{
    return ((offset & TypeTree::kCommonStringBit) ? HuaHuo::CommonString::BufferBegin : stringBuffer)
           + (offset & TypeTree::kStringOffsetMask);
}

TypeTreeString TypeTreeIterator::Type() const
{
    Assert(!IsNull());
    TypeTreeString str;
    str.m_Buffer = CalculateString(GetNode()->m_TypeStrOffset, m_TypeTreeData->StringsBuffer().data());
    return str;
}

TypeTreeString TypeTreeIterator::Name() const
{
    Assert(!IsNull());
    TypeTreeString str;
    str.m_Buffer = CalculateString(GetNode()->m_NameStrOffset, m_TypeTreeData->StringsBuffer().data());
    return str;
}

UInt32 TypeTreeIterator::ByteOffset() const
{
    Assert(!IsNull());
    return m_NodeIndex < m_TypeTreeData->ByteOffsetsCount() ? m_TypeTreeData->ByteOffsets()[m_NodeIndex] : TypeTree::kByteOffsetUnset;
}

const TypeTreeNode* TypeTreeIterator::GetNode() const
{
    Assert(!IsNull());
    Assert(m_NodeIndex < m_TypeTreeData->NodeCount());
    return &m_TypeTreeData->Nodes()[m_NodeIndex];
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void InitializeTypeTreeNode(TypeTreeNode& node, UInt8 level)
{
    node.m_Level = level;
    node.m_NameStrOffset = 0;
    node.m_TypeStrOffset = 0;
    node.m_Index = -1;
    node.m_TypeFlags = 0;
    node.m_Version = 1;
    node.m_MetaFlag = kNoTransferFlags;
    node.m_ByteSize = -1;
}

static void CopyTypeTreeNode(TypeTree& typeTree, const TypeTreeIterator& dst, const TypeTreeIterator& src)
{
    TypeTreeNode& dstNode = *dst.GetWritableNode(typeTree);
    typeTree.AssignTypeString(dst, src.Type().c_str());
    typeTree.AssignNameString(dst, src.Name().c_str());
    dstNode.m_ByteSize = src->m_ByteSize;
    dstNode.m_Index = src->m_Index;
    dstNode.m_TypeFlags = src->m_TypeFlags;
    dstNode.m_Version = src->m_Version;
    dstNode.m_MetaFlag = src->m_MetaFlag;
}

TypeTree::TypeTree(MemLabelRef label)
        : m_ReferencedTypes(nullptr)
        , m_PoolOwned(false)
{
    m_Data = HUAHUO_NEW(TypeTreeShareableData, label)(label);  // no need to Retain(), NEW does it.
}

TypeTree::TypeTree(TypeTreeShareableData * sharedType, MemLabelRef label)
        : m_ReferencedTypes(nullptr)
        , m_PoolOwned(false)
{
    DebugAssert(sharedType != NULL);
    sharedType->Retain();
    m_Data = sharedType;
}

TypeTree::TypeTree(const TypeTree &other, MemLabelRef label)
        : m_ReferencedTypes(nullptr)
        , m_PoolOwned(false)
{
    // We don't use label here because we replicate combination of:
    //     TypeTree(MemLabelRef) + operator=(...)
    // Label is only used to create content of m_Data that is overridden in assign operator with the one in argument 'other'.
    m_Data = other.m_Data;
    m_Data->Retain();

    SetReferencedTypes(other.m_ReferencedTypes, false);
}

TypeTree::TypeTree(const TypeTree &templateObj)
{
    m_PoolOwned = templateObj.m_PoolOwned;
    m_ReferencedTypes = templateObj.m_ReferencedTypes;
    if (!m_PoolOwned && m_ReferencedTypes != nullptr)
        m_ReferencedTypes->Retain();

    DebugAssert(templateObj.m_Data != nullptr);
    m_Data = templateObj.m_Data;
    m_Data->Retain();
}

TypeTree & TypeTree::operator=(const TypeTree & rhs)
{
    // Flush old data
    ReleaseSharedData();

    m_Data = rhs.m_Data;
    m_Data->Retain();

    SetReferencedTypes(rhs.m_ReferencedTypes, false);
    return *this;
}

TypeTree::~TypeTree()
{
    ReleaseSharedData();
    if (!m_PoolOwned && m_ReferencedTypes != nullptr)
        m_ReferencedTypes->Release();
}

void TypeTree::ReleaseSharedData()
{
    if (m_Data != nullptr)
    {
        m_Data->Release();
        m_Data = nullptr;
    }
}

static void RecurseCopy(TypeTree& typeTree, const TypeTreeIterator& root, const TypeTreeIterator& otherNode)
{
    CopyTypeTreeNode(typeTree, root, otherNode);

    for (TypeTreeIterator i = otherNode.Children(); !i.IsNull(); i = i.Next())
    {
        TypeTreeIterator child = root.AddChildNode(typeTree);
        RecurseCopy(typeTree, child, i);
    }
}

void TypeTree::AppendChildren(const TypeTreeIterator& srcRoot)
{
    AppendChildrenRange(srcRoot, 0, TypeTreeQueries::GetTypeChildrenCount(srcRoot), false);
}

static void RecalculateTypeTreeByteSize(TypeTree& typeTree, const TypeTreeIterator& node, int* typePosition, int options)
{
    DebugAssert(node->m_ByteSize != 0 || node.Type() != CommonString(Generic_Mono));

    TypeTreeNode& actual = *node.GetWritableNode(typeTree);

    if ((node->m_MetaFlag & kDebugPropertyMask) == 0
        || (options & kIgnoreDebugPropertiesForIndex) == 0)
    {
        actual.m_Index = *typePosition;
        (*typePosition)++;
    }
    else
    {
        actual.m_Index = -1;
    }

    if (node.Children().IsNull())
    {
        Assert(!node->IsArray());
        return;
    }

    bool cantDetermineSize = false;
    actual.m_ByteSize = 0;

    for (TypeTreeIterator i = node.Children(); !i.IsNull(); i = i.Next())
    {
        RecalculateTypeTreeByteSize(typeTree, i, typePosition, options);

        if (i->m_ByteSize == -1)
            cantDetermineSize = true;

        if (!cantDetermineSize)
            actual.m_ByteSize += i->m_ByteSize;
    }

    if (node->IsArray() || cantDetermineSize)
        actual.m_ByteSize = -1;

    DebugAssert(node->m_ByteSize != 0 || node.Type() != CommonString(Generic_Mono));
}

static void RecalculateTypeTreeByteSize(TypeTree& typeTree, const TypeTreeIterator& root, int options)
{
    root.GetWritableNode(typeTree)->m_ByteSize = -1;
    int typePosition = 0;
    RecalculateTypeTreeByteSize(typeTree, root, &typePosition, options);
}

void TypeTree::AppendChildrenRange(const TypeTreeIterator& srcRoot, size_t begin, size_t end, bool copyRoot)
{
    Assert(begin <= end);

    TypeTreeIterator root = Root();

    if (copyRoot)
        CopyTypeTreeNode(*this, root, srcRoot);

    size_t c = 0;
    for (TypeTreeIterator i = srcRoot.Children(); !i.IsNull(); i = i.Next(), ++c)
    {
        if (c >= begin && c < end)
        {
            TypeTreeIterator child = root.AddChildNode(*this);
            RecurseCopy(*this, child, i);
        }
    }

    RecalculateTypeTreeByteSize(*this, root, 0);
}

void TypeTree::AssignTypeString(const TypeTreeIterator& i, const char* content)
{
    Assert(TypeTreeIteratorVIP::IsIteratorOnTypeTree(i, this));
    TypeTreeShareableData & staticData = *TypeTreeIteratorVIP::GetTypeTreeData(i);
    staticData.CreateString(i.GetWritableNode(*this)->m_TypeStrOffset, content);
}

void TypeTree::AssignNameString(const TypeTreeIterator& i, const char* content)
{
    Assert(TypeTreeIteratorVIP::IsIteratorOnTypeTree(i, this));
    TypeTreeShareableData & staticData = *TypeTreeIteratorVIP::GetTypeTreeData(i);
    staticData.CreateString(i.GetWritableNode(*this)->m_NameStrOffset, content);
}

void TypeTree::AssignByteOffset(const TypeTreeIterator& i, UInt32 offset)
{
    Assert(TypeTreeIteratorVIP::IsIteratorOnTypeTree(i, this));
    TypeTreeShareableData & staticData = *TypeTreeIteratorVIP::GetTypeTreeData(i);
    staticData.SetByteOffset(TypeTreeIteratorVIP::GetNodeIndex(i), offset);
}

void TypeTree::DumpToConsole() const
{
    std::string buffer;
    Root().DebugPrint(buffer);
    printf_console("%s\n", buffer.c_str());
}

bool TypeTree::BlobRead(const UInt8*& iterator, const UInt8* end, SerializedFileFormatVersion version, bool swapByteOrder)
{
    return m_Data->BlobRead(iterator, end, version, swapByteOrder);
}

TypeTree::Pool & TypeTree::Pool::operator=(const Pool & src)
{
    m_Data = src.m_Data;

    for (size_t i = 0; i < m_Data.size(); i++)
        m_Data[i].second.SetReferencedTypes(this, true);

    return *this;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Compose a string offset value
static inline UInt32 MakeStringOffset(UInt32 offset, bool isCommonString)
{
    Assert(offset < static_cast<UInt32>(TypeTree::kStringOffsetMask));
    return offset | (isCommonString ? TypeTree::kCommonStringBit : 0);
}

TypeTreeShareableData::TypeTreeShareableData(MemLabelRef label)
        : m_Nodes(1)
        , m_MemLabel(label)
{
    m_Nodes.back().Initialize(0);
}

TypeTreeShareableData::TypeTreeShareableData(const TypeTreeShareableData & src, MemLabelRef label)
        : m_MemLabel(label)
{
    m_Nodes = src.m_Nodes;
    m_StringBuffer = src.m_StringBuffer;
    m_ByteOffsets = src.m_ByteOffsets;
}

TypeTreeShareableData & TypeTreeShareableData::operator=(const TypeTreeShareableData & rhs)
{
    Assert(!IsReadOnly());
    m_StringBuffer = rhs.m_StringBuffer;
    m_ByteOffsets  = rhs.m_ByteOffsets;
    m_Nodes = rhs.m_Nodes;

    return *this;
}

void TypeTreeShareableData::CreateString(UInt32 & strOffset, const char* content)
{
    Assert(content != NULL);
    Assert(!IsReadOnly());
    // Typically typetree strings are assigned only once
    // To support multiple assignment, implement destroying non-common strings
    Assert(strOffset == 0 || (strOffset & TypeTree::kCommonStringBit) != 0);

    const char* str = GetCommonStringTable().FindCommonString(content, strlen(content));
    if (str != NULL)
    {
        strOffset = MakeStringOffset(str - HuaHuo::CommonString::BufferBegin, true);
        return;
    }

    // If the string is already present in the string buffer...
    const char* begin = m_StringBuffer.data();
    const char* end = m_StringBuffer.data() + m_StringBuffer.size();
    for (const char* p = begin; p < end;)
    {
        size_t len = std::strlen(p);
        if (std::strcmp(p, content) == 0)
        {
            strOffset = MakeStringOffset(p - begin, false);
            return;
        }

        p = p + len + 1;
    }

    // Allocate a new string at the end of the buffer
    int len = std::strlen(content) + 1;
    m_StringBuffer.insert(m_StringBuffer.end(), content, content + len);
    strOffset = MakeStringOffset(m_StringBuffer.size() - len, false);
}

void TypeTreeShareableData::SetByteOffset(size_t nodeIndex, UInt32 offset)
{
    Assert(!IsReadOnly());
    if (nodeIndex >= m_ByteOffsets.size())
        m_ByteOffsets.resize_initialized(nodeIndex + 1, TypeTree::kByteOffsetUnset, kDoubleOnResize);
    m_ByteOffsets[nodeIndex] = offset;
}

bool TypeTreeShareableData::BlobRead(const UInt8*& iterator, const UInt8* end, SerializedFileFormatVersion version, bool swapByteOrder)
{
    if (version <= SerializedFileFormatVersion::kRefactorTypeData)
        return BlobReadV17AndPrior(iterator, end, version, swapByteOrder);

    Assert(!IsReadOnly());
    UInt32 numberOfNodes;
    TEST_LEN(numberOfNodes);
#if UNITY_NO_UNALIGNED_MEMORY_ACCESS || PLATFORM_IOS || PLATFORM_TVOS
    std::memcpy(&numberOfNodes, iterator, sizeof(UInt32));
#else
    numberOfNodes = *reinterpret_cast<const UInt32*>(iterator);
#endif
    iterator += sizeof(UInt32);

    if (numberOfNodes == 0)
        return true;

    UInt32 numberOfChars;
    TEST_LEN(numberOfChars);
#if UNITY_NO_UNALIGNED_MEMORY_ACCESS || PLATFORM_IOS || PLATFORM_TVOS
    std::memcpy(&numberOfChars, iterator, sizeof(UInt32));
#else
    numberOfChars = *reinterpret_cast<const UInt32*>(iterator);
#endif
    iterator += sizeof(UInt32);

    if (swapByteOrder)
    {
        SwapEndianBytes(numberOfNodes);
        SwapEndianBytes(numberOfChars);
    }

    if (iterator + numberOfNodes * sizeof(TypeTreeNode) + numberOfChars > end)
        return false;

    m_Nodes.resize(numberOfNodes);
    m_StringBuffer.resize(numberOfChars);

    std::memcpy(m_Nodes.data(), iterator, sizeof(TypeTreeNode) * numberOfNodes);
    iterator += sizeof(TypeTreeNode) * numberOfNodes;
    std::memcpy(m_StringBuffer.data(), iterator, numberOfChars);
    iterator += numberOfChars;

    if (version < SerializedFileFormatVersion::kRefactorTypeData) // type tree node gets a type flag
    {
        if (swapByteOrder)
        {
            for (UInt32 i = 0; i < numberOfNodes; ++i)
            {
                TypeTreeNode& n = m_Nodes[i];
                SwapEndianBytes(n.m_Version);
                SwapEndianBytes(n.m_TypeStrOffset);
                SwapEndianBytes(n.m_NameStrOffset);
                SwapEndianBytes(n.m_ByteSize);
                SwapEndianBytes(n.m_Index);
                SwapEndianBytes(n.m_MetaFlag);
                n.m_TypeFlags = n.m_TypeFlags != 0 ? TypeTreeNode::kFlagIsArray : 0;
            }
        }
        else
        {
            for (UInt32 i = 0; i < numberOfNodes; ++i)
            {
                TypeTreeNode& n = m_Nodes[i];
                n.m_TypeFlags = n.m_TypeFlags != 0 ? TypeTreeNode::kFlagIsArray : 0;
            }
        }
    }
    else
    {
        if (swapByteOrder)
        {
            for (UInt32 i = 0; i < numberOfNodes; ++i)
            {
                TypeTreeNode& n = m_Nodes[i];
                SwapEndianBytes(n.m_Version);
                SwapEndianBytes(n.m_TypeStrOffset);
                SwapEndianBytes(n.m_NameStrOffset);
                SwapEndianBytes(n.m_ByteSize);
                SwapEndianBytes(n.m_Index);
                SwapEndianBytes(n.m_MetaFlag);
            }
        }
    }

    return true;
}

// Reads old versions of type tree serialziation format
bool TypeTreeShareableData::BlobReadV17AndPrior(const UInt8*& iterator, const UInt8* end, SerializedFileFormatVersion version, bool swapByteOrder)
{
    Assert(!IsReadOnly());
    UInt32 numberOfNodes;
    TEST_LEN(numberOfNodes);
#if UNITY_NO_UNALIGNED_MEMORY_ACCESS || PLATFORM_IOS || PLATFORM_TVOS
    std::memcpy(&numberOfNodes, iterator, sizeof(UInt32));
#else
    numberOfNodes = *reinterpret_cast<const UInt32*>(iterator);
#endif
    iterator += sizeof(UInt32);

    if (numberOfNodes == 0)
        return true;

    UInt32 numberOfChars;
    TEST_LEN(numberOfChars);
#if UNITY_NO_UNALIGNED_MEMORY_ACCESS || PLATFORM_IOS || PLATFORM_TVOS
    std::memcpy(&numberOfChars, iterator, sizeof(UInt32));
#else
    numberOfChars = *reinterpret_cast<const UInt32*>(iterator);
#endif
    iterator += sizeof(UInt32);

    if (swapByteOrder)
    {
        SwapEndianBytes(numberOfNodes);
        SwapEndianBytes(numberOfChars);
    }

    // Prior to 18, node type was smaller.
    TypeTreeNode dummyNode;
    size_t oldNodeSize = (size_t)&(dummyNode.m_RefTypeHash) - (size_t)&dummyNode;

    if (iterator + numberOfNodes * oldNodeSize + numberOfChars > end)
        return false;

    m_Nodes.resize(numberOfNodes);
    m_StringBuffer.resize(numberOfChars);

    UInt8* dest = (UInt8*)m_Nodes.data();
    for (size_t i = 0; i < numberOfNodes; i++, iterator += oldNodeSize, dest += sizeof(TypeTreeNode))
    {
        std::memcpy(dest, iterator, oldNodeSize);
        std::memset(dest + oldNodeSize, 0, sizeof(TypeTreeNode) - oldNodeSize);
    }
    std::memcpy(m_StringBuffer.data(), iterator, numberOfChars);
    iterator += numberOfChars;

    if (version < SerializedFileFormatVersion::kTypeTreeNodeWithTypeFlags)
    {
        if (swapByteOrder)
        {
            for (UInt32 i = 0; i < numberOfNodes; ++i)
            {
                TypeTreeNode& n = m_Nodes[i];
                SwapEndianBytes(n.m_Version);
                SwapEndianBytes(n.m_TypeStrOffset);
                SwapEndianBytes(n.m_NameStrOffset);
                SwapEndianBytes(n.m_ByteSize);
                SwapEndianBytes(n.m_Index);
                SwapEndianBytes(n.m_MetaFlag);
                n.m_TypeFlags = n.m_TypeFlags != 0 ? TypeTreeNode::kFlagIsArray : 0;
            }
        }
        else
        {
            for (UInt32 i = 0; i < numberOfNodes; ++i)
            {
                TypeTreeNode& n = m_Nodes[i];
                n.m_TypeFlags = n.m_TypeFlags != 0 ? TypeTreeNode::kFlagIsArray : 0;
            }
        }
    }
    else
    {
        if (swapByteOrder)
        {
            for (UInt32 i = 0; i < numberOfNodes; ++i)
            {
                TypeTreeNode& n = m_Nodes[i];
                SwapEndianBytes(n.m_Version);
                SwapEndianBytes(n.m_TypeStrOffset);
                SwapEndianBytes(n.m_NameStrOffset);
                SwapEndianBytes(n.m_ByteSize);
                SwapEndianBytes(n.m_Index);
                SwapEndianBytes(n.m_MetaFlag);
            }
        }
    }

    return true;
}

void TypeTreeShareableData::BlobWrite(std::vector<UInt8>& cache, bool swapByteOrder) const
{
    size_t writeSize = sizeof(UInt32) + sizeof(UInt32)
                       + sizeof(TypeTreeNode) * m_Nodes.size() + m_StringBuffer.size();
    cache.resize(cache.size() + writeSize);
    UInt8* cursor = cache.data() + cache.size() - writeSize;

    UInt32* numberOfNodes = reinterpret_cast<UInt32*>(cursor);
    cursor += sizeof(UInt32);
    UInt32* numberOfChars = reinterpret_cast<UInt32*>(cursor);
    cursor += sizeof(UInt32);
    TypeTreeNode* nodes = reinterpret_cast<TypeTreeNode*>(cursor);
    cursor += sizeof(TypeTreeNode) * m_Nodes.size();
    char* stringBuf = reinterpret_cast<char*>(cursor);

    *numberOfNodes = m_Nodes.size();
    *numberOfChars = m_StringBuffer.size();
    std::memcpy(nodes, m_Nodes.data(), sizeof(TypeTreeNode) * m_Nodes.size());
    std::memcpy(stringBuf, m_StringBuffer.data(), m_StringBuffer.size());

    if (swapByteOrder)
    {
        SwapEndianBytes(*numberOfNodes);
        SwapEndianBytes(*numberOfChars);
        for (size_t i = 0; i < m_Nodes.size(); ++i)
        {
            TypeTreeNode& n = nodes[i];
            SwapEndianBytes(n.m_Version);
            SwapEndianBytes(n.m_TypeStrOffset);
            SwapEndianBytes(n.m_NameStrOffset);
            SwapEndianBytes(n.m_ByteSize);
            SwapEndianBytes(n.m_Index);
            SwapEndianBytes(n.m_MetaFlag);
        }
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include "SerializedFile.h"
#include "Serialize/SerializationCaching/MemoryCacheWriter.h"
#include "Serialize/TransferFunctions/StreamedBinaryWrite.h"

namespace TypeTreeIO
{
    static const char* ReadString(UInt8 const*& iterator, UInt8 const* end)
    {
        UInt8 const* base = iterator;
        while (iterator < end && *iterator != 0)
            iterator++;

#if ENABLE_SECURITY
        if (iterator >= end)
            return NULL;
#endif

        iterator++;
        return reinterpret_cast<const char*>(base);
    }

    static void DeprecatedConvertUnity43BetaIntegerTypeNames(UInt32& type)
    {
        if ((type & TypeTree::kCommonStringBit) == 0)
            return;

        const char* str = CalculateString(type, NULL);
        if (str == CommonString(SInt32))
            type = MakeStringOffset(SerializeTraits<SInt32>::GetTypeString() - HuaHuo::CommonString::BufferBegin, true);
        else if (str == CommonString(UInt32))
            type = MakeStringOffset(SerializeTraits<UInt32>::GetTypeString() - HuaHuo::CommonString::BufferBegin, true);
    }

    template<bool kSwap>
    static bool ReadTypeTreeImpl(TypeTree& type, const TypeTreeIterator& t, UInt8 const*& iterator, UInt8 const* end, SerializedFileFormatVersion version)
    {
        TypeTreeNode* actual = t.GetWritableNode(type);

        // Read Type
        const char* typeString = ReadString(iterator, end);
        if (typeString == NULL)
            return false;
        type.AssignTypeString(t, typeString);

        // During the 4.3 beta cycle, we had a couple of versions that were using "SInt32" and "UInt32"
        // instead of "int" and "unsigned int".  Get rid of these type names here.
        //
        // NOTE: For now, we always do this.  Ideally, we only want this for old data.  However, ATM we
        //  don't version TypeTrees independently (they are tied to kCurrentSerializeVersion).  TypeTree
        //  serialization is going to change soonish so we wait with bumping the version until then.
        DeprecatedConvertUnity43BetaIntegerTypeNames(actual->m_TypeStrOffset);

        // Read Name
        const char* nameString = ReadString(iterator, end);
        if (nameString == NULL)
            return false;
        type.AssignNameString(t, nameString);

        // Read bytesize
        TEST_LEN(actual->m_ByteSize);
        ReadHeaderCache<kSwap>(actual->m_ByteSize, iterator);

        // Read variable count
        if (version == SerializedFileFormatVersion::kUnknown_2)
        {
            SInt32 variableCount;
            TEST_LEN(variableCount);
            ReadHeaderCache<kSwap>(variableCount, iterator);
        }

        // Read Typetree position
        if (version != SerializedFileFormatVersion::kUnknown_3)
        {
            TEST_LEN(actual->m_Index);
            ReadHeaderCache<kSwap>(actual->m_Index, iterator);
        }

        // Read TypeFlags
        SInt32 nodeTypeFlags;
        TEST_LEN(nodeTypeFlags);
        ReadHeaderCache<kSwap>(nodeTypeFlags, iterator);
        actual->m_TypeFlags = static_cast<UInt8>(nodeTypeFlags);

        // Read version
        SInt32 nodeVersion;
        TEST_LEN(nodeVersion);
        ReadHeaderCache<kSwap>(nodeVersion, iterator);
        actual->m_Version = static_cast<SInt16>(nodeVersion);

        // Read metaflag
        if (version != SerializedFileFormatVersion::kUnknown_3)
        {
            TEST_LEN(actual->m_MetaFlag);
            ReadHeaderCache<kSwap>(actual->m_MetaFlag, iterator);
        }

        // Read Children count
        SInt32 childrenCount;
        TEST_LEN(childrenCount);
        ReadHeaderCache<kSwap>(childrenCount, iterator);

        enum { kMaxDepth = 50, kMaxChildrenCount = 5000 };
        static int depth = 0;
        depth++;
        if (depth > kMaxDepth || childrenCount < 0 || childrenCount > kMaxChildrenCount)
        {
            depth--;
            ErrorString("Fatal error while reading file. Header is invalid!");
            return false;
        }
        // Read children
        for (int i = 0; i < childrenCount; i++)
        {
            TypeTreeIterator child = t.AddChildNode(type);
            if (!ReadTypeTreeImpl<kSwap>(type, child, iterator, end, version))
            {
                depth--;
                return false;
            }
        }
        depth--;
        return true;
    }

    bool ReadVersionedFromVector(TypeTree& typeTree, UInt8 const*& iterator, UInt8 const* end, bool swapEndian)
    {
        if (iterator == end)
        {
            typeTree = TypeTree();
            return false;
        }
        SerializedFileFormatVersion version;

        if (swapEndian)
        {
            ReadHeaderCache<true>(*(UInt32*)&version, iterator);
            ReadTypeTreeImpl<true>(typeTree, typeTree.Root(), iterator, end, version);
        }
        else
        {
            ReadHeaderCache<false>(*(UInt32*)&version, iterator);
            ReadTypeTreeImpl<false>(typeTree, typeTree.Root(), iterator, end, version);
        }
        return true;
    }

    bool ReadTypeTree(TypeTree& t, UInt8 const*& iterator, UInt8 const* end, SerializedFileFormatVersion version, bool swapEndian)
    {
        if (version >= SerializedFileFormatVersion::kUnknown_12 || version == SerializedFileFormatVersion::kUnknown_10)
        {
            return t.BlobRead(iterator, end, version, swapEndian);
        }
        else
        {
            if (swapEndian)
                return ReadTypeTreeImpl<true>(t, t.Root(), iterator, end, version);
            else
                return ReadTypeTreeImpl<false>(t, t.Root(), iterator, end, version);
        }
    }

    void WriteTypeTree(const TypeTree& t, std::vector<UInt8>& cache, bool swapEndian)
    {
        t.BlobWrite(cache, swapEndian);
    }
} // end of namespace TypeTreeIO