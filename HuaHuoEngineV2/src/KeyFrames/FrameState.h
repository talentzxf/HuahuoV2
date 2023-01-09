//
// Created by VincentZhang on 2022-06-16.
//

#ifndef HUAHUOENGINEV2_FRAMESTATE_H
#define HUAHUOENGINEV2_FRAMESTATE_H

#include "TypeSystem/Object.h"
#include "Math/Color.h"
#include "Math/Vector3f.h"
#include "BaseClasses/PPtr.h"
#include "KeyFrame.h"

class BaseShape;

class AbstractFrameState : public Object {
    REGISTER_CLASS_TRAITS(kTypeIsAbstract);
    REGISTER_CLASS(AbstractFrameState);

    DECLARE_OBJECT_SERIALIZE()

public:
    AbstractFrameState(MemLabelId memLabelId, ObjectCreationMode creationMode)
            : Super(memLabelId, creationMode), isValidFrame(false), baseShape(NULL) {

    }

    // Return value:
    // true -- the time frame has been applied.
    // false -- can't apply the time frame. The shape can't be displayed in the frame.
    virtual bool Apply(int frameId) = 0;

    bool IsValid() {
        return isValidFrame;
    }

    virtual const std::set<int> GetKeyFrameIds() = 0;

    virtual int GetMinFrameId() = 0;
    virtual int GetMaxFrameId() = 0;
    virtual void AddAnimationOffset(int offset) = 0;

    virtual void SetBaseShape(BaseShape* pBaseShape);

    BaseShape* GetBaseShape();

    const char *GetName() const override;

    void SetName(const char *name) override;

    void SetTypeName(const char* typeName){
        this->typeName = typeName;
    }

    const char* GetTypeName(){
        if(typeName.length() == 0){
            typeName = GetType()->GetName();
        }

        return typeName.c_str();
    }

    virtual int GetKeyFrameCount() = 0;
    virtual int GetKeyFrameAtIndex(int idx) = 0;
    virtual void DeleteKeyFrame(int frameId) = 0;
    virtual std::vector<KeyFrameIdentifier> GetKeyFrameIdentifiers() = 0;

protected:
    std::string typeName;
    bool isValidFrame;
    std::string frameStateName;

private:
    BaseShape* baseShape;
    PPtr<BaseShape> mBaseShapePPtr;
};

template<class T>
class AbstractFrameStateWithKeyType: public AbstractFrameState{
public:
    AbstractFrameStateWithKeyType(MemLabelId memLabelId, ObjectCreationMode creationMode)
        :AbstractFrameState(memLabelId, creationMode)
    {

    }

    virtual int GetMinFrameId(){
        return m_KeyFrames.GetMinFrameId();
    }

    virtual int GetMaxFrameId(){
        return m_KeyFrames.GetMaxFrameId();
    }

    void AddAnimationOffset(int offset){
        m_KeyFrames.AddAnimationOffset(offset);
    }

    std::vector<KeyFrameIdentifier> GetKeyFrameIdentifiers(){
        return m_KeyFrames.GetKeyFrameIdentifiers();
    }

    vector<T>& GetKeyFrames(){
        return m_KeyFrames.GetKeyFrames();
    }

    const std::set<int> GetKeyFrameIds(){
        std::set<int> keyFrameIds;
        vector<T>& keyFrames = GetKeyFrames();
        for(auto itr = keyFrames.begin(); itr != keyFrames.end(); itr++){
            keyFrameIds.insert(itr->GetFrameId());
        }
        return keyFrameIds;
    }

    template<class TransferFunction> void Transfer(TransferFunction &transfer){
        AbstractFrameState::Transfer(transfer);
        TRANSFER(GetKeyFrames());
    }

    virtual int GetKeyFrameCount() override {
        return GetKeyFrames().size();
    }

    virtual int GetKeyFrameAtIndex(int idx) override{
        return GetKeyFrames()[idx].GetKeyFrame().GetFrameId();
    }

    void DeleteKeyFrame(int frameId) override {
        std::vector<T> & keyframes = m_KeyFrames.GetKeyFrames();
        int targetIdx = -1;
        for(int keyframeIdx = 0 ; keyframeIdx < keyframes.size(); keyframeIdx++){
            if(keyframes[keyframeIdx].GetKeyFrame().GetFrameId() == frameId){
                targetIdx = keyframeIdx;
                break;
            }
        }

        if(targetIdx >= 0)
            keyframes.erase(keyframes.begin() + targetIdx);
    }

protected:
    KeyFrameManager<T> m_KeyFrames;
};

// TODO: Binary search rather than linear search !!!!
template<class T>
bool
FindKeyFramePair(int frameId, std::vector<T> &keyFrames, std::pair<T *, T *> &result) {
    T *t_prev = NULL;
    T *t_next = NULL;

    auto itr = keyFrames.begin();

    if (keyFrames.empty()) {
        return false;
    }

    // First frame is larger than we want, return the first frame.
    if (keyFrames[0].GetFrameId() >= frameId) {
        t_prev = &keyFrames[0];
        t_next = NULL;
    } else {
        while (itr != keyFrames.end()) {
            if (itr->GetFrameId() >= frameId) {
                t_next = &(*itr);
                break;
            }
            t_prev = &(*itr);
            itr++;
        }
    }

    result.first = t_prev;
    result.second = t_next;

    return true;
}

template<typename T>
typename std::vector<T>::iterator FindLastKeyFrame(int frameId, std::vector<T> &keyFrames) {
    if(keyFrames.size() == 0){
        return keyFrames.end();
    }

    if(keyFrames.size() == 1){
        if(frameId >= keyFrames[0].GetFrameId())
            return keyFrames.begin();
        else
            return keyFrames.end();
    }

    auto retItr = keyFrames.begin();
    auto nextItr = keyFrames.begin() + 1;

    while (nextItr != keyFrames.end()) {
        if (nextItr->GetFrameId() > frameId) {
            return retItr;
        }
        nextItr++;
        retItr++;
    }
    return retItr;
}

template<typename T>
typename std::vector<T>::iterator FindInsertPosition(int frameId, std::vector<T> &keyFrames) {
    typename std::vector<T>::iterator itr = keyFrames.begin();
    while (itr != keyFrames.end()) {
        if (itr->GetFrameId() >= frameId) {
            return itr;
        }
        itr++;
    }
    return itr;
}

template<typename T>
T *InsertOrUpdateKeyFrame(int frameId, std::vector<T> &keyFrames, AbstractFrameState* pFrameState, bool* isInsert = NULL) {
    auto itr = FindInsertPosition(frameId, keyFrames);
    T *pKeyFrame = NULL;
    if (itr == keyFrames.end()) {
        int currentFrameSize = keyFrames.size();
        keyFrames.resize(currentFrameSize + 1);
        pKeyFrame = &keyFrames[currentFrameSize];
        if(isInsert){ // Inserted at last of the keyFrames.
            *isInsert = true;
        }
    } else if (itr->GetFrameId() == frameId) { // The frame exists, reassign value later
        pKeyFrame = &(*itr);

        if(isInsert){
            *isInsert = false; // Update the value
        }
    } else {
        T transformKeyFrame;
        auto newFrameItr = keyFrames.insert(itr, transformKeyFrame);
        pKeyFrame = &(*newFrameItr);

        if(isInsert){
            *isInsert = true;
        }
    }

    pKeyFrame->SetFrameState(pFrameState);
    pKeyFrame->SetFrameId(frameId);
    return pKeyFrame;
}

#endif //HUAHUOENGINEV2_FRAMESTATE_H
