//
// Created by VincentZhang on 2023-04-27.
//

#ifndef HUAHUOENGINEV2_TYPETREE_H
#define HUAHUOENGINEV2_TYPETREE_H
#include <cstdlib>
#include <cstring>
#include "Containers/CommonString.h"
#include "Configuration/IntegerDefinitions.h"
#include "SerializationMetaFlags.h"
#include <vector>
#include "Memory/MemoryMacros.h"
#include "SerializedFileFormatVersion.h"
#include "Threads/AtomicRefCounter.h"


class TypeTree;
struct TypeTreeNode;
class TypeTreeIterator;

//----------------------------------------------------------------------------------------------------------------------
// What is this: A carrier for strings produced by queries on type trees. Nothing fancy.
//----------------------------------------------------------------------------------------------------------------------
class TypeTreeString
{
public:
    const char* c_str() const { return m_Buffer; }
    size_t strlen() const { return std::strlen(m_Buffer); }

    bool operator==(const char* str) const
    {
        if (str == NULL || m_Buffer == NULL)
            return str == m_Buffer;

        if (IsCommonString(m_Buffer) && IsCommonString(str))
            return str == m_Buffer;
        else
            return std::strcmp(str, m_Buffer) == 0;
    }

    bool operator!=(const char* str) const { return !(*this == str); }
    friend bool operator==(const char* lhs, const TypeTreeString& rhs) { return rhs == lhs; }
    friend bool operator!=(const char* lhs, const TypeTreeString& rhs) { return rhs != lhs; }
    bool operator==(const TypeTreeString& str) const { return *this == str.m_Buffer; }
    bool operator!=(const TypeTreeString& str) const { return !(*this == str); }

private:
    friend class TypeTreeIterator;
    const char* m_Buffer;
};

//----------------------------------------------------------------------------------------------------------------------
// What is this: Defines a node that make up TypeTree's data.
//
// Notes:
//  - This struct is serialized in blob so change to it actually always breaks the serialization of TypeTrees.
// See BlobRead()/BlobWrite().
//----------------------------------------------------------------------------------------------------------------------
struct TypeTreeNode
{
    SInt16              m_Version;          // The version of the serialization format as represented by this type tree.  Usually determined by Transfer() functions.
    UInt8               m_Level;            // Level in the hierarchy (0 is the root)
    UInt8               m_TypeFlags;        // Possible values see ETypeFlags

    UInt32              m_TypeStrOffset;    // The type of the variable (eg. "Vector3f", "int")
    UInt32              m_NameStrOffset;    // The name of the property (eg. "m_LocalPosition")
    SInt32              m_ByteSize;         // = -1 if its not determinable (arrays)
    SInt32              m_Index;            // The index of the property (Prefabs use this index in the override bitset)

    // Serialization meta data (eg. to hide variables in the property editor)
    // Children or their meta flags with their parents!
    UInt32              m_MetaFlag;

    enum ETypeFlags
    {
        kFlagNone = 0,
        kFlagIsArray = (1 << 0),
        kFlagIsManagedReference = (1 << 1),
        kFlagIsManagedReferenceRegistry = (1 << 2),
        kFlagIsArrayOfRefs = (1 << 3)
    };

    // When node is private reference, this holds the 64bit "hash" of the TypeTreeShareableData of the refed type.
    // stores Hash128::PackToUInt64(). Why? because the Hash128 type is to expensive to initialize cpu wise(memset)
    // 0 <=> does not reference a type.
    // note: if this is deamed to much data (tends to always be zero), we could move the hash to TypeTreeShareableData as a vector and just keep a byte index here.
    UInt64 m_RefTypeHash;

    void Initialize(UInt8 level, UInt64 refTypeHash = 0)
    {
        m_Level = level;
        m_NameStrOffset = 0;
        m_TypeStrOffset = 0;
        m_Index = -1;
        m_TypeFlags = 0;
        m_Version = 1;
        m_MetaFlag = kNoTransferFlags;
        m_ByteSize = -1;
        m_RefTypeHash = 0;
    }

    void inline AddTypeFlags(ETypeFlags flags)      { m_TypeFlags |= (UInt8)flags;                          }
    bool inline IsArray() const                     { return m_TypeFlags & kFlagIsArray;                    }
    bool inline IsManagedReference() const          { return m_TypeFlags & kFlagIsManagedReference;         }
    bool inline IsManagedReferenceRegistry() const  { return m_TypeFlags & kFlagIsManagedReferenceRegistry; }
    bool inline IsArrayOfRefs() const               { return m_TypeFlags & kFlagIsArrayOfRefs;              }
};

//----------------------------------------------------------------------------------------------------------------------
// What is this:
//  - A TypeTreeShareableData describes the structure of serialized data and is tied to a type's static description.
//  - It is a tree structure when the nodes store the name, type and other information generated by the serialization code,
//    of every field that the type is made up of.
//
// Motivation(s):
//  - Need chache friendly structure to hold the serailized data, that supports sharing between multiple TypeTree objects.
//
// Notes:
//  - Can describe any type, not just types that specialize Unity's Object base class.
//  - Instances of this are sharable once Retain() is called, at which point it becomes ref counted and read-only.
//  - Release() auto-destroys the instance when ref count reaches zero.
//  - A TypeTreeShareableData is serialized in a blob: means all memory consumed by a TypeTreeShareableData hierarchy is packed all together
//    in TypeTreeShareableData object (m_Nodes and m_StringBuffer). Therefore, we don't use any pointers for tree hierarchy
//    and strings. Instead, we use m_Level on nodes and search for father/children/sibling implicitly. And we use
//    offset for strings, either into the global shared common string buffer (Unity::CommonString::BufferBegin &
//    Unity::CommonString::BufferEnd) or the TypeTreeShareableData-local string buffer (TypeTreeShareableData::m_StringBuffer).
//
// Warnings:
//  - Currently TypeTreeIterator relies on the feature that new tree node can only be appended to TypeTreeShareableData
//    Since TypeTreeIterator uses index of the node, it is never invalidated. If you change this behaviour, please
//    review all the code using a standalone iterator (e.g. GenerateTypeTreeTransfer::m_ActiveFather).
//----------------------------------------------------------------------------------------------------------------------
class TypeTreeShareableData
{
public:
    TypeTreeShareableData(MemLabelRef label = kMemTypeTree);
    TypeTreeShareableData(const TypeTreeShareableData & src, MemLabelRef label = kMemTypeTree);

    TypeTreeShareableData & operator=(const TypeTreeShareableData & rhs);

    TypeTreeNode* GetWritable(size_t nodeIndex) const
    {
        Assert(!IsReadOnly());
        return const_cast<TypeTreeNode*>(&m_Nodes[nodeIndex]); // Dont care about the const here: it's m_ReadOnly that actually matters.
    }

    const std::vector<TypeTreeNode> & Nodes() const  { return m_Nodes; }
    size_t NodeCount() const { return m_Nodes.size(); }

    const std::vector<UInt32> & ByteOffsets() const  { return m_ByteOffsets; }
    size_t ByteOffsetsCount() const   { return m_ByteOffsets.size(); }

    const std::vector<char> & StringsBuffer() const { return m_StringBuffer; }
    size_t StringsBufferCount() const   { return m_StringBuffer.size(); }

    void CreateString(UInt32& strOffset, const char* content);
    void SetByteOffset(size_t nodeIndex, UInt32 offset);

    void BlobWrite(std::vector<UInt8>& cache, bool swapByteOrder) const;
    bool BlobRead(const UInt8*& iterator, const UInt8* end, SerializedFileFormatVersion version, bool swapByteOrder);

    void Retain()   {   m_RefCount.Retain(); }
    void Release()
    {
        if (m_RefCount.Release() == true)
        {
            TypeTreeShareableData * _this = this;
            HUAHUO_DELETE(_this, m_MemLabel);
        }
    }

    int RefCount() const { return m_RefCount.Count();}

    bool IsReadOnly() const { return m_RefCount.Count() > 1;}

    // Implementation is in header as it speeds up dramaticaly the execution! (imperical observation)
    size_t AddChildNode(size_t fatherIndex)
    {
        Assert(fatherIndex < m_Nodes.size());
        Assert(m_Nodes[fatherIndex].m_Level < std::numeric_limits<UInt8>::max());
        Assert(!IsReadOnly());

        TypeTreeNode& newNode = m_Nodes.emplace_back();
        newNode.Initialize(m_Nodes[fatherIndex].m_Level + 1);
        return m_Nodes.size() - 1;
    }

    void SetGenerationFlags(TransferInstructionFlags flags) { m_FlagsAtGeneration = flags; }
    TransferInstructionFlags GetGenerationFlags() const { return m_FlagsAtGeneration; }

private:
    bool BlobReadV17AndPrior(const UInt8*& iterator, const UInt8* end, SerializedFileFormatVersion version, bool swapByteOrder);

    std::vector<TypeTreeNode> m_Nodes;
    std::vector<char> m_StringBuffer;

    // The byteoffset into the property in memory. When a variable on the stack is serialized m_ByteOffset is -1.
    // Not serialized.
    std::vector<UInt32> m_ByteOffsets;

    // Not serialized/persisted
    TransferInstructionFlags m_FlagsAtGeneration;
    AtomicRefCounter m_RefCount;
    MemLabelRef m_MemLabel;
};

//----------------------------------------------------------------------------------------------------------------------
// What is this: Implements the iterator pattern over TypeTree/TypeTreeShareableData.
// Motivation  : Need a way to traverse a TypeTree's nodes and be able to store/pass arround something that represents
//               a position in the tree and how to move in the tree.
//
// Notes       :
//  - With TypeTreeIterator alone you can only read data but you can not write data. However, just like STL iterator,
//    a const reference of TypeTreeIterator doesn't mean the underlying TypeTreeNode is const also. To get access to the
//    writable TypeTreeNode, you will need a non-const TypeTree object and call GetWritableNode().
//
//  - Warning: currently TypeTreeIterator relies on the feature that new tree node can only be appended to TypeTreeShareableData
//    Since TypeTreeIterator uses index of the node, it is never invalidated. If you change this behaviour, please
//    review all the code using a standalone iterator (e.g. GenerateTypeTreeTransfer::m_ActiveFather).
//----------------------------------------------------------------------------------------------------------------------
class TypeTreeIterator
{
public:
    TypeTreeIterator()
            : m_LinkedTypeTree(NULL)
            , m_TypeTreeData(NULL)
            , m_NodeIndex(0)
    {}

    TypeTreeIterator(const TypeTree* typeTree, const TypeTreeShareableData* data, size_t nodeIndex)
            : m_LinkedTypeTree(typeTree)
            , m_TypeTreeData(data)
            , m_NodeIndex(nodeIndex)
    {}

    TypeTreeIterator(const TypeTreeIterator & reference, size_t nodeIndex)
            : m_LinkedTypeTree(reference.m_LinkedTypeTree)
            , m_TypeTreeData(reference.m_TypeTreeData)
            , m_NodeIndex(nodeIndex)
    {}

    bool IsNull() const { return m_TypeTreeData == NULL; }
    enum class DebugPrintInstructions
    {
        kNone        = 0,
        kMetaFags    = 1 << 0,
        kTypeFlags   = 1 << 1,
        kSize        = 1 << 2,
        kIndex       = 1 << 3,
        kType        = 1 << 4,
        kName        = 1 << 5,
        kOffsets     = 1 << 6,
        kDefault     = kMetaFags | kSize | kIndex | kType | kName | kTypeFlags
    };
    void DebugPrint(std::string& buffer, int level = 0, DebugPrintInstructions instructions = DebugPrintInstructions::kDefault) const;
    bool IsBasicDataType() const { return Children().IsNull() && GetNode()->m_ByteSize > 0; }
    bool HasConstantSize() const { return GetNode()->m_ByteSize != -1 && (GetNode()->m_MetaFlag & (kAlignBytesFlag | kAnyChildUsesAlignBytesFlag)) == 0; }

    const TypeTreeNode* operator->() const { return GetNode(); }

    TypeTreeIterator Father() const;
    TypeTreeIterator Children() const;
    TypeTreeIterator Next() const;
    TypeTreeIterator Last() const;

    TypeTreeIterator FindChild(std::string name) const;

    // construct temporary TypeTreeString for type & name.
    TypeTreeString Type() const;
    TypeTreeString Name() const;

    // Read the ByteOffset value. TypeTree::kByteOffsetUnset if the value is not available.
    UInt32 ByteOffset() const;
    SInt32 ByteSize() const { return GetNode()->m_ByteSize; }
    TransferMetaFlags MetaFlags() const { return (TransferMetaFlags)(GetNode()->m_MetaFlag); }

    bool operator==(const TypeTreeIterator& rhs) const { return m_LinkedTypeTree ==  rhs.m_LinkedTypeTree && m_TypeTreeData == rhs.m_TypeTreeData && m_NodeIndex == rhs.m_NodeIndex; }
    bool operator!=(const TypeTreeIterator& rhs) const { return !(*this == rhs); }

    // Does not modify the Iterator itself, (stays const), but does alter it's associated static type tree
    // Not very nice OO wise, but results on a significant speed optimization: getting 40% speed up doing it this way over gooing through type tree instead.
    // note: the argument, mutableType, is there only to prove that caller has R/W access to the associated type tree.
    TypeTreeIterator AddChildNode(TypeTree &mutableType) const
    {
        DebugAssertMsg(m_LinkedTypeTree == &mutableType, "AddChildNode() passed a type tree that is not tha same as what this iterator instance is linked to");
        TypeTreeShareableData & stt = *const_cast<TypeTreeShareableData*>(m_TypeTreeData);
        size_t index = stt.AddChildNode(m_NodeIndex);
        return TypeTreeIterator(*this, index);
    }

    // Not very nice OO wise, but results on a significant speed optimization: getting 5x speed up doing it this way over gooing through type tree instead.
    // note: the argument, mutableType, is there only to prove that caller has R/W access to the associated type tree.
    TypeTreeNode* GetWritableNode(TypeTree &mutableType) const
    {
        DebugAssertMsg(m_LinkedTypeTree == &mutableType, "AddChildNode() passed a type tree that is not tha same as what this iterator instance is linked to");
        return m_TypeTreeData->GetWritable(m_NodeIndex);
    }

    const TypeTreeShareableData * GetTypeTreeData() const { return m_TypeTreeData; }
    const TypeTree & GetTypeTree() const { return *m_LinkedTypeTree; }

    size_t GetNodeIndex() const { return m_NodeIndex; }

private:
    friend class TypeTreeIteratorVIP;

    const TypeTreeNode* GetNode() const;

    const TypeTree* m_LinkedTypeTree;
    const TypeTreeShareableData* m_TypeTreeData;

    size_t m_NodeIndex;
};

ENUM_FLAGS(TypeTreeIterator::DebugPrintInstructions); // declares it wise operators for this enum

//----------------------------------------------------------------------------------------------------------------------
// What is this :
//  A TypeTree contains information on serialized data of a UnityEngine.Object *instance*.
//
// Motivation(s):
//  - Support reading serialized data from old data models.
//  - UI needs ability to list fields in objects
//  - UI wants to bind genericaly to fields inside objects.
//  - Prefabs need a way to reference fields by name to override values.
//
// Notes:
//  - A TypeTree can be generated using the TypeTreeCache.
//  - The actual serialization data is in TypeTreeShareableData. This class is carrier that takes care of memory management for the static data.
//  - TypeTree instances are light throw away things and are not intended to be cached. They get reconstructed and clones all the time.
//  - The offset fields held by TypeTree's are only relevent for byte arrays that have been filled through StreamBinaryWrite.
//
//  - Warning: currently TypeTreeIterator relies on the feature that new tree node can only be appended to TypeTreeShareableData
//    Since TypeTreeIterator uses index of the node, it is never invalidated. If you change this behaviour, please
//    review all the code using a standalone iterator (e.g. GenerateTypeTreeTransfer::m_ActiveFather).
//----------------------------------------------------------------------------------------------------------------------
class TypeTree
{
public:
    void SetPoolOwned(bool state) { m_PoolOwned = state;  }
    //----------------------------------------------------------------------------------------------------------------------
    // What is this: Value that can be used to identify a Type Tree Instance, based on the type it describes.
    //               (is unaffected by the structure of the type described by the TypeTree)
    //
    // Notes:
    // - Base signature: are generated from the type's FQN (managed type) or "Persitent ID" (native types)
    // - Composite signature: is formed by taking into account the transfer flags that where passed when generating the type tree.
    // - Nothing distinguishes a Base Signature from a Composite Signature: it is up to the consumers to know what is what.
    //
    // Example: All type tree instances that describe the type "Material", will all have the same base Signature but will have
    //          different Composite Signatures when used for different transfer flags.
    //----------------------------------------------------------------------------------------------------------------------
    typedef UInt64 Signature;

    //----------------------------------------------------------------------------------------------------------------------
    // What is this: Collection of type trees, accessible by Signatures (generated by TypeTreeQueries)
    // Motivation  : managed types with fields serialized as references have multiple type trees associated to the root instance
    //               that is being serialized. In certain situations, for ex: SafeBinaryRead, generating a StateBackup, iterating
    //               over an object with an active BackupState, we cannot ask the TypeTreeCache for the referenced types as these
    //               might not properly describe the data stream being accessed (type is gone, type has changed), so we need a
    //               way to pass along the list of type trees that match the data stream.
    //
    // Notes:
    // - TypeTree's fetched from a Pool are tied to the pool and must not survive them: when the pool is deleted, TypeTrees
    //      fetched from the pool become invalid.
    // - TypeTree's fetched from pools must be treated as ephemeral/temporary (this does apply to the sharable type tree data).
    // - TypeTrees taken from a pool are clones that remain tied to the pool: clones can be diconnected from the pool, making
    //   them independant of the originating pool's life time.
    // - Pools do NOT keep track of all the TypeTree clones that they produced.
    // - Assignemnt operator will duplicate all it the type tree instances, and link them to the target pool,
    //      but does NOT duplicate the sharable type tree data.
    // - Pool is ref-counted, never call UNITY_DELETE directly, always use: Release()
    //----------------------------------------------------------------------------------------------------------------------
    struct Pool
    {
    private:
        typedef std::pair<Signature, TypeTree> Entry;
        std::vector<Entry> m_Data;
        AtomicRefCounter m_RefCount;

        Pool(const Pool & rhs) {}

        // Private so that everyone one uses Pool* exclusively.
        Pool(MemLabelRef label = kMemTypeTree) /*: m_Data(label)*/ {}

    public:

        // Returns with a refcount set to 1
        inline static Pool * CreatePool()        { return HUAHUO_NEW(Pool, kMemTypeTree); /*return UNITY_NEW(Pool, kMemTypeTree)(kMemTypeTree);*/ }

        Pool & operator=(const Pool & src);

        inline bool Find(Signature signature, TypeTree &outTypeTree) const
        {
            for (size_t i = 0; i < m_Data.size(); i++)
            {
                if (m_Data[i].first == signature)
                {
                    outTypeTree = m_Data[i].second; // Copy op. strips pool ownership
                    return true;
                }
            }
            return false;
        }

        inline void Add(Signature signature, const TypeTree & typeTree);

        inline size_t Size() const { return m_Data.size(); }

        void Retain() { m_RefCount.Retain();}
        void Release()
        {
            if (m_RefCount.Release() == true)
            {
                Pool * _this = this;
                HUAHUO_DELETE(_this, kMemTypeTree); // _this->m_Data.get_memory_label());
            }
        }

        int RefCount() const { return m_RefCount.Count(); }
    };

public:
    enum
    {
        // String bits
        // the MSB is set if the string offset is an offset into the Unity::CommonString::BufferBegin; otherwise it offsets into the
        // TypeTree-local string buffer.
        kCommonStringBit = 0x80000000U,
        kStringOffsetMask = ~kCommonStringBit,

        // ByteOffset bits
        // the MSB is set if the ByteOffset is the offset into the scripting object; otherwise it offsets into the object itself
        kByteOffsetUnset = 0xffffffffU,
        kScriptingInstanceOffset = 0x80000000U,
        kByteOffsetMask = ~kScriptingInstanceOffset
    };

    explicit TypeTree(MemLabelRef label = kMemTypeTree);
    TypeTree(const TypeTree &templateObj, MemLabelRef label);
    explicit TypeTree(TypeTreeShareableData * sharedType, MemLabelRef label = kMemTypeTree);

    // Pool ownership is carried over.
    TypeTree(const TypeTree &templateObj);
    ~TypeTree();

    // Copy operation does not carry over Pool ownership!
    TypeTree & operator=(const TypeTree & rhs);

    TypeTreeIterator Root() const { return TypeTreeIterator(this, m_Data, 0); }

    // Append a range of children under srcRoot node to this TypeTree's root node.
    void AppendChildren(const TypeTreeIterator& srcRoot);
    void AppendChildrenRange(const TypeTreeIterator& srcRoot, size_t begin, size_t end, bool copyRoot);

    void AssignTypeString(const TypeTreeIterator& i, const char* content);
    void AssignNameString(const TypeTreeIterator& i, const char* content);
    void AssignByteOffset(const TypeTreeIterator& i, UInt32 offset);

    bool BlobRead(const UInt8*& iterator, const UInt8* end, SerializedFileFormatVersion version, bool swapByteOrder);
    void BlobWrite(std::vector<UInt8>& cache, bool swapByteOrder) const { m_Data->BlobWrite(cache, swapByteOrder); }

    /// Output a debug representation of the type tree to the console.
    void DumpToConsole() const;

    TypeTreeShareableData * GetData() {return m_Data;}

    // Is this type tree describing any data type structure or nothing (uninitialized)?
    bool IsEmpty() const { return Root().Children().IsNull(); }

    void SetReferencedTypes(Pool * types, bool poolOwned)
    {
        if (!poolOwned && types != nullptr)
            types->Retain();

        if (!m_PoolOwned && m_ReferencedTypes != nullptr)
            m_ReferencedTypes->Release();

        m_ReferencedTypes = types;
        m_PoolOwned = poolOwned;
    }

    const Pool * GetReferencedTypes() const { return m_ReferencedTypes; }

private:
    void ReleaseSharedData();

    TypeTreeShareableData* m_Data;
    Pool * m_ReferencedTypes;
    bool m_PoolOwned;
};

inline void TypeTree::Pool::Add(Signature signature, const TypeTree & typeTree)
{
    Entry newEntry(signature, typeTree);
#if UNITY_EDITOR
    newEntry.second.SetReferencedTypes(this, true);
#endif
    m_Data.push_back(newEntry);
}

//----------------------------------------------------------------------------------------------------------------------
// What is this: Collection of methods that perform I/O operations on type trees
// Motivation  : Just getting them out of the global namespace.
//----------------------------------------------------------------------------------------------------------------------
namespace TypeTreeIO
{
    /// Reads/Writes a typetree to a vector<UInt8> with the first four bytes being the version of the typetree
    bool ReadVersionedFromVector(TypeTree& typeTree, UInt8 const*& iterator, UInt8 const* end, bool swapEndianess);

    /// Reads/Writes a typeTree to a cache, used direcly by SerializedFile
    bool ReadTypeTree(TypeTree& t, UInt8 const*& iterator, UInt8 const* end, SerializedFileFormatVersion version, bool swapEndian);
    void WriteTypeTree(const TypeTree& t, std::vector<UInt8>& cache, bool swapEndianess);
}



#endif //HUAHUOENGINEV2_TYPETREE_H
