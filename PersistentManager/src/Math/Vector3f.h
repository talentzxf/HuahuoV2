//
// Created by VincentZhang on 4/23/2022.
//

#ifndef PERSISTENTMANAGER_VECTOR3F_H
#define PERSISTENTMANAGER_VECTOR3F_H

#include "Logging/LogAssert.h"
#include "Serialize/SerializationMetaFlags.h"


class Vector3f {
public:
    float x, y, z;

    template<class TransferFunction> void Transfer(TransferFunction& transfer);

    Vector3f() {}  // Default ctor is intentionally empty for performance reasons
    Vector3f(const Vector3f& v) : x(v.x), y(v.y), z(v.z) {}   // Necessary for correct optimized GCC codegen
    Vector3f(float inX, float inY, float inZ)  { x = inX; y = inY; z = inZ; }
    explicit Vector3f(const float* array)  { x = array[0]; y = array[1]; z = array[2]; }

    void Set(float inX, float inY, float inZ)  { x = inX; y = inY; z = inZ; }
    void Set(const float* array)   { x = array[0]; y = array[1]; z = array[2]; }
    void SetZero()                 { x = 0.0f; y = 0.0f; z = 0.0f; }

    float* GetPtr()                                { return &x; }
    const float* GetPtr() const                     { return &x; }
    float& operator[](int i)                       { DebugAssert(i >= 0 && i <= 2); return (&x)[i]; }
    const float& operator[](int i) const            { DebugAssert(i >= 0 && i <= 2); return (&x)[i]; }

    bool operator==(const Vector3f& v) const       { return x == v.x && y == v.y && z == v.z; }
    bool operator!=(const Vector3f& v) const       { return x != v.x || y != v.y || z != v.z; }

    Vector3f& operator+=(const Vector3f& inV)     { x += inV.x; y += inV.y; z += inV.z; return *this; }
    Vector3f& operator-=(const Vector3f& inV)     { x -= inV.x; y -= inV.y; z -= inV.z; return *this; }
    Vector3f& operator*=(float s)                 { x *= s; y *= s; z *= s; return *this; }
    Vector3f& operator/=(float s);

    Vector3f operator-() const                    { return Vector3f(-x, -y, -z); }

    Vector3f& Scale(const Vector3f& inV)           { x *= inV.x; y *= inV.y; z *= inV.z; return *this; }

    EXPORT_COREMODULE static const float        epsilon;
    EXPORT_COREMODULE static const float        infinity;
    EXPORT_COREMODULE static const Vector3f infinityVec;
    EXPORT_COREMODULE static const Vector3f zero;
    EXPORT_COREMODULE static const Vector3f one;
    EXPORT_COREMODULE static const Vector3f xAxis;
    EXPORT_COREMODULE static const Vector3f yAxis;
    EXPORT_COREMODULE static const Vector3f zAxis;
};

template<class TransferFunction>
inline void Vector3f::Transfer(TransferFunction& t)
{
    t.AddMetaFlag(kTransferUsingFlowMappingStyle);
    t.Transfer(x, "x");
    t.Transfer(y, "y");
    t.Transfer(z, "z");
}

#endif //PERSISTENTMANAGER_VECTOR3F_H
