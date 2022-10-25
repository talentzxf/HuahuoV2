//
// Created by VincentZhang on 2022-08-22.
//

#include "ShapeStrokeWidthFrameState.h"

IMPLEMENT_REGISTER_CLASS(ShapeStrokeWidthFrameState, 10019);

IMPLEMENT_OBJECT_SERIALIZE(ShapeStrokeWidthFrameState);

INSTANTIATE_TEMPLATE_TRANSFER(ShapeStrokeWidthFrameState);

template <class TransferFunction>
void ShapeStrokeWidthFrameState::Transfer(TransferFunction &transfer) {
    AbstractFrameStateWithKeyType::Transfer(transfer); // Can't user super, because AbstractFrameStateWithKeyType is a template class and can't be chained in the RTTI.
}

bool ShapeStrokeWidthFrameState::Apply(int frameId) {
    std::pair<StrokeWidthKeyFrame *, StrokeWidthKeyFrame *> resultKeyFrames;
    if (FindKeyFramePair(frameId, GetKeyFrames(), resultKeyFrames)) {
        this->isValidFrame = true;
        StrokeWidthKeyFrame *k1 = resultKeyFrames.first;
        StrokeWidthKeyFrame *k2 = resultKeyFrames.second;

        if (k2 == NULL || k1->frameId == k2->frameId) {
            this->m_CurrentStrokeWidth = k1->strokeWidth;
        } else {
            float ratio = float(frameId - k1->frameId) / float(k2->frameId - k1->frameId);

            this->m_CurrentStrokeWidth = Lerp(k1->strokeWidth, k2->strokeWidth, ratio);
        }
        return true;
    }

    return false;
}

void ShapeStrokeWidthFrameState::RecordStrokeWidth(int frameId, float strokeWidth) {
    StrokeWidthKeyFrame *pKeyFrame = InsertOrUpdateKeyFrame(frameId, GetKeyFrames());
    pKeyFrame->strokeWidth = strokeWidth;
    Apply(frameId);
}