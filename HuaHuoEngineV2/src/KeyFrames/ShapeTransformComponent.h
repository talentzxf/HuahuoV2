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
//struct TransformKeyFrame: public AbstractKeyFrameData{
//    DECLARE_SERIALIZE_OPTIMIZE_TRANSFER(TransformKeyFrame)
//
//    TransformData frameData;
//
//    TransformKeyFrame(){
//
//    }
//};

//template<class TransferFunction> void TransformKeyFrame::Transfer(TransferFunction &transfer){
//    AbstractKeyFrameData::Transfer(transfer);
//    TRANSFER(frameData);
//}

class ShapeTransformComponent : public CustomComponent {
    REGISTER_CLASS(ShapeTransformComponent);
    DECLARE_OBJECT_SERIALIZE();
public:
    ShapeTransformComponent(MemLabelId memLabelId, ObjectCreationMode creationMode)
            :CustomComponent(memLabelId, creationMode)
    {

    }

    void AwakeFromLoad(AwakeFromLoadMode awakeMode) override{
        if(!this->IsFieldRegistered("scale"))
            this->RegisterVector3Value("scale", 1.0, 1.0, 1.0);
        if(!this->IsFieldRegistered("rotation"))
            this->RegisterFloatValue("rotation", 0.0);
        if(!this->IsFieldRegistered("localPivotPosition"))
            this->RegisterVector3Value("localPivotPosition", 0.0f, 0.0f, 0.0f);
        if(!this->IsFieldRegistered("globalPivotPosition"))
            this->RegisterVector3Value("globalPivotPosition", 0.0f, 0.0f, 0.0f);
    }

//    virtual bool Apply(int frameId) override;

    Vector3f* GetLocalPivotPosition(){
        if(isValidFrame)
            return &m_CurrentTransformData.localPivotPosition;
        return NULL;
    }

    Vector3f* GetGlobalPivotPosition(){
        if(isValidFrame)
            return &m_CurrentTransformData.globalPivotPosition;
        return NULL;
    }

    Vector3f* GetScale(){
        if(isValidFrame)
            return &m_CurrentTransformData.scale;
        return NULL;
    }

    float GetRotation(){
        if(isValidFrame)
            return m_CurrentTransformData.rotation;
        return 0.0f;
    }

//    TransformKeyFrame* RecordLocalPivotPosition(int frameId, float x, float y, float z);
//    TransformKeyFrame* RecordGlobalPivotPosition(int frameId, float x, float y, float z);
//
//    TransformKeyFrame* RecordScale(int frameId, float xScale, float yScale, float zScale);
//
//    TransformKeyFrame* RecordRotation(int frameId, float rotation);
    friend class BaseShape;

//    void UpdateTemporaryPosition(float x, float y, float z){
//        m_CurrentTransformData.globalPivotPosition.x = x;
//        m_CurrentTransformData.globalPivotPosition.y = y;
//        m_CurrentTransformData.globalPivotPosition.z = z;
//    }
//
//    void UpdateTemporaryRotation(float rotation){
//        m_CurrentTransformData.rotation = rotation;
//    }
//
//private:
//    TransformData m_CurrentTransformData;
};


#endif //HUAHUOENGINEV2_SHAPETRANSFORMCOMPONENT_H
