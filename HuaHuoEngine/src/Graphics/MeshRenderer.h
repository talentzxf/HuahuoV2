//
// Created by VincentZhang on 5/13/2022.
//

#ifndef HUAHUOENGINE_MESHRENDERER_H
#define HUAHUOENGINE_MESHRENDERER_H
#include "Graphics/Renderer.h"
#include "Graphics/Mesh/Mesh.h"
#include "Utilities/LinkedList.h"
#include "BaseClasses/PPtr.h"

class MeshRenderer : public Renderer{
    REGISTER_CLASS(MeshRenderer);
    DECLARE_OBJECT_SERIALIZE();
public:
    MeshRenderer(MemLabelId label, ObjectCreationMode mode);
    // ~MeshRenderer ();     declared-by-macro

    static void InitializeClass();

    // virtual void MainThreadCleanup() override;
    void CalculateLocalAABB(AABB& outAABB, const MeshRenderer& renderer, const Mesh& mesh);
private:
    void UpdateLocalAABB();
    void UpdateCachedMesh();

private:
    Mesh* m_CachedMesh;

    ListNode<Object> m_AdditionalVertexStreamsNode;
    ListNode<Object> m_EnlightenVertexStreamNode;

    PPtr<Mesh> m_Mesh;
    PPtr<Mesh> m_AdditionalVertexStreams;
    PPtr<Mesh> m_EnlightenVertexStream; // Player Only serialized
};


#endif //HUAHUOENGINE_MESHRENDERER_H
