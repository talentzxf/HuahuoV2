#include "Math/Vector3f.h"
#include "Sphere.h"

void Sphere::Set(const Vector3f* inVertices, UInt32 inHowmany)
{
    m_Radius = 0.0F;
    m_Center = Vector3f::zero;
    UInt32 i;
    for (i = 0; i < inHowmany; i++)
        m_Radius = std::max(m_Radius, SqrMagnitude(inVertices[i]));
    m_Radius = sqrt(m_Radius);
}
