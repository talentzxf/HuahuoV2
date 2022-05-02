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

int main() {
    HuaHuoEngine::InitEngine();

    GameObject* go = MonoCreateGameObject("Go1");
    GameObject* go2 = MonoCreateGameObject("Go2");

    Transform* transform1 = go->QueryComponent<Transform>();
    Transform* transform2 = go2->QueryComponent<Transform>();

    transform1->SetParent(transform2);
    printf("Child cound:%d\n", transform2->GetChildrenCount());
    Assert( &transform2->GetChild(0) == transform1);

    return 0;
}
