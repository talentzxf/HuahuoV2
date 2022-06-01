//
// Created by VincentZhang on 4/21/2022.
//

#ifndef HUAHUOENGINE_STREAMEDBINARYREAD_H
#define HUAHUOENGINE_STREAMEDBINARYREAD_H

#include "TransferBase.h"
#include "Serialize/SerializationMetaFlags.h"
#include "Serialize/SerializeTraits.h"
#include "Serialize/SerializationCaching/CachedReader.h"

class StreamedBinaryRead : public TransferBase {
private:
    CachedReader    m_Cache;

public:
    CachedReader& Init(TransferInstructionFlags flags)  { m_UserData = NULL; m_Flags = flags; return m_Cache; }

    bool IsReading()                  { return true; }
    bool IsReadingPPtr()              { return true; }

    template<class T>
    void TransferBase(T& data, TransferMetaFlags metaFlag = kNoTransferFlags);
    template<class T>
    void Transfer(T& data, const char* name, TransferMetaFlags metaFlag = kNoTransferFlags);

    template<class T>
    void TransferBasicData(T& data);

    template<class T>
    void TransferSTLStyleArray(T& data, TransferMetaFlags metaFlag = kNoTransferFlags);

    void EXPORT_COREMODULE Align();
    /// Reads byteSize bytes into data. This may onle be used if UseOptimizedReading returns true.
    void EXPORT_COREMODULE ReadDirect(void* data, int byteSize);
};


template<class T>
void StreamedBinaryRead::TransferSTLStyleArray(T& data, TransferMetaFlags /*metaFlags*/)
{
    SInt32 size;
    Transfer(size, "size");

    SerializeTraits<T>::ResizeSTLStyleArray(data, size);

    if (SerializeTraits<typename T::value_type>::AllowTransferOptimization() && SerializeTraits<T>::IsContinousMemoryArray())
    {
        //Assert (size == distance (data.begin (), data.end ()));
        if (size != 0)
            ReadDirect(&*data.begin(), size * sizeof(typename T::value_type));
    }
    else
    {
        typename T::iterator i;
        typename T::iterator end = data.end();
        //Assert (size == distance (data.begin (), end));
        for (i = data.begin(); i != end; ++i)
            Transfer(*i, "data");
    }
}

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

#endif //HUAHUOENGINE_STREAMEDBINARYREAD_H
