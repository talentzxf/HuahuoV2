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

    if (k1.data.dataType != k2.data.dataType) {
        Assert("Data Type of k1 and k2 mismatch!");
        return resultData;
    }

    switch (k1.data.dataType) {
        case FLOAT:
            resultData.data.floatValue = Lerp(k1.data.floatValue, k2.data.floatValue, ratio);
            break;
        case COLOR:
            resultData.data.colorValue = Lerp(k1.data.colorValue, k2.data.colorValue, ratio);
            break;
        case SHAPEARRAY:
            resultData.data.shapeArrayValue = k1.data.shapeArrayValue;
            break;
        case COLORSTOPARRAY:
            resultData.data.colorStopArray = Lerp(k1.data.colorStopArray, k2.data.colorStopArray, ratio);
            break;
        case VECTOR3:
            resultData.data.vector3Value = Lerp(k1.data.vector3Value, k2.data.vector3Value, ratio);
            break;
        case BINARYRESOURCE:
            resultData.data.binaryResource = k1.data.binaryResource;
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

        if (k2 == NULL || k2->GetFrameId() ==
                          k1->GetFrameId()) { // Avoid 0/0 during ratio calculation. Or beyond the last frame. k1 is the last frame.
            this->m_CurrentKeyFrame = *k1;
        } else {
            float ratio = float(frameId - k1->GetFrameId()) / float(k2->GetFrameId() - k1->GetFrameId());

            this->m_CurrentKeyFrame = Lerp(*k1, *k2, ratio);
        }

        return true;
    }

    return false;
}

void CustomFrameState::SetFloatValue(float value) {
    if (this->m_DataType != FLOAT) {
        Assert("Data Type mismatch!");
        return;
    }

    Layer *shapeLayer = GetBaseShape()->GetLayer();
    int currentFrameId = shapeLayer->GetCurrentFrame();
    CustomDataKeyFrame* pKeyFrame = this->RecordFieldValue(currentFrameId, value);
    pKeyFrame->SetFrameState(this);
    shapeLayer->AddKeyFrame(&pKeyFrame->GetKeyFrame());
}

void CustomFrameState::SetBinaryResourceName(const char* resourceName){
    if(this->m_DataType != BINARYRESOURCE){
        Assert("Data Type mismatch!");
        return;
    }

    Layer *shapeLayer = GetBaseShape()->GetLayer();
    int currentFrameId = shapeLayer->GetCurrentFrame();
    CustomDataKeyFrame *pKeyFrame = InsertOrUpdateKeyFrame(currentFrameId, GetKeyFrames(), this);
    pKeyFrame->data.binaryResource.SetResourceName(resourceName);
    pKeyFrame->data.dataType = BINARYRESOURCE;

    Apply(currentFrameId);

    pKeyFrame->SetFrameState(this);
    shapeLayer->AddKeyFrame(&pKeyFrame->GetKeyFrame());
}

void CustomFrameState::SetVector3Value(float x, float y, float z) {
    if (this->m_DataType != VECTOR3) {
        Assert("Data Type mismatch!");
        return;
    }

    Layer *shapeLayer = GetBaseShape()->GetLayer();
    int currentFrameId = shapeLayer->GetCurrentFrame();
    Vector3f value(x, y, z);
    CustomDataKeyFrame* pKeyFrame = this->RecordFieldValue(currentFrameId, value);
    pKeyFrame->SetFrameState(this);
    shapeLayer->AddKeyFrame(&pKeyFrame->GetKeyFrame());
}

float CustomFrameState::GetFloatValue() {
    if (this->m_DataType != FLOAT) {
        Assert("Data Type mismatch!");
        return -1.0f;
    }

    if (isValidFrame) {
        return m_CurrentKeyFrame.data.floatValue;
    }

    return this->m_defaultValue.floatValue;
}

Vector3f* CustomFrameState::GetVector3Value(){
    if (this->m_DataType != VECTOR3) {
        Assert("Data Type mismatch!");
        return NULL;
    }

    if (isValidFrame) {
        return &m_CurrentKeyFrame.data.vector3Value;
    }

    return &m_defaultValue.vector3Value;
}

void CustomFrameState::SetColorValue(float r, float g, float b, float a) {
    if (this->m_DataType != COLOR) {
        Assert("Data Type mismatch!");
        return;
    }

    Layer *shapeLayer = GetBaseShape()->GetLayer();
    int currentFrameId = shapeLayer->GetCurrentFrame();
    ColorRGBAf value(r, g, b, a);
    CustomDataKeyFrame* pKeyFrame = this->RecordFieldValue(currentFrameId, value);
    pKeyFrame->SetFrameState(this);
    shapeLayer->AddKeyFrame(&pKeyFrame->GetKeyFrame());
}

CustomDataKeyFrame* CustomFrameState::GetColorStopArrayKeyFrame(int currentFrameId) {
    bool isInsert;
    CustomDataKeyFrame *pKeyFrame = InsertOrUpdateKeyFrame(currentFrameId, GetKeyFrames(), this, &isInsert);

    if (isInsert) {
        // Copy the whole array from the previous keyframe.
        auto itr = FindLastKeyFrame(currentFrameId - 1, GetKeyFrames());

        printf("Total frames:%lu\n", GetKeyFrames().size());
        if (itr == GetKeyFrames().begin()) {
            printf("Is begin\n");
        }

        if (itr == GetKeyFrames().end()) {
            printf("Is End\n");
        }

        pKeyFrame->data.dataType = COLORSTOPARRAY;
        for (int colorStopIdx = pKeyFrame->data.colorStopArray.GetColorStopCount();
             colorStopIdx < itr->data.colorStopArray.GetColorStopCount(); colorStopIdx++) {
            ColorStopEntry colorStopEntry = *itr->data.colorStopArray.GetColorStop(colorStopIdx);

            pKeyFrame->data.colorStopArray.AddEntry(colorStopEntry);
        }

    }

    return pKeyFrame;
}

// If color is not specified, lerp the color than add to the array.
int CustomFrameState::AddColorStop(float value) {
    auto currentColorStopArray = m_CurrentKeyFrame.data.colorStopArray;

    ColorRGBAf resultColor = currentColorStopArray.LerpColor(value);

    return AddColorStop(value, resultColor.r, resultColor.g, resultColor.b, resultColor.a);
}

int CustomFrameState::AddColorStop(float value, float r, float g, float b, float a) {
    if (this->m_DataType != COLORSTOPARRAY) {
        Assert("Data Type mismatch!");
    }

    Layer *shapeLayer = GetBaseShape()->GetLayer();
    int currentFrameId = shapeLayer->GetCurrentFrame();

    ColorStopEntry colorStopEntry(-1, value, r, g, b, a);
    CustomDataKeyFrame* pKeyFrame = GetColorStopArrayKeyFrame(currentFrameId);
    pKeyFrame->data.colorStopArray.AddEntry(colorStopEntry);

    // Add the interpolated value to all other keyframes;
    for (auto frameId = 0 ; frameId < m_KeyFrames.GetKeyFrames().size(); frameId++) {
        if(m_KeyFrames.GetKeyFrames()[frameId].GetFrameId() == pKeyFrame->GetFrameId())
            continue;

        m_KeyFrames.GetKeyFrames()[frameId].data.colorStopArray.AddEntry(colorStopEntry);
    }

    Apply(currentFrameId);
    pKeyFrame->GetKeyFrame().SetFrameState(this);
    shapeLayer->AddKeyFrame(&pKeyFrame->GetKeyFrame());

    return colorStopEntry.GetIdentifier();
}

void CustomFrameState::UpdateColorStop(int idx, float value, float r, float g, float b, float a) {
    if (this->m_DataType != COLORSTOPARRAY) {
        Assert("Data Type mismatch!");
        return;
    }

    Layer *shapeLayer = GetBaseShape()->GetLayer();
    int currentFrameId = shapeLayer->GetCurrentFrame();

    CustomDataKeyFrame* pKeyFrame = GetColorStopArrayKeyFrame(currentFrameId);

    pKeyFrame->data.colorStopArray.UpdateAtIdentifier(idx, value, r, g, b, a);
    Apply(currentFrameId);

    pKeyFrame->GetKeyFrame().SetFrameState(this);
    shapeLayer->AddKeyFrame(&pKeyFrame->GetKeyFrame());
}

void CustomFrameState::DeleteColorStop(int idx) {
    if (this->m_DataType != COLORSTOPARRAY) {
        Assert("Data Type mismatch!");
    }

    Layer *shapeLayer = GetBaseShape()->GetLayer();
    int currentFrameId = shapeLayer->GetCurrentFrame();

    // If this frame is not keyframe, need to copy all color stops over first.
    CustomDataKeyFrame* pKeyFrame = GetColorStopArrayKeyFrame(currentFrameId);

    // Delete the color stop from all key frames.
    for(int keyFrameDataIndex = 0; keyFrameDataIndex < m_KeyFrames.GetKeyFrames().size(); keyFrameDataIndex++){
        m_KeyFrames.GetKeyFrames()[keyFrameDataIndex].data.colorStopArray.DeleteEntry(idx);
    }

    m_CurrentKeyFrame.data.colorStopArray.DeleteEntry(idx);

    Apply(currentFrameId);

    pKeyFrame->GetKeyFrame().SetFrameState(this);
    shapeLayer->AddKeyFrame(&pKeyFrame->GetKeyFrame());
}

ColorRGBAf *CustomFrameState::GetColorValue() {
    if (isValidFrame) {
        return &m_CurrentKeyFrame.data.colorValue;
    }

    return &(m_defaultValue.colorValue);
}

ColorStopArray *CustomFrameState::GetColorStopArray() {
    if (isValidFrame) {
        return &m_CurrentKeyFrame.data.colorStopArray;
    }

    return NULL;
}

void CustomFrameState::CreateShapeArrayValue() {
    Layer *shapeLayer = GetBaseShape()->GetLayer();
    int currentFrameId = shapeLayer->GetCurrentFrame();

    CustomDataKeyFrame* pDataKeyFrame = this->RecordFieldValue(currentFrameId, FieldShapeArray());
    pDataKeyFrame->SetFrameState(this);
    shapeLayer->AddKeyFrame(&pDataKeyFrame->GetKeyFrame());
}

FieldShapeArray *CustomFrameState::GetShapeArrayValueForWrite() {
    Layer *shapeLayer = GetBaseShape()->GetLayer();
    int currentFrameId = shapeLayer->GetCurrentFrame();
    CustomDataKeyFrame *pKeyFrame = InsertOrUpdateKeyFrame(currentFrameId, GetKeyFrames(), this);

    FieldShapeArray *pShapeArray = &pKeyFrame->data.shapeArrayValue;
    pShapeArray->SetFrameState(this);

    return pShapeArray;
}

FieldShapeArray *CustomFrameState::GetShapeArrayValue() {
    if (isValidFrame) {
        return &m_CurrentKeyFrame.data.shapeArrayValue;
    }

    return NULL;
}

bool CustomFrameState::Apply() {
    Layer *shapeLayer = GetBaseShape()->GetLayer();
    int currentFrameId = shapeLayer->GetCurrentFrame();
    return this->Apply(currentFrameId);
}

template<typename T>
CustomDataKeyFrame * CustomFrameState::RecordFieldValue(int frameId, T value) {
    CustomDataKeyFrame *pKeyFrame = InsertOrUpdateKeyFrame(frameId, GetKeyFrames(), this);

    if constexpr(std::is_floating_point<T>()) {
        pKeyFrame->data.floatValue = value;
        pKeyFrame->data.dataType = FLOAT;
    } else if constexpr(std::is_same<T, FieldShapeArray>()) {
        pKeyFrame->data.shapeArrayValue = value;
        pKeyFrame->data.dataType = SHAPEARRAY;
    } else if constexpr(std::is_same<T, ColorRGBAf>()) {
        pKeyFrame->data.colorValue = value;
        pKeyFrame->data.dataType = COLOR;
    } else if constexpr(std::is_same<T, ColorStopEntry>()) {
        pKeyFrame->data.colorStopArray.AddEntry(value);
        pKeyFrame->data.dataType = COLORSTOPARRAY;
    } else if constexpr(std::is_same<T, Vector3f>()){
        pKeyFrame->data.vector3Value = value;
        pKeyFrame->data.dataType = VECTOR3;
    }

    Apply(frameId);

    return pKeyFrame;
}

CustomFrameState *CustomFrameState::CreateFrameState(CustomDataType dataType) {
    CustomFrameState *producedFrameState = Object::Produce<CustomFrameState>();
    producedFrameState->m_DataType = dataType;
    printf("Creating component at path:%s\n", StoreFilePath.c_str());
    GetPersistentManagerPtr()->MakeObjectPersistent(producedFrameState->GetInstanceID(), StoreFilePath);
    return producedFrameState;
}