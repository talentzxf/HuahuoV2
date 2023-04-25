//
// Created by VincentZhang on 2022-06-16.
//

#ifndef HUAHUOENGINEV2_SHAPETRANSFORMCOMPONENT_H
#define HUAHUOENGINEV2_SHAPETRANSFORMCOMPONENT_H
#include "Math/Vector3f.h"
#include "FrameState.h"
#include "CustomComponent.h"

//class TransformData{
//public:
//    Vector3f scale;
//    float rotation;
//    Vector3f localPivotPosition;
//    Vector3f globalPivotPosition;
//
//    DECLARE_SERIALIZE_OPTIMIZE_TRANSFER(TransformData)
//
//    TransformData()
//        : scale(1.0,1.0,1.0)
//        , rotation(0.0f)
//        , localPivotPosition(0.0f, 0.0f, 0.0f)
//        , globalPivotPosition(0.0f, 0.0f, 0.0f)
//    {
//
//    }
//};
//
//template<class TransferFunction> void TransformData::Transfer(TransferFunction &transfer){
//    TRANSFER(scale);
//    TRANSFER(rotation);
//    TRANSFER(localPivotPosition);
//    TRANSFER(globalPivotPosition);
//}
//
//TransformData Lerp(TransformData& k1, TransformData& k2, float ratio);
//
//struct TransformKeyFrame: public AbstractKeyFrame{
//    DECLARE_SERIALIZE_OPTIMIZE_TRANSFER(TransformKeyFrame)
//
//    TransformData frameData;
//
//    TransformKeyFrame(){
//
//    }
//};

//template<class TransferFunction> void TransformKeyFrame::Transfer(TransferFunction &transfer){
//    AbstractKeyFrame::Transfer(transfer);
//    TRANSFER(frameData);
//}

class ShapeTransformComponent : public CustomComponent {
    REGISTER_CLASS(ShapeTransformComponent);
    DECLARE_OBJECT_SERIALIZE();
public:
    ShapeTransformComponent(MemLabelId memLabelId, ObjectCreationMode creationMode)
            :CustomComponent(memLabelId, creationMode), returnTemporary(false)
    {

    }

    void AwakeFromLoad(AwakeFromLoadMode awakeMode) override{
        printf("ShapeTransformComponent AwakeFromLoad: %s %d\n", __FILE__, __LINE__);
        if(!this->IsFieldRegistered("scale")){
            printf("ShapeTransformComponent AwakeFromLoad: %s %d\n", __FILE__, __LINE__);
            this->RegisterVector3Value("scale", 1.0, 1.0, 1.0);
        }

        if(!this->IsFieldRegistered("rotation")){
            printf("ShapeTransformComponent AwakeFromLoad: %s %d\n", __FILE__, __LINE__);
            this->RegisterFloatValue("rotation", 0.0);
        }

        if(!this->IsFieldRegistered("localPivotPosition")){
            printf("ShapeTransformComponent AwakeFromLoad: %s %d\n", __FILE__, __LINE__);
            this->RegisterVector3Value("localPivotPosition", 0.0f, 0.0f, 0.0f);
        }

        if(!this->IsFieldRegistered("globalPivotPosition")){
            printf("ShapeTransformComponent AwakeFromLoad: %s %d\n", __FILE__, __LINE__);
            this->RegisterVector3Value("globalPivotPosition", 0.0f, 0.0f, 0.0f);
        }

        printf("ShapeTransformComponent AwakeFromLoad: %s %d\n", __FILE__, __LINE__);

        printf("ShapeTransformComponent AwakeFromLoad: Field count : %d\n", GetFieldCount());
        std::vector<std::string> fieldNames = GetFieldNames();
        for(auto name:fieldNames){
            printf("ShapeTransformComponent AwakeFromLoad: Field name: %s\n", name.c_str());
        }
    }

    virtual bool Apply(int frameId) override{
        returnTemporary = false;
        return Super::Apply(frameId);
    }

    Vector3f* GetLocalPivotPosition(){
        return this->GetVector3Value("localPivotPosition");
    }

    Vector3f* GetGlobalPivotPosition(){
        if(returnTemporary)
            return &this->globalPivotPosition;
        return this->GetVector3Value("globalPivotPosition");
    }

    Vector3f* GetScale(){
        return this->GetVector3Value("scale");
    }

    float GetRotation(){
        if(returnTemporary)
            return this->rotation;
        return this->GetFloatValue("rotation");
    }

    AbstractKeyFrame * RecordLocalPivotPosition(int frameId, float x, float y, float z){
        return this->SetVector3Value("localPivotPosition", x, y, z);
    }

    AbstractKeyFrame * RecordGlobalPivotPosition(int frameId, float x, float y, float z){
        return this->SetVector3Value("globalPivotPosition", x, y, z);
    }

    AbstractKeyFrame * RecordRotation(int frameId, float rotation){
        return this->SetFloatValue("rotation", rotation);
    }

    AbstractKeyFrame* RecordScale(int frameId, float xScale, float yScale, float zScale){
        return this->SetVector3Value("scale", xScale, yScale, zScale);
    }

    friend class BaseShape;

    void UpdateTemporaryPosition(float x, float y, float z){
        globalPivotPosition.x = x;
        globalPivotPosition.y = y;
        globalPivotPosition.z = z;
        returnTemporary = true;
    }

    void UpdateTemporaryRotation(float rotation){
        this->rotation = rotation;
        returnTemporary = true;
    }

private:
    // TODO: Move all these ugly logic to TS side.
    bool returnTemporary;
    Vector3f globalPivotPosition;
    float rotation;
};


#endif //HUAHUOENGINEV2_SHAPETRANSFORMCOMPONENT_H
