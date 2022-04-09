//
// Created by VincentZhang on 4/1/2022.
//

#include "PersistentManager.h"
#include "PersistentManagerConfig.h"
#include "Transform.h"

#include <cstdio>

int main(){
    PersistentManager* pManager = PersistentManager::getInstance();
    pManager->getBuffer();

    printf("Version: %d.%d", PM_VERSION_MAJOR, PM_VERSION_MINOR);

    Transform* transform = new Transform();
    // printf("%s", transform->mRTTI.getName().c_str());

    return 0;
}
