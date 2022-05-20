#include "Capsule.h"
#include "Utilities/Utility.h"

bool Capsule::IsInside(const Vector3f& inPoint) const
{
    // Project the point to the line:  if f(t) = |A + t D - P|^2, then the minimum of d occurs at df/dt=0, so
    // we have t = D^T (A-P) / |D|^2.  Then just clamp t to [0,1] and compute the distance to that point.
    Vector3f D = m_End - m_Start;
    float magSq = SqrMagnitude(D);

    float t = 0;
    if (magSq >= Vector3f::epsilon)
    {
        t = clamp01(Dot(D, m_Start - inPoint) / magSq);
    }

    return SqrMagnitude(inPoint - Lerp(m_Start, m_End, t)) < m_Radius * m_Radius;
}
