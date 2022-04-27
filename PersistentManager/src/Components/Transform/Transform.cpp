//
// Created by VincentZhang on 4/5/2022.
//

#include "Transform.h"
#include "Utilities/TypeConversion.h"
#include "TransformHierarchy.h"
#include "TransformChangeDispatch.h"
#include "Math/Simd/vec-transform.h"

using namespace TransformInternal;
static TransformChangeSystemHandle gHasChangedDeprecatedSystem;

#define RETURN_QUATERNION(x) Quaternionf outQuat; math::vstore4f(outQuat.GetPtr(), x); return outQuat;

template<class TransferFunction>
void Transform::Transfer(TransferFunction& transfer)
{
    Super::Transfer(transfer);
//
//    if (transfer.IsWriting() && IsTransformHierarchyInitialized())
//        ApplyRuntimeToSerializedData();

    TRANSFER(m_LocalRotation);
    TRANSFER(m_LocalPosition);
    TRANSFER(m_LocalScale);

//    // Complete the transform transfer.
//    CompleteTransformTransfer(transfer);
//
//    if (transfer.IsReading() && IsTransformHierarchyInitialized())
//        ApplySerializedToRuntimeData();
}

Quaternionf Transform::GetLocalRotation() const
{
#if UNITY_EDITOR
    if (!IsTransformHierarchyInitialized())
    {
        ErrorStringObject("Illegal transform access. Are you accessing a transform localRotation from OnValidate?\n", this);
        return Quaternionf::identity();
    }
#endif

    RETURN_QUATERNION(math::rotation(GetLocalTRS(GetTransformAccess())));
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

TransformAccessReadOnly Transform::GetTransformAccess() const
{
    // SyncTransformAccess(m_TransformData);
    return m_TransformData;
}

TransformAccess Transform::GetTransformAccess()
{
    // SyncTransformAccess(m_TransformData);
    return m_TransformData;
}


void Transform::SetLocalRotation(const Quaternionf& inRotation)
{
    // ABORT_INVALID_QUATERNION(inRotation, localRotation, transform);

    if (SetLocalR(GetTransformAccess(), QuaternionfTofloat4(inRotation)))
    {
        // QueueChanges();
    }
}

UInt32 Transform::InitializeTransformHierarchyRecursive(TransformHierarchy& hierarchy, int& index, int parentIndex)
{
    UInt32 newIndex = index;
    index = hierarchy.nextIndices[newIndex];
    UInt32 oldIndex = m_TransformData.index;
    TransformHierarchy* oldHierarchy = m_TransformData.hierarchy;

    // Setup hierarchy and parent indices
    m_TransformData.index = newIndex;
    m_TransformData.hierarchy = &hierarchy;
    hierarchy.parentIndices[newIndex] = parentIndex;
    hierarchy.mainThreadOnlyTransformPointers[newIndex] = this;

    bool readFromSerializedData = oldHierarchy == NULL;
    if (readFromSerializedData)
    {
#if ENABLE_DEBUG_ASSERTIONS
        // Due to ASSERT_TRANSFORM_ACCESS failure.
        hierarchy.deepChildCount[newIndex] = 1;
#endif

        ApplySerializedToRuntimeData();
        hierarchy.systemChanged[newIndex] = gHasChangedDeprecatedSystem.Mask();
        hierarchy.systemInterested[newIndex] = gHasChangedDeprecatedSystem.Mask();

        hierarchy.hierarchySystemInterested[newIndex] = 0;

        // RegisterChangeSystemInterests();
    }
    else
    {
        hierarchy.localTransforms[newIndex] = oldHierarchy->localTransforms[oldIndex];
        hierarchy.localTransformTypes[newIndex] = oldHierarchy->localTransformTypes[oldIndex];

        hierarchy.systemChanged[newIndex] = oldHierarchy->systemChanged[oldIndex];
        hierarchy.systemInterested[newIndex] = oldHierarchy->systemInterested[oldIndex];

        hierarchy.hierarchySystemInterested[newIndex] = oldHierarchy->hierarchySystemInterested[oldIndex];

#if UNITY_EDITOR
        hierarchy.eulerHints[newIndex] = oldHierarchy->eulerHints[oldIndex];
#endif
    }

    hierarchy.combinedSystemChanged |= hierarchy.systemChanged[newIndex];
    hierarchy.combinedSystemInterest |= hierarchy.systemInterested[newIndex];

    UInt32 count = 1;
    size_t childCount = m_Children.size();
    for (size_t i = 0; i != childCount; i++)
        count += m_Children[i]->InitializeTransformHierarchyRecursive(hierarchy, index, newIndex);

    hierarchy.deepChildCount[newIndex] = count;
    return count;
}


void Transform::RebuildTransformHierarchy()
{
    //@TODO: There is no real need for this to support oldHierarchy != NULL case.
    //       However during load there is some cases where we need it.
    //       Lets refactor and fix those differently.
    //       Removes complexity and potential corner cases with change masks.

    Transform* root = this;
    while (root->GetParent() != NULL)
        root = root->GetParent();

    TransformHierarchy* oldHierarchy = root->m_TransformData.hierarchy;

    SInt32 nodeCount = root->CountNodesDeep();
    TransformHierarchy* hierarchy = CreateTransformHierarchy(nodeCount);

    AllocateTransformThread(*hierarchy, 0, nodeCount - 1);

    int index = 0;
    root->InitializeTransformHierarchyRecursive(*hierarchy, index, -1);
    Assert(index == -1);
    Assert(GetDeepChildCount(*hierarchy, 0) == nodeCount);

//    QueueChanges();
//
//#if !UNITY_RELEASE
//    root->ValidateHierarchy(*hierarchy);
//#endif

    DestroyTransformHierarchy(oldHierarchy);

    GetTransformHierarchyChangeDispatch().DispatchSelfAndAllChildren(root->m_TransformData, TransformHierarchyChangeDispatch::kInterestedInTransformAccess);
}

void Transform::ApplySerializedToRuntimeData()
{
    TransformAccess transformAccess = GetTransformAccess();

    TransformInternal::InitLocalTRS(transformAccess, math::vload3f(m_LocalPosition.GetPtr()), math::vload4f(m_LocalRotation.GetPtr()), math::vload3f(m_LocalScale.GetPtr()));

#if UNITY_EDITOR
    SetEulerHint(transformAccess, math::vload3f(m_LocalEulerAnglesHint.GetPtr()));
#endif
}

UInt32 Transform::CountNodesDeep() const
{
    int count = 1;
    for (int i = 0; i < m_Children.size(); i++)
        count += m_Children[i]->CountNodesDeep();
    return count;
}


IMPLEMENT_OBJECT_SERIALIZE(Transform);
INSTANTIATE_TEMPLATE_TRANSFER(Transform);

IMPLEMENT_REGISTER_CLASS(Transform, 2);
INSTANTIATE_TEMPLATE_TRANSFER_FUNCTION(Transform, CompleteTransformTransfer);