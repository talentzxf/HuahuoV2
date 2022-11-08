//
// Created by VincentZhang on 2022-11-08.
//

#ifndef HUAHUOENGINEV2_CUSTOMCOMPONENT_H
#define HUAHUOENGINEV2_CUSTOMCOMPONENT_H

#include "FrameState.h"
#include "CustomFrameState.h"

// This is the cpp side of the user created components.
// It might contain multiple CustomFrameState(s).
class CustomComponent : public AbstractFrameState{
public:
    REGISTER_CLASS(CustomComponent);
    DECLARE_OBJECT_SERIALIZE();

    CustomComponent(MemLabelId memLabelId, ObjectCreationMode creationMode)
    : AbstractFrameState(memLabelId, creationMode){

    }

    const vector<int> GetKeyFrameIds(){
        vector<int> keyFrames;
        for(auto frameState: m_FrameStates){
            auto frameStateKeyFrames = frameState->GetKeyFrameIds();
            keyFrames.insert(keyFrames.end(), frameStateKeyFrames.begin(), frameStateKeyFrames.end());
        }

        return keyFrames;
    }

    virtual int GetMinFrameId(){
        int minFrameId = MAX_FRAMES;
        for(auto frameState: m_FrameStates){
            minFrameId = min( minFrameId, frameState->GetMinFrameId());
        }
        return minFrameId;
    }

    virtual int GetMaxFrameId(){
        int maxFrameId = -1;
        for(auto frameState: m_FrameStates){
            maxFrameId = max( maxFrameId, frameState->GetMaxFrameId());
        }
        return maxFrameId;
    }

    virtual void AddAnimationOffset(int offset){
        for(auto frameState: m_FrameStates){
            frameState->AddAnimationOffset(offset);
        }
    }

    bool Apply(int frameId) override;

    void SetFloatValue(const char* fieldName, float value){
        int idx = m_fieldNameFieldIndexMap[fieldName];
        m_FrameStates[idx]->SetFloatValue(value);
    }

    void SetColorValue(const char* fieldName, float r, float g, float b, float a){
        int idx = m_fieldNameFieldIndexMap[fieldName];
        m_FrameStates[idx]->SetColorValue(r, g, b, a);
    }

    void CreateShapeArrayValue(const char* fieldName){
        int idx = m_fieldNameFieldIndexMap[fieldName];
        m_FrameStates[idx]->CreateShapeArrayValue();
    }

    float GetFloatValue(const char* fieldName){
        int idx = m_fieldNameFieldIndexMap[fieldName];
        return m_FrameStates[idx]->GetFloatValue();
    }

    FieldShapeArray* GetShapeArrayValue(const char* fieldName){
        int idx = m_fieldNameFieldIndexMap[fieldName];
        return m_FrameStates[idx]->GetShapeArrayValue();
    }

    ColorRGBAf* GetColorValue(const char* fieldName){
        int idx = m_fieldNameFieldIndexMap[fieldName];
        return m_FrameStates[idx]->GetColorValue();
    }

private:
    int RegisterField(const char* fieldName){
        if(!m_fieldNameFieldIndexMap.contains(fieldName)){
            int index = m_fieldNameFieldIndexMap.size();
            m_fieldNameFieldIndexMap[fieldName] = index;
            m_fieldIndexFieldNameMap[index] = fieldName;

            m_FrameStates[index] = CustomFrameState::CreateFrameState();

            return index;
        }

        return m_fieldNameFieldIndexMap[fieldName];
    }

public:
    int RegisterFloatValue(const char* fieldName, float initValue){
        int fieldIdx = this->RegisterField(fieldName);
        m_fieldInitValueMap[fieldIdx].floatValue = initValue;
        return fieldIdx;
    }

    int RegisterColorValue(const char* fieldName, float r, float g, float b, float a){
        int fieldIdx = this->RegisterField(fieldName);
        ColorRGBAf initColor(r, g, b, a);
        m_fieldInitValueMap[fieldIdx].colorValue = initColor;

        return fieldIdx;
    }

    int RegisterShapeArrayValue(const char* fieldName){
        return this->RegisterField(fieldName);
    }

    static CustomComponent* CreateComponent();
private:
    std::map<string, int> m_fieldNameFieldIndexMap;
    std::map<int, string> m_fieldIndexFieldNameMap;

    std::map<int, CustomData> m_fieldInitValueMap;

    vector<PPtr<CustomFrameState>> m_FrameStates; // All the frame states. Each field has one.
};


#endif //HUAHUOENGINEV2_CUSTOMCOMPONENT_H
