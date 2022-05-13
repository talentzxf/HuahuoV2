//
// Created by VincentZhang on 5/13/2022.
//

#include "RendererScene.h"

static RendererScene gScene;

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

RendererScene& GetRendererScene()
{
    return gScene;
}

