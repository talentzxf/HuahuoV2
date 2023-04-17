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
        throw "FrameId doesn't match!!!!";
    }

    AddValue(frameId, value);
    if(mFrameState->IsValid()){
        mFrameState->SetFloatValue(value);
    }
}