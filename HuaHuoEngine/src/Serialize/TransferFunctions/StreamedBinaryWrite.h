//
// Created by VincentZhang on 4/21/2022.
//

#ifndef HUAHUOENGINE_STREAMEDBINARYWRITE_H
#define HUAHUOENGINE_STREAMEDBINARYWRITE_H

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
    void TransferSTLStyleArray(T& data, TransferMetaFlags metaFlag = kNoTransferFlags);

    template<class T>
    void TransferBasicData(T& data);

    bool IsWriting()                 { return true; }
    bool IsWritingPPtr()             { return true; }

    void Align();
};

template<class T> inline
void StreamedBinaryWrite::TransferSTLStyleArray(T& data, TransferMetaFlags /*metaFlags*/)
{
    const T& cdata = (const T&)data;

    SInt32 size = (SInt32)cdata.size();
    Transfer(size, "size");

    typedef typename T::value_type non_const_value_type;

    typename T::const_iterator end = cdata.end();
    for (typename T::const_iterator i = cdata.begin(); i != end; ++i)
    {
        non_const_value_type& p = (non_const_value_type&)(*i);
        Transfer(p, "data");
    }
}

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

#endif //HUAHUOENGINE_STREAMEDBINARYWRITE_H
