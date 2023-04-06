//
// Created by VincentZhang on 2023-04-06.
//

#ifndef HUAHUOENGINEV2_KEYFRAMECURVE_H
#define HUAHUOENGINEV2_KEYFRAMECURVE_H

#include "Math/Vector2f.h"
#include "Serialize/SerializeUtility.h"

struct KeyFrameCurvePoint {
    float value;
    int frameId;

    Vector2f handleIn;
    Vector2f handleOut;

    DECLARE_SERIALIZE_NO_PPTR(KeyFrameCurvePoint);
};

template<class TransferFunction>
inline void KeyFrameCurvePoint::Transfer(TransferFunction& t)
{
    t.AddMetaFlag(kTransferUsingFlowMappingStyle);
    t.Transfer(value, "value");
    t.Transfer(frameId, "frameId");
    t.Transfer(handleIn, "handleIn");
    t.Transfer(handleOut, "handleOut");
}

class KeyFrameCurve {
public:
    DECLARE_SERIALIZE_NO_PPTR(KeyFrameCurve);

    size_t GetTotalPoints(){
        return mCurvePoints.size();
    }

    KeyFrameCurvePoint* GetKeyFrameCurvePoint(int idx){
        if(idx >= mCurvePoints.size())
            return NULL;
        return &mCurvePoints[idx];
    }

private:
    std::vector<KeyFrameCurvePoint> mCurvePoints;
};

template<class TransferFunction>
inline void KeyFrameCurve::Transfer(TransferFunction& t)
{
    t.AddMetaFlag(kTransferUsingFlowMappingStyle);
    t.Transfer(mCurvePoints, "mCurvePoints");
}

#endif //HUAHUOENGINEV2_KEYFRAMECURVE_H
