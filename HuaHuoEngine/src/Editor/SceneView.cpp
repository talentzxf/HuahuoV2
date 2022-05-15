//
// Created by VincentZhang on 5/13/2022.
//

#include "SceneView.h"
SceneView sSceneView;

SceneView* SceneView::GetSceneView(){
    return &sSceneView;
}

void MainSceneViewLoop(){
    GetSceneView()->OnGUI();
}

void SceneView::InitWithCanvasId(const char* canvasId){
    printf("Initiing WebGL context. with canvasId:%s\n", canvasId);
    this->m_CanvasId = canvasId;

    EmscriptenWebGLContextAttributes attrs;
    emscripten_webgl_init_context_attributes(&attrs);
    m_Context = emscripten_webgl_create_context( ("#" + this->m_CanvasId).c_str(), &attrs);

    if(m_Context){
        this->m_Inited = true;
        printf("WebGL context inited!\n");
    }else{
        printf("WebGL context initialize failed!\n");
    }

    emscripten_set_main_loop(MainSceneViewLoop, 0, 0);
}

void SceneView::OnGUI() {
    if(this->m_Inited){
        emscripten_webgl_make_context_current(this->m_Context);
        this->OnUpdate();
        this->OnDraw();
    }
}

void SceneView::OnUpdate() {

}

void SceneView::OnDraw() {
    this->m_pCamera->Render();
}