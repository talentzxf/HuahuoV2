//
// Created by VincentZhang on 5/1/2022.
//

#include "SerializedFile.h"
#include "Serialize/SerializationCaching/FileCacherRead.h"

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
//    m_EnableTypeTree = false;

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

static const int kPreallocateFront = 4096;

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