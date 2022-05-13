//
// Created by VincentZhang on 5/12/2022.
//

#ifndef HUAHUOENGINE_VERTEXDATA_H
#define HUAHUOENGINE_VERTEXDATA_H

#include "Serialize/SerializeUtility.h"

// Information about all vertex data, but does not own the memory
class VertexDataInfo {
public:
    enum {
        kVertexDataAlign = 32,
        kVertexStreamAlign = 16,
        kVertexDataPadding = 16
    };

    size_t GetDataSize() const { return m_DataSize; }
    UInt32 GetVertexSize() const { return m_VertexSize; }
    UInt32 GetVertexCount() const { return m_VertexCount; }
    UInt8* GetDataPtr() const { return m_Data; }

protected:
    UInt32 m_VertexSize; // must match m_Channels

    UInt32 m_VertexCount;
    size_t m_DataSize;
    UInt8* m_Data;
};

class VertexData : public VertexDataInfo {
public:
    DECLARE_SERIALIZE(VertexData)
};


#endif //HUAHUOENGINE_VERTEXDATA_H
