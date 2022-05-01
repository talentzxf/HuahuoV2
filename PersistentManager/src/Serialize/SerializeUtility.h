//
// Created by VincentZhang on 4/23/2022.
//

#ifndef PERSISTENTMANAGER_SERIALIZEUTILITY_H
#define PERSISTENTMANAGER_SERIALIZEUTILITY_H

#include "SerializationMetaFlags.h"

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

#endif //PERSISTENTMANAGER_SERIALIZEUTILITY_H
