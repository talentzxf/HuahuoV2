//
// Created by VincentZhang on 4/1/2022.
//

#ifndef HUAHUOENGINE_HUAHUOENGINE_H
#define HUAHUOENGINE_HUAHUOENGINE_H

#include <cstddef>
#include <vector>
#include "Export/Events/ScriptEventManager.h"
#include "Serialize/PersistentManager.h"
#include "Shapes/BaseShape.h"
#include "Shapes/LineShape.h"

class HuaHuoEngine {
private:
    HuaHuoEngine();

    static HuaHuoEngine *gInstance;
public:
    static void InitEngine();

    inline static HuaHuoEngine *GetInstance() {
        return gInstance;
    }

    void RegisterEvent(std::string eventType, ScriptEventHandler* pHandler){
        GetScriptEventManager()->RegisterEventHandler(eventType, pHandler);
    }

    BaseShape* CreateShape(const char* shapeName){
        const HuaHuo::Type* shapeType = HuaHuo::Type::FindTypeByName(shapeName);
        if(!shapeType->IsDerivedFrom<BaseShape>()){
            printf("Error, this type:%s is not derived from BaseShape!\n", shapeName);
            return NULL;
        }

        return reinterpret_cast<BaseShape*>(Object::Produce(shapeType));
    }
};


#endif //HUAHUOENGINE_HUAHUOENGINE_H
