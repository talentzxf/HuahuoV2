//
// Created by VincentZhang on 4/1/2022.
//

#include "PersistentManager.h"
#include "PersistentManagerConfig.h"
#include "Components/Transform.h"

#include <cstdio>

int main(){
    PersistentManager::InitEngine();

    PersistentManager* pManager = PersistentManager::getInstance();
    pManager->getBuffer();

    printf("Version: %d.%d\n", PM_VERSION_MAJOR, PM_VERSION_MINOR);

    Transform* transform = Transform::Produce();
    if(transform == NULL){
        printf("ERROR\n");
    }
    printf("%s\n", transform->GetPPtrTypeString());

    printf("%d", TypeOf<Transform>()->IsDerivedFrom<BaseComponent>());


    return 0;
}
