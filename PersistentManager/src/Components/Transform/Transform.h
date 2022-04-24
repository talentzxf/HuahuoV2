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

    // Returns synced TransformAccess.
    TransformAccessReadOnly GetTransformAccess() const;
    TransformAccess GetTransformAccess();

protected:
    TransformAccess                  m_TransformData;
    Quaternionf                      m_LocalRotation;
    Vector3f                         m_LocalPosition;
    Vector3f                         m_LocalScale;
};


#endif //PERSISTENTMANAGER_TRANSFORM_H
