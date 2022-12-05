//
// Created by VincentZhang on 2022-12-05.
//

#include "ObjectStore.h"
#include "KeyFrame.h"

KeyFrame::KeyFrame(int frameId, AbstractFrameState *frameState) {
    this->keyFrameIdentifier = GetDefaultObjectStoreManager()->GetAndIncreaseMaxKeyFrameIdentifier();
    this->frameId = frameId;
    this->frameState = frameState;
}

int KeyFrame::GetKeyFrameIdentifier() const {
    return keyFrameIdentifier;
}

int KeyFrame::GetFrameId() const {
    return frameId;
}

void KeyFrame::SetFrameId(int frameId) {
    KeyFrame::frameId = frameId;
}

bool KeyFrameEq(const KeyFrame &k1, const KeyFrame &k2) {
    return k1.GetKeyFrameIdentifier() == k2.GetKeyFrameIdentifier();
}

bool operator<(const KeyFrame &k1, const KeyFrame &k2) {
    return k1.GetFrameId() < k2.GetFrameId();
}
