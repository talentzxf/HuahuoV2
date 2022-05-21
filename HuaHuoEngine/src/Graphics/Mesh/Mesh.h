//
// Created by VincentZhang on 5/12/2022.
//

#ifndef HUAHUOENGINE_MESH_H
#define HUAHUOENGINE_MESH_H
#include "BaseClasses/NamedObject.h"
#include "Math/Vector3f.h"
#include "MeshTypes.h"
#include "VertexData.h"
#include "SharedMeshData.h"
#include "Geometry/AABB.h"
#include "SubMesh.h"

class EXPORT_COREMODULE Mesh :public NamedObject{
    REGISTER_CLASS(Mesh);
    DECLARE_OBJECT_SERIALIZE()
public:
    void SetVertexData(const void* data, size_t stride, size_t count, size_t destOffset, int streamIndex, MeshUpdateFlags flags = MeshUpdateFlags::kDefault);
    void SetVertices(Vector3f const* data, size_t count, MeshUpdateFlags flags = MeshUpdateFlags::kDefault);

    bool HasVertexData() const { return GetVertexData().GetDataPtr() != NULL; }

    const VertexData&           GetVertexData() const           { return m_SharedData->GetVertexData(); }
    VertexData&                 GetVertexData()                 { return m_SharedData->GetVertexData(); }

    void SetBounds(const AABB& aabb);
    const AABB& GetBounds() const { return m_LocalAABB; }

    void SetBounds(unsigned submesh, const AABB& aabb);
    const AABB& GetBounds(unsigned submesh) const
    {
        DebugAssert(submesh < m_SharedData->GetSubMeshes().size());
        return m_SharedData->GetSubMeshes()[submesh].localAABB;
    }
private:
    SharedMeshData* m_SharedData;
    AABB                m_LocalAABB;
};


#endif //HUAHUOENGINE_MESH_H
