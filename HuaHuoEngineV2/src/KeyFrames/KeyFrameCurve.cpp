//
// Created by VincentZhang on 2023-04-06.
//

#include "KeyFrameCurve.h"
#include "CustomComponent.h"
#include "Layer.h"

void KeyFrameCurve::SetFrameState(CustomFrameState *frameState) {
    this->mFrameState = frameState;
}

void KeyFrameCurve::SetValue(int frameId, float value){
    if(mFrameState->GetBaseShape()->GetLayer()->GetCurrentFrame() != frameId) {
        printf("Object Frame Id: %d, setFrameId: %d\n", mFrameState->GetBaseShape()->GetLayer()->GetCurrentFrame(), frameId);
        printf("FrameId doesn't match!!!");
        throw "FrameId doesn't match!!!!";
    }

    if(mFrameState->GetInstanceID() != InstanceID_None){
        printf("Setting value:%f for this frame state\n", value);
        mFrameState->SetFloatValue(value);
    }else{
        printf("Frame state is not valid\n");
    }
}