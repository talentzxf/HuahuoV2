#pragma once

#include "Math/Vector3f.h"
// #include "Runtime/Scripting/BindingsDefs.h"

class Ray
{
    Vector3f    m_Origin;
    Vector3f    m_Direction; // Direction is always normalized

public:
    Ray() {}
    Ray(const Vector3f& orig, const Vector3f& dir) { m_Origin = orig; SetDirection(dir); }

    const Vector3f& GetDirection() const        { return m_Direction; }
    // Direction has to be normalized
    void SetDirection(const Vector3f& dir) { Assert(IsNormalized(dir)); m_Direction = dir; }
    void SetApproxDirection(const Vector3f& dir)   { m_Direction = NormalizeFast(dir); }
    void SetOrigin(const Vector3f& origin) { m_Origin = origin; }

    const Vector3f& GetOrigin() const       { return m_Origin; }
    Vector3f GetPoint(float t) const           { return m_Origin + t * m_Direction; }

    float SqrDistToPoint(const Vector3f &v) const;
};

// BIND_MANAGED_TYPE_NAME(Ray, UnityEngine_Ray);
