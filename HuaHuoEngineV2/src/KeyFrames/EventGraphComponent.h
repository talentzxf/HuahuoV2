//
// Created by VincentZhang on 2023-03-29.
//

#ifndef HUAHUOENGINEV2_EVENTGRAPHCOMPONENT_H
#define HUAHUOENGINEV2_EVENTGRAPHCOMPONENT_H


#include "CustomComponent.h"
#include <map>

class EventGraphComponent : public CustomComponent {
REGISTER_CLASS(EventGraphComponent);

DECLARE_OBJECT_SERIALIZE();

    EventGraphComponent(MemLabelId memLabelId, ObjectCreationMode creationMode)
            : CustomComponent(memLabelId, creationMode) {

    }

    BaseShape* GetShapeByNodeId(int nodeId);

    void AddNodeIdShapeMap(int nodeId, BaseShape* shape){
        m_ListenerNodeShapeMap[nodeId] = shape;
    }

private:
    // Event graph node id->cpp shape map.
    std::map<int, PPtr<BaseShape>> m_ListenerNodeShapeMap;
};


#endif //HUAHUOENGINEV2_EVENTGRAPHCOMPONENT_H
