//
// Created by VincentZhang on 2022-06-16.
//

#ifndef HUAHUOENGINEV2_SHAPETRANSFORMFRAMESTATE_H
#define HUAHUOENGINEV2_SHAPETRANSFORMFRAMESTATE_H
#include "Math/Vector3f.h"
#include "FrameState.h"

class TransformData{
public:
    Vector3f position;
    Vector3f scale;
    float rotation;

    Vector3f centerOffset;

    DECLARE_SERIALIZE_OPTIMIZE_TRANSFER(TransformData)

    TransformData()
        :position(0.0,0.0,0.0)
        ,scale(1.0,1.0,1.0)
        ,rotation(0.0f)
        ,centerOffset(0.0f, 0.0f, 0.0f)
    {

    }
};

template<class TransferFunction> void TransformData::Transfer(TransferFunction &transfer){
    TRANSFER(position);
    TRANSFER(scale);
    TRANSFER(rotation);
    TRANSFER(centerOffset);
}

TransformData Lerp(TransformData& k1, TransformData& k2, float ratio);

struct TransformKeyFrame{
    int frameId;
    TransformData transformData;

    DECLARE_SERIALIZE_OPTIMIZE_TRANSFER(TransformKeyFrame)
};

template<class TransferFunction> void TransformKeyFrame::Transfer(TransferFunction &transfer){
    TRANSFER(frameId);
    TRANSFER(transformData);
}

class ShapeTransformFrameState: public AbstractFrameState{
    REGISTER_CLASS(ShapeTransformFrameState);
    DECLARE_OBJECT_SERIALIZE();
public:
    ShapeTransformFrameState(MemLabelId memLabelId, ObjectCreationMode creationMode)
            :Super(memLabelId, creationMode)
    {

    }

    virtual bool Apply(int frameId) override;

    Vector3f* GetPosition(){
        if(isValidFrame)
            return &m_CurrentTransformData.position;
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
        return NULL;
    }

    Vector3f* GetCenterOffset(){
        if(isValidFrame)
            return &m_CurrentTransformData.centerOffset;
        return NULL;
    }

    void RecordPosition(int frameId, float x, float y, float z);

    void RecordScale(int frameId, float xScale, float yScale, float zScale);

    void RecordRotation(int frameId, float rotation);

    void RecordCenterOffset(int frameId, float x, float y, float z);

    friend class BaseShape;
private:
    std::vector<TransformKeyFrame> m_KeyFrames;

    TransformData m_CurrentTransformData;
};


#endif //HUAHUOENGINEV2_SHAPETRANSFORMFRAMESTATE_H
