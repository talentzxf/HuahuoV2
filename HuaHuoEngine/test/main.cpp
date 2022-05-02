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

    go->QueryComponent<Transform>()->SetParent(go2->QueryComponent<Transform>());

    go2->QueryComponent<Transform>()->GetChildrenCount();

    return 0;
}
