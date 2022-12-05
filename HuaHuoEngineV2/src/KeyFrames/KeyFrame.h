//
// Created by VincentZhang on 2022-12-05.
//

#ifndef HUAHUOENGINEV2_KEYFRAME_H
#define HUAHUOENGINEV2_KEYFRAME_H

#include "BaseClasses/PPtr.h"

class AbstractFrameState;

class KeyFrame {
public:
    KeyFrame() {}  // Default ctor is intentionally empty for performance reasons
    KeyFrame(const KeyFrame& v) :
        keyFrameIdentifier(v.keyFrameIdentifier),
        frameId(v.frameId),
        frameState(v.frameState) {}   // Necessary for correct optimized GCC codegen
    KeyFrame(int frameId, AbstractFrameState* frameState);

    int GetKeyFrameIdentifier() const;

    int GetFrameId() const;
    void SetFrameId(int frameId);

    AbstractFrameState* GetFrameState() const {
        return frameState;
    }

    DECLARE_SERIALIZE(KeyFrame);

    bool operator<(const KeyFrame k1){
        return GetFrameId() < k1.GetFrameId();
    }

private:
    /**
     * The keyFrameIdentifier is different than the frameId it represents!
     * keyFrameIdentifier won't be changed after the object is created but frameId might be changed in Editor UI.
     */
    int keyFrameIdentifier;
    int frameId;
    PPtr<AbstractFrameState> frameState;
};

template<class TransferFunction>
void KeyFrame::Transfer(TransferFunction &transfer) {
    TRANSFER(keyFrameIdentifier);
    TRANSFER(frameId);
    TRANSFER(frameState);
}

bool KeyFrameEq(const KeyFrame& k1, const KeyFrame& k2);

bool operator<(const KeyFrame& k1, const KeyFrame& k2);
#endif //HUAHUOENGINEV2_KEYFRAME_H
