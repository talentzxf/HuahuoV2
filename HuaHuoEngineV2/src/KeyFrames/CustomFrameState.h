//
// Created by VincentZhang on 10/23/2022.
//

#ifndef HUAHUOENGINEV2_CUSTOMFRAMESTATE_H
#define HUAHUOENGINEV2_CUSTOMFRAMESTATE_H

#include <vector>
#include <string>
#include "FrameState.h"

class CustomFloatKeyFrame{
public:
    std::vector<float> frameValues;

    DECLARE_SERIALIZE_OPTIMIZE_TRANSFER(CustomFloatKeyFrame)
};

template <class TransferFunction> void CustomFloatKeyFrame::Transfer(TransferFunction &transfer) {
    TRANSFER(frameValues);
}

CustomFloatKeyFrame Lerp(CustomFloatKeyFrame& k1, CustomFloatKeyFrame& k2, float ratio);

class CustomFrameState: public AbstractFrameStateWithKeyType<CustomFloatKeyFrame>{
    REGISTER_CLASS(CustomFrameState);
    DECLARE_OBJECT_SERIALIZE()
public:
    CustomFrameState(MemLabelId memLabelId, ObjectCreationMode creationMode)
    : AbstractFrameStateWithKeyType<CustomFloatKeyFrame>(memLabelId, creationMode){

    }
    
};


#endif //HUAHUOENGINEV2_CUSTOMFRAMESTATE_H
