//
// Created by VincentZhang on 4/23/2023.
//

#ifndef HUAHUOENGINEV2_FRAMESTATECONTAINER_H
#define HUAHUOENGINEV2_FRAMESTATECONTAINER_H
#include "FrameState.h"
#include "BaseClasses/ImmediatePtr.h"

struct FrameStatePair
{
    FrameStatePair() {}

    static FrameStatePair FromState(AbstractFrameState* component);
    DECLARE_SERIALIZE(FrameStatePair);

    inline RuntimeTypeIndex const GetTypeIndex() const { return typeIndex; }
    inline ImmediatePtr<AbstractFrameState> const& GetComponentPtr() const { return component; }

    void SetComponentPtr(AbstractFrameState* const ptr);

private:
    RuntimeTypeIndex typeIndex;
    ImmediatePtr<AbstractFrameState> component;
};

typedef std::vector<FrameStatePair>    Container;
#endif //HUAHUOENGINEV2_FRAMESTATECONTAINER_H
