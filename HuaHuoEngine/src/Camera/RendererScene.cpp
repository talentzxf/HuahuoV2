//
// Created by VincentZhang on 5/13/2022.
//

#include "RendererScene.h"
#include "Memory/MemoryMacros.h"
#include "Logging/LogAssert.h"
#include "Graphics/Renderer.h"

static RendererScene* gScene = NULL;

RendererScene::RendererScene()
{
    // GlobalCallbacks::Get().afterCullingOutputReady.Register(&RendererScene::SceneAfterCullingOutputReady);

//    m_PreventAddRemoveRenderer = 0;
//    m_RequestStaticPVSRebuild = false;
//    m_GateState = NULL;
//    m_UmbraTome = NULL;
}

RendererScene::~RendererScene()
{
//    Assert(m_RendererNodes.empty());
//
//    ClearIntermediateRenderers();
//    CleanupUmbra();
//
//    GlobalCallbacks::Get().afterCullingOutputReady.Unregister(&RendererScene::SceneAfterCullingOutputReady);
}

void RendererScene::BeginCameraRender()
{
//    ApplyPendingAddRemoveNodes();
//    // Prepare Static SceneNode array
//    if (m_RequestStaticPVSRebuild)
//    {
//        m_RequestStaticPVSRebuild = false;
//        InitializeUmbra();
//    }
}

void RendererScene::EndCameraRender()
{
    // ApplyPendingAddRemoveNodes();
}

SceneHandle RendererScene::AddRenderer(Renderer *renderer)
{
    Assert(renderer);
//    if (m_PreventAddRemoveRenderer != 0)
//    {
//        // The renderer could already be in the list of nodes queued to be added to the scene
//        // A queued node returns kInvalidSceneHande which the Renderer treats as not in scene
//        // which means the Renderer will try to add to the scene again
//        Renderer* pendingAddInfo = NULL;
//        for (size_t i = 0, n = m_PendingAddition.size(); i < n; ++i)
//        {
//            if (m_PendingAddition[i] == renderer)
//            {
//                pendingAddInfo = m_PendingAddition[i];
//                break;
//            }
//        }
//
//        if (!pendingAddInfo)
//        {
//            pendingAddInfo = m_PendingAddition.emplace_back(renderer);
//        }
//
//        return kInvalidSceneHandle;
//    }
//
//    // Flush pending removals if we are adding an object that is scheduled to be removed
//    for (size_t i = 0, n = m_PendingRemoval.size(); i < n; ++i)
//    {
//        if (GetRendererNode(m_PendingRemoval[i]).renderer == renderer)
//        {
//            // Flush the removal first - it is safe to assume the removal happens before the addition of the current renderer.
//            // As RemoveRenderer will remove any pending additions for the removal request. This also guards us from recursion.
//            ApplyPendingAddRemoveNodes();
//            break;
//        }
//    }

#if DEBUGMODE
    DebugAssert(!HasNodeForRenderer(renderer));
#endif
    return AddRendererInternal(renderer, renderer->GetLayer());
}


SceneHandle RendererScene::AddRendererInternal(Renderer *renderer, int layer)
{
    SceneHandle handle = m_RendererNodes.size();
//    Assert((int)m_BoundingBoxes.size() == handle);
//    Assert((int)m_VisibilityBits.size() == handle);

    SceneNode& node = m_RendererNodes.emplace_back();
    node.renderer = renderer;
    node.layer = layer;
#if UNITY_EDITOR
    if (renderer != NULL)
        node.sceneMask = renderer->GetGameObjectPtr()->GetCullSceneMask();
#endif
//    m_BoundingBoxes.emplace_back_uninitialized();
//    m_VisibilityBits.push_back(0);
    return handle;
}

RendererScene& GetRendererScene()
{
    if(gScene == NULL)
    {
        gScene = NEW(RendererScene);
    }
    return *gScene;
}

