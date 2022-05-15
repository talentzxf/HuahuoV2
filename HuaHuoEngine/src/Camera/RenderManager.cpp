//
// Created by VincentZhang on 5/14/2022.
//

#include "RenderManager.h"

Camera& RenderManager::GetCurrentCamera()           { /*Assert(!m_CurrentCamera.IsNull());*/ return *m_CurrentCamera; }
Camera* RenderManager::GetCurrentCameraPtr()        { return m_CurrentCamera; }
// PPtr<Camera> RenderManager::GetCurrentCameraPPtr()  { return m_CurrentCamera; }

RenderManager::RenderManager()
        :   m_CurrentCameraStackState(NULL)
//        ,   m_CamerasToAdd(kMemManager)
//        ,   m_CamerasToRemove(kMemManager)
//        ,   m_InsideRenderOrCull(false)
#if UNITY_EDITOR
,   m_UseScriptableRenderPipeline(true)
#endif
{
    m_CurrentCamera = NULL;
}

RenderManager::~RenderManager()
{
//    m_OnRenderObjectCallbacks.clear();
//    Assert(m_Cameras.GetSize() == 0);
}

// void RenderManager::SetCurrentCameraAndStackState(PPtr<Camera> cam, CameraStackRenderingState* state)
void RenderManager::SetCurrentCameraAndStackState(Camera* cam, CameraStackRenderingState* state)
{
    m_CurrentCamera = cam;
    m_CurrentCameraStackState = state;
}

void RenderManager::UpdateAllRenderers()
{
//    if (GetIParticleSystem())
//        GetIParticleSystem()->SyncSimulationJobs();
//
//    GetRendererUpdateManager().OncePerFrameUpdate(GetRendererScene());
//    GetRendererUpdateManager().UpdateAll(GetRendererScene());
//    GetLightManager().UpdateAllLightTransformData();
//#if GFX_SUPPORTS_RAY_TRACING
//    GetRayTracingAccelerationStructureManager().Update();
//#endif
}

static RenderManager* gRenderManager = NULL;

void RenderManager::InitializeClass()
{
    Assert(gRenderManager == NULL);
    // gRenderManager = UNITY_NEW(RenderManager, kMemRenderer);
    gRenderManager = NEW(RenderManager);

//    InitializeHaloManager();
//    InitializeFlareManager();

#if UNITY_EDITOR
    GfxDevice::CleanupGfxDeviceResourcesCallbacks.Register(&RemoveAllIntermediateRenderers);
    GfxDevice::InitializeGfxDeviceResourcesCallbacks.Register(&RemoveAllIntermediateRenderers);
    GlobalCallbacks::Get().suspendPointHook.Register(&SuspendPointHook);
    GlobalCallbacks::Get().didReloadMonoDomain.Register(&DidReloadMonoDomain);
#endif
}

void RenderManager::CleanupClass()
{
#if UNITY_EDITOR
    GfxDevice::CleanupGfxDeviceResourcesCallbacks.Unregister(&RemoveAllIntermediateRenderers);
    GfxDevice::InitializeGfxDeviceResourcesCallbacks.Unregister(&RemoveAllIntermediateRenderers);
    GlobalCallbacks::Get().suspendPointHook.Unregister(&SuspendPointHook);
    GlobalCallbacks::Get().didReloadMonoDomain.Unregister(&DidReloadMonoDomain);
#endif

//    CleanupFlareManager();
//    CleanupHaloManager();

    Assert(gRenderManager != NULL);
    // UNITY_DELETE(gRenderManager, kMemRenderer);
    DELETE(gRenderManager);
    gRenderManager = NULL;
}

RenderManager& GetRenderManager()
{
    return *gRenderManager;
}

RenderManager* GetRenderManagerPtr()
{
    return gRenderManager;
}

void RenderManager::AddCamera(Camera *c)
{
    Assert(c != NULL);

//    PPtr<Camera> cam(c);
//    if (m_InsideRenderOrCull)
//    {
//        erase_swap_back(m_CamerasToRemove, cam);
//        m_CamerasToAdd.push_back(cam);
//        return;
//    }
//
//    erase_swap_back(m_CamerasToAdd, cam);
//    erase_swap_back(m_CamerasToRemove, cam);
//
//    m_Cameras.Remove(cam);
//    m_OffScreenCameras.Remove(cam);
//
//    SortedCameraContainer& queue = (c->GetTargetTexture() == NULL) ? m_Cameras : m_OffScreenCameras;
//    queue.Add(c);
}

void RenderManager::RemoveCamera(Camera *c)
{
//    PPtr<Camera> cam(c);

//    erase_swap_back(m_CamerasToAdd, cam);
//    erase_swap_back(m_CamerasToRemove, cam);
//
//    if (m_InsideRenderOrCull)
//    {
//        m_CamerasToRemove.push_back(cam);
//    }
//    else
//    {
//        m_Cameras.Remove(cam);
//        m_OffScreenCameras.Remove(cam);
//    }
//
//    Camera* currentCamera = m_CurrentCamera;
//    if (currentCamera == c && !m_InsideRenderOrCull)
//    {
//        if (m_Cameras.GetSize() == 0)
//            m_CurrentCamera = NULL;
//        else
//            m_CurrentCamera = m_Cameras.Get().front();  // ??? maybe better choose next
//    }
}
