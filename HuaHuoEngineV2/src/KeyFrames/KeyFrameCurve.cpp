//
// Created by VincentZhang on 2023-04-06.
//

#include "KeyFrameCurve.h"
#include "CustomComponent.h"
#include "Layer.h"

void KeyFrameCurve::SetValue(int frameId, float value){
    if(setValueCallBackFunc){
        setValueCallBackFunc(frameId, value);
    }
}

void KeyFrameCurve::SetValueByIndex(int index, int frameId, float value){
//    if(mFrameState->GetBaseShape()->GetLayer()->GetCurrentFrame() != frameId) {
//        printf("Object Frame Id: %d, setFrameId: %d\n", mFrameState->GetBaseShape()->GetLayer()->GetCurrentFrame(), frameId);
//        printf("FrameId doesn't match!!!");
//        throw "FrameId doesn't match!!!!";
//    }
//
//    if(mFrameState->GetInstanceID() != InstanceID_None){
//        printf("Setting value:%f for this frame state\n", value);
//        mFrameState->SetFloatValueByIndex(index, frameId, value);
//    }else{
//        printf("Frame State is not set for the key frame curve.\n");
//        throw "Frame State is not set???";
//    }
    if(setValueByIndexCallbackFunc){
        setValueByIndexCallbackFunc(index, frameId, value);
    }

    mCurvePoints[index].SetFrameId(frameId);
    mCurvePoints[index].SetValue(value);
}