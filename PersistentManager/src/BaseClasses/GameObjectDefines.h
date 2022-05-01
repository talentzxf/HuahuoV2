//
// Created by VincentZhang on 4/30/2022.
//

#ifndef PERSISTENTMANAGER_GAMEOBJECTDEFINES_H
#define PERSISTENTMANAGER_GAMEOBJECTDEFINES_H
enum DeactivateOperation
{
    kNormalDeactivate = 0,
    // Deactivate was called because the component will be destroyed
    kWillDestroySingleComponentDeactivate = 1,
    // Deactivate was called because the entire game object will be destroyed
    kWillDestroyGameObjectDeactivate = 2
};
#endif //PERSISTENTMANAGER_GAMEOBJECTDEFINES_H
