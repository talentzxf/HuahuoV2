//
// Created by VincentZhang on 6/1/2022.
//

#include "BaseShape.h"
#include "Export/Events/ScriptEventManager.h"
#include "Serialize/SerializeUtility.h"

IMPLEMENT_REGISTER_CLASS(BaseShape, 10002);

IMPLEMENT_OBJECT_SERIALIZE(BaseShape);
INSTANTIATE_TEMPLATE_TRANSFER(BaseShape);

template<class TransferFunction>
void BaseShape::Transfer(TransferFunction &transfer) {
    Super::Transfer(transfer);
    TRANSFER(m_Position);
    TRANSFER(m_Color);
}

void BaseShape::AwakeFromLoad(AwakeFromLoadMode awakeMode) {
    ShapeLoadedEventArgs args(this);

    GetScriptEventManager()->TriggerEvent("OnShapeLoaded", &args);
}

BaseShape* BaseShape::CreateShape(const char* shapeName){
    const HuaHuo::Type* shapeType = HuaHuo::Type::FindTypeByName(shapeName);
    if(shapeType == NULL || !shapeType->IsDerivedFrom<BaseShape>()){
        printf("Error, the shape:%s is not derived from baseshape.\n", shapeName);
        return NULL;
    }

    BaseShape* baseShape = (BaseShape*)Object::Produce(shapeType);
    return baseShape;
}