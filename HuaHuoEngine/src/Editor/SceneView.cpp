//
// Created by VincentZhang on 5/13/2022.
//

#include "SceneView.h"
#include "HuaHuoEngine.h"
#include "Misc/GameObjectUtility.h"

#if !WEB_ENV
typedef void (*loop)();
void emscripten_webgl_init_context_attributes(EmscriptenWebGLContextAttributes*) {}
EMSCRIPTEN_WEBGL_CONTEXT_HANDLE emscripten_webgl_create_context(const char*, EmscriptenWebGLContextAttributes*) { return 1;}
void emscripten_set_main_loop(loop, int, int){}
void emscripten_webgl_make_context_current(EMSCRIPTEN_WEBGL_CONTEXT_HANDLE){}
#endif

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

    OnCreate();

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

void SceneView::OnCreate(){
    GameObject& cameraGO = CreateGameObject("SceneCamera", "Transform", "Camera");
    this->m_pCamera = cameraGO.QueryComponent<Camera>();
}

void SceneView::OnDraw() {
    printf("MainSceneViewLoop\n");
    if(this->m_pCamera){
        this->m_pCamera->Render();
    }else{
        printf("Camera is null!\n");
    }
}