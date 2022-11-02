//
// Created by VincentZhang on 10/23/2022.
//

#include "CustomFrameState.h"
#include "Layer.h"

IMPLEMENT_REGISTER_CLASS(CustomFrameState, 10021);

IMPLEMENT_OBJECT_SERIALIZE(CustomFrameState);

INSTANTIATE_TEMPLATE_TRANSFER(CustomFrameState);

template<class TransferFunction>
void CustomFrameState::Transfer(TransferFunction &transfer) {
    AbstractFrameStateWithKeyType::Transfer(transfer);

    TRANSFER(m_fieldNameFieldIndexMap);
    TRANSFER(m_fieldIndexFieldNameMap);
    TRANSFER(m_fieldInitValues);
}

CustomDataKeyFrame Lerp(CustomDataKeyFrame &k1, CustomDataKeyFrame &k2, float ratio) {
    CustomDataKeyFrame resultData;

    if (k1.floatFrameValues.size() != k2.floatFrameValues.size()) {
        Assert("value size mismatch!!!");
        return resultData;
    }

    for (auto itr = k1.floatFrameValues.begin(); itr != k1.floatFrameValues.end(); itr++) {
        int fieldIdx = itr->first;
        if (!k2.floatFrameValues.contains(fieldIdx)) {
            resultData.floatFrameValues[fieldIdx] = k1.floatFrameValues[fieldIdx];
        } else {
            resultData.floatFrameValues[fieldIdx] = Lerp(k1.floatFrameValues[fieldIdx], k2.floatFrameValues[fieldIdx],
                                                         ratio);
        }
    }

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
            this->m_CurrentcustomFloatKeyFrame = *k1;
        } else {
            float ratio = float(frameId - k1->frameId) / float(k2->frameId - k1->frameId);

            this->m_CurrentcustomFloatKeyFrame = Lerp(*k1, *k2, ratio);
        }

        return true;
    }

    return false;
}

void CustomFrameState::SetValue(const char *fieldName, float value) {
    Layer *shapeLayer = baseShape->GetLayer();
    int currentFrameId = shapeLayer->GetCurrentFrame();
    this->RecordFieldValue(currentFrameId, fieldName, value);
    shapeLayer->AddKeyFrame(currentFrameId, this->baseShape);
}

float CustomFrameState::GetValue(const char *fieldName) {
    int fieldIdx = m_fieldNameFieldIndexMap[fieldName];

    if (isValidFrame) {
        return m_CurrentcustomFloatKeyFrame.floatFrameValues[fieldIdx];
    }

    return this->m_fieldInitValues[fieldIdx];
}

void CustomFrameState::RecordFieldValue(int frameId, const char *fieldName, float value) {
    CustomDataKeyFrame *pKeyFrame = InsertOrUpdateKeyFrame(frameId, GetKeyFrames());
    if (!pKeyFrame->inited) {
        // Init it.
        for (int fieldIdx = 0; fieldIdx < m_fieldInitValues.size(); fieldIdx++) {
            pKeyFrame->floatFrameValues[fieldIdx] = m_fieldInitValues[fieldIdx];
        }
        pKeyFrame->inited = true;
    }

    int idx = m_fieldNameFieldIndexMap[fieldName];
    pKeyFrame->floatFrameValues[idx] = value;

    Apply(frameId);
}

CustomFrameState *CustomFrameState::CreateFrameState() {
    CustomFrameState *producedFrameState = Object::Produce<CustomFrameState>();
    printf("Creating component at path:%s\n", StoreFilePath.c_str());
    GetPersistentManagerPtr()->MakeObjectPersistent(producedFrameState->GetInstanceID(), StoreFilePath);
    return producedFrameState;
}