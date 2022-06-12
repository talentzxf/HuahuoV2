//
// Created by VincentZhang on 6/1/2022.
//

#ifndef HUAHUOENGINEV2_BASESHAPE_H
#define HUAHUOENGINEV2_BASESHAPE_H
#include "TypeSystem/Object.h"
#include "Export/Events/ScriptEventManager.h"

class BaseShape;
class ShapeLoadedEventArgs: public ScriptEventHandlerArgs{
public:
    ShapeLoadedEventArgs(BaseShape* baseShape):m_BaseShape(baseShape){

    }

    BaseShape* GetBaseShape(){
        return m_BaseShape;
    }
private:
    BaseShape* m_BaseShape;
};

class BaseShape : public Object{
    REGISTER_CLASS_TRAITS(kTypeIsAbstract);
    REGISTER_CLASS(BaseShape);
    DECLARE_OBJECT_SERIALIZE();
public:
    BaseShape(MemLabelId label, ObjectCreationMode mode)
        :Super(label, mode)
    {}

    virtual char* GetName(){
        return "Unknown";
    }

    virtual void AwakeFromLoad(AwakeFromLoadMode awakeMode) override;
    static BaseShape* CreateShape(const char* shapeName);
};


#endif //HUAHUOENGINEV2_BASESHAPE_H
