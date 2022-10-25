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

class CustomFloatKeyFrame{
public:
    std::vector<float> frameValues;
    int frameId;
    bool inited;

    DECLARE_SERIALIZE_OPTIMIZE_TRANSFER(CustomFloatKeyFrame)

    CustomFloatKeyFrame():
        frameId(-1), inited(false){
    }
};

template <class TransferFunction> void CustomFloatKeyFrame::Transfer(TransferFunction &transfer) {
    TRANSFER(frameId);
    TRANSFER(frameValues);
    TRANSFER(inited);
}

CustomFloatKeyFrame Lerp(CustomFloatKeyFrame& k1, CustomFloatKeyFrame& k2, float ratio);

class CustomFrameState: public AbstractFrameStateWithKeyType<CustomFloatKeyFrame>{
    REGISTER_CLASS(CustomFrameState);
    DECLARE_OBJECT_SERIALIZE()
public:
    CustomFrameState(MemLabelId memLabelId, ObjectCreationMode creationMode)
    : AbstractFrameStateWithKeyType<CustomFloatKeyFrame>(memLabelId, creationMode){

    }

    virtual bool Apply(int frameId) override;

public:
    int RegisterFloatValue(const char* fieldName, float initValue){
        int index = m_fieldInitValues.size();
        m_fieldNameFieldIndexMap[fieldName] = index;
        m_fieldIndexFieldNameMap[index] = fieldName;

        m_fieldInitValues.push_back(initValue);
        return index;
    }

    void SetValue(const char* fieldName, float value);

    float GetValue(const char* fieldName);

    static CustomFrameState* CreateFrameState();

private:
    void RecordFieldValue(int frameId, const char* fieldName, float value);

private:
    std::map<string, int> m_fieldNameFieldIndexMap;
    std::map<int, string> m_fieldIndexFieldNameMap;
    std::vector<float> m_fieldInitValues;

    CustomFloatKeyFrame m_CurrentcustomFloatKeyFrame;
};


#endif //HUAHUOENGINEV2_CUSTOMFRAMESTATE_H
