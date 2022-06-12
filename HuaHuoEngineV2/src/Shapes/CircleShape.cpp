//
// Created by VincentZhang on 6/12/2022.
//

#include "CircleShape.h"
#include "Serialize/SerializeUtility.h"

IMPLEMENT_REGISTER_CLASS(CircleShape, 10005);

IMPLEMENT_OBJECT_SERIALIZE(CircleShape);
INSTANTIATE_TEMPLATE_TRANSFER(CircleShape);

template<class TransferFunction>
void CircleShape::Transfer(TransferFunction &transfer) {
    Super::Transfer(transfer);
    TRANSFER(m_Center);
    TRANSFER(m_Radius);
}