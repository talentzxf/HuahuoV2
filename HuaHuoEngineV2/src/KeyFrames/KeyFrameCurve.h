//
// Created by VincentZhang on 2023-04-06.
//

#ifndef HUAHUOENGINEV2_KEYFRAMECURVE_H
#define HUAHUOENGINEV2_KEYFRAMECURVE_H

#include "Math/Vector2f.h"
#include "Serialize/SerializeUtility.h"
#include <vector>

class KeyFrameCurvePoint {
public:
    KeyFrameCurvePoint() : value(-1.0), frameId(-1) {

    }

    DECLARE_SERIALIZE_NO_PPTR(KeyFrameCurvePoint);

    void SetValue(float value){
        this->value = value;
    }

    void SetFrameId(long frameId){
        this->frameId = frameId;
    }

    float GetValue() {
        return value;
    }

    long GetFrameId() {
        return frameId;
    }

    Vector2f *GetHandleIn() {
        return &handleIn;
    }

    Vector2f *GetHandleOut() {
        return &handleOut;
    }

private:
    float value;
    int frameId;

    Vector2f handleIn;
    Vector2f handleOut;
};

template<class TransferFunction>
inline void KeyFrameCurvePoint::Transfer(TransferFunction &t) {
    t.AddMetaFlag(kTransferUsingFlowMappingStyle);
    t.Transfer(value, "value");
    t.Transfer(frameId, "frameId");
    t.Transfer(handleIn, "handleIn");
    t.Transfer(handleOut, "handleOut");
}

class KeyFrameCurve {
public:
    DECLARE_SERIALIZE_NO_PPTR(KeyFrameCurve);

    size_t GetTotalPoints() {
        return mCurvePoints.size();
    }

    KeyFrameCurvePoint *GetKeyFrameCurvePoint(int idx) {
        if (idx >= mCurvePoints.size())
            return NULL;
        return &mCurvePoints[idx];
    }

    void AddValue(float value, int frameId) {
        KeyFrameCurvePoint curvePoint;
        curvePoint.SetValue(value);
        curvePoint.SetFrameId(frameId);

        // Find the approporiate position to insert.
        // TODO: User binary search
        auto curPointItr = std::lower_bound(mCurvePoints.begin(), mCurvePoints.end(), frameId,
                                            [](KeyFrameCurvePoint p1, int frameId) {
                                                return p1.GetFrameId() < frameId;
                                            });

        mCurvePoints.insert(curPointItr, curvePoint);
    }

private:
    std::vector<KeyFrameCurvePoint> mCurvePoints;
};

template<class TransferFunction>
inline void KeyFrameCurve::Transfer(TransferFunction &t) {
    t.AddMetaFlag(kTransferUsingFlowMappingStyle);
    t.Transfer(mCurvePoints, "mCurvePoints");
}

#endif //HUAHUOENGINEV2_KEYFRAMECURVE_H
