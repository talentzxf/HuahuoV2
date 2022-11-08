//
// Created by VincentZhang on 2022-11-08.
//

#include "CustomComponent.h"

IMPLEMENT_REGISTER_CLASS(CustomComponent, 10018);

IMPLEMENT_OBJECT_SERIALIZE(CustomComponent);

INSTANTIATE_TEMPLATE_TRANSFER(CustomComponent);

template<class TransferFunction>
void CustomComponent::Transfer(TransferFunction &transfer) {
    Super::Transfer(transfer);

    TRANSFER(m_fieldNameFieldIndexMap);
    TRANSFER(m_fieldIndexFieldNameMap);
    TRANSFER(m_FrameStates);
}

CustomComponent *CustomComponent::CreateComponent() {
    CustomComponent *producedComponent = Object::Produce<CustomComponent>();
    GetPersistentManagerPtr()->MakeObjectPersistent(producedComponent->GetInstanceID(), StoreFilePath);
    return producedComponent;
}

bool CustomComponent::Apply(int frameId) {
    for(auto frameState : m_FrameStates){
        frameState->Apply(frameId);
    }

    return true;
}

void CustomComponent::SetBaseShape(BaseShape *pBaseShape) {
    AbstractFrameState::SetBaseShape(pBaseShape);

    for(auto frameState : m_FrameStates){
        frameState->SetBaseShape(pBaseShape);
    }
}
