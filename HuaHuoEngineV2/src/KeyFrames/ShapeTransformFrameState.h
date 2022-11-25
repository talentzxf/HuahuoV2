//
// Created by VincentZhang on 2022-06-16.
//

#ifndef HUAHUOENGINEV2_SHAPETRANSFORMFRAMESTATE_H
#define HUAHUOENGINEV2_SHAPETRANSFORMFRAMESTATE_H
#include "Math/Vector3f.h"
#include "FrameState.h"

class TransformData{
public:
    Vector3f scale;
    float rotation;
    Vector3f localPivotPosition;
    Vector3f globalPivotPosition;

    DECLARE_SERIALIZE_OPTIMIZE_TRANSFER(TransformData)

    TransformData()
        : scale(1.0,1.0,1.0)
        , rotation(0.0f)
        , localPivotPosition(0.0f, 0.0f, 0.0f)
        , globalPivotPosition(0.0f, 0.0f, 0.0f)
    {

    }
};

template<class TransferFunction> void TransformData::Transfer(TransferFunction &transfer){
    TRANSFER(scale);
    TRANSFER(rotation);
    TRANSFER(localPivotPosition);
    TRANSFER(globalPivotPosition);
}

TransformData Lerp(TransformData& k1, TransformData& k2, float ratio);

struct TransformKeyFrame{
    int frameId;
    TransformData transformData;

    DECLARE_SERIALIZE_OPTIMIZE_TRANSFER(TransformKeyFrame)

    TransformKeyFrame():frameId(-1){

    }
};

template<class TransferFunction> void TransformKeyFrame::Transfer(TransferFunction &transfer){
    TRANSFER(frameId);
    TRANSFER(transformData);
}

class ShapeTransformFrameState: public AbstractFrameStateWithKeyType<TransformKeyFrame>{
    REGISTER_CLASS(ShapeTransformFrameState);
    DECLARE_OBJECT_SERIALIZE();
public:
    ShapeTransformFrameState(MemLabelId memLabelId, ObjectCreationMode creationMode)
            :AbstractFrameStateWithKeyType(memLabelId, creationMode)
    {

    }

    virtual bool Apply(int frameId) override;

    Vector3f* GetLocalPivotPosition(){
        if(isValidFrame)
            return &m_CurrentTransformData.localPivotPosition;
        return NULL;
    }

    Vector3f* GetGlobalPivotPosition(){
        if(isValidFrame)
            return &m_CurrentTransformData.globalPivotPosition;
        return NULL;
    }

    Vector3f* GetScale(){
        if(isValidFrame)
            return &m_CurrentTransformData.scale;
        return NULL;
    }

    float GetRotation(){
        if(isValidFrame)
            return m_CurrentTransformData.rotation;
        return 0.0f;
    }

    void RecordLocalPivotPosition(int frameId, float x, float y, float z);
    void RecordGlobalPivotPosition(int frameId, float x, float y, float z);

    void RecordScale(int frameId, float xScale, float yScale, float zScale);

    void RecordRotation(int frameId, float rotation);
    friend class BaseShape;

    void UpdateTemporaryPosition(float x, float y, float z){
        m_CurrentTransformData.globalPivotPosition.x = x;
        m_CurrentTransformData.globalPivotPosition.y = y;
        m_CurrentTransformData.globalPivotPosition.z = z;
    }

    void UpdateTemporaryRotation(float rotation){
        m_CurrentTransformData.rotation = rotation;
    }

private:
    TransformData m_CurrentTransformData;
};


#endif //HUAHUOENGINEV2_SHAPETRANSFORMFRAMESTATE_H
