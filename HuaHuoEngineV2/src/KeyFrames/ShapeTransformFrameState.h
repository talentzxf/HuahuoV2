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

    DECLARE_SERIALIZE_OPTIMIZE_TRANSFER(TransformData)
};

template<class TransferFunction> void TransformData::Transfer(TransferFunction &transfer){
    TRANSFER(position);
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

    void RecordPosition(int frameId, float x, float y, float z);

private:
    std::vector<TransformKeyFrame> m_KeyFrames;

    TransformData m_CurrentTransformData;
};


#endif //HUAHUOENGINEV2_SHAPETRANSFORMFRAMESTATE_H
