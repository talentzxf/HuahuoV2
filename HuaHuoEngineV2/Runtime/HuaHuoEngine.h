//
// Created by VincentZhang on 4/1/2022.
//

#ifndef HUAHUOENGINE_HUAHUOENGINE_H
#define HUAHUOENGINE_HUAHUOENGINE_H

#include <cstddef>
#include <vector>
#include "Export/Events/ScriptEventManager.h"
#include "Serialize/PersistentManager.h"

class HuaHuoEngine {
private:
    HuaHuoEngine();

    static HuaHuoEngine *gInstance;
public:
    static void InitEngine();

    inline static HuaHuoEngine *getInstance() {
        return gInstance;
    }

    PersistentManager* GetPersistentManager(){
        return ::GetPersistentManagerPtr();
    }

    void RegisterEvent(std::string eventType, ScriptEventHandler* pHandler){
        GetScriptEventManager()->RegisterEventHandler(eventType, pHandler);
    }
};


#endif //HUAHUOENGINE_HUAHUOENGINE_H
