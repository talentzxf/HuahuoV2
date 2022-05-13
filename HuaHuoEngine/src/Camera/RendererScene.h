//
// Created by VincentZhang on 5/13/2022.
//

#ifndef HUAHUOENGINE_RENDERSCENE_H
#define HUAHUOENGINE_RENDERSCENE_H

#include <vector>
#include "SceneNode.h"

class RendererScene {
public:
    RendererScene();
    ~RendererScene();

    void BeginCameraRender();
    void EndCameraRender();

private:
    // These arrays are always kept in sync
    std::vector<SceneNode>    m_RendererNodes;
};

RendererScene& GetRendererScene();

#endif //HUAHUOENGINE_RENDERSCENE_H
