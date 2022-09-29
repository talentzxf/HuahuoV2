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
#include "BaseClasses/ImmediatePtr.h"
#include "KeyFrames/ShapeStrokeColorFrameState.h"
#include "KeyFrames/ShapeStrokeWidthFrameState.h"

extern const int MAX_FRAMES;

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
    REGISTER_CLASS(BaseShape);
    DECLARE_OBJECT_SERIALIZE();
public:
    struct FrameStatePair
    {
        FrameStatePair() {}

        static FrameStatePair FromState(AbstractFrameState* component);
        DECLARE_SERIALIZE(ComponentPair);

        inline RuntimeTypeIndex const GetTypeIndex() const { return typeIndex; }
        inline ImmediatePtr<AbstractFrameState> const& GetComponentPtr() const { return component; }

        void SetComponentPtr(AbstractFrameState* const ptr);

    private:
        RuntimeTypeIndex typeIndex;
        ImmediatePtr<AbstractFrameState> component;
    };

    typedef std::vector<FrameStatePair>    Container;

    Container& GetFrameStateContainerInternal() { return mFrameStates; }
private:


    Container   mFrameStates;

    Layer* mLayer;
    SInt32 mBornFrameId;
    SInt32 mIndex;
    bool mIsVisible;
    std::string mShapeName;

private:
    AbstractFrameState* AddFrameStateInternal(AbstractFrameState* frameState);
    AbstractFrameState* ProduceFrameStateByType(const HuaHuo::Type* type);
    template<class TransferFunction> void TransferFrameStates(TransferFunction& transfer);
public:
    BaseShape(MemLabelId label, ObjectCreationMode mode)
        :Super(label, mode)
        ,mLayer(NULL)
        ,mBornFrameId(-1)
        ,mIndex(-1)
        ,mIsVisible(true)
    {
        AddFrameStateByName("ShapeTransformFrameState");
        AddFrameStateByName("ShapeSegmentFrameState");
        AddFrameStateByName("ShapeColorFrameState");
        AddFrameStateByName("ShapeStrokeColorFrameState");
        AddFrameStateByName("ShapeStrokeWidthFrameState");
        AddFrameStateByName("ShapeFollowCurveFrameState");
    }

    /// Get and set the name
    virtual char* GetName() const override{
        return const_cast<char*>(mShapeName.c_str());
    }

    virtual void SetName(char const* name) override{
        this->mShapeName = name;
    }

    // ------------------------------------------------------------------------

    template<class T> T& GetFrameState() const;
    template<class T> T* QueryFrameState() const;

    AbstractFrameState* QueryFrameStateByType(const HuaHuo::Type* type) const;
    AbstractFrameState* QueryFrameStateByExactType(const HuaHuo::Type* type) const;

    void SetBornFrameId(SInt32 bornFrameId);

    SInt32 GetBornFrameId(){
        return mBornFrameId;
    }

    int GetMaxFrameId(){
        int maxFrameId = -1;

        Container::const_iterator i;
        Container::const_iterator end = mFrameStates.end();
        for (i = mFrameStates.begin(); i != end; ++i) {
            int frameStateFrameId = i->GetComponentPtr()->GetMaxFrameId();
            if(frameStateFrameId > maxFrameId){
                maxFrameId = frameStateFrameId;
            }
        }

        return maxFrameId;
    }

        int GetMinFrameId(){
            int minFrameId = MAX_FRAMES;

            Container::const_iterator i;
            Container::const_iterator end = mFrameStates.end();
            for (i = mFrameStates.begin(); i != end; ++i) {
                int frameStateFrameId = i->GetComponentPtr()->GetMinFrameId();
                if(frameStateFrameId >= 0){
                    if(frameStateFrameId < minFrameId){
                        minFrameId = frameStateFrameId;
                    }
                }
            }

            return minFrameId;
        }

    bool IsVisibleInFrame(SInt32 frameId);
    bool IsVisible();

    void SetIsVisible(bool isVisible){
        mIsVisible = isVisible;
    }

    void SetLayer(Layer* layer){
        this->mLayer = layer;
    }

    int GetStoreId();

    Layer* GetLayer(bool assignDefaultIfNotExist = true);

    virtual char* GetTypeName(){
        return "Unknown";
    }

    float GetRotation(){
        return GetFrameState<ShapeTransformFrameState>().GetRotation();
    }

    virtual void Apply(int frameId){
        Container::const_iterator i;
        Container::const_iterator end = mFrameStates.end();
        for (i = mFrameStates.begin(); i != end; ++i) {
            i->GetComponentPtr()->Apply(frameId);
        }
    }

    void SetScale(float xScale, float yScale, float zScale);
    Vector3f* GetScale();

    void SetLocalPivotPosition(float x, float y, float z);
    void SetGlobalPivotPosition(float x, float y, float z);

    void SetColor(float r, float g, float b, float a);

    void SetStrokeColor(float r, float g, float b, float a);

    void SetRotation(float rotation);

    void SetSegments(float segmentBuffer[], int size);

    void SetSegmentsAtFrame(float segmentBuffer[], int size, int keyFrameId);

    void RemoveSegment(int index);

    int GetSegmentCount(){
        return GetFrameState<ShapeSegmentFrameState>().GetSegmentCount();
    }

    Vector3f* GetSegmentPosition(int segmentId){
        return GetFrameState<ShapeSegmentFrameState>().GetSegmentPosition(segmentId);
    }

    Vector3f* GetSegmentHandleIn(int segmentId){
        return GetFrameState<ShapeSegmentFrameState>().GetSegmentHandleIn(segmentId);
    }

    Vector3f* GetSegmentHandleOut(int segmentId){
        return GetFrameState<ShapeSegmentFrameState>().GetSegmentHandleOut(segmentId);
    }

    Vector3f* GetLocalPivotPosition(){
        return GetFrameState<ShapeTransformFrameState>().GetLocalPivotPosition();
    }

    Vector3f* GetGlobalPivotPosition(){
        return GetFrameState<ShapeTransformFrameState>().GetGlobalPivotPosition();
    }

    int GetSegmentKeyFrameCount(){
        return GetFrameState<ShapeSegmentFrameState>().GetKeyFrameCount();
    }

    SegmentKeyFrame* GetSegmentKeyFrameAtKeyFrameIndex(int keyFrameIndex){
        return GetFrameState<ShapeSegmentFrameState>().GetSegmentKeyFrameAtFrameIndex(keyFrameIndex);
    }

    ColorRGBAf* GetColor(){
        return GetFrameState<ShapeColorFrameState>().GetColor();
    }

    ColorRGBAf* GetStrokeColor(){
        return GetFrameState<ShapeStrokeColorFrameState>().GetColor();
    }

    void SetIndex(SInt32 index){
        this->mIndex = index;
    }

    SInt32 GetIndex(){
        return this->mIndex;
    }

    void SetStrokeWidth(float strokeWidth);

    float GetStrokeWidth(){
        return GetFrameState<ShapeStrokeWidthFrameState>().GetStrokeWidth();
    }

    virtual void AwakeFromLoad(AwakeFromLoadMode awakeMode) override;
    static BaseShape* CreateShape(const char* shapeName);

    AbstractFrameState* AddFrameStateByName(const char* frameStateName);

    AbstractFrameState* GetFrameStateByName(char* frameStateName);
};

template<class T> inline
T& BaseShape::GetFrameState() const
{
    T* component = QueryFrameState<T>();
    DebugAssertMsg(component != NULL, "GetComponent returned NULL. You cannot use GetComponent unless the component is guaranteed to be part of the GO");
    return *component;
}

template<class T> inline
T* BaseShape::QueryFrameState() const
{
    return static_cast<T*>(QueryFrameStateByType(TypeOf<T>()));
}

inline BaseShape::FrameStatePair BaseShape::FrameStatePair::FromState(AbstractFrameState* frameState)
{
    FrameStatePair ret;
    ret.typeIndex = frameState->GetType()->GetRuntimeTypeIndex();
    ret.component = frameState;
    return ret;
}


#endif //HUAHUOENGINEV2_BASESHAPE_H
