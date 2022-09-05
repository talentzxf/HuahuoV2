//
// Created by VincentZhang on 2022-09-05.
//

#ifndef HUAHUOENGINEV2_SHAPEFOLLOWCURVEFRAMESTATE_H
#define HUAHUOENGINEV2_SHAPEFOLLOWCURVEFRAMESTATE_H

#include "Shapes/BaseShape.h"

class ShapeFollowCurveData{
public:
    PPtr<BaseShape> followCurveTarget;
    float lengthRatio;

    DECLARE_SERIALIZE_OPTIMIZE_TRANSFER(ShapeFollowCurveData)

    ShapeFollowCurveData(): lengthRatio(-1.0)
    {

    }
};

template<class TransferFunction> void ShapeFollowCurveData::Transfer(TransferFunction &transfer){
    TRANSFER(followCurveTarget);
    TRANSFER(lengthRatio);
}

ShapeFollowCurveData Lerp(ShapeFollowCurveData& k1, ShapeFollowCurveData& k2, float ratio);

struct ShapeFollowCurveKeyFrame{
    int frameId;
    ShapeFollowCurveData followCurveData;

    DECLARE_SERIALIZE_OPTIMIZE_TRANSFER(TransformKeyFrame)
};

template<class TransferFunction> void ShapeFollowCurveKeyFrame::Transfer(TransferFunction &transfer){
    TRANSFER(frameId);
    TRANSFER(followCurveData);
}

class ShapeFollowCurveFrameState: public AbstractFrameState {
    REGISTER_CLASS(ShapeFollowCurveFrameState);
    DECLARE_OBJECT_SERIALIZE();
public:
    ShapeFollowCurveFrameState(MemLabelId memLabelId, ObjectCreationMode creationMode)
    :Super(memLabelId, creationMode)
    {

    }

    virtual bool Apply(int frameId) override;

    BaseShape* GetFollowTarget(){
        return m_CurrentShapeFollowCurveData.followCurveTarget;
    }

    float GetLengthRatio(){
        return m_CurrentShapeFollowCurveData.lengthRatio;
    }

    void RecordTargetShape(int frameId, BaseShape* targetCurve);
    void RecordLengthRatio(int frameId, float lengthRatio);

private:
    std::vector<ShapeFollowCurveKeyFrame> m_KeyFrames;
    ShapeFollowCurveData m_CurrentShapeFollowCurveData;
};


#endif //HUAHUOENGINEV2_SHAPEFOLLOWCURVEFRAMESTATE_H
