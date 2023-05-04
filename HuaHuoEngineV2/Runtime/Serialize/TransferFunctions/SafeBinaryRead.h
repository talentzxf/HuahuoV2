#pragma once

// #if SUPPORT_SERIALIZED_TYPETREES

#include "TypeSystem/Type.h"
#include "Serialize/SerializationCaching/CachedReader.h"
#include "Serialize/SerializeTraits.h"
#include "Serialize/SwapEndianBytes.h"
#include "Serialize/TransferFunctions/TransferBase.h"
#include "Serialize/TypeTree.h"
#include "Serialize/SerializationCaching/StreamingInfo.h"

class SafeBinaryRead;
class AllowNameConversions;

#define LOG_CONVERTING_VARIBALES 0
#define LOG_MISSING_VARIBALES 0

typedef bool ConversionFunction (void* inData, SafeBinaryRead& transfer);

class EXPORT_COREMODULE SafeBinaryRead : public TransferBase
{
private:
    CachedReader            m_Cache;
    size_t           m_BaseBytePosition;
    SInt64                  m_BaseByteSize;  // size of the section to be read, not the size of the source file.

    TypeTreeIterator        m_OldBaseType;

    enum { kNotFound = 0, kMatchesType = 1, kFastPathKnownByteSizeArrayType = 2, kNeedConversion = -1 };

    struct StackedInfo
    {
        TypeTreeIterator    type;               /// The type tree of the old type we are reading data from
        const char*         currentTypeName;    /// The name of the type we are currently reading (This is the new type name and not from the stored data)
        size_t       bytePosition;       /// byte position of that element
        int                 version;            /// current version (This is the new version and not from the stored data)

        size_t       cachedBytePosition; /// The cached byte position of the last visited child
        size_t       cachedBytePositionNext;
        TypeTreeIterator    cachedIterator;     /// The cached iterator of the last visited child

#if !UNITY_RELEASE
        UInt32              currentTypeNameCheck;   /// For debugging purposes in case someone changes the typename string while still reading!
#endif
    };

    StackedInfo*               m_CurrentStackInfo;
    SInt32*                    m_CurrentPositionInArray;
    std::vector<StackedInfo> m_StackInfo;
    struct ArrayPositionInfo
    {
        SInt32 arrayPosition;
        size_t cachedBytePosition;
        SInt32 cachedArrayPosition;
    };

    std::vector<ArrayPositionInfo>  m_PositionInArray;// position in an array

    bool m_DidReadLastProperty;
    const AllowNameConversions*   m_AllowNameConversions;

    friend class SerializableManagedRefBackupGenerator;
    friend class SerializableManagedRefTransfer;

public:

    SafeBinaryRead();
    ~SafeBinaryRead();

    CachedReader& Init(const TypeTreeIterator& oldBase, size_t bytePosition, SInt64 byteSize, TransferInstructionFlags flags, void * managedIdsReferenceToReuse = nullptr);
    CachedReader& Init(SafeBinaryRead& transfer);

    void SetAllowNameConversions(const AllowNameConversions* conversions) { m_AllowNameConversions = conversions; }
    const AllowNameConversions* AllowedNameConversions() const { return m_AllowNameConversions; }

    void SetVersion(int version);
    bool IsCurrentVersion();
    bool IsOldVersion(int version);
    bool IsVersionSmallerOrEqual(int version);
    bool IsReading()                          { return true; }
    bool IsReadingPPtr()                      { return true; }
    bool IsReadingBackwardsCompatible()       { return true; }
    bool NeedsInstanceIDRemapping()           { return (m_Flags & kReadWriteFromSerializedFile) != 0; }
    bool IsCloningObject()                      { return (m_Flags & kIsCloningObject) != 0; }
    bool ConvertEndianess()                   { return (m_Flags & kSwapEndianess) != 0; }

    bool DidReadLastProperty()                { return m_DidReadLastProperty; }
    bool DidReadLastPPtrProperty()            { return m_DidReadLastProperty; }

    inline const TypeTree::Pool * GetReferencedTypes() const { return m_OldBaseType.GetTypeTree().GetReferencedTypes(); }

    CachedReader& GetCachedReader()           { return m_Cache; }

    template<class T>
    void TransferBase(T& data, TransferMetaFlags metaFlag = kNoTransferFlags);
    template<class T>
    void Transfer(T& data, const char* name, TransferMetaFlags metaFlag = kNoTransferFlags);

    template<class T>
    void TransferWithTypeString(T& data, const char* name, const char* typeName, TransferMetaFlags metaFlag = kNoTransferFlags);

    /// In order to transfer typeless data (Read: transfer data real fast)
    /// Call TransferTypeless. You have to always do this. Even for a GenerateTypeTreeTransfer. Then when you want to access the datablock.
    /// Call TransferTypelessData
    /// On return:
    /// When reading bytesize will contain the size of the data block that should be read,
    /// when writing bytesize has to contain the size of the datablock.
    /// MarkerID will contain an marker which you have to give TransferTypelessData when you want to start the actual transfer.
    /// optional: A serializedFile will be separated into two chunks. One is the normal object data. (It is assumed that they are all relatively small)
    /// So caching them makes a lot of sense. Big datachunks will be stored in another part of the file.
    /// They will not be cached but usually read directly into the allocated memory, probably reading them asynchronously
    void TransferTypeless(UInt32* byteSize, const char* name, TransferMetaFlags metaFlag = kNoTransferFlags);
    // markerID is the id that was given by TransferTypeless.
    // byteStart is the bytestart relative to the beginning of the typeless data
    // copyData is a pointer to where the data will be written or read from
    /// optional: if metaFlag is kTransferBigData the data will be optimized into separate blocks,
    void TransferTypelessData(UInt32 byteSize, void* copyData, int metaData = 0);

    template<class T>
    void TransferBasicData(T& data);

//    template<class T>
//    void TransferPtr(bool, ReduceCopyData*) {}

    template<class T>
    void TransferSTLStyleArray(T& data, TransferMetaFlags metaFlag = kNoTransferFlags);

    template<class T>
    void TransferSTLStyleMap(T& data, TransferMetaFlags metaFlag = kNoTransferFlags);

    const TypeTreeIterator& GetActiveOldTypeTreeIterator() const { return m_CurrentStackInfo->type; }

    static void RegisterConverter(const char* oldType, const char* newType, ConversionFunction* converter);
    template<typename OldType, typename NewType>
    static void RegisterConverter(ConversionFunction* converter)
    {
        RegisterConverter(SerializeTraits<OldType>::GetTypeString(NULL), SerializeTraits<NewType>::GetTypeString(NULL), converter);
    }

    static void CleanupConverterTable();

    void TransferResourceImage(ActiveResourceImage targetResourceImage, const char* name, StreamingInfo& streamingInfo, void* buffer, UInt32 byteSize, InstanceID instanceID, const HuaHuo::Type* type);

    int BeginTransfer(const char* name, const char* typeString, ConversionFunction** converter, bool allowTypeConversion);
    void EndTransfer();

private:

    // BeginTransfer / EndTransfer
    bool BeginArrayTransfer(const char* name, const char* typeString, SInt32& size);

    // Override the root type name, this is used by scripts that can only determine the class type name after the mono class has actually been loaded
    void OverrideRootTypeName(const char* typeString);

    void EndArrayTransfer();

    void Walk(const TypeTreeIterator& type, size_t* bytePosition, TypeTreeNode::ETypeFlags terminateWalkMask);
};

template<class T> inline
void SafeBinaryRead::TransferBasicData(T& data)
{
    m_Cache.Read(data, m_CurrentStackInfo->bytePosition);
    if (ConvertEndianess())
    {
        SwapEndianBytes(data);
    }
}

template<class T> inline
void SafeBinaryRead::TransferSTLStyleArray(T& data, TransferMetaFlags)
{
    SInt32 size = data.size();
    if (!BeginArrayTransfer("Array", "Array", size))
        return;

    SerializeTraits<T>::ResizeSTLStyleArray(data, size);

    typename T::iterator i;
    typename T::iterator end = data.end();
    if (size != 0)
    {
        int conversion = BeginTransfer("data", SerializeTraits<typename T::value_type>::GetTypeString(&*data.begin()), NULL, SerializeTraits<typename T::value_type>::AllowTypeConversion());

        size_t elementSize = m_CurrentStackInfo->type->m_ByteSize;
        *m_CurrentPositionInArray = 0;

        // If the data types are matching and element size can be determined
        // then we fast path the whole thing and skip all the duplicate stack walking
        if (conversion == kFastPathKnownByteSizeArrayType)
        {
            size_t basePosition = m_CurrentStackInfo->bytePosition;

            for (i = data.begin(); i != end; ++i)
            {
                size_t currentBytePosition = basePosition + (*m_CurrentPositionInArray) * elementSize;
                m_CurrentStackInfo->cachedBytePosition = currentBytePosition;
                m_CurrentStackInfo->bytePosition = currentBytePosition;
                m_CurrentStackInfo->cachedIterator = m_CurrentStackInfo->type.Children();
                (*m_CurrentPositionInArray)++;
                SerializeTraits<typename T::value_type>::Transfer(*i, *this);
            }
            EndTransfer();
        }
        // Fall back to converting variables
        else
        {
            EndTransfer();
            for (i = data.begin(); i != end; ++i)
                Transfer(*i, "data");
        }
    }

    EndArrayTransfer();
}

template<class T> inline
void SafeBinaryRead::TransferSTLStyleMap(T& data, TransferMetaFlags)
{
    SInt32 size = data.size();
    if (!BeginArrayTransfer("Array", "Array", size))
        return;

    // maps value_type is: pair<const First, Second>
    // So we have to write to maps non-const value type
    typename NonConstContainerValueType<T>::value_type p;

    ContainerClear(data);
    for (int i = 0; i < size; i++)
    {
        Transfer(p, "data");
        data.insert(p);
    }
    EndArrayTransfer();
}

template<class T> inline
void SafeBinaryRead::TransferWithTypeString(T& data, const char* name, const char* typeName, TransferMetaFlags)
{
    ConversionFunction* converter;
    int conversion = BeginTransfer(name, typeName, &converter, SerializeTraits<T>::AllowTypeConversion());
    if (conversion == kNotFound)
        return;

    if (conversion >= kMatchesType)
        SerializeTraits<T>::Transfer(data, *this);
    // Try conversion
    else
    {
        bool success = false;
        if (converter != NULL)
            success = converter(&data, *this);

        #if LOG_CONVERTING_VARIBALES
        {
            core::string s("Converting variable ");
            if (success)
                s += " succeeded ";
            else
                s += " failed ";

            GetTypePath(m_OldType.top(), s);
            s = s + " new type: ";
            s = s + " new type: (" + SerializeTraits<T>::GetTypeString() + ")\n";
            m_OldBaseType->DebugPrint(s);
            AssertStringQuiet(s);
        }
        #else
        UNUSED(success);
        #endif
    }
    EndTransfer();
}

template<class T>
void SafeBinaryRead::TransferBase(T& data, TransferMetaFlags metaFlag)
{
    Transfer(data, kTransferNameIdentifierBase, metaFlag);
}

template<class T> inline
void SafeBinaryRead::Transfer(T& data, const char* name, TransferMetaFlags metaFlag)
{
#if UNITY_EDITOR
    if (AssetMetaDataOnly() && HasFlag(metaFlag, kIgnoreInMetaFiles))
        return;
#endif

    TransferWithTypeString(data, name, SerializeTraits<T>::GetTypeString(&data), kNoTransferFlags);
#if UNITY_EDITOR
    SerializeTraits<T>::EnsureValid(data, metaFlag);
#endif
}

//#else
//
//namespace SafeBinaryReadManager
//{
//    inline void StaticInitialize() {}
//    inline void StaticDestroy() {}
//}
//
//#endif
