//
// Created by VincentZhang on 2022-12-05.
//

#include "ObjectStore.h"
#include "KeyFrame.h"

void AbstractKeyFrame::SetObjectStore(ObjectStore *pStore) {
    mStorePPtr = pStore;
}

KeyFrame &AbstractKeyFrame::GetKeyFrame() {
    if (this->keyFrameId <= 0) {
        if (!mStorePPtr.IsValid()) {
            mStorePPtr = GetDefaultObjectStoreManager()->GetCurrentStore();
        }
        this->keyFrameId = mStorePPtr->ProduceKeyFrame();
    }


    return mStorePPtr->GetKeyFrameById(this->keyFrameId);
}

void AbstractKeyFrame::SetFrameState(AbstractFrameState *frameState) {
    Assert(frameState != NULL);
    ObjectStore *pStore = frameState->GetObjectStore();
    this->SetObjectStore(pStore);
    GetKeyFrame().SetFrameState(frameState);
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

BaseShape *KeyFrame::GetBaseShape() {
    if (frameState.IsValid()) {
        return frameState->GetBaseShape();
    }

    return baseShape;
}

void KeyFrame::SetBaseShape(BaseShape *shape) {
    this->baseShape = shape;
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
