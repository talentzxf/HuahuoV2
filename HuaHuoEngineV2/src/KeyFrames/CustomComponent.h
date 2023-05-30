//
// Created by VincentZhang on 2022-11-08.
//

#ifndef HUAHUOENGINEV2_CUSTOMCOMPONENT_H
#define HUAHUOENGINEV2_CUSTOMCOMPONENT_H

#include "FrameState.h"
#include "CustomFrameState.h"
#include "KeyFrames/FrameStateContainer.h"


// This is the cpp side of the user created components.
// It might contain multiple CustomFrameState(s).
class CustomComponent : public AbstractFrameState {
public:
REGISTER_CLASS(CustomComponent);

DECLARE_OBJECT_SERIALIZE();

    CustomComponent(MemLabelId memLabelId, ObjectCreationMode creationMode)
            : AbstractFrameState(memLabelId, creationMode) {

    }

    Container &GetFrameStates() {
        return m_FrameStates;
    }

    const set<int> GetKeyFrameIds() override {
        set<int> keyFrames;
        for (auto frameState: m_FrameStates) {
            auto frameStateKeyFrames = frameState.GetComponentPtr()->GetKeyFrameIds();
            keyFrames.insert(frameStateKeyFrames.begin(), frameStateKeyFrames.end());
        }

        return keyFrames;
    }

    vector<KeyFrameIdentifier> GetKeyFrameIdentifiers() override;

    virtual int GetMinFrameId() override {
        int minFrameId = MAX_FRAMES;
        for (auto frameState: m_FrameStates) {
            minFrameId = min(minFrameId, frameState.GetComponentPtr()->GetMinFrameId());
        }
        return minFrameId;
    }

    virtual int GetMaxFrameId() override {
        int maxFrameId = -1;
        for (auto frameState: m_FrameStates) {
            maxFrameId = max(maxFrameId, frameState.GetComponentPtr()->GetMaxFrameId());
        }
        return maxFrameId;
    }

    virtual void AddAnimationOffset(int offset) override {
        for (auto frameState: m_FrameStates) {
            frameState.GetComponentPtr()->AddAnimationOffset(offset);
        }
    }

    bool Apply(int frameId) override;

    void SetBinaryResourceByMD5(const char* fieldName, const char* resourceMD5){
        if(!m_fieldNameFieldIndexMap.contains(fieldName))
            return;

        int idx = m_fieldNameFieldIndexMap[fieldName];
        CustomFrameState *pComponent = (CustomFrameState *) &(*m_FrameStates[idx].GetComponentPtr());
        pComponent->SetBinaryResourceMD5(resourceMD5);
    }

    AbstractKeyFrame* SetVector3Value(const char *fieldName, float x, float y, float z) {
        if(!m_fieldNameFieldIndexMap.contains(fieldName))
            return NULL;

        int idx = m_fieldNameFieldIndexMap[fieldName];
        CustomFrameState *pComponent = (CustomFrameState *) &(*m_FrameStates[idx].GetComponentPtr());
        return pComponent->SetVector3Value(x, y, z);
    }

    void SetBooleanValue(const char *fieldName, float value) {
        if(!m_fieldNameFieldIndexMap.contains(fieldName))
            return;

        int idx = m_fieldNameFieldIndexMap[fieldName];
        CustomFrameState *pComponent = (CustomFrameState *) &(*m_FrameStates[idx].GetComponentPtr());
        pComponent->SetBooleanValue(value);
    }

    AbstractKeyFrame * SetFloatValue(const char *fieldName, float value) {
        if(!m_fieldNameFieldIndexMap.contains(fieldName))
            return NULL;

        int idx = m_fieldNameFieldIndexMap[fieldName];
        CustomFrameState *pComponent = (CustomFrameState *) &(*m_FrameStates[idx].GetComponentPtr());
        return pComponent->SetFloatValue(value);
    }

    void SetColorValue(const char *fieldName, float r, float g, float b, float a) {
        if(!m_fieldNameFieldIndexMap.contains(fieldName))
            return;

        int idx = m_fieldNameFieldIndexMap[fieldName];
        CustomFrameState *pComponent = (CustomFrameState *) &(*m_FrameStates[idx].GetComponentPtr());
        pComponent->SetColorValue(r, g, b, a);
    }

    void SetShapeValue(const char *fieldName, BaseShape* pShape){
        if(!m_fieldNameFieldIndexMap.contains(fieldName))
            return;

        int idx = m_fieldNameFieldIndexMap[fieldName];
        CustomFrameState *pComponent = (CustomFrameState *) &(*m_FrameStates[idx].GetComponentPtr());
        pComponent->SetShapeValue(pShape);
    }

    void SetStringValue(const char* fieldName, const char* strValue){
        if(!m_fieldNameFieldIndexMap.contains(fieldName))
            return;

        int idx = m_fieldNameFieldIndexMap[fieldName];
        CustomFrameState *pComponent = (CustomFrameState *) &(*m_FrameStates[idx].GetComponentPtr());
        pComponent->SetStringValue(strValue);
    }

    int AddColorStop(const char *fieldName, float value, float r, float g, float b, float a) {
        if(!m_fieldNameFieldIndexMap.contains(fieldName))
            return -1;

        int idx = m_fieldNameFieldIndexMap[fieldName];
        CustomFrameState *pComponent = (CustomFrameState *) &(*m_FrameStates[idx].GetComponentPtr());
        return pComponent->AddColorStop(value, r, g, b, a);
    }

    int AddColorStop(const char *fieldName, float value) {
        if(!m_fieldNameFieldIndexMap.contains(fieldName))
            return -1;

        int idx = m_fieldNameFieldIndexMap[fieldName];
        CustomFrameState *pComponent = (CustomFrameState *) &(*m_FrameStates[idx].GetComponentPtr());
        return pComponent->AddColorStop(value);
    }

    void
    UpdateColorStop(const char *fieldName, int colorStopIdentifier, float value, float r, float g, float b, float a) {
        if(!m_fieldNameFieldIndexMap.contains(fieldName))
            return ;

        int idx = m_fieldNameFieldIndexMap[fieldName];

        CustomFrameState *pComponent = (CustomFrameState *) &(*m_FrameStates[idx].GetComponentPtr());
        pComponent->UpdateColorStop(colorStopIdentifier, value, r, g, b, a);
    }

    void DeleteColorStop(const char *fieldName, int colorStopIdentifier) {
        if(!m_fieldNameFieldIndexMap.contains(fieldName))
            return ;

        int idx = m_fieldNameFieldIndexMap[fieldName];
        CustomFrameState *pComponent = (CustomFrameState *) &(*m_FrameStates[idx].GetComponentPtr());
        pComponent->DeleteColorStop(colorStopIdentifier);
    }

    void CreateShapeArrayValue(const char *fieldName) {
        if(!m_fieldNameFieldIndexMap.contains(fieldName))
            return ;

        int idx = m_fieldNameFieldIndexMap[fieldName];
        CustomFrameState *pComponent = (CustomFrameState *) &(*m_FrameStates[idx].GetComponentPtr());
        pComponent->CreateShapeArrayValue();
    }

    FieldShapeArray *GetShapeArrayValueForWrite(const char *fieldName) {
        if(!m_fieldNameFieldIndexMap.contains(fieldName)) // The field has not been registered.
            return NULL;

        int idx = m_fieldNameFieldIndexMap[fieldName];
        CustomFrameState *pComponent = (CustomFrameState *) &(*m_FrameStates[idx].GetComponentPtr());
        return pComponent->GetShapeArrayValueForWrite();
    }

    Vector3f *GetVector3Value(const char *fieldName) {
        if(!m_fieldNameFieldIndexMap.contains(fieldName)) // The field has not been registered.
        {
            return NULL;
        }

        int idx = m_fieldNameFieldIndexMap[fieldName];
        CustomFrameState *pComponent = (CustomFrameState *) &(*m_FrameStates[idx].GetComponentPtr());
        return pComponent->GetVector3Value();
    }

    bool GetBooleanValue(const char* fieldName) {
        if(!m_fieldNameFieldIndexMap.contains(fieldName)) // The field has not been registered.
            return false;

        int idx = m_fieldNameFieldIndexMap[fieldName];
        CustomFrameState *pComponent = (CustomFrameState *) &(*m_FrameStates[idx].GetComponentPtr());
        return pComponent->GetBooleanValue();
    }

    float GetFloatValue(const char *fieldName) {
        if(!m_fieldNameFieldIndexMap.contains(fieldName)) // The field has not been registered.
            return -1.0f;

        int idx = m_fieldNameFieldIndexMap[fieldName];
        CustomFrameState *pComponent = (CustomFrameState *) &(*m_FrameStates[idx].GetComponentPtr());
        return pComponent->GetFloatValue();
    }

    FieldShapeArray *GetShapeArrayValue(const char *fieldName) {
        if(!m_fieldNameFieldIndexMap.contains(fieldName)) // The field has not been registered.
            return NULL;

        int idx = m_fieldNameFieldIndexMap[fieldName];
        CustomFrameState *pComponent = (CustomFrameState *) &(*m_FrameStates[idx].GetComponentPtr());
        return pComponent->GetShapeArrayValue();
    }

    ColorRGBAf *GetColorValue(const char *fieldName) {
        if(!m_fieldNameFieldIndexMap.contains(fieldName)) // The field has not been registered.
            return NULL;

        int idx = m_fieldNameFieldIndexMap[fieldName];
        CustomFrameState *pComponent = (CustomFrameState *) &(*m_FrameStates[idx].GetComponentPtr());
        return pComponent->GetColorValue();
    }

    ColorStopArray *GetColorStopArray(const char *fieldName) {
        if(!m_fieldNameFieldIndexMap.contains(fieldName)) // The field has not been registered.
            return NULL;

        int idx = m_fieldNameFieldIndexMap[fieldName];
        CustomFrameState *pComponent = (CustomFrameState *) &(*m_FrameStates[idx].GetComponentPtr());
        return pComponent->GetColorStopArray();
    }

    BinaryResourceWrapper *GetBinaryResource(const char *fieldName) {
        if(!m_fieldNameFieldIndexMap.contains(fieldName)) // The field has not been registered.
            return NULL;

        int idx = m_fieldNameFieldIndexMap[fieldName];
        CustomFrameState *pComponent = (CustomFrameState *) &(*m_FrameStates[idx].GetComponentPtr());
        return pComponent->GetBinaryResource();
    }

    BaseShape* GetShapeValue(const char* fieldName){
        if(!m_fieldNameFieldIndexMap.contains(fieldName)) // The field has not been registered.
            return NULL;

        int idx = m_fieldNameFieldIndexMap[fieldName];
        CustomFrameState *pComponent = (CustomFrameState *) &(*m_FrameStates[idx].GetComponentPtr());
        return pComponent->GetShapeValue();
    }

    const char* GetStringValue(const char* fieldName){
        if(!m_fieldNameFieldIndexMap.contains(fieldName)) // The field has not been registered.
            return NULL;

        int idx = m_fieldNameFieldIndexMap[fieldName];
        CustomFrameState *pComponent = (CustomFrameState *) &(*m_FrameStates[idx].GetComponentPtr());
        return pComponent->GetStringValue();
    }

    void SetBaseShape(BaseShape *pBaseShape) override;

    void DeleteKeyFrame(int frameId, bool notifyFrontEnd = true) override;

    bool ReverseKeyFrame(int startFrameId, int endFrameId, int currentFrameId) override;

    int GetSubComponentCount() {
        return m_SubComponents.size();
    }

    CustomComponent *GetSubComponentByIdx(int idx) {
        return (CustomComponent *) &(*m_SubComponents[idx].GetComponentPtr());
    }

    CustomComponent *GetSubComponentArrayByName(const char *fieldName) {
        if(!m_fieldNameFieldIndexMap.contains(fieldName))
            return NULL;

        int idx = m_fieldNameFieldIndexMap[fieldName];
        return (CustomComponent *) &(*m_FrameStates[idx].GetComponentPtr());
    }

    void AddSubComponent(CustomComponent *pSubComponent) {
        std::vector<FrameStatePair>::iterator itr = std::find_if(m_SubComponents.begin(), m_SubComponents.end(),
                                                                 [pSubComponent](FrameStatePair componentPtr) {
                                                                     return componentPtr.GetComponentPtr().GetInstanceID() ==
                                                                            pSubComponent->GetInstanceID();
                                                                 });
        if (itr != m_SubComponents.end()) // Already added, no need to add again.
            return;

        pSubComponent->SetBaseShape(this->GetBaseShape());
        m_FrameStates.push_back(FrameStatePair::FromState(pSubComponent));
        m_SubComponents.push_back(FrameStatePair::FromState(pSubComponent));

        printf("Successfully added subcomponent with type:%s\n", pSubComponent->GetTypeName());
    }

    void MoveToStore(ObjectStore *pStore) override;

private:
    int RegisterField(const char *fieldName, CustomDataType dataType) {
        if (m_FrameStates.size() != m_fieldNameFieldIndexMap.size()) {
            Assert("Error, field dim mismatch!");
            return -1;
        }

        if (!m_fieldNameFieldIndexMap.contains(fieldName)) {
            int index = m_fieldNameFieldIndexMap.size();
            m_fieldNameFieldIndexMap[fieldName] = index;
            m_fieldIndexFieldNameMap[index] = fieldName;

            CustomFrameState *pFrameState = CustomFrameState::CreateFrameState(dataType);
            pFrameState->SetBaseShape(this->GetBaseShape());
            m_FrameStates.push_back(FrameStatePair::FromState(pFrameState));

            return index;
        }

        return m_fieldNameFieldIndexMap[fieldName];
    }

public:
    void SaveAsKeyFrame() override;

public:
    // This function will also clear and rebuild the keyFrameIdCache.
    // TODO: Rename it so that caller knows this function will clear and rebuild the keyFrameIdCache??
    int GetKeyFrameCount() override;

    int GetKeyFrameAtIndex(int idx) override;

public:
    int RegisterShapeValue(const char *fieldName);

    int RegisterBooleanValue(const char *fieldName, bool initValue) {
        int fieldIdx = this->RegisterField(fieldName, BOOLEAN);
        CustomFrameState *pComponent = (CustomFrameState *) &(*m_FrameStates[fieldIdx].GetComponentPtr());
        pComponent->GetDefaultValueData()->dataType = BOOLEAN;
        pComponent->GetDefaultValueData()->booleanValue = initValue;
        return fieldIdx;
    }

    int RegisterFloatValue(const char *fieldName, float initValue) {
        int fieldIdx = this->RegisterField(fieldName, FLOAT);
        CustomFrameState *pComponent = (CustomFrameState *) &(*m_FrameStates[fieldIdx].GetComponentPtr());
        pComponent->GetDefaultValueData()->dataType = FLOAT;
        pComponent->GetDefaultValueData()->floatValue = initValue;
        return fieldIdx;
    }

    int RegisterStringValue(const char* fieldName, const char* defaultValue){
        int fieldIdx = this->RegisterField(fieldName, STRING);
        CustomFrameState *pComponent = (CustomFrameState *) &(*m_FrameStates[fieldIdx].GetComponentPtr());
        pComponent->GetDefaultValueData()->dataType = STRING;
        pComponent->GetDefaultValueData()->stringValue = defaultValue;
        return fieldIdx;
    }

    int RegisterVector3Value(const char *fieldName, float x, float y, float z) {
        int fieldIdx = this->RegisterField(fieldName, VECTOR3);
        CustomFrameState *pComponent = (CustomFrameState *) &(*m_FrameStates[fieldIdx].GetComponentPtr());
        pComponent->GetDefaultValueData()->vector3Value = Vector3f(x, y, z);

        pComponent->GetDefaultValueData()->dataType = VECTOR3;
        Vector3f& defaultValue = pComponent->GetDefaultValueData()->vector3Value;
        return fieldIdx;
    }

    int RegisterColorValue(const char *fieldName, float r, float g, float b, float a) {
        int fieldIdx = this->RegisterField(fieldName, COLOR);
        CustomFrameState *pComponent = (CustomFrameState *) &(*m_FrameStates[fieldIdx].GetComponentPtr());
        ColorRGBAf initColor(r, g, b, a);
        pComponent->GetDefaultValueData()->dataType = COLOR;
        pComponent->GetDefaultValueData()->colorValue = initColor;
        return fieldIdx;
    }

    int RegisterColorStopArrayValue(const char *fieldName) {
        return this->RegisterField(fieldName, COLORSTOPARRAY);
    }

    int RegisterBinaryResource(const char* fieldName){
        return this->RegisterField(fieldName, BINARYRESOURCE);
    }

    int RegisterShapeArrayValue(const char *fieldName) {
        return this->RegisterField(fieldName, SHAPEARRAY);
    }

    int RegisterSubcomponentArray(const char *fieldName) { // TODO: Duplicate with RegisterField.
        if (m_FrameStates.size() != m_fieldNameFieldIndexMap.size()) {
            Assert("Error, field dim mismatch!");
            return -1;
        }

        if (!m_fieldNameFieldIndexMap.contains(fieldName)) {
            int index = m_fieldNameFieldIndexMap.size();
            m_fieldNameFieldIndexMap[fieldName] = index;
            m_fieldIndexFieldNameMap[index] = fieldName;

            CustomComponent *pComponent = CreateComponent("CustomComponent");
            // This type name should correspond to the GroupComponent in the ts side.
            pComponent->SetTypeName("GroupComponent");

            this->AddSubComponent(pComponent);

            return index;
        }
        printf("ERROR: Field: %s has already been registered.\n", fieldName);
        return m_fieldNameFieldIndexMap[fieldName];
    }

    bool IsFieldRegistered(const char* fieldName){
        return m_fieldNameFieldIndexMap.contains(fieldName);
    }

    static CustomComponent *CreateComponent(const char* componentTypeName);

    KeyFrameCurve* GetFloatKeyFrameCurve(const char* fieldName){
        if(!m_fieldNameFieldIndexMap.contains(fieldName))
            return NULL;

        int fieldIdx = m_fieldNameFieldIndexMap[fieldName];
        CustomFrameState *pComponent = (CustomFrameState *) &(*m_FrameStates[fieldIdx].GetComponentPtr());
        return pComponent->GetFloatKeyFrameCurve();
    }

    KeyFrameCurve* GetVectorKeyFrameCurve(const char* fieldName, int index){
        if(!m_fieldNameFieldIndexMap.contains(fieldName))
        {
            return NULL;
        }

        int fieldIdx = m_fieldNameFieldIndexMap[fieldName];
        CustomFrameState *pComponent = (CustomFrameState *) &(*m_FrameStates[fieldIdx].GetComponentPtr());
        return pComponent->GetVectorKeyFrameCurve(index);
    }

    int GetFieldCount(){
        return m_fieldNameFieldIndexMap.size();
    }

    std::vector<std::string> GetFieldNames(){
        vector<std::string> returnVector;
        for(auto const& x: m_fieldNameFieldIndexMap){
            returnVector.push_back(x.first);
        }

        return returnVector;
    }

private:
    std::map<string, int> m_fieldNameFieldIndexMap;
    std::map<int, string> m_fieldIndexFieldNameMap;

    Container m_FrameStates; // All the frame states. Each field has one.

    Container m_SubComponents; // All the subcomponent arrays. Each field has one.

    std::vector<int> keyFrameIdCache;
};

#endif //HUAHUOENGINEV2_CUSTOMCOMPONENT_H
