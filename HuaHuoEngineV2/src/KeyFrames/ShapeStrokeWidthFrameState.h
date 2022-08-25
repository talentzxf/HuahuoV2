//
// Created by VincentZhang on 2022-08-22.
//

#ifndef HUAHUOENGINEV2_SHAPESTROKEWIDTHFRAMESTATE_H
#define HUAHUOENGINEV2_SHAPESTROKEWIDTHFRAMESTATE_H

#include "Serialize/SerializeUtility.h"
#include "FrameState.h"

class StrokeWidthKeyFrame{
public:
    int frameId;
    float strokeWidth;
    DECLARE_SERIALIZE_OPTIMIZE_TRANSFER(StrokeWidthKeyFrame)
};

template<class TransferFunction> void StrokeWidthKeyFrame::Transfer(TransferFunction &transfer) {
    TRANSFER(frameId);
    TRANSFER(strokeWidth);
}

class ShapeStrokeWidthFrameState : public AbstractFrameState{
    REGISTER_CLASS(ShapeStrokeWidthFrameState);
    DECLARE_OBJECT_SERIALIZE();
public:
    ShapeStrokeWidthFrameState(MemLabelId memLabelId, ObjectCreationMode creationMode)
        :Super(memLabelId, creationMode)
    {
    }

    virtual bool Apply(int frameId) override;

    float GetStrokeWidth(){
        if(isValidFrame)
            return m_CurrentStrokeWidth;
        return NULL;
    }

    void RecordStrokeWidth(int frameId, float strokeWidth);

private:
    float m_CurrentStrokeWidth;
    std::vector<StrokeWidthKeyFrame> m_KeyFrames;
};


#endif //HUAHUOENGINEV2_SHAPESTROKEWIDTHFRAMESTATE_H
