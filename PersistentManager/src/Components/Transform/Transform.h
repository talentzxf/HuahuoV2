//
// Created by VincentZhang on 4/5/2022.
//

#ifndef PERSISTENTMANAGER_TRANSFORM_H
#define PERSISTENTMANAGER_TRANSFORM_H

#include "TypeSystem//Object.h"
#include "TypeSystem/ObjectDefines.h"
#include "Components/BaseComponent.h"
#include "Math/Quaternionf.h"
#include "Math/Vector3f.h"
#include "Math/Simd/RotationOrder.h"
#include "TransformAccess.h"
#include "BaseClasses/ImmediatePtr.h"

class Transform : public BaseComponent {
    REGISTER_CLASS(Transform);
    DECLARE_OBJECT_SERIALIZE();
public:
    Transform(ObjectCreationMode mode)
        :Super(mode), m_TransformData(TransformAccess::Null())
    {

    }

    template<class TransferFunction> void CompleteTransformTransfer(TransferFunction& transfer);

    /// Sets the rotation in local space
    void SetLocalRotation(const Quaternionf& rotation);
    /// Sets the Rotation in world space
    void SetRotation(const Quaternionf& rotation);
    /// Sets the local euler angles
    void SetLocalEulerAngles(const Vector3f& eulerAngles, math::RotationOrder order = math::kOrderUnityDefault);

    /// Sets the position in local space
    /// (If the object has no father, localspace is basically the same as world space)
    void SetLocalPosition(const Vector3f& inTranslation);
    /// Sets the position in world space
    void SetPosition(const Vector3f& position);

    Quaternionf GetLocalRotation() const;

    // Returns synced TransformAccess.
    TransformAccessReadOnly GetTransformAccess() const;
    TransformAccess GetTransformAccess();

    void RebuildTransformHierarchy();

    /// Returns a ptr to the father transformcomponent (NULL if no father)
    Transform* GetParent() const   { return m_Father; }

    typedef std::vector<ImmediatePtr<Transform> > TransformComList;
protected:
    void ApplySerializedToRuntimeData();

private:
    UInt32 CountNodesDeep() const;
    UInt32 InitializeTransformHierarchyRecursive(TransformHierarchy& hierarchy, int& index, int parentIndex);

protected:
    TransformAccess                  m_TransformData;
    Quaternionf                      m_LocalRotation;
    Vector3f                         m_LocalPosition;
    Vector3f                         m_LocalScale;

private:
    TransformComList                 m_Children;
    ImmediatePtr<Transform>          m_Father;
};


#endif //PERSISTENTMANAGER_TRANSFORM_H
