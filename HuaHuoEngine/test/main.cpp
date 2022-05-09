//
// Created by VincentZhang on 4/1/2022.
//

#include "HuaHuoEngine.h"
#include "PersistentManagerConfig.h"
#include "Components/Transform/Transform.h"
#include "Serialize/SerializationCaching/BlockMemoryCacheWriter.h"
#include "Serialize/SerializationCaching/MemoryCacherReadBlocks.h"

#include <cstdio>
#include "Export/Scripting/GameObjectExport.h"
#include "Editor/SceneInspector.h"

void testTransform(){
    GameObject* go = MonoCreateGameObject("Go1");
    GameObject* go2 = MonoCreateGameObject("Go2");

    Transform* transform1 = go->QueryComponent<Transform>();
    Transform* transform2 = go2->QueryComponent<Transform>();

    transform1->SetParent(transform2);
    printf("Child cound:%d\n", transform2->GetChildrenCount());
    Assert( &transform2->GetChild(0) == transform1);
}

class TestScriptEventHandler: public ScriptEventHandler{
    void handleEvent(ScriptEventHandlerArgs* args){
        printf("HelloHello");
    }
};

void testScene(){
    HuaHuoScene* pScene = GetSceneManager().CreateScene();
    GetSceneManager().SetActiveScene(pScene);

    std::string goName("Go!!!");
    MonoCreateGameObject(goName.c_str());

    GetScriptEventManager()->RegisterEventHandler("OnHierarchyChangedSetParent", new TestScriptEventHandler());

    for(auto itr = pScene->RootBegin(); itr != pScene->RootEnd(); itr++){
        printf("name: %s", itr->GetData()->GetName());
    }
    Assert(pScene->RootBegin() != pScene->RootEnd());
}

int main() {
    HuaHuoEngine::InitEngine();
    // testTransform();
    testScene();

    return 0;
}
