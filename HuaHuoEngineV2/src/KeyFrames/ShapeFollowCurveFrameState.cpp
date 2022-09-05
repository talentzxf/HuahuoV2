//
// Created by VincentZhang on 2022-09-05.
//

#include "ShapeFollowCurveFrameState.h"

IMPLEMENT_REGISTER_CLASS(ShapeFollowCurveFrameState, 10020);

IMPLEMENT_OBJECT_SERIALIZE(ShapeFollowCurveFrameState);

INSTANTIATE_TEMPLATE_TRANSFER(ShapeFollowCurveFrameState);

template<class TransferFunction>
void ShapeFollowCurveFrameState::Transfer(TransferFunction &transfer) {
    Super::Transfer(transfer);
    TRANSFER(m_KeyFrames);
}

ShapeFollowCurveData Lerp(ShapeFollowCurveData &k1, ShapeFollowCurveData &k2, float ratio) {
    ShapeFollowCurveData resultData;

    if(k1.followCurveTarget.IsValid() && k2.followCurveTarget.IsValid() && k1.followCurveTarget == k2.followCurveTarget){
        resultData.followCurveTarget = k1.followCurveTarget;
        resultData.lengthRatio = Lerp(k1.lengthRatio, k2.lengthRatio, ratio);
    }

    return resultData;
}

bool ShapeFollowCurveFrameState::Apply(int frameId) {
    std::pair<ShapeFollowCurveKeyFrame *, ShapeFollowCurveKeyFrame *> resultKeyFrames;
    if (FindKeyFramePair(frameId, this->m_KeyFrames, resultKeyFrames)) {
        this->isValidFrame = true;
        ShapeFollowCurveKeyFrame *k1 = resultKeyFrames.first;
        ShapeFollowCurveKeyFrame *k2 = resultKeyFrames.second;

        if (k2 == NULL || k2->frameId == k1->frameId ) { // Avoid 0/0 during ratio calculation. Or beyond the last frame. k1 is the last frame.
            this->m_CurrentShapeFollowCurveData = k1->followCurveData;
        }
        else
        {
            float ratio = float(frameId - k1->frameId) / float(k2->frameId - k1->frameId);

            this->m_CurrentShapeFollowCurveData = Lerp(k1->followCurveData, k2->followCurveData, ratio);
        }

        return true;
    }

    return false;
}

void ShapeFollowCurveFrameState::RecordLengthRatio(int frameId, float lengthRatio) {
    ShapeFollowCurveKeyFrame *pKeyFrame = InsertOrUpdateKeyFrame(frameId, this->m_KeyFrames);
    pKeyFrame->followCurveData = m_CurrentShapeFollowCurveData;
    pKeyFrame->followCurveData.lengthRatio = lengthRatio;

    Apply(frameId);
}

void ShapeFollowCurveFrameState::RecordTargetShape(int frameId, BaseShape *targetCurve) {
    ShapeFollowCurveKeyFrame *pKeyFrame = InsertOrUpdateKeyFrame(frameId, this->m_KeyFrames);
    pKeyFrame->followCurveData = m_CurrentShapeFollowCurveData;
    pKeyFrame->followCurveData.followCurveTarget = targetCurve;

    Apply(frameId);
}