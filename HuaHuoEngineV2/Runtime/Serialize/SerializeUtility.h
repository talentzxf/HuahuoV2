//
// Created by VincentZhang on 4/23/2022.
//

#ifndef HUAHUOENGINE_SERIALIZEUTILITY_H
#define HUAHUOENGINE_SERIALIZEUTILITY_H

#include "SerializationMetaFlags.h"
#include "Utilities/EnumTraits.h"
#include "Utilities/StaticAssert.h"

#define TRANSFER_WITH_FLAGS(x, metaFlag) transfer.Transfer (x, #x, metaFlag)
#define TRANSFER_WITH_NAME(x, name) transfer.Transfer (x, name)
#define TRANSFER(x) TRANSFER_WITH_NAME(x, #x)

#define DECLARE_SERIALIZE(x) \
    inline static const char* GetTypeString ()  { return #x; }  \
    inline static bool MightContainPPtr ()  { return true; }\
    inline static bool AllowTransferOptimization () { return false; }\
    template<class TransferFunction> \
    void Transfer (TransferFunction& transfer);

template<class T>
inline bool SerializePrefabIgnoreProperties(T& transfer)
{
    return (transfer.GetFlags() & kSerializeForPrefabSystem) == 0;
}

#define DECLARE_SERIALIZE_NO_PPTR(x) \
    inline static const char* GetTypeString ()  { return #x; }  \
    inline static bool MightContainPPtr ()  { return false; }\
    inline static bool AllowTransferOptimization () { return false; }\
    template<class TransferFunction> \
    void Transfer (TransferFunction& transfer);

#define DECLARE_SERIALIZE_OPTIMIZE_TRANSFER(x) \
    inline static const char* GetTypeString ()  { return #x; }  \
    inline static bool MightContainPPtr ()  { return false; }\
    inline static bool AllowTransferOptimization () { return true; }\
    template<class TransferFunction> \
    void Transfer (TransferFunction& transfer);

// This template ensures enums are serialized as ints.
// Some compilers (ARMCC) defaults to have 'packed' enums, which causes a mismatch between runtime type size and the serialized size.
template<class T, typename E>
inline void TransferEnumWithNameForceIntSize(T& transfer, E& e, const char* name, TransferMetaFlags metaFlags = kNoTransferFlags)
{
    CompileTimeAssert(sizeof(E) <= sizeof(int), "Enum size must be 4 bytes or less!");
    int value = EnumTraits::ToInt<E>(e);
    transfer.Transfer(value, name, metaFlags);
    e = EnumTraits::FromIntUnchecked<E>(value);
}

#define TRANSFER_ENUM_WITH_NAME_AND_FLAGS(x, name, metaFlags){ TransferEnumWithNameForceIntSize(transfer, x, name, metaFlags); }
#define TRANSFER_ENUM_WITH_NAME(x, name) { TransferEnumWithNameForceIntSize(transfer, x, name); }
#define TRANSFER_ENUM_WITH_FLAGS(x, metaFlags) TRANSFER_ENUM_WITH_NAME_AND_FLAGS(x, #x, metaFlags)
#define TRANSFER_ENUM(x) TRANSFER_ENUM_WITH_NAME(x, #x)

#endif //HUAHUOENGINE_SERIALIZEUTILITY_H
