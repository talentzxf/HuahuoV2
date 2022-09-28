//
// Created by VincentZhang on 2022-08-22.
//

#ifndef HUAHUOENGINEV2_SHAPESTROKEWIDTHFRAMESTATE_H
#define HUAHUOENGINEV2_SHAPESTROKEWIDTHFRAMESTATE_H

#include "Serialize/SerializeUtility.h"
#include "FrameState.h"

class StrokeWidthKeyFrame{
public:
    int frameId;
    float strokeWidth;
    DECLARE_SERIALIZE_OPTIMIZE_TRANSFER(StrokeWidthKeyFrame)

    StrokeWidthKeyFrame():frameId(-1){
        
    }
};

template<class TransferFunction> void StrokeWidthKeyFrame::Transfer(TransferFunction &transfer) {
    TRANSFER(frameId);
    TRANSFER(strokeWidth);
}

class ShapeStrokeWidthFrameState : public AbstractFrameState{
    REGISTER_CLASS(ShapeStrokeWidthFrameState);
    DECLARE_OBJECT_SERIALIZE();
public:
    ShapeStrokeWidthFrameState(MemLabelId memLabelId, ObjectCreationMode creationMode)
        :Super(memLabelId, creationMode)
    {
    }

    virtual bool Apply(int frameId) override;

    float GetStrokeWidth(){
        if(isValidFrame)
            return m_CurrentStrokeWidth;
        return NULL;
    }

    void RecordStrokeWidth(int frameId, float strokeWidth);

    virtual int GetMaxFrameId(){
        int maxFrameId = -1;
        for(StrokeWidthKeyFrame keyframe: m_KeyFrames){
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
private:
    float m_CurrentStrokeWidth;
    std::vector<StrokeWidthKeyFrame> m_KeyFrames;
};


#endif //HUAHUOENGINEV2_SHAPESTROKEWIDTHFRAMESTATE_H
