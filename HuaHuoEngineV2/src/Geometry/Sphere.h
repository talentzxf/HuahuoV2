#pragma once

#include "Math/Vector3f.h"

class Sphere
{
    Vector3f    m_Center;
    float       m_Radius;

public:

    Sphere() {}
    Sphere(const Vector3f& p0, float r)                {Set(p0, r); }

    void Set(const Vector3f& p0)                       {m_Center = p0; m_Radius = 0; }
    void Set(const Vector3f& p0, float r)              {m_Center = p0; m_Radius = r; }

    void Set(const Vector3f& p0, const Vector3f& p1);

    void Set(const Vector3f* inVertices, UInt32 inHowmany);

    Vector3f& GetCenter() {return m_Center; }
    const Vector3f& GetCenter() const  {return m_Center; }

    float& GetRadius() {return m_Radius; }
    const float& GetRadius() const {return m_Radius; }

    bool IsInside(const Sphere& inSphere) const;
};

bool Intersect(const Sphere& inSphere0, const Sphere& inSphere1);


inline void Sphere::Set(const Vector3f& p0, const Vector3f& p1)
{
    Vector3f dhalf = (p1 - p0) * 0.5;

    m_Center = dhalf + p0;
    m_Radius = Magnitude(dhalf);
}

inline bool Sphere::IsInside(const Sphere& inSphere) const
{
    if (inSphere.GetRadius() >= GetRadius())
        return false;
    const float squaredDistance = SqrMagnitude(GetCenter() - inSphere.GetCenter());
    const float squaredRadius = Sqr(GetRadius() - inSphere.GetRadius());
    if (squaredDistance < squaredRadius)
        return true;
    else
        return false;
}

inline bool Intersect(const Sphere& inSphere0, const Sphere& inSphere1)
{
    const float squaredDistance = SqrMagnitude(inSphere0.GetCenter() - inSphere1.GetCenter());
    const float squaredRadius = Sqr(inSphere0.GetRadius() + inSphere1.GetRadius());
    if (squaredDistance < squaredRadius)
        return true;
    else
        return false;
}
