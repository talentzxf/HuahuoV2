//
// Created by VincentZhang on 2022-06-16.
//

#ifndef HUAHUOENGINEV2_FRAMESTATE_H
#define HUAHUOENGINEV2_FRAMESTATE_H
#include "TypeSystem/Object.h"
#include "Math/Color.h"
#include "Math/Vector3f.h"

class AbstractFrameState : public Object{
    REGISTER_CLASS_TRAITS(kTypeIsAbstract);
    REGISTER_CLASS(AbstractFrameState);
public:
    AbstractFrameState(MemLabelId memLabelId, ObjectCreationMode creationMode)
        :Super(memLabelId, creationMode)
    {

    }

    // Return value:
    // true -- the time frame has been applied.
    // false -- can't apply the time frame. The shape can't be displayed in the frame.
    virtual bool Apply(int frameId) = 0;
};

template<class T>
bool FindKeyFramePair(int frameId, std::vector<T> &keyFrames, std::pair<T *, T *> result) {
    int status = 0;
    int lastFrameId = -1;
    int nextFrameId = -1;

    T *t_prev = NULL;
    T *t_next = NULL;

    auto itr = keyFrames.begin();

    if (keyFrames.empty()) {
        return false;
    }

    // First frame is larger than we want
    if (keyFrames[0].frameId > frameId) {
        return false;
    }

    if (keyFrames.size() == 1) {
        if (keyFrames[0].frameId != frameId)
            return false;
        else {
            lastFrameId = frameId;
            nextFrameId = frameId;

            t_prev = &keyFrames[0];
            t_next = &keyFrames[0];
        }
    } else {
        while (itr != keyFrames.end()) {
            if (itr->frameId >= frameId) {
                t_next = &(*itr);
                nextFrameId = itr->frameId;
                break;
            }
            t_prev = &(*itr);
            lastFrameId = itr->frameId;
            itr++;
        }

        if (lastFrameId == -1) {
            return false;
        }

        if (itr == keyFrames.end()) {
            return false;
        }
    }

    result.first = t_prev;
    result.second = t_next;

    return true;
}

#endif //HUAHUOENGINEV2_FRAMESTATE_H
