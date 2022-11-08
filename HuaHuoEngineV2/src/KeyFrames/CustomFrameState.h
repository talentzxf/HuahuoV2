//
// Created by VincentZhang on 10/23/2022.
//

#ifndef HUAHUOENGINEV2_CUSTOMFRAMESTATE_H
#define HUAHUOENGINEV2_CUSTOMFRAMESTATE_H

#include <map>
#include <vector>
#include <string>
#include "FrameState.h"
#include "FieldShapeArray.h"

class BaseShape;

enum CustomDataType{
    FLOAT,
    COLOR,
    SHAPEARRAY
};

// Use union to save space.
struct CustomData{
    float floatValue;
    FieldShapeArray shapeArrayValue;
    ColorRGBAf colorValue;
    CustomDataType dataType;

    DECLARE_SERIALIZE(CustomData);
};

template<class TransferFunction> void CustomData::Transfer(TransferFunction &transfer) {
    TRANSFER_ENUM(dataType);

    switch (dataType) {
        case FLOAT:
            TRANSFER(floatValue);
            break;
        case COLOR:
            TRANSFER(colorValue);
            break;
        case SHAPEARRAY:
            TRANSFER(shapeArrayValue);
            break;
    }
}


class CustomDataKeyFrame{
public:
    CustomData data;
    int frameId;

    DECLARE_SERIALIZE(CustomDataKeyFrame);

    CustomDataKeyFrame():
        frameId(-1){
    }
};

template <class TransferFunction> void CustomDataKeyFrame::Transfer(TransferFunction &transfer) {
    TRANSFER(frameId);
    TRANSFER(data);
}

CustomDataKeyFrame Lerp(CustomDataKeyFrame& k1, CustomDataKeyFrame& k2, float ratio);

class CustomFrameState: public AbstractFrameStateWithKeyType<CustomDataKeyFrame>{
    REGISTER_CLASS(CustomFrameState);
    DECLARE_OBJECT_SERIALIZE()
public:
    CustomFrameState(MemLabelId memLabelId, ObjectCreationMode creationMode)
    : AbstractFrameStateWithKeyType<CustomDataKeyFrame>(memLabelId, creationMode){

    }

    virtual bool Apply();
    virtual bool Apply(int frameId) override;

public:
    void SetFloatValue(float value);

    float GetFloatValue();

    void SetColorValue(float r, float g, float b, float a);
    ColorRGBAf* GetColorValue();

    void CreateShapeArrayValue();
    FieldShapeArray* GetShapeArrayValueForWrite();
    FieldShapeArray* GetShapeArrayValue(); // Don't insert into this fieldShapeArray, it will have no effect.

    static CustomFrameState* CreateFrameState(CustomDataType dataType);

    CustomData* GetDefaultValueData(){
        return &m_defaultValue;
    }
private:
    template <typename T> void RecordFieldValue(int frameId, T value);

private:
    CustomDataType m_DataType;
    CustomData m_defaultValue;
    CustomDataKeyFrame m_CurrentKeyFrame;
};


#endif //HUAHUOENGINEV2_CUSTOMFRAMESTATE_H
