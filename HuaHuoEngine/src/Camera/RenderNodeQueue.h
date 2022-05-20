//
// Created by VincentZhang on 5/18/2022.
//

#ifndef HUAHUOENGINE_RENDERNODEQUEUE_H
#define HUAHUOENGINE_RENDERNODEQUEUE_H
#include <vector>
#include "RenderNode.h"
#include "Configuration/IntegerDefinitions.h"
#include "Shaders/SharedMaterialData.h"

class RenderNodeQueue {
public:
    RenderNodeQueue(/*MemLabelRef memLabel*/);
    ~RenderNodeQueue();

    RenderNode& GetNode(UInt32 id)              { return m_Nodes[id]; }
    const RenderNode& GetNode(UInt32 id) const  { return m_Nodes[id]; }

    void   SetRenderNodesCount(UInt32 rendererNodeCount)    { m_RenderNodeCount = rendererNodeCount; }
    UInt32 GetRenderNodesCount() const                      { return m_RenderNodeCount; }

private:
    std::vector<RenderNode>       m_Nodes;
    UInt32                          m_RenderNodeCount;
    UInt32                          m_ProjectorCount;
};

// NOTE: Extracted into global namespace to avoid circular dependency between "RenderNodeQueue.h" and "RenderNode.h"
// TODO: Remove in next PR.
struct MaterialInfo
{
    const SharedMaterialData*   sharedMaterialData;
    int                         customRenderQueue;
};


#endif //HUAHUOENGINE_RENDERNODEQUEUE_H
