//
// Created by VincentZhang on 4/23/2022.
//

#include "Quaternionf.h"
#include "Matrix3x3.h"
#include "Matrix4x4.h"

inline float Dot(const Quaternionf& q1, const Quaternionf& q2)
{
    return (q1.x * q2.x + q1.y * q2.y + q1.z * q2.z + q1.w * q2.w);
}

void QuaternionToMatrix(const Quaternionf& q, Matrix3x3f& m)
{
    // If q is guaranteed to be a unit quaternion, s will always
    // be 1.  In that case, this calculation can be optimized out.
#if DEBUGMODE
    if (!CompareApproximately(SqrMagnitude(q), 1.0F, Vector3f::epsilon))
    {
        AssertString(Format("Quaternion To Matrix conversion failed because input Quaternion is invalid {%f, %f, %f, %f} l=%f", q.x, q.y, q.z, q.w, SqrMagnitude(q)));
    }
#endif

    // Precalculate coordinate products
    float x = q.x * 2.0F;
    float y = q.y * 2.0F;
    float z = q.z * 2.0F;
    float xx = q.x * x;
    float yy = q.y * y;
    float zz = q.z * z;
    float xy = q.x * y;
    float xz = q.x * z;
    float yz = q.y * z;
    float wx = q.w * x;
    float wy = q.w * y;
    float wz = q.w * z;

    // Calculate 3x3 matrix from orthonormal basis
    m.m_Data[0] = 1.0f - (yy + zz);
    m.m_Data[1] = xy + wz;
    m.m_Data[2] = xz - wy;

    m.m_Data[3] = xy - wz;
    m.m_Data[4] = 1.0f - (xx + zz);
    m.m_Data[5] = yz + wx;

    m.m_Data[6]  = xz + wy;
    m.m_Data[7]  = yz - wx;
    m.m_Data[8] = 1.0f - (xx + yy);
}


void QuaternionToMatrix(const Quaternionf& q, Matrix4x4f& m)
{
    // If q is guaranteed to be a unit quaternion, s will always
    // be 1.  In that case, this calculation can be optimized out.
#if DEBUGMODE
    if (!CompareApproximately(SqrMagnitude(q), 1.0F, Vector3f::epsilon))
    {
        AssertString(Format("Quaternion To Matrix conversion failed because input Quaternion is invalid {%f, %f, %f, %f} l=%f", q.x, q.y, q.z, q.w, SqrMagnitude(q)));
    }
#endif

    // Precalculate coordinate products
    float x = q.x * 2.0F;
    float y = q.y * 2.0F;
    float z = q.z * 2.0F;
    float xx = q.x * x;
    float yy = q.y * y;
    float zz = q.z * z;
    float xy = q.x * y;
    float xz = q.x * z;
    float yz = q.y * z;
    float wx = q.w * x;
    float wy = q.w * y;
    float wz = q.w * z;

    // Calculate 3x3 matrix from orthonormal basis
    m.m_Data[0] = 1.0f - (yy + zz);
    m.m_Data[1] = xy + wz;
    m.m_Data[2] = xz - wy;
    m.m_Data[3] = 0.0F;

    m.m_Data[4] = xy - wz;
    m.m_Data[5] = 1.0f - (xx + zz);
    m.m_Data[6] = yz + wx;
    m.m_Data[7] = 0.0F;

    m.m_Data[8]  = xz + wy;
    m.m_Data[9]  = yz - wx;
    m.m_Data[10] = 1.0f - (xx + yy);
    m.m_Data[11] = 0.0F;

    m.m_Data[12] = 0.0F;
    m.m_Data[13] = 0.0F;
    m.m_Data[14] = 0.0F;
    m.m_Data[15] = 1.0F;
}

void MatrixToQuaternion(const Matrix4x4f& m, Quaternionf& q)
{
    Matrix3x3f mat(
            m.Get(0, 0), m.Get(0, 1), m.Get(0, 2),
            m.Get(1, 0), m.Get(1, 1), m.Get(1, 2),
            m.Get(2, 0), m.Get(2, 1), m.Get(2, 2));

    MatrixToQuaternion(mat, q);
}

void MatrixToQuaternion(const Matrix3x3f& kRot, Quaternionf& q)
{
    // Algorithm in Ken Shoemake's article in 1987 SIGGRAPH course notes
    // article "Quaternionf Calculus and Fast Animation".
#if DEBUGMODE
    float det = kRot.GetDeterminant();
    Assert(CompareApproximately(det, 1.0F, .005f));
#endif
    float fTrace = kRot.Get(0, 0) + kRot.Get(1, 1) + kRot.Get(2, 2);
    float fRoot;

    if (fTrace > 0.0f)
    {
        // |w| > 1/2, may as well choose w > 1/2
        fRoot = std::sqrt(fTrace + 1.0f);   // 2w
        q.w = 0.5f * fRoot;
        fRoot = 0.5f / fRoot;  // 1/(4w)
        q.x = (kRot.Get(2, 1) - kRot.Get(1, 2)) * fRoot;
        q.y = (kRot.Get(0, 2) - kRot.Get(2, 0)) * fRoot;
        q.z = (kRot.Get(1, 0) - kRot.Get(0, 1)) * fRoot;
    }
    else
    {
        // |w| <= 1/2
        int s_iNext[3] = { 1, 2, 0 };
        int i = 0;
        if (kRot.Get(1, 1) > kRot.Get(0, 0))
            i = 1;
        if (kRot.Get(2, 2) > kRot.Get(i, i))
            i = 2;
        int j = s_iNext[i];
        int k = s_iNext[j];

        fRoot = std::sqrt(kRot.Get(i, i) - kRot.Get(j, j) - kRot.Get(k, k) + 1.0f);
        float* apkQuat[3] = { &q.x, &q.y, &q.z };
        Assert(fRoot >= Vector3f::epsilon);
        *apkQuat[i] = 0.5f * fRoot;
        fRoot = 0.5f / fRoot;
        q.w = (kRot.Get(k, j) - kRot.Get(j, k)) * fRoot;
        *apkQuat[j] = (kRot.Get(j, i) + kRot.Get(i, j)) * fRoot;
        *apkQuat[k] = (kRot.Get(k, i) + kRot.Get(i, k)) * fRoot;
    }
    q = Normalize(q);
}