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
}

TransformData Lerp(TransformData &k1, TransformData &k2, float ratio) {
    TransformData resultData;

    resultData.localPivotPosition = Lerp(k1.localPivotPosition, k2.localPivotPosition, ratio);
    resultData.globalPivotPosition = Lerp(k1.globalPivotPosition, k2.globalPivotPosition, ratio);
    resultData.scale = Lerp(k1.scale, k2.scale, ratio);
    resultData.rotation = Lerp(k1.rotation, k2.rotation, ratio);
    return resultData;
}


bool ShapeTransformFrameState::Apply(int frameId) {
    std::pair<TransformKeyFrame *, TransformKeyFrame *> resultKeyFrames;
    if (FindKeyFramePair(frameId, GetKeyFrames(), resultKeyFrames)) {
        this->isValidFrame = true;
        TransformKeyFrame *k1 = resultKeyFrames.first;
        TransformKeyFrame *k2 = resultKeyFrames.second;

        if (k2 == NULL || k2->frameId == k1->frameId ) { // Avoid 0/0 during ratio calculation. Or beyond the last frame. k1 is the last frame.
            this->m_CurrentTransformData = k1->transformData;
        }
        else
        {
            float ratio = float(frameId - k1->frameId) / float(k2->frameId - k1->frameId);

            this->m_CurrentTransformData = Lerp(k1->transformData, k2->transformData, ratio);
        }

        return true;
    }

    return false;
}

void ShapeTransformFrameState::RecordLocalPivotPosition(int frameId, float x, float y, float z){
    TransformKeyFrame *pKeyFrame = InsertOrUpdateKeyFrame(frameId, GetKeyFrames());
    pKeyFrame->transformData = m_CurrentTransformData;
    pKeyFrame->transformData.localPivotPosition.Set(x, y, z);

    Apply(frameId);
}

void ShapeTransformFrameState::RecordGlobalPivotPosition(int frameId, float x, float y, float z){
    TransformKeyFrame *pKeyFrame = InsertOrUpdateKeyFrame(frameId, GetKeyFrames());
    pKeyFrame->transformData = m_CurrentTransformData;
    pKeyFrame->transformData.globalPivotPosition.Set(x, y, z);

    Apply(frameId);
}

void ShapeTransformFrameState::RecordScale(int frameId, float xScale, float yScale, float zScale) {
    TransformKeyFrame *pKeyFrame = InsertOrUpdateKeyFrame(frameId, GetKeyFrames());
    pKeyFrame->transformData = m_CurrentTransformData; // Record current state.
    pKeyFrame->transformData.scale.Set(xScale, yScale, zScale); // Change the scale.

    Apply(frameId);
}

void ShapeTransformFrameState::RecordRotation(int frameId, float rotation) {
    TransformKeyFrame *pKeyFrame = InsertOrUpdateKeyFrame(frameId, GetKeyFrames());
    pKeyFrame->transformData = m_CurrentTransformData; // Record current state.
    pKeyFrame->transformData.rotation = rotation;

    Apply(frameId);
}