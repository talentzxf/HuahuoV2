//
// Created by VincentZhang on 4/30/2022.
//

#ifndef HUAHUOENGINE_GAMEOBJECTDEFINES_H
#define HUAHUOENGINE_GAMEOBJECTDEFINES_H
enum DeactivateOperation
{
    kNormalDeactivate = 0,
    // Deactivate was called because the component will be destroyed
    kWillDestroySingleComponentDeactivate = 1,
    // Deactivate was called because the entire game object will be destroyed
    kWillDestroyGameObjectDeactivate = 2
};
#endif //HUAHUOENGINE_GAMEOBJECTDEFINES_H
