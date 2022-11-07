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

    TRANSFER(m_fieldNameFieldIndexMap);
    TRANSFER(m_fieldIndexFieldNameMap);
    TRANSFER(m_floatFieldInitValues);
}

CustomDataKeyFrame Lerp(CustomDataKeyFrame &k1, CustomDataKeyFrame &k2, float ratio) {
    CustomDataKeyFrame resultData;

    if (k1.floatFrameValues.size() != k2.floatFrameValues.size()) {
        Assert("value size mismatch!!!");
        return resultData;
    }

    // TODO: Merge these two similar code block.
    // Interpolate float values.
    for (auto itr = k1.floatFrameValues.begin(); itr != k1.floatFrameValues.end(); itr++) {
        int fieldIdx = itr->first;
        if (!k2.floatFrameValues.contains(fieldIdx)) {
            resultData.floatFrameValues[fieldIdx] = k1.floatFrameValues[fieldIdx];
        } else {
            resultData.floatFrameValues[fieldIdx] = Lerp(k1.floatFrameValues[fieldIdx], k2.floatFrameValues[fieldIdx],
                                                         ratio);
        }
    }

    // Interpolate color values.
    for (auto itr = k1.colorFrameValues.begin(); itr != k1.colorFrameValues.end(); itr++) {
        int fieldIdx = itr->first;
        if (!k2.colorFrameValues.contains(fieldIdx)) {
            resultData.colorFrameValues[fieldIdx] = k1.colorFrameValues[fieldIdx];
        } else {
            resultData.colorFrameValues[fieldIdx] = Lerp(k1.colorFrameValues[fieldIdx], k2.colorFrameValues[fieldIdx],
                                                         ratio);
        }
    }

    // Copy other fields.
    resultData.shapeArrayValues = k1.shapeArrayValues;

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

void CustomFrameState::SetFloatValue(const char *fieldName, float value) {
    Layer *shapeLayer = baseShape->GetLayer();
    int currentFrameId = shapeLayer->GetCurrentFrame();
    this->RecordFieldValue(currentFrameId, fieldName, value);
    shapeLayer->AddKeyFrame(currentFrameId, this->baseShape);
}

float CustomFrameState::GetFloatValue(const char *fieldName) {
    int fieldIdx = m_fieldNameFieldIndexMap[fieldName];

    if (isValidFrame) {
        return m_CurrentKeyFrame.floatFrameValues[fieldIdx];
    }

    return this->m_floatFieldInitValues[fieldIdx];
}

void CustomFrameState::SetColorValue(const char* fieldName, float r, float g, float b, float a){
    Layer *shapeLayer = baseShape->GetLayer();
    int currentFrameId = shapeLayer->GetCurrentFrame();

    ColorRGBAf colorRgbAf(r, g, b, a);
    this->RecordFieldValue(currentFrameId, fieldName, colorRgbAf);
    shapeLayer->AddKeyFrame(currentFrameId, this->baseShape);
}

ColorRGBAf* CustomFrameState::GetColorValue(const char* fieldName){
    int fieldIdx = m_fieldNameFieldIndexMap[fieldName];

    if (isValidFrame) {
        return &m_CurrentKeyFrame.colorFrameValues[fieldIdx];
    }

    return &(m_colorFieldInitValues[fieldIdx]);
}

void CustomFrameState::CreateShapeArrayValue(const char *fieldName){
    Layer *shapeLayer = baseShape->GetLayer();
    int currentFrameId = shapeLayer->GetCurrentFrame();

    this->RecordFieldValue(currentFrameId, fieldName, FieldShapeArray());
    shapeLayer->AddKeyFrame(currentFrameId, this->baseShape);
}

FieldShapeArray* CustomFrameState::GetShapeArrayValueForWrite(const char* fieldName){
    Layer *shapeLayer = baseShape->GetLayer();
    int currentFrameId = shapeLayer->GetCurrentFrame();
    CustomDataKeyFrame *pKeyFrame = InsertOrUpdateKeyFrame(currentFrameId, GetKeyFrames());

    int fieldId = m_fieldNameFieldIndexMap[fieldName];
    FieldShapeArray* pShapeArray = &pKeyFrame->shapeArrayValues[fieldId];
    pShapeArray->SetFrameState(this);

    return pShapeArray;
}

FieldShapeArray* CustomFrameState::GetShapeArrayValue(const char* fieldName){
    int fieldIdx = m_fieldNameFieldIndexMap[fieldName];

    if (isValidFrame) {
        return &m_CurrentKeyFrame.shapeArrayValues[fieldIdx];
    }

    return NULL;
}

bool CustomFrameState::Apply() {
    Layer *shapeLayer = baseShape->GetLayer();
    int currentFrameId = shapeLayer->GetCurrentFrame();
    return this->Apply(currentFrameId);
}

template <typename T>
void CustomFrameState::RecordFieldValue(int frameId, const char *fieldName, T value) {
    CustomDataKeyFrame *pKeyFrame = InsertOrUpdateKeyFrame(frameId, GetKeyFrames());

    int idx = m_fieldNameFieldIndexMap[fieldName];

    if constexpr(std::is_floating_point<T>()) {
        pKeyFrame->floatFrameValues[idx] = value;
    } else if constexpr(std::is_same<T, FieldShapeArray>()){
        pKeyFrame->shapeArrayValues[idx] = value;
    } else if constexpr(std::is_same<T, ColorRGBAf>()){
        pKeyFrame->colorFrameValues[idx] = value;
    }

    Apply(frameId);
}

CustomFrameState *CustomFrameState::CreateFrameState() {
    CustomFrameState *producedFrameState = Object::Produce<CustomFrameState>();
    printf("Creating component at path:%s\n", StoreFilePath.c_str());
    GetPersistentManagerPtr()->MakeObjectPersistent(producedFrameState->GetInstanceID(), StoreFilePath);
    return producedFrameState;
}