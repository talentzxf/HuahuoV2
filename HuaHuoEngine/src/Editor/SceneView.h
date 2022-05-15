//
// Created by VincentZhang on 5/13/2022.
//

#ifndef HUAHUOENGINE_SCENEVIEW_H
#define HUAHUOENGINE_SCENEVIEW_H
#include "Camera/Camera.h"
#include <string>
#include <emscripten.h>
#include <emscripten/html5.h>

class SceneView {
private:
    Camera* m_pCamera;
    std::string m_CanvasId;
    bool m_Inited;
    EMSCRIPTEN_WEBGL_CONTEXT_HANDLE m_Context;

    void OnUpdate();
    void OnDraw();
public:
    SceneView():m_Inited(false){
        m_pCamera = Object::Produce<Camera>();
    };
    ~SceneView(){};
    void OnGUI();

    typedef void (SceneView::*GUICallback)();

    void InitWithCanvasId(const char* canvasId);
    static SceneView* GetSceneView();
};

static SceneView* GetSceneView(){
    return SceneView::GetSceneView();
}

#endif //HUAHUOENGINE_SCENEVIEW_H
