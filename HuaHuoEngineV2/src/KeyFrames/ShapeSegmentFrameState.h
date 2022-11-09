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

    SegmentKeyFrame():frameId(-1){

    }
};

template<class TransferFunction>
void SegmentKeyFrame::Transfer(TransferFunction &transfer) {
    TRANSFER(frameId);
    TRANSFER(positionArray);
    TRANSFER(handleInArray);
    TRANSFER(handleOutArray);
}

class ShapeSegmentFrameState : public AbstractFrameStateWithKeyType<SegmentKeyFrame> {
REGISTER_CLASS(ShapeSegmentFrameState);

DECLARE_OBJECT_SERIALIZE();
public:
    ShapeSegmentFrameState(MemLabelId memLabelId, ObjectCreationMode creationMode)
            : AbstractFrameStateWithKeyType(memLabelId, creationMode) {

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
        return GetKeyFrames().size();
    }

    void RemoveSegment(int index);

    SegmentKeyFrame *GetSegmentKeyFrameAtFrameIndex(int keyFrameIndex) {
        if (keyFrameIndex >= GetKeyFrames().size()) {
            printf("Error, totally %ld keyframes, but want to get the %dth keyframe.\n", GetKeyFrames().size(),
                   keyFrameIndex);
            return NULL;
        }

        return &GetKeyFrames()[keyFrameIndex];
    };

    friend class BaseShape;

private:
    std::vector<Vector3f> m_currentPositionArray;
    std::vector<Vector3f> m_currentHandleInArray;
    std::vector<Vector3f> m_currentHandleOutArray;
};


#endif //HUAHUOENGINEV2_SHAPESEGMENTFRAMESTATE_H
