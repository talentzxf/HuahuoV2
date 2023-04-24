//
// Created by VincentZhang on 2023-04-21.
//

#ifndef HUAHUOENGINEV2_ABSTRACTFRAMESTATEWITHKEYFRAMECURVE_H
#define HUAHUOENGINEV2_ABSTRACTFRAMESTATEWITHKEYFRAMECURVE_H

#include "FrameState.h"
#include "KeyFrameCurve.h"

template<class T>
class AbstractFrameStateWithKeyFrameCurve : public AbstractFrameStateWithKeyType<T> {
public:
    AbstractFrameStateWithKeyFrameCurve(MemLabelId memLabelId, ObjectCreationMode creationMode)
            : AbstractFrameStateWithKeyType<T>(memLabelId, creationMode) {
    }

protected:
    KeyFrameCurve *GetVectorKeyFrameCurve(int indexOfVector);

    KeyFrameCurve *GetFloatKeyFrameCurve();

    void AddFloatCurveValue(int frameId, float value) {
        mKeyFrameCurve.AddValue(frameId, value);
    }

    void SetVectorKeyFrameCurveValue(int frameId, float x, float y, float z) {
        if (mKeyFrameCurves.size() != 3) {
            mKeyFrameCurves.reserve(3);
        }

        mKeyFrameCurves[0].AddValue(frameId, x);
        mKeyFrameCurves[1].AddValue(frameId, y);
        mKeyFrameCurves[2].AddValue(frameId, z);
    }

    template<class TransferFunction>
    void Transfer(TransferFunction &transfer) {
        AbstractFrameStateWithKeyType<T>::Transfer(transfer);
        TRANSFER(mKeyFrameCurve);
        TRANSFER(mKeyFrameCurves);
    }

    virtual float GetFloatValue() = 0;
    virtual AbstractKeyFrame * SetFloatValue(float value) = 0;
    virtual Vector3f* GetVector3Value() =0;
    virtual AbstractKeyFrame * SetVector3Value(float x, float y, float z) = 0;
    virtual void SetFloatValueByIndex(int index, int frameId, float value) =0;
    virtual void SetVectorValueByIndex(int index, int indexOfVector, int frameId, float value) = 0;

private:
    // In case this is just one float value, use this curve.
    KeyFrameCurve mKeyFrameCurve;

    // In case this is a vector of float value, use this array.
    std::vector<KeyFrameCurve> mKeyFrameCurves;
};

template<class T>
void VerifyFrameIdAndSetValue(AbstractFrameStateWithKeyFrameCurve<T> *pFrameState, int frameId,
                              std::function<void()> setValueFunction) {
    if (pFrameState->GetBaseShape()->GetLayer()->GetCurrentFrame() != frameId) {
        printf("Object Frame Id: %d, setFrameId: %d\n", pFrameState->GetBaseShape()->GetLayer()->GetCurrentFrame(),
               frameId);
        printf("FrameId doesn't match!!!");
        throw "FrameId doesn't match!!!!";
    }

    if (pFrameState->GetInstanceID() != InstanceID_None) {
        setValueFunction();
    } else {
        printf("Frame State is not set for the key frame curve.\n");
        throw "Frame State is not set???";
    }
}

template<class T>
KeyFrameCurve *AbstractFrameStateWithKeyFrameCurve<T>::GetFloatKeyFrameCurve() {
    mKeyFrameCurve.SetCallBacks([this](int frameId, float value) {
        VerifyFrameIdAndSetValue(this, frameId, [this, value]() {
            this->SetFloatValue(value);
        });
    },
    [this](int index, int frameId, float value){
        VerifyFrameIdAndSetValue(this, frameId, [this, index, frameId, value](){
            this->SetFloatValueByIndex(index, frameId, value);
        });
    });
    return &mKeyFrameCurve;
}

template<class T>
KeyFrameCurve *AbstractFrameStateWithKeyFrameCurve<T>::GetVectorKeyFrameCurve(int indexOfVector) {
    mKeyFrameCurves[indexOfVector].SetCallBacks([this, indexOfVector](int frameId, float value) {
        VerifyFrameIdAndSetValue(this, frameId, [this, indexOfVector, value]() {
            printf("Setting value:%f for this frame state\n", value);
            Vector3f *currentValue = this->GetVector3Value();
            float x = currentValue->x;
            float y = currentValue->y;
            float z = currentValue->z;

            switch (indexOfVector) { //Foolish!!!!
                case 0:
                    this->SetVector3Value(value, y, z);
                    break;
                case 1:
                    this->SetVector3Value(x, value, z);
                    break;
                case 2:
                    this->SetVector3Value(x, y, value);
                    break;
            }
        });
    },
    [this, indexOfVector](int index, int frameId, float value) {
        VerifyFrameIdAndSetValue(this, frameId, [this, indexOfVector, index, frameId, value]() {
            this->SetVectorValueByIndex(index, indexOfVector, frameId, value);
        });
    });

    return &mKeyFrameCurves[indexOfVector];
}

#endif //HUAHUOENGINEV2_ABSTRACTFRAMESTATEWITHKEYFRAMECURVE_H
