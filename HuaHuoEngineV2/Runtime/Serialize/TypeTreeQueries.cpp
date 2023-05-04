//
// Created by VincentZhang on 4/29/2023.
//

#include "TypeTreeQueries.h"
#include "Utilities/HashFunctions.h"
#include "Utilities/MdFourGenerator.h"


namespace TypeTreeQueries
{
    bool IsStreamedBinaryCompatible(const TypeTreeIterator& lhs, const TypeTreeIterator& rhs)
    {
        if (lhs->m_ByteSize != rhs->m_ByteSize || lhs->m_Version != rhs->m_Version
            || lhs.Name() != rhs.Name() || lhs.Type() != rhs.Type())
            return false;
        if ((lhs->m_MetaFlag & kAlignBytesFlag) != (rhs->m_MetaFlag & kAlignBytesFlag))
            return false;

        for (TypeTreeIterator i = lhs.Children(), j = rhs.Children();
             !i.IsNull() || !j.IsNull();
             i = i.Next(), j = j.Next())
        {
            if (i.IsNull() || j.IsNull())
                return false;

            if (!IsStreamedBinaryCompatible(i, j))
                return false;
        }

        return true;
    }

    TypeTree::Signature GenerateTypeTreeSignature(TransferInstructionFlags flags, const Object &object)
    {
        TypeTree::Signature key = ::ComputeHash64(&flags, sizeof(flags));

        PersistentTypeID ptid = object.GetType()->GetPersistentTypeID();
        key = ::ComputeHash64(&ptid, sizeof(PersistentTypeID), key);

//    const SerializableManagedRef * managedObj = IManagedObjectHost::GetManagedReference(object);
//    if (managedObj != NULL && managedObj->IsNotNull(&object))
//    {
//        ScriptingClassPtr klass = managedObj->GetClass();
//
//        TypeTree::Signature classHash = GenerateTypeTreeSignature(klass);
//        key = ::ComputeHash64(&classHash, sizeof(UInt64), key);
//    }

        return key;
    }

    int GetTypeChildrenCount(const TypeTreeIterator& type)
    {
        int c = 0;
        for (TypeTreeIterator i = type.Children(); !i.IsNull(); i = i.Next())
            ++c;
        return c;
    }

    void HashTypeTree(MdFourGenerator& gen, const TypeTreeIterator& type)
    {
        gen.Feed(type.Type().c_str(), type.Type().strlen());
        gen.Feed(type.Name().c_str(), type.Name().strlen());
        gen.Feed(type->m_ByteSize);
        gen.Feed(type->m_TypeFlags);
        gen.Feed(type->m_Version);
        gen.Feed(type->m_MetaFlag & kAlignBytesFlag);

        for (TypeTreeIterator i = type.Children(); !i.IsNull(); i = i.Next())
        {
            HashTypeTree(gen, i);
        }
    }

    Hash128 HashTypeTree(const TypeTreeIterator& type)
    {
        MdFourGenerator gen;
        HashTypeTree(gen, type);
        return gen.Finish();
    }

    // Using asm/ns/class seperately because safe binary read may not have access to the ScriptingClassPtr (missing/renamed class)
    // but the hash key must remain unchanged since it's how it will find it from the list of type trees it was provided with by the caller.
    // NOTE: !!! make sure that the className has properly been constructed, in particular for nested classes.
    //           See Runtime\Mono\SerializationBackend_DirectMemoryAccess\SerializationCommands\ManagedObjectReference.h : ScriptingClassConverter::ToFullFQN
    TypeTree::Signature GenerateTypeTreeSignature(const std::string & className, const std::string & ns, const std::string & asmx)
    {
        UInt64 hash = ::ComputeHash64(asmx.c_str(), asmx.length());
        hash = ::ComputeHash64(ns.c_str(), ns.length(), hash);
        hash = ::ComputeHash64(className.c_str(), className.length(), hash);

        return hash;
    }

}
