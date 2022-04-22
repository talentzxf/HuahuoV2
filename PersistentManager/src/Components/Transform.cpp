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


// Separate out the basic transfer properties from specialized processing during transfers.
// This is done so that types derived from Transform can perform their own special handling of the basic
// transform properties.  An example of this is RectTransform when it uses its driven properties.
template<class TransferFunction>
void Transform::CompleteTransformTransfer(TransferFunction& transfer)
{
//    // When cloning objects for prefabs and instantiate, we don't use serialization to duplicate the hierarchy,
//    // we duplicate the hierarchy directly
//    if (SerializePrefabIgnoreProperties(transfer))
//    {
//#if UNITY_EDITOR
//        if (transfer.IsWriting() && transfer.NeedsInstanceIDRemapping())
//        {
//            WriteAllValidChildren(transfer, m_Children);
//        }
//        else
//#endif
//        {
//            transfer.Transfer(m_Children, "m_Children", kHideInEditorMask | kStrongPPtrMask);
//        }
//
//        transfer.Transfer(m_Father, "m_Father", kHideInEditorMask);
//    }
//
//#if UNITY_EDITOR
//
//    if (transfer.IsWriting())
//    {
//        m_RootOrder = GetSiblingIndex();
//    }
//    else if (transfer.IsReading())
//    {
//        // Set to kSemiNumericCompare so if it is not read in it will get inserted by name to the rootMapOrder
//        m_RootOrder = kSemiNumericCompare;
//    }
//
//    TRANSFER_EDITOR_ONLY_HIDDEN(m_RootOrder);
//    TRANSFER_EDITOR_ONLY_HIDDEN(m_LocalEulerAnglesHint);
//
//#endif
}


IMPLEMENT_OBJECT_SERIALIZE(Transform);
INSTANTIATE_TEMPLATE_TRANSFER(Transform);

IMPLEMENT_REGISTER_CLASS(Transform, 2);
INSTANTIATE_TEMPLATE_TRANSFER_FUNCTION(Transform, CompleteTransformTransfer);