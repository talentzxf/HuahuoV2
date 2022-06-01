#pragma once

#include "Math/Vector3f.h"

class MinMaxAABB;
class Quaternionf;
class Matrix3x3f;

class Capsule
{
public:
    Capsule() {}

    Capsule(const Vector3f& start, const Vector3f& end, float radius)
        : m_Start(start), m_End(end), m_Radius(radius)
    {
    }

    // Exact comparison
    bool operator==(const Capsule& other) const
    {
        return m_Start == other.m_Start && m_End == other.m_End && m_Radius == other.m_Radius;
    }

    // Point query
    bool IsInside(const Vector3f& inPoint) const;

    // Getters
    const Vector3f &GetStart() const { return m_Start; }
    const Vector3f &GetEnd() const { return m_End; }
    float GetRadius() const { return m_Radius; }

    // Setters
    void Set(const Vector3f &start, const Vector3f &end, float r)
    {
        SetStart(start);
        SetEnd(end);
        SetRadius(r);
    }

    void SetStart(const Vector3f &v) { m_Start = v; }
    void SetEnd(const Vector3f &v) { m_End = v; }
    void SetRadius(float r) { m_Radius = r; }

protected:
    Vector3f    m_Start;
    Vector3f    m_End;
    float       m_Radius;
};
