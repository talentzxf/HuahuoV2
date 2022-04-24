//
// Created by VincentZhang on 4/21/2022.
//

#ifndef PERSISTENTMANAGER_STREAMEDBINARYREAD_H
#define PERSISTENTMANAGER_STREAMEDBINARYREAD_H

#include "TransferBase.h"
#include "Serialize/SerializationMetaFlags.h"
#include "Serialize/SerializeTraits.h"
#include "Serialize/SerializationCaching/CachedReader.h"

class StreamedBinaryRead : public TransferBase {
private:
    CachedReader    m_Cache;

public:
    template<class T>
    void TransferBase(T& data, TransferMetaFlags metaFlag = kNoTransferFlags);
    template<class T>
    void Transfer(T& data, const char* name, TransferMetaFlags metaFlag = kNoTransferFlags);

    template<class T>
    void TransferBasicData(T& data);
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

template<class T> inline
void StreamedBinaryRead::TransferBasicData(T& data)
{
    Assert(sizeof(T) <= 8);
    m_Cache.Read(data);
}

#endif //PERSISTENTMANAGER_STREAMEDBINARYREAD_H
