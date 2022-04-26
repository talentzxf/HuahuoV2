//
// Created by VincentZhang on 4/21/2022.
//

#ifndef PERSISTENTMANAGER_STREAMEDBINARYWRITE_H
#define PERSISTENTMANAGER_STREAMEDBINARYWRITE_H

#include "TransferBase.h"
#include "Serialize/SerializationMetaFlags.h"
#include "Serialize/SerializeTraits.h"
#include "Serialize/SerializationCaching/CachedWriter.h"

class StreamedBinaryWrite : public TransferBase{
private:
    CachedWriter m_Cache;
public:
    StreamedBinaryWrite() {}

    CachedWriter& Init(TransferInstructionFlags flags); //, BuildTargetSelection target, void * manageReferenceToReuse = nullptr);
    CachedWriter& Init(const CachedWriter& cachedWriter, TransferInstructionFlags flags);//, BuildTargetSelection target, const BuildUsageTag& buildUsageTag, const GlobalBuildData& globalBuildData);

    template<class T>
    void TransferBase(T& data, TransferMetaFlags metaFlag = kNoTransferFlags);

    template<class T>
    void Transfer(T& data, const char* name, TransferMetaFlags metaFlag = kNoTransferFlags);

    template<class T>
    void TransferWithTypeString(T& data, const char* name, const char* typeName, TransferMetaFlags metaFlag = kNoTransferFlags);

    template<class T>
    void TransferBasicData(T& data);
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

template<class T> inline
void StreamedBinaryWrite::TransferBasicData(T& data)
{
    m_Cache.Write(data);
}

#endif //PERSISTENTMANAGER_STREAMEDBINARYWRITE_H
