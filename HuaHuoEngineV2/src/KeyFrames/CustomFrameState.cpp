//
// Created by VincentZhang on 10/23/2022.
//

#include "CustomFrameState.h"

IMPLEMENT_REGISTER_CLASS(CustomFrameState, 10021);

IMPLEMENT_OBJECT_SERIALIZE(CustomFrameState);

INSTANTIATE_TEMPLATE_TRANSFER(CustomFrameState);

template<class TransferFunction>
void CustomFrameState::Transfer(TransferFunction &transfer) {
    Super::Transfer(transfer);
    TRANSFER(GetKeyFrames());

    TRANSFER(m_fieldNameFieldIndexMap);
    TRANSFER(m_fieldIndexFieldNameMap);
    TRANSFER(m_fieldInitValues);
}

CustomFloatKeyFrame Lerp(CustomFloatKeyFrame& k1, CustomFloatKeyFrame& k2, float ratio){
    CustomFloatKeyFrame resultData;

    if(k1.frameValues.size() != k2.frameValues.size()){
        Assert("value size mismatch!!!");
        return resultData;
    }

    int valueSize = k1.frameValues.size();
    resultData.frameValues.reserve(valueSize);

    for(int i = 0 ; i < valueSize; i++){
        resultData.frameValues[i] = Lerp(k1.frameValues[i], k2.frameValues[i], ratio);
    }

    return resultData;
}

bool CustomFrameState::Apply(int frameId) {
    std::pair<CustomFloatKeyFrame *, CustomFloatKeyFrame *> resultKeyFrames;
    if (FindKeyFramePair(frameId, GetKeyFrames(), resultKeyFrames)) {
        this->isValidFrame = true;
        CustomFloatKeyFrame *k1 = resultKeyFrames.first;
        CustomFloatKeyFrame *k2 = resultKeyFrames.second;

        if (k2 == NULL || k2->frameId == k1->frameId ) { // Avoid 0/0 during ratio calculation. Or beyond the last frame. k1 is the last frame.
            this->m_CurrentcustomFloatKeyFrame = *k1;
        }
        else
        {
            float ratio = float(frameId - k1->frameId) / float(k2->frameId - k1->frameId);

            this->m_CurrentcustomFloatKeyFrame = Lerp(*k1, *k2, ratio);
        }

        return true;
    }

    return false;
}

void CustomFrameState::RecordFieldValue(int frameId, const char* fieldName, float value) {
    CustomFloatKeyFrame *pKeyFrame = InsertOrUpdateKeyFrame(frameId, GetKeyFrames());
    if(!pKeyFrame->inited){
        // Init it.
        pKeyFrame->frameValues.resize(m_fieldInitValues.size());
        for(int fieldIdx = 0; fieldIdx < m_fieldInitValues.size(); fieldIdx++){
            pKeyFrame->frameValues[fieldIdx] = m_fieldInitValues[fieldIdx];
        }
    }

    int idx = m_fieldNameFieldIndexMap[fieldName];
    pKeyFrame->frameValues[idx] = value;

    Apply(frameId);
}