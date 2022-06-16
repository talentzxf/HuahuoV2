//
// Created by VincentZhang on 2022-06-16.
//

#include "ShapeTransformFrameState.h"


IMPLEMENT_REGISTER_CLASS(ShapeTransformFrameState, 10008);

IMPLEMENT_OBJECT_SERIALIZE(ShapeTransformFrameState);

INSTANTIATE_TEMPLATE_TRANSFER(ShapeTransformFrameState);

template<class TransferFunction>
void ShapeTransformFrameState::Transfer(TransferFunction &transfer) {
    Super::Transfer(transfer);
    TRANSFER(m_KeyFrames);
}

TransformData Lerp(TransformData &k1, TransformData &k2, float ratio) {
    TransformData resultData;
    resultData.position = Lerp(k1.position, k2.position, ratio);
    return resultData;
}


bool ShapeTransformFrameState::Apply(int frameId) {
    std::pair<TransformKeyFrames *, TransformKeyFrames *> resultKeyFrames;
    if (FindKeyFramePair(frameId, this->m_KeyFrames, resultKeyFrames)) {
        TransformKeyFrames *k1 = resultKeyFrames.first;
        TransformKeyFrames *k2 = resultKeyFrames.second;

        float ratio = float(frameId - k1->frameId) / float(k2->frameId - k1->frameId);

        this->m_CurrentTransformData = Lerp(k1->transformData, k2->transformData, ratio);

        return true;
    }

    return false;
}
