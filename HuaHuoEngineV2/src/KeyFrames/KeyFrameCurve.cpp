//
// Created by VincentZhang on 2023-04-06.
//

#include "KeyFrameCurve.h"

void KeyFrameCurve::SetValue(int frameId, float value){
    if(setValueCallBackFunc){
        setValueCallBackFunc(frameId, value);
    }
}

void KeyFrameCurve::SetValueByIndex(int index, int frameId, float value){
    if(setValueByIndexCallbackFunc){
        setValueByIndexCallbackFunc(index, frameId, value);
    }

    mCurvePoints[index].SetFrameId(frameId);
    mCurvePoints[index].SetValue(value);
}