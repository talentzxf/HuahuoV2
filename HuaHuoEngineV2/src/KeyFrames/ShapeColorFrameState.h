//
// Created by VincentZhang on 2022-06-16.
//

#ifndef HUAHUOENGINEV2_SHAPECOLORFRAMESTATE_H
#define HUAHUOENGINEV2_SHAPECOLORFRAMESTATE_H


#include "FrameState.h"

class ColorKeyFrame{
public:
    int frameId;
    ColorRGBAf color;
    DECLARE_SERIALIZE_OPTIMIZE_TRANSFER(ColorKeyFrame)
};

template<class TransferFunction> void ColorKeyFrame::Transfer(TransferFunction &transfer){
    TRANSFER(frameId);
    TRANSFER(color);
}


class ShapeColorFrameState: public AbstractFrameState{
    REGISTER_CLASS(ShapeColorFrameState);
    DECLARE_OBJECT_SERIALIZE();
public:
    ShapeColorFrameState(MemLabelId memLabelId, ObjectCreationMode creationMode)
            :Super(memLabelId, creationMode)
    {

    }

    virtual bool Apply(int frameId) override;

private:
    std::vector<ColorKeyFrame> m_KeyFrames;
};



#endif //HUAHUOENGINEV2_SHAPECOLORFRAMESTATE_H
