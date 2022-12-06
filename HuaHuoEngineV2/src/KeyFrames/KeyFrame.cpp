//
// Created by VincentZhang on 2022-12-05.
//

#include "ObjectStore.h"
#include "KeyFrame.h"

KeyFrame &KeyFrameInfo::GetKeyFrame() {
    if(this->keyFrameId <= 0){
        this->keyFrameId = GetDefaultObjectStoreManager()->ProduceKeyFrame();
    }
    return GetDefaultObjectStoreManager()->GetKeyFrameById(this->keyFrameId);
}

AbstractFrameState *KeyFrame::GetFrameState() const {
    return frameState;
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

void KeyFrame::SetFrameState(AbstractFrameState *frameState) {
    this->frameState = frameState;
}

bool KeyFrameEq(const KeyFrame &k1, const KeyFrame &k2) {
    return k1.GetKeyFrameIdentifier() == k2.GetKeyFrameIdentifier();
}

bool operator<(const KeyFrame &k1, const KeyFrame &k2) {
    return k1.GetFrameId() < k2.GetFrameId();
}
