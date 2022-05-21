//
// Created by VincentZhang on 5/12/2022.
//

#ifndef HUAHUOENGINE_SHAREDMESHDATA_H
#define HUAHUOENGINE_SHAREDMESHDATA_H
#include <cstdlib>
#include "VertexData.h"
#include "Math/Matrix4x4.h"
#include "SubMesh.h"
#include "MeshTypes.h"
#include <vector>

class SharedMeshData {
public:
    explicit SharedMeshData();
    SharedMeshData(const SharedMeshData& src);
    ~SharedMeshData();

    // Index data is used for dynamic batching
    typedef std::vector<UInt8>                IndexContainer;
    typedef std::vector<SubMesh>              SubMeshContainer;
    typedef std::vector<Matrix4x4f>       MatrixContainer;
    typedef std::vector<int>                  BoneIndicesContainer;
    typedef std::vector<BoneWeights2>     BoneWeights2Container;
    typedef std::vector<BoneWeights4>     BoneWeights4Container;

    size_t                  GetVertexCount() const         { return m_VertexData.GetVertexCount(); }
    const VertexData&       GetVertexData() const          { return m_VertexData; }
    VertexData&             GetVertexData()                { return m_VertexData; }

    const SubMeshContainer& GetSubMeshes() const            { return m_SubMeshes; }
    SubMeshContainer&       GetSubMeshes()                  { return m_SubMeshes; }


    size_t                  GetSubMeshCount() const         { return m_SubMeshes.size(); }
    const SubMesh&          GetSubMeshClamped(size_t index) const { return index < m_SubMeshes.size() ? m_SubMeshes[index] : m_SubMeshes.back(); }

private:
    VertexData m_VertexData;
    SubMeshContainer m_SubMeshes;
};


#endif //HUAHUOENGINE_SHAREDMESHDATA_H
