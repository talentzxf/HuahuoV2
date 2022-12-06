//
// Created by VincentZhang on 2022-12-05.
//

#include "ObjectStore.h"
#include "KeyFrame.h"

void KeyFrame::AssignKeyFrameIdentifier() {
    if (keyFrameIdentifier <= 0)
        this->keyFrameIdentifier = GetDefaultObjectStoreManager()->GetAndIncreaseMaxKeyFrameIdentifier();
}

AbstractFrameState *KeyFrame::GetFrameState() const {
    return frameState;
}

KeyFrame::KeyFrame(int frameId, AbstractFrameState *frameState) {
    this->frameId = frameId;
    this->frameState = frameState;

    AssignKeyFrameIdentifier();
}

int KeyFrame::GetKeyFrameIdentifier() const {
    return keyFrameIdentifier;
}

int KeyFrame::GetFrameId() const {
    return frameId;
}

void KeyFrame::SetFrameId(int frameId) {
    KeyFrame::frameId = frameId;
    AssignKeyFrameIdentifier();
}

void KeyFrame::SetFrameState(AbstractFrameState *frameState) {
    this->frameState = frameState;
    AssignKeyFrameIdentifier();
}

bool KeyFrameEq(const KeyFrame &k1, const KeyFrame &k2) {
    return k1.GetKeyFrameIdentifier() == k2.GetKeyFrameIdentifier();
}

bool operator<(const KeyFrame &k1, const KeyFrame &k2) {
    return k1.GetFrameId() < k2.GetFrameId();
}
