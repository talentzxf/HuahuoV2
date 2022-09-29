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

    ColorKeyFrame():frameId(-1){

    }
};

template<class TransferFunction> void ColorKeyFrame::Transfer(TransferFunction &transfer){
    TRANSFER(frameId);
    TRANSFER(color);
}


class ShapeColorFrameState: public AbstractFrameStateWithKeyType<ColorKeyFrame>{
    REGISTER_CLASS(ShapeColorFrameState);
    DECLARE_OBJECT_SERIALIZE();
public:
    ShapeColorFrameState(MemLabelId memLabelId, ObjectCreationMode creationMode)
            :AbstractFrameStateWithKeyType(memLabelId, creationMode)
    {

    }

    virtual bool Apply(int frameId) override;

    ColorRGBAf* GetColor(){
        if(isValidFrame)
            return &m_CurrentColor;
        return NULL;
    }

    void RecordColor(int frameId, float r, float g, float b, float a);

    friend class BaseShape;

private:
    ColorRGBAf m_CurrentColor;
    KeyFrameManager<ColorKeyFrame> m_KeyFrames;
};



#endif //HUAHUOENGINEV2_SHAPECOLORFRAMESTATE_H
