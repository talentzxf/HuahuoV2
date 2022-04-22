//
// Created by VincentZhang on 4/21/2022.
//

#ifndef PERSISTENTMANAGER_STREAMEDBINARYREAD_H
#define PERSISTENTMANAGER_STREAMEDBINARYREAD_H

#include "TransferBase.h"
#include "Serialize/SerializationMetaFlags.h"
#include "Serialize/SerializeTraits.h"

class StreamedBinaryRead : public TransferBase {
public:
    template<class T>
    void TransferBase(T& data, TransferMetaFlags metaFlag = kNoTransferFlags);
    template<class T>
    void Transfer(T& data, const char* name, TransferMetaFlags metaFlag = kNoTransferFlags);
};

template<class T>
void StreamedBinaryRead::TransferBase(T& data, TransferMetaFlags metaFlag)
{
    Transfer(data, kTransferNameIdentifierBase, metaFlag);
}

template<class T>
void StreamedBinaryRead::Transfer(T& data, const char*, TransferMetaFlags metaFlag)
{
    SerializeTraits<T>::Transfer(data, *this);
}

#endif //PERSISTENTMANAGER_STREAMEDBINARYREAD_H
