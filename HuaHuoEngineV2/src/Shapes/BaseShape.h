//
// Created by VincentZhang on 6/1/2022.
//

#ifndef HUAHUOENGINEV2_BASESHAPE_H
#define HUAHUOENGINEV2_BASESHAPE_H
#include "TypeSystem/Object.h"
#include "Math/Vector3f.h"
#include "Math/Color.h"
#include "Export/Events/ScriptEventManager.h"
#include "KeyFrames/ShapeTransformComponent.h"
#include "BaseClasses/PPtr.h"
#include "Serialize/PersistentManager.h"
#include "KeyFrames/ShapeSegmentFrameState.h"
#include "BaseClasses/ImmediatePtr.h"
#include "KeyFrames/KeyFrame.h"
#include <algorithm>
#include "KeyFrames/FrameStateContainer.h"

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

    Container& GetFrameStateContainerInternal() { return mFrameStates; }

    int GetFrameStateCount(){
        return mFrameStates.size();
    }

    AbstractFrameState* GetFrameState(int idx){
        return mFrameStates[idx].GetComponentPtr();
    }

    void SetRecordTransformationOfKeyFrame(bool isRecordPosition){
        this->mRecordTransformationOfKeyFrame = isRecordPosition;
    }

    bool GetRecordTransformationOfKeyFrame(){
        return this->mRecordTransformationOfKeyFrame;
    }

    // This keyframe is baseshape level. It doesn't belong to any framestate.
    void AddKeyFrame(KeyFrameIdentifier keyFrameIdentifier){
        mBaseShapeLevelKeyFrames.insert(keyFrameIdentifier);
    }
private:

    Container   mFrameStates;

    std::set<KeyFrameIdentifier> mBaseShapeLevelKeyFrames;

    Layer* mLayer;
    SInt32 mBornFrameId;
    SInt32 mIndex;
    bool mIsVisible;
    std::string mShapeName;

    // This is just a temporary cache to fetch all keyframes
    std::vector<KeyFrameIdentifier> mKeyFrameCache;

    // The flag indicates whether we need to "really" update global position in key frames.
    // In some cases, the position are just temporary, we don't need to update it permanently.
    bool mRecordTransformationOfKeyFrame;
private:
    AbstractFrameState* ProduceFrameStateByType(const HuaHuo::Type* type);
    template<class TransferFunction> void TransferFrameStates(TransferFunction& transfer);
public:
    BaseShape(MemLabelId label, ObjectCreationMode mode)
        : Super(label, mode)
        , mLayer(NULL)
        , mBornFrameId(-1)
        , mIndex(-1)
        , mIsVisible(true)
        , mRecordTransformationOfKeyFrame(true)
        , mTypeName("")
    {
        AddFrameStateByName("ShapeTransformComponent");
        AddFrameStateByName("ShapeSegmentFrameState");
        AddFrameStateByName("ShapeFollowCurveFrameState");
    }

    AbstractFrameState* AddFrameState(AbstractFrameState* frameState);

    AbstractFrameState* GetFrameState(const char* name);

    /// Get and set the name
    virtual const char* GetName() const override{
        return mShapeName.c_str();
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

    virtual const char* GetTypeName(){
        return mTypeName.c_str();
    }

    float GetRotation(){
        return GetFrameState<ShapeTransformComponent>().GetRotation();
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
        return GetFrameState<ShapeTransformComponent>().GetLocalPivotPosition();
    }

    Vector3f* GetGlobalPivotPosition(){
        return GetFrameState<ShapeTransformComponent>().GetGlobalPivotPosition();
    }

    int GetSegmentKeyFrameCount(){
        return GetFrameState<ShapeSegmentFrameState>().GetKeyFrameCount();
    }

    SegmentKeyFrame* GetSegmentKeyFrameAtKeyFrameIndex(int keyFrameIndex){
        return GetFrameState<ShapeSegmentFrameState>().GetSegmentKeyFrameAtFrameIndex(keyFrameIndex);
    }

    void SetIndex(SInt32 index){
        this->mIndex = index;
    }

    SInt32 GetIndex(){
        return this->mIndex;
    }

    ShapeTransformComponent* GetTransform();

    virtual void AwakeFromLoad(AwakeFromLoadMode awakeMode) override;
    static BaseShape* CreateShape(const char* shapeName);

    AbstractFrameState* AddFrameStateByName(const char* frameStateName);

    AbstractFrameState* GetFrameStateByTypeName(const char* frameStateName);

    void AddAnimationOffset(int offset);

    // If one keyframe is deleted, we need to set the born frame as the smallest of all it's compnents.
    void SyncBornFrameIdWithComponents(){
        int smallestFrameId = MAX_FRAMES;
        for (Container::const_iterator i = mFrameStates.begin(); i != mFrameStates.end(); ++i) {
            int curMinFrameId = i->GetComponentPtr()->GetMinFrameId();
            if(curMinFrameId < smallestFrameId){
                smallestFrameId = curMinFrameId;
            }
        }

        if(this->mBornFrameId < smallestFrameId){
            this->SetBornFrameId(smallestFrameId);
        }
    }

    void RefreshKeyFrameCache(){
        Container::const_iterator end = mFrameStates.end();

        std::set<KeyFrameIdentifier> allKeyFrames;
        for (Container::const_iterator i = mFrameStates.begin(); i != end; ++i) {
            const vector<KeyFrameIdentifier>& keyFrames = i->GetComponentPtr()->GetKeyFrameIdentifiers();
            for(auto itr = keyFrames.begin(); itr != keyFrames.end(); itr++){
                allKeyFrames.insert(*itr);
            }
        }

        if(!mBaseShapeLevelKeyFrames.empty()){ // Not sure why, but sometimes empty set will also iterate the set and stuck there???
            for(auto itr = mBaseShapeLevelKeyFrames.begin(); itr != mBaseShapeLevelKeyFrames.end(); itr++){
                allKeyFrames.insert(*itr);
            }
        }

        mKeyFrameCache.clear();

        int smallestKeyFrameId = -1;
        for(KeyFrameIdentifier keyFrameIdentifier: allKeyFrames){
            mKeyFrameCache.push_back(keyFrameIdentifier);
        }
   }

    int GetKeyFrameCount(){
        RefreshKeyFrameCache();
        return mKeyFrameCache.size();
    }

    int GetKeyFrameAtIdx(int idx){
        if(idx >= mKeyFrameCache.size())
            return -1;
        return GetKeyFrameFromCache(idx).GetFrameId();
    }

    KeyFrame* GetKeyFrameObjectAtIdx(int idx){
        if(idx >= mKeyFrameCache.size())
            return NULL;
        return &GetKeyFrameFromCache(idx);
    }

private:
    KeyFrame& GetKeyFrameFromCache(int idx);
    std::string mTypeName;
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

inline FrameStatePair FrameStatePair::FromState(AbstractFrameState* frameState)
{
    FrameStatePair ret;
    ret.typeIndex = frameState->GetType()->GetRuntimeTypeIndex();
    ret.component = frameState;
    return ret;
}


#endif //HUAHUOENGINEV2_BASESHAPE_H
