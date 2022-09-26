//
// Created by VincentZhang on 2022-06-21.
//

#ifndef HUAHUOENGINEV2_SHAPESEGMENTFRAMESTATE_H
#define HUAHUOENGINEV2_SHAPESEGMENTFRAMESTATE_H


#include "FrameState.h"

struct SegmentKeyFrame {
    int frameId;

    std::vector<Vector3f> positionArray;
    std::vector<Vector3f> handleInArray;
    std::vector<Vector3f> handleOutArray;

    DECLARE_SERIALIZE(SegmentKeyFrame)

    int GetFrameId() {
        return frameId;
    }

    int GetTotalSegments() {
        return positionArray.size();
    }

    Vector3f *GetPosition(long segmentId) {
        return &positionArray[segmentId];
    }

    Vector3f *GetHandleIn(long segmentId) {
        return &handleInArray[segmentId];
    }

    Vector3f *GetHandleOut(long segmentId) {
        return &handleOutArray[segmentId];
    }

    void removeSegment(int index) {
        positionArray.erase(positionArray.begin() + index);
        handleInArray.erase(handleInArray.begin() + index);
        handleOutArray.erase(handleOutArray.begin() + index);
    }
};

template<class TransferFunction>
void SegmentKeyFrame::Transfer(TransferFunction &transfer) {
    TRANSFER(frameId);
    TRANSFER(positionArray);
    TRANSFER(handleInArray);
    TRANSFER(handleOutArray);
}

class ShapeSegmentFrameState : public AbstractFrameState {
REGISTER_CLASS(ShapeSegmentFrameState);

DECLARE_OBJECT_SERIALIZE();
public:
    ShapeSegmentFrameState(MemLabelId memLabelId, ObjectCreationMode creationMode)
            : Super(memLabelId, creationMode) {

    }

    virtual bool Apply(int frameId) override;

    void RecordSegments(int currentFrameId, float segmentBuffer[], int size);

    int GetSegmentCount() {
        return m_currentPositionArray.size();
    }

    Vector3f *GetSegmentPosition(int segmentId) {
        return &m_currentPositionArray[segmentId];
    }

    Vector3f *GetSegmentHandleIn(int segmentId) {
        return &m_currentHandleInArray[segmentId];
    }

    Vector3f *GetSegmentHandleOut(int segmentId) {
        return &m_currentHandleOutArray[segmentId];
    }

    int GetKeyFrameCount() {
        return m_KeyFrames.size();
    }

    void RemoveSegment(int index);

    SegmentKeyFrame *GetSegmentKeyFrameAtFrameIndex(int keyFrameIndex) {
        if (keyFrameIndex >= m_KeyFrames.size()) {
            printf("Error, totally %d keyframes, but want to get the %dth keyframe.\n", m_KeyFrames.size(),
                   keyFrameIndex);
            return NULL;
        }

        return &m_KeyFrames[keyFrameIndex];
    };

    friend class BaseShape;

    virtual int GetMaxFrameId(){
        int maxFrameId = -1;
        for(SegmentKeyFrame keyframe: m_KeyFrames){
            if(keyframe.frameId > maxFrameId){
                maxFrameId = keyframe.frameId;
            }
        }

        return maxFrameId;
    }
private:
    std::vector<Vector3f> m_currentPositionArray;
    std::vector<Vector3f> m_currentHandleInArray;
    std::vector<Vector3f> m_currentHandleOutArray;

    std::vector<SegmentKeyFrame> m_KeyFrames;
};


#endif //HUAHUOENGINEV2_SHAPESEGMENTFRAMESTATE_H
