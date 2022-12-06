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

    DECLARE_SERIALIZE(ShapeFollowCurveData)

    ShapeFollowCurveData(): lengthRatio(-1.0)
    {

    }
};

template<class TransferFunction> void ShapeFollowCurveData::Transfer(TransferFunction &transfer){
    TRANSFER(followCurveTarget);
    TRANSFER(lengthRatio);
}

ShapeFollowCurveData Lerp(ShapeFollowCurveData& k1, ShapeFollowCurveData& k2, float ratio);

struct ShapeFollowCurveKeyFrame: public KeyFrameInfo{
    ShapeFollowCurveData followCurveData;

    DECLARE_SERIALIZE(TransformKeyFrame)

    ShapeFollowCurveKeyFrame(){

    }
};

template<class TransferFunction> void ShapeFollowCurveKeyFrame::Transfer(TransferFunction &transfer){
    KeyFrameInfo::Transfer(transfer);
    TRANSFER(followCurveData);
}

class ShapeFollowCurveFrameState: public AbstractFrameStateWithKeyType<ShapeFollowCurveKeyFrame> {
    REGISTER_CLASS(ShapeFollowCurveFrameState);
    DECLARE_OBJECT_SERIALIZE();
public:
    ShapeFollowCurveFrameState(MemLabelId memLabelId, ObjectCreationMode creationMode)
    :AbstractFrameStateWithKeyType(memLabelId, creationMode)
    {

    }

    virtual bool Apply(int frameId) override;

    void RecordTargetShape(int frameId, BaseShape* targetCurve);
    void RecordLengthRatio(int frameId, float lengthRatio);

    BaseShape* GetTargetShape(){
        if(isValidFrame){
            return m_CurrentShapeFollowCurveData.followCurveTarget;
        }

        return NULL;
    }

    float GetLengthRatio(){
        if(isValidFrame){
            return m_CurrentShapeFollowCurveData.lengthRatio;
        }

        return -1.0f;
    }

private:
    ShapeFollowCurveData m_CurrentShapeFollowCurveData;
};


#endif //HUAHUOENGINEV2_SHAPEFOLLOWCURVEFRAMESTATE_H
