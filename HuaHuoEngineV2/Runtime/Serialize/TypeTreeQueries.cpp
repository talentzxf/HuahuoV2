//
// Created by VincentZhang on 4/29/2023.
//

#include "TypeTreeQueries.h"
#include "Utilities/HashFunctions.h"

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