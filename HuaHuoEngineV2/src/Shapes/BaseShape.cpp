//
// Created by VincentZhang on 6/1/2022.
//

#include "BaseShape.h"
#include "Export/Events/ScriptEventManager.h"
#include "Serialize/SerializeUtility.h"
#include "Layer.h"
#include "ObjectStore.h"

IMPLEMENT_REGISTER_CLASS(BaseShape, 10002);

IMPLEMENT_OBJECT_SERIALIZE(BaseShape);

INSTANTIATE_TEMPLATE_TRANSFER(BaseShape);

template<class TransferFunction>
void BaseShape::Transfer(TransferFunction &transfer) {
    Super::Transfer(transfer);
    TRANSFER(mBornFrameId);
    TRANSFER(mTransformKeyFrames);
    TRANSFER(mColorKeyFrames);
}

void BaseShape::AwakeFromLoad(AwakeFromLoadMode awakeMode) {

    int frameId = this->GetLayer()->GetCurrentFrame();
    Apply(frameId);

    ShapeLoadedEventArgs args(this);
    GetScriptEventManager()->TriggerEvent("OnShapeLoaded", &args);
}

BaseShape *BaseShape::CreateShape(const char *shapeName) {
    const HuaHuo::Type *shapeType = HuaHuo::Type::FindTypeByName(shapeName);
    if (shapeType == NULL || !shapeType->IsDerivedFrom<BaseShape>()) {
        return NULL;
    }

    BaseShape *baseShape = (BaseShape *) Object::Produce(shapeType);
    return baseShape;
}

Layer *BaseShape::GetLayer() {
    if (!this->mLayer) {
        this->mLayer = GetDefaultObjectStoreManager()->GetCurrentStore()->GetCurrentLayer();
    }

    return mLayer;
}

void BaseShape::SetPosition(float x, float y, float z) {

    Layer *shapeLayer = GetLayer();

    int currentFrameId = shapeLayer->GetCurrentFrame();

    printf("Recording shape position:%d, %f, %f, %f\n", currentFrameId, x, y, z);
    mTransformKeyFrames->RecordPosition(currentFrameId, x, y, z);
}


void BaseShape::SetColor(float r, float g, float b, float a) {
    Layer *shapeLayer = GetLayer();
    int currentFrameId = shapeLayer->GetCurrentFrame();
    mColorKeyFrames->RecordColor(currentFrameId, r, g, b, a);
}

bool BaseShape::IsVisibleInFrame(SInt32 frameId){
    if(this->mBornFrameId < 0)
        return false;

    Layer *shapeLayer = GetLayer();
    unsigned int bornFrameSpanHead = shapeLayer->GetTimeLineCellManager()->GetSpanHead(mBornFrameId);
    unsigned int currentFrameSpanHead = shapeLayer->GetTimeLineCellManager()->GetSpanHead(frameId);

    return bornFrameSpanHead == currentFrameSpanHead;
}

bool BaseShape::IsVisible(){
    return IsVisibleInFrame(GetLayer()->GetCurrentFrame());
}
