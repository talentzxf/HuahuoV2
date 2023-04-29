#include "GenerateTypeTreeTransfer.h"
#include "Utilities/Align.h"
#include "Serialize/SerializationCaching/StreamingInfo.h"

static inline int GetObjectOffset(const TypeTreeIterator& type)
{
    UInt32 offset = type.ByteOffset();
    return offset != TypeTree::kByteOffsetUnset && (offset & TypeTree::kScriptingInstanceOffset) == 0
        ? static_cast<int>(offset & TypeTree::kByteOffsetMask) : -1;
}

static void AssertOptimizeTransferImpl(const TypeTreeIterator& type, int baseOffset, int* totalSize)
{
    Assert(GetObjectOffset(type) == -1 || GetObjectOffset(type) - baseOffset == *totalSize);
    Assert((type->m_MetaFlag & kAlignBytesFlag) == 0);

    if (type.IsBasicDataType())
    {
        *totalSize += type->m_ByteSize;
        return;
    }

    for (TypeTreeIterator i = type.Children(); !i.IsNull(); i = i.Next())
        AssertOptimizeTransferImpl(i, baseOffset, totalSize);
}

GenerateTypeTreeTransfer::GenerateTypeTreeTransfer(TypeTree& t, TransferInstructionFlags options, void* objectPtr, int objectSize)
    : m_TypeTree(t)
{
    m_Flags = options;
    m_TypeTree.GetData()->SetGenerationFlags(m_Flags);
    m_Index = 0;
    m_SimulatedByteOffset = 0;
    m_RequireTypelessData = false;
    m_DidErrorAlignment = false;

    m_ObjectPtr = reinterpret_cast<char*>(objectPtr);
    m_ObjectSize = objectSize;

    m_ScriptingObjectPtr = NULL;
    m_ScriptingObjectSize = 0;
}

//void GenerateTypeTreeTransfer::SetScriptingObject(ScriptingObjectPtr scriptingObjectPtr, int scriptingObjectSize)
//{
//    Assert(m_ScriptingObjectPtr == NULL && m_ScriptingObjectSize == 0);
//    Assert(m_ActiveFather.IsNull());
//    m_ScriptingObjectPtr = UnsafeExtractPointerFromScriptingObjectPtr(scriptingObjectPtr);
//    m_ScriptingObjectSize = scriptingObjectSize;
//}

void GenerateTypeTreeTransfer::BeginArrayTransfer(const char* name, const char* typeString, SInt32& size, TransferMetaFlags metaFlag)
{
    Assert(!m_ActiveFather->IsArray());
    BeginTransfer(name, typeString, NULL, metaFlag);
    m_ActiveFather.GetWritableNode(m_TypeTree)->AddTypeFlags(TypeTreeNode::kFlagIsArray);

    // transfer size
    Transfer(size, "size");
}

void GenerateTypeTreeTransfer::BeginTransfer(const char* name, const char* typeString, char* dataPtr, TransferMetaFlags metaFlag)
{
    #if !UNITY_RELEASE
    if (m_RequireTypelessData)
    {
        AssertString("TransferTypeless needs to be followed by TransferTypelessData with no other variables in between!");
    }
    #endif

    #if DEBUGMODE
    // skip this check for debug mode inspector, as we can have interface names from C# in the debug data.
    if (!(metaFlag & kDebugPropertyMask) && (strstr(name, ".") != NULL || strstr(name, "Array[") != NULL))
    {
        core::string s = "Illegal serialize property name :";
        TypeTreeQueries::GetTypePath(m_ActiveFather, s);
        s += name;
        s += "\n The name may not contain '.' or Array[";
        ErrorString(s);
    }
    #endif

    TypeTreeIterator typeItr;
    TypeTreeNode* typeTreeNode;

    // Setup a normal typetree child
    if (!m_ActiveFather.IsNull())
    {
        // Check for multiple occurences of same name
        #if DEBUGMODE
        for (TypeTreeIterator i = m_ActiveFather.Children(); !i.IsNull(); i = i.Next())
        {
            if (i.Name() == name)
            {
                core::string s = "The same field name is serialized multiple times in the class or its parent class. This is not supported: ";
                TypeTreeQueries::GetTypePath(m_ActiveFather, s);
                s += name;
                ErrorString(s);
            }
        }
        #endif

        typeItr = m_ActiveFather.AddChildNode(m_TypeTree);
        typeTreeNode = typeItr.GetWritableNode(m_TypeTree);
        typeTreeNode->m_MetaFlag = metaFlag | m_ActiveFather->m_MetaFlag;
        Assert((typeTreeNode->m_MetaFlag & kAlignBytesFlag) == 0);
        typeTreeNode->m_MetaFlag &= ~(kAnyChildUsesAlignBytesFlag);
    }
    // Setup root TypeTree
    else
    {
        typeItr = m_TypeTree.Root();
        typeTreeNode = typeItr.GetWritableNode(m_TypeTree);
        typeTreeNode->m_MetaFlag = metaFlag;
    }

    m_TypeTree.AssignTypeString(typeItr, typeString);
    m_TypeTree.AssignNameString(typeItr, name);
    typeTreeNode->m_ByteSize = 0;

    // Calculate typetree index
    typeTreeNode->m_Index = (typeItr->m_MetaFlag & kDebugPropertyMask) == 0
        || (m_Flags & kIgnoreDebugPropertiesForIndex) == 0
        ? m_Index++ : -1;

    if (dataPtr != NULL)
    {
        if (m_ObjectPtr != NULL)
        {
            int offset = dataPtr - m_ObjectPtr;
            if (offset >= 0 && offset < m_ObjectSize)
                m_TypeTree.AssignByteOffset(typeItr, static_cast<UInt32>(offset));
            else
            {
                offset = dataPtr - m_ScriptingObjectPtr;
                if (offset >= 0 && offset < m_ScriptingObjectSize)
                    m_TypeTree.AssignByteOffset(typeItr, offset | TypeTree::kScriptingInstanceOffset);
            }
        }
        else if (m_ScriptingObjectPtr != NULL)
        {
            int offset = dataPtr - m_ScriptingObjectPtr;
            if (offset >= 0 && offset < m_ScriptingObjectSize)
                m_TypeTree.AssignByteOffset(typeItr, offset | TypeTree::kScriptingInstanceOffset);
        }
    }

    m_ActiveFather = typeItr;
}

void GenerateTypeTreeTransfer::AssertContainsNoPPtr(const TypeTreeIterator& typeItr)
{
    AssertFormatMsg(std::strncmp(typeItr.Type().c_str(), "PPtr<", sizeof("PPtr<") - 1) != 0, "TypeTree contains a unexpected PPtr member (%s %s) - did you declare with DECLARE_SERIALIZE_NO_PPTR by mistake?", typeItr.Type().c_str(), typeItr.Name().c_str());
    for (TypeTreeIterator i = typeItr.Children(); !i.IsNull(); i = i.Next())
        AssertContainsNoPPtr(i);
}

void GenerateTypeTreeTransfer::AssertOptimizeTransfer(int sizeofSize)
{
    if (m_ActiveFather.IsBasicDataType())
    {
        Assert(sizeofSize == m_ActiveFather->m_ByteSize);
        return;
    }

    int size = 0;
    int baseOffset = GetObjectOffset(m_ActiveFather);
    for (TypeTreeIterator i = m_ActiveFather.Children(); !i.IsNull(); i = i.Next())
        AssertOptimizeTransferImpl(i, baseOffset, &size);

    // Assert if serialized size is different from sizeof size.
    // - Ignore when serializing for game release. We might be serializing differently in that case. (AnimationCurves)
    Assert(sizeofSize == size || (m_Flags & kSerializeGameRelease) != 0);
}

void GenerateTypeTreeTransfer::EndTransfer()
{
    TypeTreeIterator current = m_ActiveFather;
    // Add bytesize to parent!
    m_ActiveFather = m_ActiveFather.Father();
    if (!m_ActiveFather.IsNull())
    {
        m_ActiveFather.GetWritableNode(m_TypeTree)->m_ByteSize =
            current->m_ByteSize != -1 && m_ActiveFather->m_ByteSize != -1
            ? m_ActiveFather->m_ByteSize + current->m_ByteSize : -1;

        // Propagate if any child uses alignment up to parents
        if (current->m_MetaFlag & kAnyChildUsesAlignBytesFlag)
        {
            m_ActiveFather.GetWritableNode(m_TypeTree)->m_MetaFlag |= kAnyChildUsesAlignBytesFlag;
        }

        DebugAssert(m_ActiveFather->m_ByteSize != 0 || m_ActiveFather.Type() != CommonString(Generic_Mono));
    }
}

void GenerateTypeTreeTransfer::EndArrayTransfer()
{
    m_ActiveFather.GetWritableNode(m_TypeTree)->m_ByteSize = -1;
    EndTransfer();
}

void GenerateTypeTreeTransfer::SetVersion(int version)
{
    // You can not set the version twice on the same type.
    // Probably an inherited class already calls SetVersion
    Assert(m_ActiveFather->m_Version == 1);
    Assert(version >= std::numeric_limits<SInt16>::min() && version <= std::numeric_limits<SInt16>::max());

    m_ActiveFather.GetWritableNode(m_TypeTree)->m_Version = static_cast<SInt16>(version);
}

void GenerateTypeTreeTransfer::TransferTypeless(UInt32* byteSize, const char* name, TransferMetaFlags metaFlag)
{
    SInt32 size;
    BeginArrayTransfer(name, "TypelessData", size, metaFlag);

    UInt8 temp;
    Transfer(temp, "data", metaFlag);

    m_RequireTypelessData = true;

    EndArrayTransfer();

    Align();
}

void GenerateTypeTreeTransfer::TransferTypelessData(UInt32 byteSize, void* copyData, int metaData)
{
    m_RequireTypelessData = false;
}

void GenerateTypeTreeTransfer::Align()
{
    m_SimulatedByteOffset = Align4(m_SimulatedByteOffset);

    if (!m_ActiveFather.IsNull() && !m_ActiveFather.Children().IsNull())
    {
        m_ActiveFather.Children().Last().GetWritableNode(m_TypeTree)->m_MetaFlag |= kAlignBytesFlag;
        m_ActiveFather.GetWritableNode(m_TypeTree)->m_MetaFlag |= kAnyChildUsesAlignBytesFlag;
    }
    else
    {
        AssertString("Trying to align type data before anything has been serialized!");
    }
}

void GenerateTypeTreeTransfer::TransferResourceImage(ActiveResourceImage targetResourceImage, const char* name, StreamingInfo& streamingInfo, void* buffer, UInt32 byteSize, InstanceID instanceID, const HuaHuo::Type* type)
{
    Transfer(streamingInfo, name);
}

#if UNITY_EDITOR
void GenerateTypeTreeTransfer::LogUnalignedTransfer()
{
    if (m_DidErrorAlignment)
        return;

    // For now we only support 4 byte alignment
    int size = m_ActiveFather->m_ByteSize;
    if (size == 8)
        size = 4;
    if (m_SimulatedByteOffset % size == 0)
        return;

    m_DidErrorAlignment = true;

    core::string path;
    TypeTreeQueries::GetTypePath(TypeTreeIterator(m_ActiveFather), path);
    LogString(Format("Unaligned transfer in '%s' at variable '%s'.\nNext unaligned data path: %s", m_TypeTree.Root().Type().c_str(), m_ActiveFather.Name().c_str(), path.c_str()));
}

#endif
