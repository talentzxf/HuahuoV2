//
// Created by VincentZhang on 4/1/2022.
//

#include "PersistentManager.h"
#include "PersistentManagerConfig.h"

#include <cstdio>

using namespace HuaHuo;
int main(){
    PersistentManager* pManager = PersistentManager::getInstance();
    pManager->getBuffer();

    printf("Version: %d.%d", PM_VERSION_MAJOR, PM_VERSION_MINOR);


    return 0;
}
