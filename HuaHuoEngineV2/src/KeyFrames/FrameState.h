//
// Created by VincentZhang on 2022-06-16.
//

#ifndef HUAHUOENGINEV2_FRAMESTATE_H
#define HUAHUOENGINEV2_FRAMESTATE_H

#include "TypeSystem/Object.h"
#include "Math/Color.h"
#include "Math/Vector3f.h"
#include "BaseClasses/PPtr.h"

extern const int MAX_FRAMES;

template <class T>
class KeyFrameManager{
    DECLARE_SERIALIZE(KeyFrameManager)
public:
    virtual int GetMaxFrameId(){
        int maxFrameId = -1;
        for(auto keyframe: m_KeyFrames){
            if(keyframe.frameId > maxFrameId){
                maxFrameId = keyframe.frameId;
            }
        }

        return maxFrameId;
    }

    virtual int GetMinFrameId(){
        int minFrameId = MAX_FRAMES;
        for(auto keyframe: m_KeyFrames){
            if(keyframe.frameId < minFrameId){
                minFrameId = keyframe.frameId;
            }
        }

        return minFrameId;
    }

    virtual void AddAnimationOffset(int offsetFrames){
        for(auto keyframeItr = m_KeyFrames.begin(); keyframeItr != m_KeyFrames.end(); keyframeItr++ ){
            if(keyframeItr->frameId >= 0){
                keyframeItr->frameId = max(0, keyframeItr->frameId + offsetFrames);
                keyframeItr->frameId = min(MAX_FRAMES, keyframeItr->frameId);
            }
        }
    }

    std::vector<T>& GetKeyFrames(){
        return m_KeyFrames;
    }

private:
    std::vector<T> m_KeyFrames;
};

class BaseShape;

class AbstractFrameState : public Object {
    REGISTER_CLASS_TRAITS(kTypeIsAbstract);
    REGISTER_CLASS(AbstractFrameState);

public:
    AbstractFrameState(MemLabelId memLabelId, ObjectCreationMode creationMode)
            : Super(memLabelId, creationMode), isValidFrame(false) {

    }

    // Return value:
    // true -- the time frame has been applied.
    // false -- can't apply the time frame. The shape can't be displayed in the frame.
    virtual bool Apply(int frameId) = 0;

    bool IsValid() {
        return isValidFrame;
    }

    virtual const vector<int> GetKeyFrameIds() = 0;

    virtual int GetMinFrameId() = 0;
    virtual int GetMaxFrameId() = 0;
    virtual void AddAnimationOffset(int offset) = 0;


    virtual void SetBaseShape(BaseShape* pBaseShape);

    template<class TransferFunction> void Transfer(TransferFunction &transfer){
        Super::Transfer(transfer);
        TRANSFER(baseShape);
        TRANSFER(typeName);
        TRANSFER(frameStateName);
    }

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

protected:
    std::string typeName;
    bool isValidFrame;
    PPtr<BaseShape> baseShape;
    std::string frameStateName;
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

    vector<T>& GetKeyFrames(){
        return m_KeyFrames.GetKeyFrames();
    }

    const vector<int> GetKeyFrameIds(){
        vector<int> keyFrameIds;
        vector<T>& keyFrames = GetKeyFrames();
        for(auto itr = keyFrames.begin(); itr != keyFrames.end(); itr++){
            keyFrameIds.push_back(itr->frameId);
        }
        return keyFrameIds;
    }

    template<class TransferFunction> void Transfer(TransferFunction &transfer){
        AbstractFrameState::Transfer(transfer);
        TRANSFER(GetKeyFrames());
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
    if (keyFrames[0].frameId >= frameId) {
        t_prev = &keyFrames[0];
        t_next = NULL;
    } else {
        while (itr != keyFrames.end()) {
            if (itr->frameId >= frameId) {
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
        if(frameId >= keyFrames[0].frameId)
            return keyFrames.begin();
        else
            return keyFrames.end();
    }

    auto retItr = keyFrames.begin();
    auto nextItr = keyFrames.begin() + 1;

    while (nextItr != keyFrames.end()) {
        if (nextItr->frameId > frameId) {
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
        if (itr->frameId >= frameId) {
            return itr;
        }
        itr++;
    }
    return itr;
}

template<typename T>
T *InsertOrUpdateKeyFrame(int frameId, std::vector<T> &keyFrames, bool* isInsert = NULL) {
    auto itr = FindInsertPosition(frameId, keyFrames);
    T *pKeyFrame = NULL;
    if (itr == keyFrames.end()) {
        int currentFrameSize = keyFrames.size();
        keyFrames.resize(currentFrameSize + 1);
        pKeyFrame = &keyFrames[currentFrameSize];
        if(isInsert){ // Inserted at last of the keyFrames.
            *isInsert = true;
        }
    } else if (itr->frameId == frameId) { // The frame exists, reassign value later
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
    pKeyFrame->frameId = frameId;
    return pKeyFrame;
}

#endif //HUAHUOENGINEV2_FRAMESTATE_H
