//
// Created by VincentZhang on 6/1/2022.
//

#ifndef HUAHUOENGINEV2_BASESHAPE_H
#define HUAHUOENGINEV2_BASESHAPE_H
#include "TypeSystem/Object.h"
#include "Math/Vector3f.h"
#include "Math/Color.h"
#include "Export/Events/ScriptEventManager.h"
#include "KeyFrames/ShapeTransformFrameState.h"
#include "BaseClasses/PPtr.h"
#include "KeyFrames/ShapeColorFrameState.h"
#include "Serialize/PersistentManager.h"
#include "KeyFrames/ShapeSegmentFrameState.h"

extern std::string StoreFilePath;
class BaseShape;
class ShapeLoadedEventArgs: public ScriptEventHandlerArgs{
public:
    ShapeLoadedEventArgs(BaseShape* baseShape):m_BaseShape(baseShape){

    }

    BaseShape* GetBaseShape(){
        return m_BaseShape;
    }
private:
    BaseShape* m_BaseShape;
};

class Layer;
class BaseShape : public Object{
    REGISTER_CLASS_TRAITS(kTypeIsAbstract);
    REGISTER_CLASS(BaseShape);
    DECLARE_OBJECT_SERIALIZE();
private:
    ShapeTransformFrameState mTransformKeyFrames;
    ShapeColorFrameState mColorKeyFrames;
    ShapeSegmentFrameState mSegmentFrames;
    Layer* mLayer;
    SInt32 mBornFrameId;
    SInt32 mIndex;
    bool mIsVisible;

public:
    BaseShape(MemLabelId label, ObjectCreationMode mode)
        :Super(label, mode)
        ,mTransformKeyFrames(label, mode)
        ,mColorKeyFrames(label, mode)
        ,mSegmentFrames(label, mode)
        ,mLayer(NULL)
        ,mBornFrameId(-1)
        ,mIndex(-1)
        ,mIsVisible(true)
    {
    }

    void SetBornFrameId(SInt32 bornFrameId){
        mBornFrameId = bornFrameId;
    }

    SInt32 GetBornFrameId(){
        return mBornFrameId;
    }

    bool IsVisibleInFrame(SInt32 frameId);
    bool IsVisible();

    void SetIsVisible(bool isVisible){
        mIsVisible = isVisible;
    }

    void SetLayer(Layer* layer){
        this->mLayer = layer;
    }

    Layer* GetLayer();

    virtual char* GetName(){
        return "Unknown";
    }

    Vector3f* GetPosition(){
        return mTransformKeyFrames.GetPosition();
    }

    float GetRotation(){
        return mTransformKeyFrames.GetRotation();
    }

    virtual void Apply(int frameId){
        mTransformKeyFrames.Apply(frameId);
        mColorKeyFrames.Apply(frameId);
        mSegmentFrames.Apply(frameId);
    }

    void SetScale(float xScale, float yScale, float zScale);
    Vector3f* GetScale();

    void SetPosition(float x, float y, float z);

    void SetColor(float r, float g, float b, float a);

    void SetRotation(float rotation);

    void SetSegments(float segmentBuffer[], int size);

    void SetSegmentsAtFrame(float segmentBuffer[], int size, int keyFrameId);

    int GetSegmentCount(){
        return mSegmentFrames.GetSegmentCount();
    }

    Vector3f* GetSegmentPosition(int segmentId){
        return mSegmentFrames.GetSegmentPosition(segmentId);
    }

    Vector3f* GetSegmentHandleIn(int segmentId){
        return mSegmentFrames.GetSegmentHandleIn(segmentId);
    }

    Vector3f* GetSegmentHandleOut(int segmentId){
        return mSegmentFrames.GetSegmentHandleOut(segmentId);
    }

    int GetSegmentKeyFrameCount(){
        return mSegmentFrames.GetKeyFrameCount();
    }

    SegmentKeyFrame* GetSegmentKeyFrameAtKeyFrameIndex(int keyFrameIndex){
        return mSegmentFrames.GetSegmentKeyFrameAtFrameIndex(keyFrameIndex);
    }

    ColorRGBAf* GetColor(){
        return mColorKeyFrames.GetColor();
    }

    void SetIndex(SInt32 index){
        this->mIndex = index;
    }

    SInt32 GetIndex(){
        return this->mIndex;
    }

    virtual void AwakeFromLoad(AwakeFromLoadMode awakeMode) override;
    static BaseShape* CreateShape(const char* shapeName);
};


#endif //HUAHUOENGINEV2_BASESHAPE_H
