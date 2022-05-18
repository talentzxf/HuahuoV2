//
// Created by VincentZhang on 5/13/2022.
//

#ifndef HUAHUOENGINE_RENDERSCENE_H
#define HUAHUOENGINE_RENDERSCENE_H

#include <vector>
#include "SceneNode.h"

class Renderer;
class RendererScene {
public:
    RendererScene();
    ~RendererScene();

    void BeginCameraRender();
    void EndCameraRender();

    // Adds/removes from the scene
    SceneHandle AddRenderer(Renderer *renderer);
    Renderer* RemoveRenderer(SceneHandle handle);
    void RemoveRendererFromPendingNodes(Renderer* renderer);

    // Const access only to SceneNode and AABB!
    // RendererScene needs to be notified about changes and some data is private
    const SceneNode&    GetRendererNode(SceneHandle handle)                { return m_RendererNodes[handle]; }

    int                 GetNumRenderers() { return m_RendererNodes.size(); }

private:
    SceneHandle AddRendererInternal(Renderer *renderer, int layer);
private:
    // These arrays are always kept in sync
    std::vector<SceneNode>    m_RendererNodes;
};

RendererScene& GetRendererScene();

#endif //HUAHUOENGINE_RENDERSCENE_H
