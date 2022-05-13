//
// Created by VincentZhang on 5/12/2022.
//

#ifndef HUAHUOENGINE_SHAREDMESHDATA_H
#define HUAHUOENGINE_SHAREDMESHDATA_H
#include <cstdlib>
#include "VertexData.h"

class SharedMeshData {
public:
    explicit SharedMeshData();
    SharedMeshData(const SharedMeshData& src);
    ~SharedMeshData();

    size_t                  GetVertexCount() const         { return m_VertexData.GetVertexCount(); }
    const VertexData&       GetVertexData() const          { return m_VertexData; }
    VertexData&             GetVertexData()                { return m_VertexData; }

private:
    VertexData m_VertexData;
};


#endif //HUAHUOENGINE_SHAREDMESHDATA_H
