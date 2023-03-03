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
#include "ColorStop.h"

class BaseShape;

enum CustomDataType{
    FLOAT,
    COLOR,
    SHAPEARRAY,
    COLORSTOPARRAY,
    VECTOR3,
    BINARYRESOURCE
};

class BinaryResource{
public:
    std::string GetResourceName(){
        return mResourceName;
    }

    void SetResourceName(std::string resourceName){
        mResourceName = resourceName;
    }

    DECLARE_SERIALIZE(BinaryResource);
private:
    std::string mResourceName;
};

template<class TransferFunction>
void BinaryResource::Transfer(TransferFunction& transfer)
{
    TRANSFER(mResourceName);
}

// Use union to save space.
struct CustomData{
    float floatValue;
    Vector3f vector3Value;

    FieldShapeArray shapeArrayValue;
    ColorRGBAf colorValue;
    ColorStopArray colorStopArray;
    BinaryResource binaryResource;
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
        case COLORSTOPARRAY:
            TRANSFER(colorStopArray);
            break;
        case VECTOR3:
            TRANSFER(vector3Value);
            break;
        case BINARYRESOURCE:
            TRANSFER(binaryResource);
            break;
    }
}


class CustomDataKeyFrame: public AbstractKeyFrameData{
public:
    CustomData data;

    DECLARE_SERIALIZE(CustomDataKeyFrame);

    CustomDataKeyFrame(){
    }
};

template <class TransferFunction> void CustomDataKeyFrame::Transfer(TransferFunction &transfer) {
    AbstractKeyFrameData::Transfer(transfer);
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
    void SetVector3Value(float x, float y, float z);

    void SetBinaryResourceName(const char* resourceName);


    float GetFloatValue();
    Vector3f* GetVector3Value();

    void SetColorValue(float r, float g, float b, float a);
    int AddColorStop(float value);
    int AddColorStop(float value, float r, float g, float b, float a);
    void UpdateColorStop(int idx, float value, float r, float g, float b, float a);
    void DeleteColorStop(int idx);

    ColorRGBAf* GetColorValue();

    void CreateShapeArrayValue();
    FieldShapeArray* GetShapeArrayValueForWrite();
    FieldShapeArray* GetShapeArrayValue(); // Don't insert into this fieldShapeArray, it will have no effect.

    static CustomFrameState* CreateFrameState(CustomDataType dataType);

    ColorStopArray* GetColorStopArray();

    CustomData* GetDefaultValueData(){
        return &m_defaultValue;
    }
private:
    template <typename T> CustomDataKeyFrame* RecordFieldValue(int frameId, T value);

    CustomDataKeyFrame* GetColorStopArrayKeyFrame(int currentFrameId);

private:
    CustomDataType m_DataType;
    CustomData m_defaultValue;
    CustomDataKeyFrame m_CurrentKeyFrame;
};


#endif //HUAHUOENGINEV2_CUSTOMFRAMESTATE_H
