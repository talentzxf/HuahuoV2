//
// Created by VincentZhang on 5/1/2022.
//

#ifndef HUAHUOENGINE_SERIALIZEDFILE_H
#define HUAHUOENGINE_SERIALIZEDFILE_H
#include "Utilities/EnumFlags.h"
#include "Utilities/GUID.h"
#include "Utilities/Hash128.h"
#include <vector>
#include "TypeSystem/Type.h"
#include "SerializedFileFormatVersion.h"
#include "Utilities/dynamic_block_array.h"
#include "TypeSystem/Object.h"
#include "Utilities/vector_map.h"
#include "SerializedFileLoadError.h"
#include "SwapEndianBytes.h"

struct FileIdentifier
{
    // kNonAssetType : guid is valid if it's a const guid otherwise null, pathName can point to anywhere except in registered asset folders
    // kSerializedAssetType : guid is valid, pathName is empty, the resolved path is in a registered asset folder
    // kMetaAssetType : guid is valid, pathName is empty, the resolved path is an artifact path
    enum { kNonAssetType = 0, kDeprecatedCachedAssetType = 1, kSerializedAssetType = 2, kMetaAssetType = 3, kAssetTypeCount = 4 };
    enum InsertMode { kDontCreate = 0, kCreate = 1, kAllowRemap = 2 };
    ENUM_FLAGS_AS_MEMBER(FileIdentifier::InsertMode)

    std::string pathName;
    SInt32 type;
    HuaHuoGUID guid;

#if !UNITY_EDITOR
    void CheckValidity() {}
#else

    FileIdentifier(core::string_ref p, const UnityGUID& g, int t)
        : pathName(p, kMemTempAlloc), guid(g), type(t)
    {
        CheckValidity();
    }

    void CheckValidity();
    void Fix_3_5_BackwardsCompatibility();

    bool operator==(const FileIdentifier &other) const
    {
        if (type == other.type)
            return true;
        else if (guid == other.guid)
            return true;
        return pathName == other.pathName;
    }

#endif

    FileIdentifier() { type = 0; }
    FileIdentifier(MemLabelId label) { type = 0; }
    FileIdentifier(const FileIdentifier& other/*, MemLabelId label*/) :
            pathName(other.pathName/*, label*/),
            type(other.type),
            guid(other.guid)
    {}

#if SUPPORT_TEXT_SERIALIZATION
    DECLARE_SERIALIZE(FileIdentifier);
#endif
};

template<bool kSwap, class T>
// Need this in order for Asset bundles to load correctly when the code is optimized
#if PLATFORM_ANDROID
__attribute__((noinline))
#endif
void ReadHeaderCache(T& t, UInt8 const*& c)
{
#if UNITY_NO_UNALIGNED_MEMORY_ACCESS
    memcpy(&t, c, sizeof(T));
#else
    t = *(T const*)c;
#endif
    if (kSwap)
        SwapEndianBytes(t);
    c += sizeof(T);
}

class SerializedFile {
public:
    enum
    {
        kLittleEndian = 0,
        kBigEndian = 1,

#if PLATFORM_ARCH_BIG_ENDIAN
        kActiveEndianess = kBigEndian,
        kOppositeEndianess = kLittleEndian
#else
        kActiveEndianess = kLittleEndian,
        kOppositeEndianess = kBigEndian
#endif
    };

    class SerializedType
    {
        friend class SerializedFile;
    public:
        typedef std::vector<SerializedType> TypeVector;

    private:
        const HuaHuo::Type* m_Type;

        Hash128 m_ScriptID;         // Hash generated from assembly name, namespace, and class name, only available for script.
        Hash128 m_OldTypeHash;      // Old type tree hash.

        bool m_IsStrippedType;
        bool m_PerClassTypeTree;
        SInt16 m_ScriptTypeIndex;



        const TypeTree* m_OldType;  // Type load from file.
        int m_Equals;              // Are old type and new type equal.

        typedef std::vector<std::set<SInt32>> TypeDependencies;

        // When a SerializedType instance is describing a UnityEngine.Object type:
        //  This is the collection of all the types that where found to be referenced by fields that are marked as [SerializeRefence].
        // (the scope of the list if the whole file and not limited to a single entry in the file)
        TypeDependencies m_TypeDependencies;

#if SUPPORT_SERIALIZED_TYPETREES




        // Only used for Referenced types
        core::string m_KlassName;
        core::string m_NameSpace;
        core::string m_AsmName;
#if !UNITY_EXTERNAL_TOOL
        TypeTree::Signature m_TypeTreeCacheId; // does not get serialized and most not in case hashing function changes
#endif
#endif
    public:
        SerializedType(const HuaHuo::Type* type, bool isStrippedType, SInt16 scriptTypeIdx = -1);
        ~SerializedType();

        // Note that deserialized persistent Type ID might not have a runtime class present anymore in
        // case of deprecated ids. In that case GetType() will return a stub type that
        // holds the unknown persistent Type ID, but with a NULL factory.
        const HuaHuo::Type* GetType() const { return m_Type; }
        PersistentTypeID GetPersistentTypeID() const { return (m_Type == NULL) ? HuaHuo::Type::UndefinedPersistentTypeID : m_Type->GetPersistentTypeID(); }
        bool IsStripped() const { return m_IsStrippedType; }
        bool GetPerClassTypeTree() const { return m_PerClassTypeTree; }
        SInt16 GetScriptTypeIndex() const { return m_ScriptTypeIndex; }
        void SetScriptID(const Hash128& hash) { m_ScriptID = hash; }
        const Hash128& GetScriptID() const { return m_ScriptID; }

        void SetOldTypeHash(const Hash128& hash) { m_OldTypeHash = hash; }
        const Hash128& GetOldTypeHash() const
        {
#if SUPPORT_SERIALIZED_TYPETREES && SUPPORT_SERIALIZE_WRITE
            DebugAssert(m_TypeTreeCacheId == 0); // m_TypeTreeCacheId == 0 states that this is NOT a referenced type. type hashes only work/available for non-refrenced types
#endif
            return m_OldTypeHash;
        }

        const TypeTree* GetOldType() const { return m_OldType; }

        void SetOldType(const TypeTree* t);
        enum { kEqual = 0, kNotEqual = 1, kNotCompared = -1 };

        int GetEqualState() const { return m_Equals; }

        void CompareAgainstNewType(Object& object, TypeVector &refTypesPool, TransferInstructionFlags options);

#if SUPPORT_SERIALIZED_TYPETREES

    #if !UNITY_EXTERNAL_TOOL || SUPPORT_SERIALIZE_WRITE  // this an odd way of expression it, but basicaly: player or editor.
        UInt64 GetTypeTreeCacheId();
#endif

    #if SUPPORT_SERIALIZE_WRITE
        void SetFQN(ScriptingClassPtr klass);
        void SetTypeTreeCacheId(UInt64 id) { m_TypeTreeCacheId = id; }
    #endif
#endif  // SUPPORT_SERIALIZED_TYPETREES

        template<bool kSwap, bool kWriteTypeDependencies>
        void WriteType(TypeVector& referencedTypesPool, bool enableTypeTree, std::vector<UInt8>& cache);

        template<bool kSwap, bool kReadTypeDependencies>
        bool ReadType(SerializedFileFormatVersion version, bool enableTypeTree, UInt8 const*& iterator, UInt8 const* end, int* originalTypeId = NULL, bool ignoreScriptTypeForHash = false);
    };
    explicit SerializedFile(MemLabelRef label);
    ~SerializedFile();

    // options: kSerializeGameRelease, kSwapEndianess, kBuildPlayerOnlySerializeBuildProperties
    SerializedFileLoadError InitializeWrite(CachedWriter& cachedWriter/*, BuildTargetSelection target*/, TransferInstructionFlags options);
    SerializedFileLoadError InitializeRead(const std::string& path, ResourceImageGroup& resourceImage, size_t cacheSize, bool prefetch, TransferInstructionFlags options, size_t readOffset = 0, size_t readEndOffset = SIZE_MAX);

    typedef SerializedType::TypeVector TypeVector;
    typedef dynamic_block_array<FileIdentifier, 64> FileIdentifierArray;

    // Add an external reference
    void AddExternalRef(const FileIdentifier& pathName);
    // Get/Set the list of FileIdentifiers this file uses
    const FileIdentifierArray& GetExternalRefs() const { return m_Externals; }

    // Returns the biggest id of all the objects in the file.
    // if no objects are in the file 0 is returned
    LocalIdentifierInFileType GetHighestID() const;

    // objects: On return, all fileIDs to all objects in this Serialize
    void GetAllFileIDs(std::vector<LocalIdentifierInFileType>& objects) const;

    void Release();

    inline bool ShouldSwapEndian() const { return m_FileEndianess != kActiveEndianess; }

#if SUPPORT_TEXT_SERIALIZATION
    bool IsTextFile() const { return m_IsTextFile; }
#else
    bool IsTextFile() const { return false; }
#endif
    // Returns whether or not an object is available in the stream
    bool IsAvailable(LocalIdentifierInFileType id) const;

    template<bool kSwap> void BuildMetadataSection(std::vector<UInt8>& cache, size_t dataOffsetInFile);
    template<bool kSwap> bool WriteHeader(std::vector<UInt8>& cache, size_t* outDataOffset = NULL);

    void ReadObject(LocalIdentifierInFileType fileID, ObjectCreationMode mode, bool isPersistent, const TypeTree** oldTypeTree, bool* safeLoaded, Object& object);

    // Extract all data necessary to produce the object
    bool GetProduceData(LocalIdentifierInFileType fileID, const HuaHuo::Type*& type, LocalSerializedObjectIdentifier& scriptTypeReference, MemLabelId& label);

    // Returns the seek position in the file where the object with id is stored
    size_t GetByteStart(LocalIdentifierInFileType id) const;

    // Returns the size the object takes up on the disk
    UInt32 GetByteSize(LocalIdentifierInFileType id) const;

    // Writes an object with id to the file.
    // Writing to a stream which includes objects with an older typetree version is not possible and false will be returned
    void WriteObject(Object& object, LocalIdentifierInFileType fileID ,SInt16 scriptTypeIndex/*, const BuildUsageTag& buildUsage, const GlobalBuildData& globalBuildData*/);

    bool FinishWriting(size_t* outDataOffset = NULL);

    enum { kSectionAlignment = 16 };

private:
    void FinalizeInitCommon(TransferInstructionFlags options);

    SerializedFileLoadError FinalizeInitRead(TransferInstructionFlags options);
    SerializedFileLoadError FinalizeInitWrite(TransferInstructionFlags options);

    template<bool kSwap> bool ReadMetadata(SerializedFileFormatVersion version, size_t dataOffset, UInt8 const* data, size_t length, size_t dataFileSize);
    void BuildRefTypePoolIfRelevant();

    TypeTree::Pool * m_RefTypePool;
    MemLabelId                          m_MemLabel;

    UInt8                               m_FileEndianess;

    FileIdentifierArray m_Externals;

    TransferInstructionFlags            m_Options;
    struct ObjectInfo
    {
        size_t byteStart;
        UInt32 byteSize;
        UInt32 typeID;

#if SUPPORT_TEXT_SERIALIZATION
        SInt64 debugLineStart;
#endif
    };

    size_t                       m_ReadOffset;
    size_t                       m_ReadEndOffset;
    size_t                       m_WriteDataOffset;

    CacheReaderBase*                    m_ReadFile;

    typedef vector_map<LocalIdentifierInFileType, ObjectInfo>   ObjectMap;
    ObjectMap                           m_Object;

    TypeVector m_Types;
    TypeVector m_RefTypes;
    bool                                m_EnableTypeTree;

    SerializedFileLoadError ReadHeader();

    CachedWriter*                       m_CachedWriter;

    ResourceImageGroup                  m_ResourceImageGroup;
};

template<bool kSwap, class T>
void WriteHeaderCache(const T& t, std::vector<UInt8>& vec)
{
    vec.resize(vec.size() + sizeof(T));//, kDoubleOnResize);
    T& dst = *reinterpret_cast<T*>(&vec[vec.size() - sizeof(T)]);
    dst = t;
    if (kSwap)
        SwapEndianBytes(dst);
}

bool TypeNeedsRemappingToNewTypeForBuild(const HuaHuo::Type* type);


#endif //HUAHUOENGINE_SERIALIZEDFILE_H
