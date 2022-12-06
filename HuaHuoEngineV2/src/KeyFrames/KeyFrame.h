//
// Created by VincentZhang on 2022-12-05.
//

#ifndef HUAHUOENGINEV2_KEYFRAME_H
#define HUAHUOENGINEV2_KEYFRAME_H

#include "BaseClasses/PPtr.h"
#include <vector>

extern const int MAX_FRAMES;

class KeyFrameInfo;
class AbstractFrameState;

class KeyFrame {
public:
    KeyFrame() : frameId(-1),keyFrameIdentifier(-1) {}  // Default ctor is intentionally empty for performance reasons
    KeyFrame(const KeyFrame &v) :
            keyFrameIdentifier(v.keyFrameIdentifier),
            frameId(v.frameId),
            frameState(v.frameState) {}   // Necessary for correct optimized GCC codegen

    int GetKeyFrameIdentifier() const;

    int GetFrameId() const;

    void SetFrameId(int frameId);

    void SetFrameState(AbstractFrameState* frameState);

    AbstractFrameState *GetFrameState() const;

    DECLARE_SERIALIZE(KeyFrame);

    bool operator<(const KeyFrame k1) {
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

bool KeyFrameEq(const KeyFrame &k1, const KeyFrame &k2);

bool operator<(const KeyFrame &k1, const KeyFrame &k2);

template<class T>
class KeyFrameManager {
    DECLARE_SERIALIZE(KeyFrameManager)

public:
    virtual int GetMaxFrameId() {
        int maxFrameId = -1;
        for (auto keyframe: m_KeyFrameData) {
            if (keyframe.GetFrameId() > maxFrameId) {
                maxFrameId = keyframe.GetFrameId();
            }
        }

        return maxFrameId;
    }

    virtual int GetMinFrameId() {
        int minFrameId = MAX_FRAMES;
        for (auto keyframe: m_KeyFrameData) {
            if (keyframe.GetFrameId() < minFrameId) {
                minFrameId = keyframe.GetFrameId();
            }
        }

        return minFrameId;
    }

    virtual void AddAnimationOffset(int offsetFrames) {
        for (auto keyframeItr = m_KeyFrameData.begin(); keyframeItr != m_KeyFrameData.end(); keyframeItr++) {
            if (keyframeItr->GetFrameId() >= 0) {
                keyframeItr->SetFrameId(max(0, keyframeItr->GetFrameId() + offsetFrames));
                keyframeItr->SetFrameId(min(MAX_FRAMES, keyframeItr->GetFrameId()));
            }
        }
    }

    std::vector<T> &GetKeyFrames() {
        return m_KeyFrameData;
    }

    std::vector<KeyFrameInfo*> GetKeyFrameInfos(){
        vector<KeyFrameInfo*> keyframeInfos;
        for(T keyFrame: m_KeyFrameData){
            keyframeInfos.push_back(&keyFrame);
        }

        return keyframeInfos;
    }

private:
    std::vector<T> m_KeyFrameData;
};


struct KeyFrameInfo{
protected:
    int keyFrameId;

public:
    KeyFrameInfo(): keyFrameId(-1){

    }

    KeyFrame& GetKeyFrame();

    int GetFrameId() {
        return GetKeyFrame().GetFrameId();
    }

    void SetFrameId(int frameId){
        GetKeyFrame().SetFrameId(frameId);
    }

    void SetFrameState(AbstractFrameState* frameState){
        GetKeyFrame().SetFrameState(frameState);
    }
};

#endif //HUAHUOENGINEV2_KEYFRAME_H
