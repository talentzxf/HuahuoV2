//
// Created by VincentZhang on 2023-03-29.
//

#include "EventGraphComponent.h"

IMPLEMENT_REGISTER_CLASS(EventGraphComponent, 10026);

IMPLEMENT_OBJECT_SERIALIZE(EventGraphComponent);

INSTANTIATE_TEMPLATE_TRANSFER(EventGraphComponent);

template<class TransferFunction>
void EventGraphComponent::Transfer(TransferFunction &transfer) {
    Super::Transfer(transfer);

    TRANSFER(m_ListenerNodeShapeMap);
}