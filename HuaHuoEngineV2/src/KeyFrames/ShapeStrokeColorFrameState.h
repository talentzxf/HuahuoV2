//
// Created by VincentZhang on 2022-08-22.
//

#ifndef HUAHUOENGINEV2_SHAPESTROKECOLORFRAMESTATE_H
#define HUAHUOENGINEV2_SHAPESTROKECOLORFRAMESTATE_H
#include "ShapeColorFrameState.h"

class ShapeStrokeColorFrameState: public ShapeColorFrameState {
    REGISTER_CLASS(ShapeStrokeColorFrameState);
    DECLARE_OBJECT_SERIALIZE();

public:
    ShapeStrokeColorFrameState(MemLabelId memLabelId, ObjectCreationMode creationMode)
    :Super(memLabelId, creationMode) {
    }
};


#endif //HUAHUOENGINEV2_SHAPESTROKECOLORFRAMESTATE_H
