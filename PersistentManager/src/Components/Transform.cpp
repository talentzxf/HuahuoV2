//
// Created by VincentZhang on 4/5/2022.
//

#include "Transform.h"

template<class TransferFunction>
void Transform::Transfer(TransferFunction& transfer)
{
//    Super::Transfer(transfer);
//
//    if (transfer.IsWriting() && IsTransformHierarchyInitialized())
//        ApplyRuntimeToSerializedData();
//
//    TRANSFER(m_LocalRotation);
//    TRANSFER(m_LocalPosition);
//    TRANSFER(m_LocalScale);
//
//    // Complete the transform transfer.
//    CompleteTransformTransfer(transfer);
//
//    if (transfer.IsReading() && IsTransformHierarchyInitialized())
//        ApplySerializedToRuntimeData();
}

IMPLEMENT_OBJECT_SERIALIZE(Transform);
INSTANTIATE_TEMPLATE_TRANSFER(Transform);

IMPLEMENT_REGISTER_CLASS(Transform, 2);