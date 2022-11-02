//
// Created by VincentZhang on 10/23/2022.
//

#ifndef HUAHUOENGINEV2_CUSTOMFRAMESTATE_H
#define HUAHUOENGINEV2_CUSTOMFRAMESTATE_H

#include <map>
#include <vector>
#include <string>
#include "FrameState.h"

class BaseShape;

class CustomDataKeyFrame{
public:
    std::map<int, float> floatFrameValues; // Map from fieldId->float value.
    int frameId;
    bool inited;

    DECLARE_SERIALIZE(CustomDataKeyFrame)

    CustomDataKeyFrame():
        frameId(-1), inited(false){
    }
};

template <class TransferFunction> void CustomDataKeyFrame::Transfer(TransferFunction &transfer) {
    TRANSFER(frameId);
    TRANSFER(floatFrameValues);
    TRANSFER(inited);
}

CustomDataKeyFrame Lerp(CustomDataKeyFrame& k1, CustomDataKeyFrame& k2, float ratio);

class CustomFrameState: public AbstractFrameStateWithKeyType<CustomDataKeyFrame>{
    REGISTER_CLASS(CustomFrameState);
    DECLARE_OBJECT_SERIALIZE()
public:
    CustomFrameState(MemLabelId memLabelId, ObjectCreationMode creationMode)
    : AbstractFrameStateWithKeyType<CustomDataKeyFrame>(memLabelId, creationMode){

    }

    virtual bool Apply(int frameId) override;

public:
    int RegisterFloatValue(const char* fieldName, float initValue){
        if(!m_fieldNameFieldIndexMap.contains(fieldName)){
            int index = m_fieldInitValues.size();
            m_fieldNameFieldIndexMap[fieldName] = index;
            m_fieldIndexFieldNameMap[index] = fieldName;

            m_fieldInitValues[index] = initValue;
            return index;
        }

        return m_fieldNameFieldIndexMap[fieldName];
    }

    void SetValue(const char* fieldName, float value);

    float GetValue(const char* fieldName);

    static CustomFrameState* CreateFrameState();

private:
    void RecordFieldValue(int frameId, const char* fieldName, float value);

private:
    std::map<string, int> m_fieldNameFieldIndexMap;
    std::map<int, string> m_fieldIndexFieldNameMap;
    std::map<int, float> m_fieldInitValues;

    CustomDataKeyFrame m_CurrentcustomFloatKeyFrame;
};


#endif //HUAHUOENGINEV2_CUSTOMFRAMESTATE_H
