//
// Created by VincentZhang on 4/10/2022.
//

#include "BaseComponent.h"

IMPLEMENT_REGISTER_CLASS(BaseComponent, 1);

template<class TransferFunction>
void BaseComponent::Transfer(TransferFunction& transfer)
{
    Super::Transfer(transfer);

//    if (SerializePrefabIgnoreProperties(transfer))
//        transfer.Transfer(m_GameObject, "m_GameObject", kHideInEditorMask | kStrongPPtrMask);
}

IMPLEMENT_OBJECT_SERIALIZE(BaseComponent);
INSTANTIATE_TEMPLATE_TRANSFER(BaseComponent);