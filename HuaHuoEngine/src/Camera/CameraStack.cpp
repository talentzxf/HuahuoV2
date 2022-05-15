//
// Created by VincentZhang on 5/14/2022.
//

#include "CameraStack.h"
#include "RenderManager.h"

// -------------------------------------------------------------------------------------------
// AutoScopedCameraStackRenderingState


AutoScopedCameraStackRenderingState::AutoScopedCameraStackRenderingState(Camera& camera)
{
    m_PrevCamera = GetRenderManager().GetCurrentCameraPtr();
    m_PrevCameraStackState = GetRenderManager().GetCurrentCameraStackStatePtr();
    GetRenderManager().SetCurrentCameraAndStackState(&camera, &m_StackState);
    m_StackState.BeginRenderingOneCamera(camera);
}

AutoScopedCameraStackRenderingState::~AutoScopedCameraStackRenderingState()
{
    m_StackState.ReleaseResources();
    GetRenderManager().SetCurrentCameraAndStackState(m_PrevCamera, m_PrevCameraStackState);
}
