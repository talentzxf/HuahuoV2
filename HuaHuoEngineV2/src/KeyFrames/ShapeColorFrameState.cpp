//
// Created by VincentZhang on 2022-06-16.
//

#include "ShapeColorFrameState.h"


IMPLEMENT_REGISTER_CLASS(ShapeColorFrameState, 10009);

IMPLEMENT_OBJECT_SERIALIZE(ShapeColorFrameState);

INSTANTIATE_TEMPLATE_TRANSFER(ShapeColorFrameState);

template<class TransferFunction>
void ShapeColorFrameState::Transfer(TransferFunction &transfer) {
    Super::Transfer(transfer);
}

// TODO: Implement this
bool ShapeColorFrameState::Apply(int frameId) {
    std::pair<ColorKeyFrame *, ColorKeyFrame *> resultKeyFrames;
    if (FindKeyFramePair(frameId, GetKeyFrames(), resultKeyFrames)) {
        this->isValidFrame = true;
        ColorKeyFrame *k1 = resultKeyFrames.first;
        ColorKeyFrame *k2 = resultKeyFrames.second;

        if (k2 == NULL || k1->frameId == k2->frameId) {
            this->m_CurrentColor = k1->color;
        } else {
            float ratio = float(frameId - k1->frameId) / float(k2->frameId - k1->frameId);

            this->m_CurrentColor = Lerp(k1->color, k2->color, ratio);
        }
        return true;
    }

    return false;
}

void ShapeColorFrameState::RecordColor(int frameId, float r, float g, float b, float a) {
    ColorKeyFrame *pKeyFrame = InsertOrUpdateKeyFrame(frameId, GetKeyFrames());
    pKeyFrame->color.Set(r, g, b, a);

    Apply(frameId);
}