#include "Ray.h"

float Ray::SqrDistToPoint(const Vector3f& P) const
{
    Vector3f v = m_Direction;
    Vector3f w = P - m_Origin;

    float c1 = Dot(w, v);
    float c2 = Dot(v, v);
    float b = c1 / c2;

    Vector3f Pb = GetPoint(b);
    return SqrMagnitude(P - Pb);
}
