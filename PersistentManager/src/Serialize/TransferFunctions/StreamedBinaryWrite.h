//
// Created by VincentZhang on 4/21/2022.
//

#ifndef PERSISTENTMANAGER_STREAMEDBINARYWRITE_H
#define PERSISTENTMANAGER_STREAMEDBINARYWRITE_H

#include "TransferBase.h"
#include "Serialize/SerializationMetaFlags.h"
#include "Serialize/SerializeTraits.h"

class StreamedBinaryWrite : public TransferBase{
public:
    template<class T>
    void TransferBase(T& data, TransferMetaFlags metaFlag = kNoTransferFlags);

    template<class T>
    void Transfer(T& data, const char* name, TransferMetaFlags metaFlag = kNoTransferFlags);

    template<class T>
    void TransferWithTypeString(T& data, const char* name, const char* typeName, TransferMetaFlags metaFlag = kNoTransferFlags);
};

template<class T>
void StreamedBinaryWrite::TransferBase(T& data, TransferMetaFlags metaFlag)
{
    Transfer(data, kTransferNameIdentifierBase, metaFlag);
}

template<class T> inline
void StreamedBinaryWrite::Transfer(T& data, const char* name, TransferMetaFlags metaFlag)
{
    SerializeTraits<T>::Transfer(data, *this);
}

#endif //PERSISTENTMANAGER_STREAMEDBINARYWRITE_H
