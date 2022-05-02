//
// Created by VincentZhang on 4/1/2022.
//

#ifndef HUAHUOENGINE_HUAHUOENGINE_H
#define HUAHUOENGINE_HUAHUOENGINE_H

#include <cstddef>
#include <vector>
#include "BaseClasses/BaseTypes.h"
#include "TypeSystem/Object.h"
#include "BaseClasses/GameObject.h"
#include "Export/Scripting/GameObjectExport.h"

class HuaHuoEngine {
private:
    HuaHuoEngine();

    static HuaHuoEngine *gInstance;
public:
    static void InitEngine();

    inline static HuaHuoEngine *getInstance() {
        return gInstance;
    }

    GameObject* CreateGameObject(const char* name){
        return MonoCreateGameObject(name);
    }
};


#endif //HUAHUOENGINE_HUAHUOENGINE_H
