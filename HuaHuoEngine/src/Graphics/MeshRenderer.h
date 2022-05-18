//
// Created by VincentZhang on 5/13/2022.
//

#ifndef HUAHUOENGINE_MESHRENDERER_H
#define HUAHUOENGINE_MESHRENDERER_H
#include "Graphics/Renderer.h"
#include "Graphics/Mesh/Mesh.h"

class MeshRenderer : public Renderer{
    REGISTER_CLASS(MeshRenderer);
    DECLARE_OBJECT_SERIALIZE();
public:
    MeshRenderer(ObjectCreationMode mode);

    static void InitializeClass();

    // virtual void MainThreadCleanup() override;
private:
    Mesh* m_CachedMesh;
};


#endif //HUAHUOENGINE_MESHRENDERER_H
