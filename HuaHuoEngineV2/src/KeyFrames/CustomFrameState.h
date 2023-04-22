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
#include "AbstractFrameStateWithKeyFrameCurve.h"

class BaseShape;

enum CustomDataType{
    FLOAT,
    COLOR,
    SHAPEARRAY,
    COLORSTOPARRAY,
    VECTOR3,
    BINARYRESOURCE,
    STRING,
    BOOLEAN
};

class BinaryResource{
public:
    BinaryResource():mFileDataPointer(NULL){}

    const char* GetResourceName(){
        return mResourceName.c_str();
    }

    const char* GetMimeType();

    void SetResourceName(std::string resourceName){
        mResourceName = resourceName;
    }

    BinaryResource* Reset(){
        mFileDataPointer = NULL;
        return this;
    }

    UInt8 GetDataAtIndex(UInt32 index);
    UInt32 GetDataSize();

    DECLARE_SERIALIZE(BinaryResource);
private:
    std::vector<UInt8>& GetFileDataPointer();
private:
    std::string mResourceName;
    std::vector<UInt8>* mFileDataPointer;
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
    std::string stringValue;
    bool  booleanValue;
    CustomDataType dataType;

    DECLARE_SERIALIZE(CustomData);
};

// TODO: Refactor! (One of) The most ugly function in the whole system. :(
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
        case STRING:
            TRANSFER(stringValue);
            break;
        case BOOLEAN:
            TRANSFER(booleanValue);
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

class CustomFrameState: public AbstractFrameStateWithKeyFrameCurve<CustomDataKeyFrame>{
    REGISTER_CLASS(CustomFrameState);
    DECLARE_OBJECT_SERIALIZE()
public:
    CustomFrameState(MemLabelId memLabelId, ObjectCreationMode creationMode)
    : AbstractFrameStateWithKeyFrameCurve<CustomDataKeyFrame>(memLabelId, creationMode){
    }

    virtual bool Apply();
    virtual bool Apply(int frameId) override;

    KeyFrameCurve *GetVectorKeyFrameCurve(int index);

    KeyFrameCurve *GetFloatKeyFrameCurve();

public:
    // This is used during dragging of the value in the keyFrameCurve.
    void SetFloatValueByIndex(int index, int frameId, float value) override;
    void SetVectorValueByIndex(int index, int vectorCoordinate, int frameId, float value) override;

    void SetBooleanValue(bool value);
    void SetFloatValue(float value) override;
    void SetVector3Value(float x, float y, float z) override;

    void SetBinaryResourceName(const char* resourceName);

    void SetStringValue(const char* stringValue);

    BinaryResource* GetBinaryResource();

    bool GetBooleanValue();
    float GetFloatValue() override;
    Vector3f* GetVector3Value() override;

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

    const char* GetStringValue();

    void AddAnimationOffset(int offset) override;

private:
    template <typename T> CustomDataKeyFrame* RecordFieldValue(int frameId, T value);

    CustomDataKeyFrame* GetColorStopArrayKeyFrame(int currentFrameId);

private:
    CustomDataType m_DataType;
    CustomData m_defaultValue;
    CustomDataKeyFrame m_CurrentKeyFrame;
};


#endif //HUAHUOENGINEV2_CUSTOMFRAMESTATE_H
