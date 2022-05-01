//
// Created by VincentZhang on 4/23/2022.
//

#ifndef PERSISTENTMANAGER_QUATERNIONF_H
#define PERSISTENTMANAGER_QUATERNIONF_H

#include <cmath>
#include "Serialize/SerializationMetaFlags.h"
#include "Serialize/SerializeUtility.h"

class Quaternionf;
inline float SqrtImpl(float f)
{
    return sqrt(f);
}

float Dot(const Quaternionf& q1, const Quaternionf& q2);

inline float SqrMagnitude(const Quaternionf& q)
{
    return Dot(q, q);
}

inline float Magnitude(const Quaternionf& q)
{
    return SqrtImpl(SqrMagnitude(q));
}

class Quaternionf {
public:
    float x, y, z, w;
    template<class TransferFunction> void Transfer(TransferFunction& transfer);

    Quaternionf() {}  // Default ctor is intentionally empty for performance reasons
    Quaternionf(float inX, float inY, float inZ, float inW);
    explicit Quaternionf(const float* array)   { x = array[0]; y = array[1]; z = array[2]; w = array[3]; }

    // methods

    const float* GetPtr() const             { return &x; }
    float* GetPtr()                                { return &x; }

    const float& operator[](int i) const   { return GetPtr()[i]; }
    float& operator[](int i)                  { return GetPtr()[i]; }

    void Set(float inX, float inY, float inZ, float inW);
    void Set(const Quaternionf& aQuat);
    void Set(const float* array)   { x = array[0]; y = array[1]; z = array[2]; w = array[3]; }

//    friend Quaternionf Normalize(const Quaternionf& q) {    return q / Magnitude(q); }
    friend Quaternionf NormalizeSafe(const Quaternionf& q);

    static Quaternionf identity() { return Quaternionf(0.0F, 0.0F, 0.0F, 1.0F); }
};

// operator overloads
//  inlines

inline Quaternionf::Quaternionf(float inX, float inY, float inZ, float inW)
{
    x = inX;
    y = inY;
    z = inZ;
    w = inW;
}

template<class TransferFunction> inline
void Quaternionf::Transfer(TransferFunction& transfer)
{
    transfer.AddMetaFlag(kTransferUsingFlowMappingStyle);
    TRANSFER(x);
    TRANSFER(y);
    TRANSFER(z);
    TRANSFER(w);
}

#endif //PERSISTENTMANAGER_QUATERNIONF_H
