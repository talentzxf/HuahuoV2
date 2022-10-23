//
// Created by VincentZhang on 10/23/2022.
//

#include "CustomFrameState.h"

CustomFloatKeyFrame Lerp(CustomFloatKeyFrame& k1, CustomFloatKeyFrame& k2, float ratio){
    CustomFloatKeyFrame resultData;

    if(k1.frameValues.size() != k2.frameValues.size()){
        Assert("value size mismatch!!!");
        return resultData;
    }

    int valueSize = k1.frameValues.size();
    resultData.frameValues.reserve(valueSize);

    for(int i = 0 ; i < valueSize; i++){
        resultData.frameValues[i] = Lerp(k1.frameValues[i], k2.frameValues[i], ratio);
    }

    return resultData;
}