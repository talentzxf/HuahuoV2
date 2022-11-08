//
// Created by VincentZhang on 10/23/2022.
//

#include "CustomFrameState.h"
#include "Layer.h"
#include <type_traits>

IMPLEMENT_REGISTER_CLASS(CustomFrameState, 10021);

IMPLEMENT_OBJECT_SERIALIZE(CustomFrameState);

INSTANTIATE_TEMPLATE_TRANSFER(CustomFrameState);

template<class TransferFunction>
void CustomFrameState::Transfer(TransferFunction &transfer) {
    AbstractFrameStateWithKeyType::Transfer(transfer);
    TRANSFER_ENUM(m_DataType);
    TRANSFER(m_defaultValue);
}

CustomDataKeyFrame Lerp(CustomDataKeyFrame &k1, CustomDataKeyFrame &k2, float ratio) {
    CustomDataKeyFrame resultData;

    if(k1.data.dataType != k2.data.dataType){
        Assert("Data Type of k1 and k2 mismatch!");
        return resultData;
    }

    switch(k1.data.dataType){
        case FLOAT:
            resultData.data.floatValue = Lerp(k1.data.floatValue, k2.data.floatValue, ratio);
            break;
        case COLOR:
            resultData.data.colorValue = Lerp(k1.data.colorValue, k2.data.colorValue, ratio);
            break;
        case SHAPEARRAY:
            resultData.data.shapeArrayValue = k1.data.shapeArrayValue;
            break;
    }

    resultData.data.dataType = k1.data.dataType;
    return resultData;
}

bool CustomFrameState::Apply(int frameId) {
    std::pair<CustomDataKeyFrame *, CustomDataKeyFrame *> resultKeyFrames;
    if (FindKeyFramePair(frameId, GetKeyFrames(), resultKeyFrames)) {
        this->isValidFrame = true;
        CustomDataKeyFrame *k1 = resultKeyFrames.first;
        CustomDataKeyFrame *k2 = resultKeyFrames.second;

        if (k2 == NULL || k2->frameId ==
                          k1->frameId) { // Avoid 0/0 during ratio calculation. Or beyond the last frame. k1 is the last frame.
            this->m_CurrentKeyFrame = *k1;
        } else {
            float ratio = float(frameId - k1->frameId) / float(k2->frameId - k1->frameId);

            this->m_CurrentKeyFrame = Lerp(*k1, *k2, ratio);
        }

        return true;
    }

    return false;
}

void CustomFrameState::SetFloatValue(float value) {
    if(this->m_DataType != FLOAT)
    {
        Assert("Data Type mismatch!");
        return;
    }

    Layer *shapeLayer = baseShape->GetLayer();
    int currentFrameId = shapeLayer->GetCurrentFrame();
    this->RecordFieldValue(currentFrameId, value);
    shapeLayer->AddKeyFrame(currentFrameId, this->baseShape);
}

float CustomFrameState::GetFloatValue() {
    if(this->m_DataType != FLOAT)
    {
        Assert("Data Type mismatch!");
        return -1.0f;
    }

    if (isValidFrame) {
        return m_CurrentKeyFrame.data.floatValue;
    }

    return this->m_defaultValue.floatValue;
}

void CustomFrameState::SetColorValue(float r, float g, float b, float a){
    if(this->m_DataType != COLOR)
    {
        Assert("Data Type mismatch!");
        return;
    }

    Layer *shapeLayer = baseShape->GetLayer();
    int currentFrameId = shapeLayer->GetCurrentFrame();
    ColorRGBAf value(r,g,b,a);
    this->RecordFieldValue(currentFrameId, value);
    shapeLayer->AddKeyFrame(currentFrameId, this->baseShape);
}

ColorRGBAf* CustomFrameState::GetColorValue(){
    if (isValidFrame) {
        return &m_CurrentKeyFrame.data.colorValue;
    }

    return &(m_defaultValue.colorValue);
}

void CustomFrameState::CreateShapeArrayValue(){
    Layer *shapeLayer = baseShape->GetLayer();
    int currentFrameId = shapeLayer->GetCurrentFrame();

    this->RecordFieldValue(currentFrameId, FieldShapeArray());
    shapeLayer->AddKeyFrame(currentFrameId, this->baseShape);
}

FieldShapeArray* CustomFrameState::GetShapeArrayValueForWrite(){
    Layer *shapeLayer = baseShape->GetLayer();
    int currentFrameId = shapeLayer->GetCurrentFrame();
    CustomDataKeyFrame *pKeyFrame = InsertOrUpdateKeyFrame(currentFrameId, GetKeyFrames());

    FieldShapeArray* pShapeArray = &pKeyFrame->data.shapeArrayValue;
    pShapeArray->SetFrameState(this);

    return pShapeArray;
}

FieldShapeArray* CustomFrameState::GetShapeArrayValue(){
    if (isValidFrame) {
        return &m_CurrentKeyFrame.data.shapeArrayValue;
    }

    return NULL;
}

bool CustomFrameState::Apply() {
    Layer *shapeLayer = baseShape->GetLayer();
    int currentFrameId = shapeLayer->GetCurrentFrame();
    return this->Apply(currentFrameId);
}

template <typename T>
void CustomFrameState::RecordFieldValue(int frameId, T value) {
    CustomDataKeyFrame *pKeyFrame = InsertOrUpdateKeyFrame(frameId, GetKeyFrames());

    if constexpr(std::is_floating_point<T>()) {
        pKeyFrame->data.floatValue = value;
    } else if constexpr(std::is_same<T, FieldShapeArray>()){
        pKeyFrame->data.shapeArrayValue = value;
    } else if constexpr(std::is_same<T, ColorRGBAf>()){
        pKeyFrame->data.colorValue = value;
    }

    Apply(frameId);
}

CustomFrameState *CustomFrameState::CreateFrameState() {
    CustomFrameState *producedFrameState = Object::Produce<CustomFrameState>();
    printf("Creating component at path:%s\n", StoreFilePath.c_str());
    GetPersistentManagerPtr()->MakeObjectPersistent(producedFrameState->GetInstanceID(), StoreFilePath);
    return producedFrameState;
}