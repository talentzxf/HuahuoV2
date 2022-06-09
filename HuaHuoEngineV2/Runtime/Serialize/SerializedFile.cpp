//
// Created by VincentZhang on 5/1/2022.
//

#include "SerializedFile.h"
#include "Serialize/SerializationCaching/FileCacherRead.h"
#include "HuaHuoEngineConfig.h"
#include "Utilities/Align.h"
#include "Serialize/SerializationCaching/CachedWriter.h"
#include "Serialize/SerializationCaching/CacheWriterBase.h"
#include "Utilities/Utility.h"
#include "Utilities/File.h"

const char* kAssetBundleVersionNumber = "2";


static const int kPreallocateFront = 4096;

struct SerializedFileHeader32
{
    static const int kHeaderSize_Ver8 = 12;

    // This header is always in BigEndian when in file
    // Metadata follows directly after the header
    UInt32  m_MetadataSize;
    UInt32  m_FileSize;
    SerializedFileFormatVersion  m_Version;
    UInt32  m_DataOffset;
    UInt8   m_Endianess;
    UInt8   m_Reserved[3];

    void SwapEndianess()
    {
        SwapEndianBytes(m_MetadataSize);
        SwapEndianBytes(m_FileSize);
        SwapEndianBytes(*(UInt32*)&m_Version);
        SwapEndianBytes(m_DataOffset);
    }
};

struct SerializedFileHeader
{
    // This header is always in BigEndian when in file
    // Metadata follows directly after the header
    UInt8   m_Legacy[sizeof(UInt32) * 2]; // unused, allows start of struct to be same as SerializedFileHeader32
    SerializedFileFormatVersion  m_Version;
    size_t m_MetadataSize;
    size_t m_FileSize;
    size_t  m_DataOffset;
    UInt8   m_Endianess;
    UInt8   m_Reserved[3];

    void SwapEndianess()
    {
        UInt64 x;
        x = m_MetadataSize;  SwapEndianBytes(x); m_MetadataSize = x;
        x = m_FileSize;      SwapEndianBytes(x); m_FileSize = x;
        x = m_DataOffset;    SwapEndianBytes(x); m_DataOffset = x;
        SwapEndianBytes(*(UInt32*)&m_Version);
    }
};

SerializedFile::SerializedType::~SerializedType()
{
#if SUPPORT_SERIALIZED_TYPETREES
    UNITY_DELETE(m_OldType, kMemTypeTree);
#endif
}


void SerializedFile::AddExternalRef(const FileIdentifier& pathName)
{
    // Dont' check for pathname here - it can be empty, if we are getting a GUID from
    // a text serialized file, and the file belonging to the GUID is missing. In that
    // case we just keep the GUID.
#if SUPPORT_SERIALIZE_WRITE
    Assert(m_CachedWriter != NULL);
#endif
    m_Externals.push_back(pathName);
    m_Externals.back().CheckValidity();
}

LocalIdentifierInFileType SerializedFile::GetHighestID() const
{
    if (m_Object.empty())
        return 0;
    else
        return m_Object.rbegin()->first;
}

void SerializedFile::FinalizeInitCommon(TransferInstructionFlags options)
{
    m_Options = options;
#if SUPPORT_SERIALIZE_WRITE
    m_EnableTypeTree = (m_Options & kDisableWriteTypeTree) == 0;
#endif
#if !UNITY_EDITOR
    m_Options |= kSerializeGameRelease;
#endif

    if (m_Options & kSwapEndianess)
        m_FileEndianess = kOppositeEndianess;
    else
        m_FileEndianess = kActiveEndianess;
}

// objects: On return, all fileIDs to all objects in this Serialize
void SerializedFile::GetAllFileIDs(std::vector<LocalIdentifierInFileType>& objects) const
{
    objects.reserve(m_Object.size());
    ObjectMap::const_iterator i;
    for (i = m_Object.begin(); i != m_Object.end(); i++)
    {
        const HuaHuo::Type* unityType = m_Types[i->second.typeID].GetType();
        if (unityType != NULL && unityType->GetFactory() != NULL)
            objects.push_back(i->first);
    }
}


SerializedFileLoadError SerializedFile::FinalizeInitRead(TransferInstructionFlags options)
{
    FinalizeInitCommon(options);

    if (m_ReadFile)
    {
#if SUPPORT_TEXT_SERIALIZATION
        static const size_t magiclen = strlen(kUnityTextMagicString);
        char signature[256];
        if (m_ReadEndOffset.Cast<UInt64>() >= magiclen)
        {
            static const int bomLen = 3;
            // Read possible UTF8 marker and text signature
            ReadFileCache(*m_ReadFile, signature, m_ReadOffset, magiclen + bomLen);
            signature[magiclen + bomLen] = '\0';

            // Skip BOM if present
            const size_t bomOffset = HasUTF8BOM(signature) ? bomLen : 0;
            // Verify text serialized file signature
            if (strncmp(signature + bomOffset, kUnityTextMagicString, magiclen) == 0)
            {
                m_IsTextFile = true;
                m_ReadOffset += (UInt64)(bomOffset + magiclen);
                return ReadHeaderText();
            }
        }
        m_IsTextFile = false;
#endif
        return ReadHeader();
    }
    else
    {
        return SerializedFileLoadError::kSerializedFileLoadError_Unknown;
    }
}

SerializedFile::SerializedType::SerializedType(const HuaHuo::Type* unityType, bool isStrippedType, SInt16 scriptTypeIdx)
        : m_Type(unityType)
        , m_IsStrippedType(isStrippedType)
        , m_PerClassTypeTree(true)
        , m_ScriptTypeIndex(scriptTypeIdx)
#if SUPPORT_SERIALIZED_TYPETREES
, m_OldType(NULL)
    , m_Equals(kNotCompared)
    #if !UNITY_EXTERNAL_TOOL || SUPPORT_SERIALIZE_WRITE  // this an odd way of expression it, but basicaly: player or editor.
    , m_TypeTreeCacheId(0)
    #endif
#endif
{
}

SerializedFileLoadError SerializedFile::ReadHeader()
{
//    Assert(m_ReadFile != NULL);
//
//    SerializedFileHeader header{};
//
//    if (m_ReadEndOffset.Cast<UInt64>() <= 0)
//        return kSerializedFileLoadError_EmptyOrCorruptFile;
//
//    if (m_ReadEndOffset.Cast<UInt64>() < sizeof(header))
//        return kSerializedFileLoadError_Unknown;
//
//    SerializedFileHeader32 legacyHeader{};
//    ReadFileCache(*m_ReadFile, &legacyHeader, m_ReadOffset, sizeof(legacyHeader));
//    if (kActiveEndianess == kLittleEndian)
//        legacyHeader.SwapEndianess();
//
//    if (legacyHeader.m_Version < SerializedFileFormatVersion::kLargeFilesSupport)
//    {
//        header.m_Version = legacyHeader.m_Version;
//        header.m_DataOffset = (UInt64)legacyHeader.m_DataOffset;
//        header.m_MetadataSize = (UInt64)legacyHeader.m_MetadataSize;
//        header.m_FileSize = (UInt64)legacyHeader.m_FileSize;
//        header.m_Endianess = legacyHeader.m_Endianess;
//
//        if (header.m_FileSize.Cast<UInt32>() == std::numeric_limits<UInt32>::max())
//            header.m_FileSize = VFS::FileSize::Max();
//    }
//    else
//    {
//        ReadFileCache(*m_ReadFile, &header, m_ReadOffset, sizeof(header));
//        if (kActiveEndianess == kLittleEndian)
//            header.SwapEndianess();
//    }
//
//    // Consistency check if the file is a valid serialized file.
//    if (header.m_MetadataSize == VFS::FileSize::Max())
//        return kSerializedFileLoadError_Unknown;
//    if (header.m_Version == SerializedFileFormatVersion::kUnsupported)
//        return kSerializedFileLoadError_Unknown;
//    if (header.m_Version > SerializedFileFormatVersion::kCurrentSerializeVersion)
//        return kSerializedFileLoadError_HigherSerializedFileVersion;
//
//    VFS::FileSize metadataSize, metadataOffset;
//    VFS::FileSize dataSize, dataOffset;
//    VFS::FileSize dataEnd;
//
//    if (header.m_Version >= SerializedFileFormatVersion::kUnknown_9)
//    {
//        // If we're reading a stream file, m_ReadOffset + header.m_FileSize will not necessarilly be equal to m_ReadEndOffset,
//        // because there can be few padding bytes which doesn't count into header.m_FileSize
//        // See WriteStreamFile in BuildPlayerUtility.cpp
//        if ((m_ReadOffset + header.m_FileSize) > m_ReadEndOffset
//            || header.m_DataOffset > header.m_FileSize
//            || header.m_FileSize.Cast<UInt64>() == 0
//            || header.m_FileSize == VFS::FileSize::Max())
//            return kSerializedFileLoadError_Unknown;
//
//        // [header][metadata[...]][data]
//
//        if (header.m_Version < SerializedFileFormatVersion::kLargeFilesSupport)
//            metadataOffset = (UInt64)sizeof(SerializedFileHeader32);
//        else
//            metadataOffset = (UInt64)sizeof(SerializedFileHeader);
//        metadataSize = header.m_MetadataSize;
//
//        m_FileEndianess = header.m_Endianess;
//
//        dataOffset = header.m_DataOffset;
//        dataSize = header.m_FileSize - header.m_DataOffset;
//        dataEnd = dataOffset + dataSize;
//
//        if (dataEnd.Cast<UInt64>() == 0)
//            return kSerializedFileLoadError_Unknown;
//    }
//    else
//    {
//        // [header][data][metadata]
//
//        // We set dataOffset to zero, because offsets in object table are file-start based
//        dataOffset = (UInt64)0;
//        dataSize = header.m_FileSize - header.m_MetadataSize - (UInt64)SerializedFileHeader32::kHeaderSize_Ver8;
//        dataEnd = header.m_FileSize - header.m_MetadataSize;
//
//        // Offset by one, because we're reading the endianess flag right here
//        metadataOffset = header.m_FileSize - header.m_MetadataSize + (UInt64)1;
//        metadataSize = header.m_MetadataSize - (UInt64)1;
//
//        if (metadataSize == VFS::FileSize::Max() || (m_ReadOffset + header.m_FileSize) > m_ReadEndOffset || dataEnd > header.m_FileSize)
//            return kSerializedFileLoadError_Unknown;
//
//        ReadFileCache(*m_ReadFile, &m_FileEndianess, m_ReadOffset + metadataOffset - (UInt64)1, sizeof(m_FileEndianess));
//    }
//
//    // Check endianess validity
//    if (m_FileEndianess != kBigEndian && m_FileEndianess != kLittleEndian)
//        return kSerializedFileLoadError_Unknown;
//
//    dynamic_array<UInt8> metadataBuffer(kMemSerialization);
//    metadataBuffer.resize_uninitialized(metadataSize.Cast<size_t>());
//    ReadFileCache(*m_ReadFile, metadataBuffer.data(), m_ReadOffset + metadataOffset, metadataSize.Cast<size_t>());
//
//    bool metaDataRead;
//    if (m_FileEndianess == kActiveEndianess)
//        metaDataRead = ReadMetadata<false>(header.m_Version, dataOffset, metadataBuffer.begin(), metadataBuffer.size(), dataEnd);
//    else
//        metaDataRead = ReadMetadata<true>(header.m_Version, dataOffset, metadataBuffer.begin(), metadataBuffer.size(), dataEnd);
//    if (!metaDataRead)
//    {
//#if UNITY_EDITOR
//        ErrorStringMsg("Invalid serialized file header. File: \"%s\".", m_DebugPath.c_str());
//#endif
//        return kSerializedFileLoadError_Unknown;
//    }
//
//    PatchRemapDeprecatedClasses();

    return kSerializedFileLoadError_None;
}

void SerializedFile::Release()
{
//    if (m_RefCount.Release())
//    {
//        SerializedFile* temp = this;
//        HUAHUO_DELETE(temp, kMemSerialization);
//    }
}

SerializedFile::SerializedFile(MemLabelRef label)
        : m_MemLabel(label)
        //, m_ProduceDataMemLabel(kMemBaseObject)
        , m_Externals(label)
//        , m_Types(label)
//        , m_ScriptTypes(label)
#if SUPPORT_SERIALIZED_TYPETREES && !UNITY_EXTERNAL_TOOL
, m_RefTypePool(nullptr)
#endif
{
//    m_MemoryStream = false;
//    m_HasErrors = false;
//    m_CachedFileStream = false;
//    m_TargetPlatform = BuildTargetSelection(kBuildNoTargetPlatform, 0, false);
    m_EnableTypeTree = false;

#if SUPPORT_TEXT_SERIALIZATION
    m_IsTextFile = false;
#endif

#if SUPPORT_SERIALIZE_WRITE
    m_CachedWriter = NULL;
#endif

    m_ReadFile = NULL;
}


SerializedFileLoadError SerializedFile::InitializeRead(const std::string& path, ResourceImageGroup& resourceImage, size_t cacheSize, bool prefetch, TransferInstructionFlags options, size_t readOffset, size_t readEndOffset)
{
    SET_ALLOC_OWNER(m_MemLabel);
    m_ReadOffset = readOffset;
    m_ReadFile = HUAHUO_NEW(FileCacherRead, m_MemLabel)(m_MemLabel, path, cacheSize, prefetch);
    SerializedFileLoadError loadError = kSerializedFileLoadError_None;

    size_t fileSize = m_ReadFile->GetFileLength();
    m_ReadEndOffset = readEndOffset == SIZE_MAX ? (UInt64)fileSize : readEndOffset;
    m_ResourceImageGroup = resourceImage;
    loadError = FinalizeInitRead(options);

//    if (loadError != kSerializedFileLoadError_None)
//        PrintSerializedFileLoadError(path, fileSize, loadError);

    return loadError;
}

SerializedFileLoadError SerializedFile::FinalizeInitWrite(TransferInstructionFlags options)
{
    FinalizeInitCommon(options);

#if SUPPORT_TEXT_SERIALIZATION
    if (m_IsTextFile)
    {
        TempString label = kUnityTextMagicString;
        label += "\n%TAG !u! tag:unity3d.com,2011:\n";
        m_CachedWriter->Write(&label[0], label.length());
    }
#endif
    return kSerializedFileLoadError_None;
}

SerializedFileLoadError SerializedFile::InitializeWrite(CachedWriter& cachedWriter/*, BuildTargetSelection target*/,  TransferInstructionFlags options)
{
            SET_ALLOC_OWNER(m_MemLabel);
    // m_TargetPlatform = target;
    m_CachedWriter = &cachedWriter;

//    Assert(!((options & kAllowTextSerialization) && (options & kSerializeGameRelease)));
//    m_IsTextFile = options & kAllowTextSerialization;
//
//    if (!m_IsTextFile)
//    {
        void* buffer = alloca(kPreallocateFront);
        memset(buffer, 0, kPreallocateFront);

        // Write header and reserve space for metadata. In case the resulting metadata will not fit
        // in the preallocated space we'll remove it and write it tightly packed in FinalizeWrite later.
        // In case it fits, we'll have a hole between meta and object data and that's fine.

        m_CachedWriter->Write(buffer, kPreallocateFront);
        m_WriteDataOffset = m_CachedWriter->GetPosition();
//    }

    return FinalizeInitWrite(options);
}

static SInt32 FindOrCreateSerializedTypeForUnityType(SerializedFile::TypeVector& serializedTypes, const HuaHuo::Type* unityType, bool isStripped, SInt16 scriptTypeIndex, SInt32 originalTypeId = -1)
{
    PersistentTypeID findPersistentTypeID = (unityType == NULL) ? HuaHuo::Type::UndefinedPersistentTypeID : unityType->GetPersistentTypeID();
    for (int i = 0; i < serializedTypes.size(); ++i)
    {
        const SerializedFile::SerializedType& serializedType = serializedTypes[i];
        PersistentTypeID serializedPersistentTypeID = serializedType.GetPersistentTypeID();
        if (serializedPersistentTypeID == findPersistentTypeID &&
            serializedType.IsStripped() == isStripped &&
            serializedType.GetScriptTypeIndex() == scriptTypeIndex &&
            (originalTypeId < 0 || serializedTypes[originalTypeId].GetPersistentTypeID() == findPersistentTypeID))
        {
            return i;
        }
    }

    SerializedFile::SerializedType serializedType(unityType, isStripped, scriptTypeIndex);
    serializedTypes.push_back(serializedType);

    if (originalTypeId >= 0 && serializedTypes[originalTypeId].GetOldTypeHash() != serializedTypes[serializedTypes.size() - 1].GetOldTypeHash())
    {
#if SUPPORT_SERIALIZED_TYPETREES
        if (serializedTypes[originalTypeId].GetOldType() != NULL)
        {
            TypeTree* typeTreeCopy = UNITY_NEW(TypeTree, kMemTypeTree);
            *typeTreeCopy = *serializedTypes[originalTypeId].GetOldType();

            serializedTypes[serializedTypes.size() - 1].SetOldType(typeTreeCopy);
        }
#endif // SUPPORT_SERIALIZED_TYPETREES
        serializedTypes[serializedTypes.size() - 1].SetOldTypeHash(serializedTypes[originalTypeId].GetOldTypeHash());
    }

    return serializedTypes.size() - 1;
}

size_t SerializedFile::GetByteStart(LocalIdentifierInFileType id) const
{
    ObjectMap::const_iterator i = m_Object.find(id);
    Assert(i != m_Object.end());
    return i->second.byteStart;
}

UInt32 SerializedFile::GetByteSize(LocalIdentifierInFileType id) const
{
    ObjectMap::const_iterator i = m_Object.find(id);
    Assert(i != m_Object.end());
    return i->second.byteSize;
}

void SerializedFile::WriteObject(Object& object, LocalIdentifierInFileType fileID/*, SInt16 scriptTypeIndex, const BuildUsageTag& buildUsage, const GlobalBuildData& globalBuildData*/)
{
    Assert(m_CachedWriter != NULL);
            SET_ALLOC_OWNER(m_MemLabel);

    TransferInstructionFlags mask = kReadWriteFromSerializedFile | m_Options;

#if UNITY_EDITOR
    object.SetFileIDHint(fileID);
#endif

    const HuaHuo::Type* objectType = object.GetType();
    const PersistentTypeID objectPersistentTypeID = objectType->GetPersistentTypeID();
    SInt32 typeID = -1;


//    if (!IsTextFile())
//    {
//        // Native C++ object typetrees share typetree by class id
//        bool perObjectTypeTree = object.GetNeedsPerObjectTypeTree(); // || buildUsage.strippedPrefabObject;
//        if (!perObjectTypeTree)
//        {
//            // typeID = FindOrCreateSerializedTypeForUnityType(m_Types, objectType, buildUsage.strippedPrefabObject, scriptTypeIndex);
//            typeID = FindOrCreateSerializedTypeForUnityType(m_Types, objectType, false, 0);// , scriptTypeIndex);
//            SerializedType& serializedType = m_Types[typeID];
//            if (serializedType.GetOldType() == NULL)
//            {
//                TypeTree* typeTree = UNITY_NEW(TypeTree, kMemTypeTree);
//                TypeTreeCache::GetTypeTree(&object, mask | kDontRequireAllMetaFlags, *typeTree);
//                serializedType.SetOldType(typeTree);
//
//                Hash128 tempHash;
//                if ((mask & kBuildPlayerOnlySerializeBuildProperties) != 0 && TypeNeedsRemappingToNewTypeForBuild(objectType))
//                {
//                    SetObjectLockForWrite();
//                    tempHash = CalculateClassHash(objectType, mask | kDontRequireAllMetaFlags);
//                    ReleaseObjectLock();
//                }
//                else
//                {
//                    tempHash = TypeTreeQueries::HashTypeTree(typeTree->Root());
//                }
//
//                serializedType.SetOldTypeHash(tempHash);
//                serializedType.m_ScriptTypeIndex = scriptTypeIndex;
//            }
//        }
//            // Scripted objects we search the registered typetrees for duplicates and share
//            // or otherwise allocate a new typetree.
//        else
//        {
//            Hash128 scriptID;
//            Hash128 typeHash;
//
//            TypeTree* typeTree = UNITY_NEW(TypeTree, kMemTypeTree);
//            if (buildUsage.strippedPrefabObject)
//            {
//                // As strippedPrefabObject is only used when writing scene, so we just generate the hash from the type tree
//                // to eliminate duplicate type trees.
//                GenerateStrippedTypeTree(object, *typeTree, buildUsage, mask | kDontRequireAllMetaFlags);
//                typeHash = TypeTreeQueries::HashTypeTree(typeTree->Root());
//            }
//            else
//            {
//                TypeTreeCache::GetTypeTree(&object, mask | kDontRequireAllMetaFlags, *typeTree);
//
//                // For now, only IManagedObjectHost has per-object type tree: assert to make sure the typeHash is generated correctly
//                // if anyone introduces a new type which has per-object type tree.
//                Assert(IManagedObjectHost::IsObjectsTypeAHost(&object));
//                MonoScript* script = IManagedObjectHost::GetManagedReference(object)->GetScript();
//                if (script != NULL)
//                {
//                    scriptID = script->GenerateScriptID();
//                    typeHash = script->GetPropertiesHash(); // Generate the script hash in exactly same way as when we generate the script hashes for BuildSettings.
//                }
//            }
//
//            if (typeID < 0)
//            {
//                // Find the type id if there is one matches.
//                for (int i = 0; i < m_Types.size(); ++i)
//                {
//                    const SerializedType& serializedType = m_Types[i];
//
//                    if (serializedType.GetPersistentTypeID() != objectPersistentTypeID)
//                        continue;
//
//                    if (serializedType.GetOldTypeHash().IsValid() && serializedType.GetOldTypeHash() == typeHash && serializedType.IsStripped() == buildUsage.strippedPrefabObject && serializedType.GetScriptTypeIndex() == scriptTypeIndex)
//                    {
//                        typeID = i;
//                        UNITY_DELETE(typeTree, kMemTypeTree);
//                        break;
//                    }
//                }
//            }
//
//            // If no, generate a new type id and set the type tree and type tree hash.
//            if (typeID < 0)
//            {
//                typeID = m_Types.size();
//                m_Types.push_back(SerializedType(objectType, buildUsage.strippedPrefabObject));
//                SerializedType& serializedType = m_Types[typeID];
//
//                if (m_EnableTypeTree)
//                    serializedType.SetOldType(typeTree);
//                else
//                    UNITY_DELETE(typeTree, kMemTypeTree);
//
//                serializedType.SetOldTypeHash(typeHash);
//                serializedType.SetScriptID(scriptID);
//                serializedType.m_ScriptTypeIndex = scriptTypeIndex;
//            }
//
//            // For IManagedObjectHost, register all type trees of the ManagedReferences
//            if (IManagedObjectHost::IsObjectsTypeAHost(object))
//            {
//                DependencyCollectorForSerializeRef serializedRefs;
//
//                RemapPPtrTransfer transferFunction(mask, true);
//                transferFunction.SetReferencedObjectFunctor(&serializedRefs);
//                transferFunction.SetGenerateIDFunctor(&serializedRefs);
//
//                object.VirtualRedirectTransfer(transferFunction);
//                for (auto& j: serializedRefs.m_ScriptObjects)
//                {
//                    AddSerializedTypeForManagedReference(typeID, buildUsage.strippedPrefabObject, ::scripting_object_get_class(j), mask);
//                }
//            }
//        }
//    }
//    else
    {
        typeID = FindOrCreateSerializedTypeForUnityType(m_Types, objectType, false, /* buildUsage.strippedPrefabObject, scriptTypeIndex*/ 0);
        // AssertMsg(m_Types[typeID].GetScriptTypeIndex() == scriptTypeIndex, "Type has not the requested ScriptTypeIndex.");
    }

    // We are not taking care of fragmentation.
    const size_t kFileAlignment = 8;

    UInt64 unalignedByteStart = m_CachedWriter->GetPosition();//.Cast<UInt64>();

    // Align the object to a kFileAlignment byte boundary
    UInt64 alignedByteStart = unalignedByteStart;
    if (unalignedByteStart % kFileAlignment != 0)
        alignedByteStart += kFileAlignment - unalignedByteStart % kFileAlignment;

    ObjectMap::const_iterator tmpObject = m_Object.find(fileID);
    AssertFormatMsg(!(tmpObject != m_Object.end() && m_Types[tmpObject->second.typeID].GetPersistentTypeID() != objectPersistentTypeID),
                    "File contains the same file identifier (%d) for multiple object types (%s) (%s). Object may be overwritten.", fileID, objectType->GetName(), m_Types[tmpObject->second.typeID].GetType()->GetName());

    ObjectInfo& info = m_Object[fileID];
    info.byteStart = alignedByteStart;
    info.typeID = typeID;
//    Assert(m_Types[typeID].GetScriptTypeIndex() == scriptTypeIndex);
//    Assert(m_Types[typeID].IsStripped() == buildUsage.strippedPrefabObject);


    /*  ////// PRINT OUT serialized Data as ascii to console
        if (false && gPrintfDataHack)
        {
            printf_console ("\n\nPrinting object: %d\n", fileID);

            // Set write marker to end of file and register the objects position in file
            StreamedTextWrite writeStream;
            CachedWriter& cache = writeStream.Init (kReadWriteFromSerializedFile);
            cache.Init (m_FileCacher, alignedByteStart, 0, false);

            // Write the object
            object.VirtualRedirectTransfer (writeStream);
            cache.End ();
        }
    */

#if SUPPORT_TEXT_SERIALIZATION
    if (m_IsTextFile)
    {
        core::string label;
        if (buildUsage.strippedPrefabObject)
            label = Format("--- !u!%d &%lld stripped\n", m_Types[info.typeID].GetPersistentTypeID(), fileID);
        else
            label = Format("--- !u!%d &%lld\n", m_Types[info.typeID].GetPersistentTypeID(), fileID);

        WriteTextSerialized(label, object, buildUsage, mask);
    }
    else
#endif
    if (!ShouldSwapEndian())
    {
        // Set write marker to end of file and register the objects position in file
        StreamedBinaryWrite writeStream;

        CachedWriter& cache = writeStream.Init(*m_CachedWriter, mask/*, m_TargetPlatform, buildUsage, globalBuildData*/);
        char kZeroAlignment[kFileAlignment] = {0, 0, 0, 0, 0, 0, 0, 0};
        cache.Write(kZeroAlignment, alignedByteStart - unalignedByteStart);

        // Write the object
//        if (buildUsage.strippedPrefabObject)
//            object.VirtualStrippedRedirectTransfer(writeStream);
//        else
            object.VirtualRedirectTransfer(writeStream);

        *m_CachedWriter = cache;
    }
    else
    {
        AssertString("Writing endian swapped data is not supported.");
    }

    info.byteSize = (m_CachedWriter->GetPosition() - info.byteStart); //.Cast<UInt32>();
}

template<bool kSwap>
void WriteHeaderCache(const std::string & str, std::vector<UInt8>& cache)
{
    int size = cache.size();
    cache.resize(size + str.size() + 1); //, kDoubleOnResize);
    memcpy(&cache[size], str.data(), str.size());
    cache.back() = '\0';
}

static void Write4Alignment(std::vector<UInt8>& cache)
{
    UInt32 leftOver = Align4LeftOver(cache.size());
    UInt8 value = 0;
    for (UInt32 i = 0; i < leftOver; ++i)
    {
        WriteHeaderCache<true>(value, cache);
    }
}

template<bool kSwap>
void SerializedFile::BuildMetadataSection(std::vector<UInt8>& cache, size_t dataOffsetInFile)
{
    // Write Unity version file is being built with
    std::string version = HHE_VERSION_STR;

//    if (HasFlag(m_Options, kDontWriteUnityVersion))
//        version = UNITY_STRIPPED_VERSION;

    if (m_Options & kSerializedAssetBundleVersion)
    {
        version += "\n";
        version += kAssetBundleVersionNumber;
    }

    WriteHeaderCache<kSwap>(version, cache);
//    WriteHeaderCache<kSwap>(static_cast<UInt32>(m_TargetPlatform.platform), cache);
//    WriteHeaderCache<kSwap>(m_EnableTypeTree, cache);

    // Write number of type info.
    SInt32 typeCount = m_Types.size();
    WriteHeaderCache<kSwap>(typeCount, cache);

    // Write non-referenced type info.
    for (int i = 0; i < typeCount; ++i)
    {
        m_Types[i].WriteType<kSwap, false>(m_RefTypes, m_EnableTypeTree, cache);
    }

    // Write number of objects
    SInt32 objectCount = m_Object.size();
    WriteHeaderCache<kSwap>(objectCount, cache);
    for (ObjectMap::iterator i = m_Object.begin(); i != m_Object.end(); i++)
    {
        Write4Alignment(cache);
        WriteHeaderCache<kSwap>(i->first, cache);

        WriteHeaderCache<kSwap>((UInt64)(i->second.byteStart - dataOffsetInFile)/*.Cast<UInt64>()*/, cache);
        WriteHeaderCache<kSwap>(i->second.byteSize, cache);
        WriteHeaderCache<kSwap>(i->second.typeID, cache);
        //printf_console ("fileID: %d byteStart: %d typeID: %d \n", i->first, i->second.byteStart, i->second.typeID);
    }

//    // Write Script Types
//    objectCount = m_ScriptTypes.size();
//    WriteHeaderCache<kSwap>(objectCount, cache);
//    for (int i = 0; i < objectCount; i++)
//    {
//        WriteHeaderCache<kSwap>(m_ScriptTypes[i].localSerializedFileIndex, cache);
//        Write4Alignment(cache);
//        WriteHeaderCache<kSwap>(m_ScriptTypes[i].localIdentifierInFile, cache);
//    }

    // Write externals
    objectCount = m_Externals.size();
    WriteHeaderCache<kSwap>(objectCount, cache);
    for (int i = 0; i < objectCount; i++)
    {
        std::string tempEmpty;
        WriteHeaderCache<kSwap>(tempEmpty, cache);
        for (int g = 0; g < 4; g++)
            WriteHeaderCache<kSwap>(m_Externals[i].guid.data[g], cache);
        WriteHeaderCache<kSwap>(m_Externals[i].type, cache);
        WriteHeaderCache<kSwap>(m_Externals[i].pathName, cache);
    }
    // Write Ref Types
    // Write number of type info.
    typeCount = m_RefTypes.size();
    WriteHeaderCache<kSwap>(typeCount, cache);

    // Write ref types info.
    for (int i = 0; i < typeCount; ++i)
    {
        m_RefTypes[i].WriteType<kSwap, true>(m_RefTypes, m_EnableTypeTree, cache);
    }

    // Write User info
    std::string tempUserInformation;
    WriteHeaderCache<kSwap>(tempUserInformation, cache);
}


bool SerializedFile::FinishWriting(size_t* outDataOffset)
{
    Assert(m_CachedWriter != NULL);

    if (m_CachedWriter != NULL)
    {
        if (!IsTextFile())
        {
            std::vector<UInt8> metadataBuffer; //(kMemSerialization);

            if (!ShouldSwapEndian())
            {
                BuildMetadataSection<false>(metadataBuffer, m_WriteDataOffset);
                return WriteHeader<false>(metadataBuffer, outDataOffset);
            }
            else
            {
                BuildMetadataSection<true>(metadataBuffer, m_WriteDataOffset);
                return WriteHeader<true>(metadataBuffer, outDataOffset);
            }
        }
        else
        {
            bool success = m_CachedWriter->CompleteWriting();
            success &= m_CachedWriter->GetCacheBase().WriteHeaderAndCloseFile(NULL, 0, 0);
            if (outDataOffset != NULL)
                (*outDataOffset) = (UInt64)0;
            return success;
        }
    }

    return false;
}

static void WriteAlignmentData(File& file, size_t misalignment)
{
    Assert(misalignment < SerializedFile::kSectionAlignment);
    UInt8 data[SerializedFile::kSectionAlignment];
    memset(data, 0, misalignment);
    file.Write(data, misalignment);
}

template<bool kSwap, bool kIsAReferencedType>
void SerializedFile::SerializedType::WriteType(TypeVector & referencedTypesPool, bool enableTypeTree, std::vector<UInt8>& cache)
{
    SInt32 persistentTypeID = GetPersistentTypeID();

    WriteHeaderCache<kSwap>(persistentTypeID, cache);
    WriteHeaderCache<kSwap>(m_IsStrippedType, cache);
    WriteHeaderCache<kSwap>(m_ScriptTypeIndex, cache);

//    // Only write the script ID for scripts.
//    if (persistentTypeID == kMonoBehaviourPersistentID || m_ScriptTypeIndex >= 0)
//    {
//        WriteHeaderCache<kSwap>(m_ScriptID.hashData.u32[0], cache);
//        WriteHeaderCache<kSwap>(m_ScriptID.hashData.u32[1], cache);
//        WriteHeaderCache<kSwap>(m_ScriptID.hashData.u32[2], cache);
//        WriteHeaderCache<kSwap>(m_ScriptID.hashData.u32[3], cache);
//    }

//    WriteHeaderCache<kSwap>(m_OldTypeHash.hashData.u32[0], cache);
//    WriteHeaderCache<kSwap>(m_OldTypeHash.hashData.u32[1], cache);
//    WriteHeaderCache<kSwap>(m_OldTypeHash.hashData.u32[2], cache);
//    WriteHeaderCache<kSwap>(m_OldTypeHash.hashData.u32[3], cache);

//    if (enableTypeTree)
//    {
//        TypeTreeIO::WriteTypeTree(*m_OldType, cache, kSwap);
//
//        if (kIsAReferencedType)
//        {
//            WriteHeaderCache<kSwap>(m_KlassName, cache);
//            WriteHeaderCache<kSwap>(m_NameSpace, cache);
//            WriteHeaderCache<kSwap>(m_AsmName, cache);
//        }
//        else
//        {
//            // only write dependencies for Object types (MonoBehaviour and the like)
//            SInt32 dependenciesCount = m_TypeDependencies.size();
//            WriteHeaderCache<kSwap>(dependenciesCount, cache);
//            if (dependenciesCount > 0)
//            {
//                size_t writeSize = sizeof(SInt32) * dependenciesCount;
//                cache.resize_uninitialized(cache.size() + writeSize);
//                UInt32* dst = (UInt32*)(cache.data() + cache.size() - writeSize);
//                std::memcpy(dst, &*m_TypeDependencies.begin(), writeSize);
//            }
//        }
//    }
}

template<bool kSwap>
bool SerializedFile::WriteHeader(std::vector<UInt8>& metadata, size_t* outDataOffset)
{
    bool success = true;

    // The aggregated metadata fits into the pre-written block, so write it directly.
    if (metadata.size() <= kPreallocateFront - sizeof(SerializedFileHeader))
    {
        UInt8* temp = (UInt8*)alloca(kPreallocateFront);
        memset(temp, 0, kPreallocateFront);

        // Make sure to zero this out, as padding can result in uninitialised data that gets copied into the file
        // This can cause an unstable content hash
        memset(temp, 0, sizeof(SerializedFileHeader));
        SerializedFileHeader& header = *(SerializedFileHeader*)temp;
        header.m_MetadataSize = (UInt64)metadata.size();
        header.m_FileSize = size_t(m_CachedWriter->GetPosition());
        header.m_Version = SerializedFileFormatVersion::kCurrentSerializeVersion;
        header.m_DataOffset = m_WriteDataOffset;
        header.m_Endianess = m_FileEndianess;

        if (outDataOffset != NULL)
            (*outDataOffset) = m_WriteDataOffset - (UInt64)kPreallocateFront;

        if (kActiveEndianess != kBigEndian)
            header.SwapEndianess();

        std::copy(metadata.begin(), metadata.end(), temp + sizeof(SerializedFileHeader));

        success &= m_CachedWriter->CompleteWriting();
        success &= m_CachedWriter->GetCacheBase().WriteHeaderAndCloseFile(temp, 0, sizeof(SerializedFileHeader) + metadata.size());
    }
    else
    {
        // metadata doesn't fit, therefore close the file, write header + metadata to another file
        // and copy data over from 'this' one.

        success &= m_CachedWriter->CompleteWriting();
        success &= m_CachedWriter->GetCacheBase().WriteHeaderAndCloseFile(NULL, 0, 0);

        size_t dataFileSize = m_CachedWriter->GetPosition();
        if (dataFileSize < kPreallocateFront)
            return false;

        size_t dataSize = dataFileSize - (UInt64)kPreallocateFront;
        size_t dataOffsetOriginal = (UInt64)(metadata.size() + sizeof(SerializedFileHeader));
        size_t dataOffset(RoundUp64(dataOffsetOriginal, (SInt64)kSectionAlignment));

        if (outDataOffset != NULL)
            (*outDataOffset) = dataOffset - (UInt64)kPreallocateFront;

        std::string originalPath = m_CachedWriter->GetCacheBase().GetPathName();
        std::string tempPath = originalPath;//GetUniqueTempPathInProject();

        SerializedFileHeader header{};

        // Make sure to zero this out, as padding can result in uninitialised data that gets copied into the file
        // This can cause an unstable content hash
        memset(&header, 0, sizeof(header));
        header.m_Version = SerializedFileFormatVersion::kCurrentSerializeVersion;
        header.m_MetadataSize = (UInt64)metadata.size();
        header.m_FileSize = dataOffset + dataSize;
        header.m_DataOffset = dataOffset;
        header.m_Endianess = m_FileEndianess;

        if (kActiveEndianess != kBigEndian)
            header.SwapEndianess();

        File file;
        success &= file.Open(tempPath, kWritePermission);

        // header
        success &= file.Write(&header, sizeof(header));

        // metadata
        success &= file.Write(metadata.data(), metadata.size());
        if (dataOffset != dataOffsetOriginal)
            WriteAlignmentData(file, (dataOffset - dataOffsetOriginal));
        // FatalErrorIf(dataOffset != file.GetPosition());

        {
            enum { kCopyChunck = 1 * 1024 * 1024 };

            UInt8* buffer;
            ALLOC_TEMP_AUTO(buffer, kCopyChunck);

            File srcFile;
            success &= srcFile.Open(originalPath, kReadPermission);

            size_t position = kPreallocateFront;
            size_t left = dataSize;
            while (left > 0 && success)
            {
                size_t toRead = std::min<size_t>(kCopyChunck, left);
                size_t wasRead = srcFile.Read((UInt64)position, buffer, toRead);
                success &= file.Write(buffer, wasRead);
                position += toRead;
                left -= toRead;
            }
            success &= srcFile.Close();

            success &= file.Close();
        }

//        // move the temp file over to the destination
//        success &= DeleteFile(originalPath);
//        success &= MoveFileOrDirectory(tempPath, originalPath);
    }

    return success;
}