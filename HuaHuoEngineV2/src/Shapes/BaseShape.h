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
    PPtr<ShapeTransformFrameState> mTransformKeyFrames;
    PPtr<ShapeColorFrameState> mColorKeyFrames;
    Layer* mLayer;

public:
    BaseShape(MemLabelId label, ObjectCreationMode mode)
        :Super(label, mode)
        ,mLayer(NULL)
    {
        mTransformKeyFrames = Object::Produce<ShapeTransformFrameState>();
        mColorKeyFrames = Object::Produce<ShapeColorFrameState>();

        GetPersistentManager().MakeObjectPersistent(mTransformKeyFrames.GetInstanceID(), StoreFilePath);
        GetPersistentManager().MakeObjectPersistent(mColorKeyFrames->GetInstanceID(), StoreFilePath);
    }

    void SetLayer(Layer* layer){
        this->mLayer = layer;
    }

    Layer* GetLayer();

    virtual char* GetName(){
        return "Unknown";
    }

    Vector3f* GetPosition(){
        return mTransformKeyFrames->GetPosition();
    }

    virtual void Apply(int frameId){
        mTransformKeyFrames->Apply(frameId);
        mColorKeyFrames->Apply(frameId);
    }

    void SetPosition(float x, float y, float z);

    void SetColor(float r, float g, float b, float a);

    ColorRGBAf* GetColor(){
        return mColorKeyFrames->GetColor();
    }

    virtual void AwakeFromLoad(AwakeFromLoadMode awakeMode) override;
    static BaseShape* CreateShape(const char* shapeName);
};


#endif //HUAHUOENGINEV2_BASESHAPE_H
