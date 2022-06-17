//
// Created by VincentZhang on 6/1/2022.
//

#include "BaseShape.h"
#include "Export/Events/ScriptEventManager.h"
#include "Serialize/SerializeUtility.h"
#include "Layer.h"

IMPLEMENT_REGISTER_CLASS(BaseShape, 10002);

IMPLEMENT_OBJECT_SERIALIZE(BaseShape);

INSTANTIATE_TEMPLATE_TRANSFER(BaseShape);

template<class TransferFunction>
void BaseShape::Transfer(TransferFunction &transfer) {
    Super::Transfer(transfer);
    TRANSFER(mTransformKeyFrames);
    TRANSFER(mColorKeyFrames);
}

void BaseShape::AwakeFromLoad(AwakeFromLoadMode awakeMode) {
    ShapeLoadedEventArgs args(this);

    GetScriptEventManager()->TriggerEvent("OnShapeLoaded", &args);
}

BaseShape *BaseShape::CreateShape(const char *shapeName) {
    const HuaHuo::Type *shapeType = HuaHuo::Type::FindTypeByName(shapeName);
    if (shapeType == NULL || !shapeType->IsDerivedFrom<BaseShape>()) {
        printf("Error, the shape:%s is not derived from baseshape.\n", shapeName);
        return NULL;
    }

    BaseShape *baseShape = (BaseShape *) Object::Produce(shapeType);
    return baseShape;
}

void BaseShape::SetPosition(float x, float y, float z) {
    if (this->mLayer == NULL) {
        printf("Error, layer has not been set, can't record it's position");
        return;
    }

    int currentFrameId = this->mLayer->GetCurrentFrame();
    mTransformKeyFrames->RecordPosition(currentFrameId, x, y, z);
}


void BaseShape::SetColor(float r, float g, float b, float a) {
    if (this->mLayer == NULL) {
        printf("Error, layer has not been set, can't record it's position");
        return;
    }

    int currentFrameId = this->mLayer->GetCurrentFrame();
    mColorKeyFrames->RecordColor(currentFrameId, r, g, b, a);
}