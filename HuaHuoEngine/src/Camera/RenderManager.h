//
// Created by VincentZhang on 5/14/2022.
//

#ifndef HUAHUOENGINE_RENDERMANAGER_H
#define HUAHUOENGINE_RENDERMANAGER_H
#include "BaseClasses/PPtr.h"

class CameraStackRenderingState;
class Camera;
class RenderManager {
public:
    RenderManager();
    ~RenderManager();

public:
    Camera &GetCurrentCamera();
    Camera* GetCurrentCameraPtr();
    PPtr<Camera> GetCurrentCameraPPtr();

    static void UpdateAllRenderers();

    static void InitializeClass();
    static void CleanupClass();

    CameraStackRenderingState& GetCurrentCameraStackState() { AssertMsg(m_CurrentCameraStackState != NULL, "GetCurrentCameraStackState called outside of camera stack rendering"); return *m_CurrentCameraStackState; }
    CameraStackRenderingState* GetCurrentCameraStackStatePtr() { return m_CurrentCameraStackState; }
    // void SetCurrentCameraAndStackState(PPtr<Camera> cam, CameraStackRenderingState* state);
    void SetCurrentCameraAndStackState(Camera* cam, CameraStackRenderingState* state);
private:
    Camera* m_CurrentCamera;
    CameraStackRenderingState* m_CurrentCameraStackState;
};

RenderManager& GetRenderManager();
RenderManager* GetRenderManagerPtr();

inline Camera &GetCurrentCamera() { return GetRenderManager().GetCurrentCamera(); }
inline Camera* GetCurrentCameraPtr() { return GetRenderManager().GetCurrentCameraPtr(); }
// inline PPtr<Camera> GetCurrentCameraPPtr() { return GetRenderManager().GetCurrentCameraPPtr(); }

#endif //HUAHUOENGINE_RENDERMANAGER_H
