//
// Created by VincentZhang on 2022-08-22.
//

#ifndef HUAHUOENGINEV2_SHAPESTROKEWIDTHFRAMESTATE_H
#define HUAHUOENGINEV2_SHAPESTROKEWIDTHFRAMESTATE_H

#include "Serialize/SerializeUtility.h"

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

//class ShapeStrokeWidthFrameState : public AbstractFrameS{
//
//};


#endif //HUAHUOENGINEV2_SHAPESTROKEWIDTHFRAMESTATE_H
