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

    Container& GetChildComponents(){
        return m_FrameStates;
    }

    const vector<int> GetKeyFrameIds() override{
        vector<int> keyFrames;
        for(auto frameState: m_FrameStates){
            auto frameStateKeyFrames = frameState.GetComponentPtr()->GetKeyFrameIds();
            keyFrames.insert(keyFrames.end(), frameStateKeyFrames.begin(), frameStateKeyFrames.end());
        }

        return keyFrames;
    }

    virtual int GetMinFrameId() override{
        int minFrameId = MAX_FRAMES;
        for(auto frameState: m_FrameStates){
            minFrameId = min( minFrameId, frameState.GetComponentPtr()->GetMinFrameId());
        }
        return minFrameId;
    }

    virtual int GetMaxFrameId() override{
        int maxFrameId = -1;
        for(auto frameState: m_FrameStates){
            maxFrameId = max( maxFrameId, frameState.GetComponentPtr()->GetMaxFrameId());
        }
        return maxFrameId;
    }

    virtual void AddAnimationOffset(int offset) override{
        for(auto frameState: m_FrameStates){
            frameState.GetComponentPtr()->AddAnimationOffset(offset);
        }
    }

    bool Apply(int frameId) override;

    void SetFloatValue(const char* fieldName, float value){
        int idx = m_fieldNameFieldIndexMap[fieldName];
        CustomFrameState* pComponent = (CustomFrameState *) &(*m_FrameStates[idx].GetComponentPtr());
        pComponent->SetFloatValue(value);
    }

    void SetColorValue(const char* fieldName, float r, float g, float b, float a){
        int idx = m_fieldNameFieldIndexMap[fieldName];
        CustomFrameState* pComponent = (CustomFrameState *) &(*m_FrameStates[idx].GetComponentPtr());
        pComponent->SetColorValue(r, g, b, a);
    }

    void AddColorStop(const char* fieldName, float value, float r, float g, float b, float a){
        int idx = m_fieldNameFieldIndexMap[fieldName];
        CustomFrameState* pComponent = (CustomFrameState *) &(*m_FrameStates[idx].GetComponentPtr());
        pComponent->AddColorStop(value, r, g, b, a);
    }

    void AddColorStop(const char* fieldName, float value){
        int idx = m_fieldNameFieldIndexMap[fieldName];
        CustomFrameState* pComponent = (CustomFrameState *) &(*m_FrameStates[idx].GetComponentPtr());
        pComponent->AddColorStop(value);
    }

    void UpdateColorStop(const char* fieldName, int colorStopIndex, float value, float r, float g, float b, float a){
        int idx = m_fieldNameFieldIndexMap[fieldName];

        CustomFrameState* pComponent = (CustomFrameState *) &(*m_FrameStates[idx].GetComponentPtr());
        pComponent->UpdateColorStop(colorStopIndex, value, r, g, b, a);
    }

    void DeleteColorStop(const char* fieldName, int colorStopIndex){
        int idx = m_fieldNameFieldIndexMap[fieldName];
        CustomFrameState* pComponent = (CustomFrameState *) &(*m_FrameStates[idx].GetComponentPtr());
        pComponent->DeleteColorStop(colorStopIndex);
    }

    void CreateShapeArrayValue(const char* fieldName){
        int idx = m_fieldNameFieldIndexMap[fieldName];
        CustomFrameState* pComponent = (CustomFrameState *) &(*m_FrameStates[idx].GetComponentPtr());
        pComponent->CreateShapeArrayValue();
    }

    FieldShapeArray* GetShapeArrayValueForWrite(const char* fieldName){
        int idx = m_fieldNameFieldIndexMap[fieldName];
        CustomFrameState* pComponent = (CustomFrameState *) &(*m_FrameStates[idx].GetComponentPtr());
        return pComponent->GetShapeArrayValueForWrite();
    }

    float GetFloatValue(const char* fieldName){
        int idx = m_fieldNameFieldIndexMap[fieldName];
        CustomFrameState* pComponent = (CustomFrameState *) &(*m_FrameStates[idx].GetComponentPtr());
        return pComponent->GetFloatValue();
    }

    FieldShapeArray* GetShapeArrayValue(const char* fieldName){
        int idx = m_fieldNameFieldIndexMap[fieldName];
        CustomFrameState* pComponent = (CustomFrameState *) &(*m_FrameStates[idx].GetComponentPtr());
        return pComponent->GetShapeArrayValue();
    }

    ColorRGBAf* GetColorValue(const char* fieldName){
        int idx = m_fieldNameFieldIndexMap[fieldName];
        CustomFrameState* pComponent = (CustomFrameState *) &(*m_FrameStates[idx].GetComponentPtr());
        return pComponent->GetColorValue();
    }

    ColorStopArray* GetColorStopArray(const char* fieldName){
        int idx = m_fieldNameFieldIndexMap[fieldName];
        CustomFrameState* pComponent = (CustomFrameState *) &(*m_FrameStates[idx].GetComponentPtr());
        return pComponent->GetColorStopArray();
    }

    void SetBaseShape(BaseShape *pBaseShape) override;

private:
    int RegisterField(const char* fieldName, CustomDataType dataType){
        if(m_FrameStates.size() != m_fieldNameFieldIndexMap.size()){
            Assert("Error, field dim mismatch!");
            return -1;
        }

        if(!m_fieldNameFieldIndexMap.contains(fieldName)){
            int index = m_fieldNameFieldIndexMap.size();
            m_fieldNameFieldIndexMap[fieldName] = index;
            m_fieldIndexFieldNameMap[index] = fieldName;

            CustomFrameState* pFrameState = CustomFrameState::CreateFrameState(dataType);
            pFrameState->SetBaseShape(this->baseShape);
            m_FrameStates.push_back(FrameStatePair::FromState(pFrameState));

            return index;
        }

        return m_fieldNameFieldIndexMap[fieldName];
    }

public:
    int RegisterFloatValue(const char* fieldName, float initValue){
        int fieldIdx = this->RegisterField(fieldName, FLOAT);
        CustomFrameState* pComponent = (CustomFrameState *) &(*m_FrameStates[fieldIdx].GetComponentPtr());
        pComponent->GetDefaultValueData()->floatValue = initValue;
        return fieldIdx;
    }

    int RegisterColorValue(const char* fieldName, float r, float g, float b, float a){
        int fieldIdx = this->RegisterField(fieldName, COLOR);
        CustomFrameState* pComponent = (CustomFrameState *) &(*m_FrameStates[fieldIdx].GetComponentPtr());
        ColorRGBAf initColor(r, g, b, a);
        pComponent->GetDefaultValueData()->colorValue = initColor;
        return fieldIdx;
    }

    int RegisterColorStopArrayValue(const char* fieldName){
        return this->RegisterField(fieldName, COLORSTOPARRAY);
    }

    int RegisterShapeArrayValue(const char* fieldName){
        return this->RegisterField(fieldName, SHAPEARRAY);
    }

    static CustomComponent* CreateComponent();

private:
    std::map<string, int> m_fieldNameFieldIndexMap;
    std::map<int, string> m_fieldIndexFieldNameMap;

    Container m_FrameStates; // All the frame states. Each field has one.
};


#endif //HUAHUOENGINEV2_CUSTOMCOMPONENT_H
