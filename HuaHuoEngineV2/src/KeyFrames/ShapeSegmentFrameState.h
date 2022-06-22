//
// Created by VincentZhang on 2022-06-21.
//

#ifndef HUAHUOENGINEV2_SHAPESEGMENTFRAMESTATE_H
#define HUAHUOENGINEV2_SHAPESEGMENTFRAMESTATE_H


#include "FrameState.h"

struct SegmentKeyFrame{
    int frameId;

    std::vector<Vector3f> positionArray;
    std::vector<Vector3f> handleInArray;
    std::vector<Vector3f> handleOutArray;

    DECLARE_SERIALIZE(SegmentKeyFrame)
};

template<class TransferFunction> void SegmentKeyFrame::Transfer(TransferFunction &transfer){
    TRANSFER(frameId);
    TRANSFER(positionArray);
    TRANSFER(handleInArray);
    TRANSFER(handleOutArray);
}

class ShapeSegmentFrameState: public AbstractFrameState {
    REGISTER_CLASS(ShapeSegmentFrameState);
    DECLARE_OBJECT_SERIALIZE();
public:
    ShapeSegmentFrameState(MemLabelId memLabelId, ObjectCreationMode creationMode)
    :Super(memLabelId, creationMode)
    {

    }

    virtual bool Apply(int frameId) override;
    void RecordSegments(int currentFrameId, float segmentBuffer[], int size);

    int GetSegmentCount(){
        return m_currentPositionArray.size();
    }

    Vector3f* GetSegmentPositions(int segmentId){
        return &m_currentPositionArray[segmentId];
    }

    Vector3f* GetSegmentHandleIns(int segmentId){
        return &m_currentHandleInArray[segmentId];
    }

    Vector3f* GetSegmentHandleOuts(int segmentId){
        return &m_currentHandleOutArray[segmentId];
    }

private:
    std::vector<Vector3f> m_currentPositionArray;
    std::vector<Vector3f> m_currentHandleInArray;
    std::vector<Vector3f> m_currentHandleOutArray;

    std::vector<SegmentKeyFrame> m_KeyFrames;
};


#endif //HUAHUOENGINEV2_SHAPESEGMENTFRAMESTATE_H
