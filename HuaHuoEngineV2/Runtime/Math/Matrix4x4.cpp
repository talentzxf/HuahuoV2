// #include "UnityPrefix.h"
#include "Matrix4x4.h"
#include "Quaternionf.h"
#include "Math/Simd/vec-transform.h"
#include "Math/Simd/vec-types.h"
#include "Utilities/TypeConversion.h"
#include "Matrix3x3.h"
#include "Utilities/Utility.h"

const Matrix4x4f Matrix4x4f::identity(kIdentity);

Matrix4x4f::Matrix4x4f(const float data[16])
{
    for (int i = 0; i < 16; i++)
        m_Data[i] = data[i];
}

Matrix4x4f::Matrix4x4f(const Matrix3x3f &other)
{
    m_Data[0] = other.m_Data[0];
    m_Data[1] = other.m_Data[1];
    m_Data[2] = other.m_Data[2];
    m_Data[3] = 0.0F;

    m_Data[4] = other.m_Data[3];
    m_Data[5] = other.m_Data[4];
    m_Data[6] = other.m_Data[5];
    m_Data[7] = 0.0F;

    m_Data[8] = other.m_Data[6];
    m_Data[9] = other.m_Data[7];
    m_Data[10] = other.m_Data[8];
    m_Data[11] = 0.0F;

    m_Data[12] = 0.0F;
    m_Data[13] = 0.0F;
    m_Data[14] = 0.0F;
    m_Data[15] = 1.0F;
}

Matrix4x4f& Matrix4x4f::operator=(const Matrix3x3f& other)
{
    m_Data[0] = other.m_Data[0];
    m_Data[1] = other.m_Data[1];
    m_Data[2] = other.m_Data[2];
    m_Data[3] = 0.0F;

    m_Data[4] = other.m_Data[3];
    m_Data[5] = other.m_Data[4];
    m_Data[6] = other.m_Data[5];
    m_Data[7] = 0.0F;

    m_Data[8] = other.m_Data[6];
    m_Data[9] = other.m_Data[7];
    m_Data[10] = other.m_Data[8];
    m_Data[11] = 0.0F;

    m_Data[12] = 0.0F;
    m_Data[13] = 0.0F;
    m_Data[14] = 0.0F;
    m_Data[15] = 1.0F;
    return *this;
}

bool Matrix4x4f::IsIdentity(float threshold) const
{
    if (CompareApproximately(Get(0, 0), 1.0f, threshold) && CompareApproximately(Get(0, 1), 0.0f, threshold) && CompareApproximately(Get(0, 2), 0.0f, threshold) && CompareApproximately(Get(0, 3), 0.0f, threshold) &&
        CompareApproximately(Get(1, 0), 0.0f, threshold) && CompareApproximately(Get(1, 1), 1.0f, threshold) && CompareApproximately(Get(1, 2), 0.0f, threshold) && CompareApproximately(Get(1, 3), 0.0f, threshold) &&
        CompareApproximately(Get(2, 0), 0.0f, threshold) && CompareApproximately(Get(2, 1), 0.0f, threshold) && CompareApproximately(Get(2, 2), 1.0f, threshold) && CompareApproximately(Get(2, 3), 0.0f, threshold) &&
        CompareApproximately(Get(3, 0), 0.0f, threshold) && CompareApproximately(Get(3, 1), 0.0f, threshold) && CompareApproximately(Get(3, 2), 0.0f, threshold) && CompareApproximately(Get(3, 3), 1.0f, threshold))
        return true;
    return false;
}

float Matrix4x4f::GetDeterminant() const
{
    double m00 = Get(0, 0);  double m01 = Get(0, 1);  double m02 = Get(0, 2);  double m03 = Get(0, 3);
    double m10 = Get(1, 0);  double m11 = Get(1, 1);  double m12 = Get(1, 2);  double m13 = Get(1, 3);
    double m20 = Get(2, 0);  double m21 = Get(2, 1);  double m22 = Get(2, 2);  double m23 = Get(2, 3);
    double m30 = Get(3, 0);  double m31 = Get(3, 1);  double m32 = Get(3, 2);  double m33 = Get(3, 3);

    double result =
        m03 * m12 * m21 * m30 - m02 * m13 * m21 * m30 - m03 * m11 * m22 * m30 + m01 * m13 * m22 * m30 +
        m02 * m11 * m23 * m30 - m01 * m12 * m23 * m30 - m03 * m12 * m20 * m31 + m02 * m13 * m20 * m31 +
        m03 * m10 * m22 * m31 - m00 * m13 * m22 * m31 - m02 * m10 * m23 * m31 + m00 * m12 * m23 * m31 +
        m03 * m11 * m20 * m32 - m01 * m13 * m20 * m32 - m03 * m10 * m21 * m32 + m00 * m13 * m21 * m32 +
        m01 * m10 * m23 * m32 - m00 * m11 * m23 * m32 - m02 * m11 * m20 * m33 + m01 * m12 * m20 * m33 +
        m02 * m10 * m21 * m33 - m00 * m12 * m21 * m33 - m01 * m10 * m22 * m33 + m00 * m11 * m22 * m33;
    return (float)result;
}

Matrix4x4f& Matrix4x4f::operator*=(const Matrix4x4f& inM1)
{
    Assert(&inM1 != this);
    Matrix4x4f tmp;
    MultiplyMatrices4x4(this, &inM1, &tmp);
    *this = tmp;
    return *this;
}

void MultiplyMatrices3x4(const Matrix4x4f& lhs, const Matrix4x4f& rhs, Matrix4x4f& res)
{
    for (int i = 0; i < 3; i++)
    {
        res.m_Data[i]    = lhs.m_Data[i] * rhs.m_Data[0]  + lhs.m_Data[i + 4] * rhs.m_Data[1]  + lhs.m_Data[i + 8] * rhs.m_Data[2];//  + lhs.m_Data[i+12] * rhs.m_Data[3];
        res.m_Data[i + 4]  = lhs.m_Data[i] * rhs.m_Data[4]  + lhs.m_Data[i + 4] * rhs.m_Data[5]  + lhs.m_Data[i + 8] * rhs.m_Data[6];//  + lhs.m_Data[i+12] * rhs.m_Data[7];
        res.m_Data[i + 8]  = lhs.m_Data[i] * rhs.m_Data[8]  + lhs.m_Data[i + 4] * rhs.m_Data[9]  + lhs.m_Data[i + 8] * rhs.m_Data[10];// + lhs.m_Data[i+12] * rhs.m_Data[11];
        res.m_Data[i + 12] = lhs.m_Data[i] * rhs.m_Data[12] + lhs.m_Data[i + 4] * rhs.m_Data[13] + lhs.m_Data[i + 8] * rhs.m_Data[14] + lhs.m_Data[i + 12];// * rhs.m_Data[15];
    }

    res.m_Data[3]  = 0.0f;
    res.m_Data[7]  = 0.0f;
    res.m_Data[11] = 0.0f;
    res.m_Data[15] = 1.0f;
}

Matrix4x4f& Matrix4x4f::SetIdentity()
{
    Get(0, 0) = 1.0;   Get(0, 1) = 0.0;   Get(0, 2) = 0.0;   Get(0, 3) = 0.0;
    Get(1, 0) = 0.0;   Get(1, 1) = 1.0;   Get(1, 2) = 0.0;   Get(1, 3) = 0.0;
    Get(2, 0) = 0.0;   Get(2, 1) = 0.0;   Get(2, 2) = 1.0;   Get(2, 3) = 0.0;
    Get(3, 0) = 0.0;   Get(3, 1) = 0.0;   Get(3, 2) = 0.0;   Get(3, 3) = 1.0;
    return *this;
}

Matrix4x4f& Matrix4x4f::SetZero()
{
    Get(0, 0) = 0.0;   Get(0, 1) = 0.0;   Get(0, 2) = 0.0;   Get(0, 3) = 0.0;
    Get(1, 0) = 0.0;   Get(1, 1) = 0.0;   Get(1, 2) = 0.0;   Get(1, 3) = 0.0;
    Get(2, 0) = 0.0;   Get(2, 1) = 0.0;   Get(2, 2) = 0.0;   Get(2, 3) = 0.0;
    Get(3, 0) = 0.0;   Get(3, 1) = 0.0;   Get(3, 2) = 0.0;   Get(3, 3) = 0.0;
    return *this;
}

Matrix4x4f& Matrix4x4f::SetBasis(const Vector3f& inX, const Vector3f& inY, const Vector3f& inZ)
{
    Get(0, 0) = inX[0];    Get(0, 1) = inY[0];    Get(0, 2) = inZ[0];    Get(0, 3) = 0.0;
    Get(1, 0) = inX[1];    Get(1, 1) = inY[1];    Get(1, 2) = inZ[1];    Get(1, 3) = 0.0;
    Get(2, 0) = inX[2];    Get(2, 1) = inY[2];    Get(2, 2) = inZ[2];    Get(2, 3) = 0.0;
    Get(3, 0) = 0.0;       Get(3, 1) = 0.0;       Get(3, 2) = 0.0;       Get(3, 3) = 1.0;
    return *this;
}

Matrix4x4f& Matrix4x4f::SetBasisTransposed(const Vector3f& inX, const Vector3f& inY, const Vector3f& inZ)
{
    Get(0, 0) = inX[0];    Get(1, 0) = inY[0];    Get(2, 0) = inZ[0];    Get(3, 0) = 0.0;
    Get(0, 1) = inX[1];    Get(1, 1) = inY[1];    Get(2, 1) = inZ[1];    Get(3, 1) = 0.0;
    Get(0, 2) = inX[2];    Get(1, 2) = inY[2];    Get(2, 2) = inZ[2];    Get(3, 2) = 0.0;
    Get(0, 3) = 0.0;       Get(1, 3) = 0.0;       Get(2, 3) = 0.0;       Get(3, 3) = 1.0;
    return *this;
}

Matrix4x4f& Matrix4x4f::SetPositionAndOrthoNormalBasis(const Vector3f& inPosition, const Vector3f& inX, const Vector3f& inY, const Vector3f& inZ)
{
    Get(0, 0) = inX[0];    Get(0, 1) = inY[0];    Get(0, 2) = inZ[0];    Get(0, 3) = inPosition[0];
    Get(1, 0) = inX[1];    Get(1, 1) = inY[1];    Get(1, 2) = inZ[1];    Get(1, 3) = inPosition[1];
    Get(2, 0) = inX[2];    Get(2, 1) = inY[2];    Get(2, 2) = inZ[2];    Get(2, 3) = inPosition[2];
    Get(3, 0) = 0.0;       Get(3, 1) = 0.0;       Get(3, 2) = 0.0;       Get(3, 3) = 1.0;
    return *this;
}

Matrix4x4f& Matrix4x4f::SetScale(const Vector3f& inScale)
{
    Get(0, 0) = inScale[0];    Get(0, 1) = 0.0;           Get(0, 2) = 0.0;           Get(0, 3) = 0.0;
    Get(1, 0) = 0.0;           Get(1, 1) = inScale[1];    Get(1, 2) = 0.0;           Get(1, 3) = 0.0;
    Get(2, 0) = 0.0;           Get(2, 1) = 0.0;           Get(2, 2) = inScale[2];    Get(2, 3) = 0.0;
    Get(3, 0) = 0.0;           Get(3, 1) = 0.0;           Get(3, 2) = 0.0;           Get(3, 3) = 1.0;
    return *this;
}

Matrix4x4f& Matrix4x4f::Scale(const Vector3f& inScale)
{
    Get(0, 0) *= inScale[0];
    Get(1, 0) *= inScale[0];
    Get(2, 0) *= inScale[0];
    Get(3, 0) *= inScale[0];

    Get(0, 1) *= inScale[1];
    Get(1, 1) *= inScale[1];
    Get(2, 1) *= inScale[1];
    Get(3, 1) *= inScale[1];

    Get(0, 2) *= inScale[2];
    Get(1, 2) *= inScale[2];
    Get(2, 2) *= inScale[2];
    Get(3, 2) *= inScale[2];
    return *this;
}

Matrix4x4f& Matrix4x4f::Translate(const Vector3f& inTrans)
{
    Get(0, 3) = Get(0, 0) * inTrans[0] + Get(0, 1) * inTrans[1] + Get(0, 2) * inTrans[2] + Get(0, 3);
    Get(1, 3) = Get(1, 0) * inTrans[0] + Get(1, 1) * inTrans[1] + Get(1, 2) * inTrans[2] + Get(1, 3);
    Get(2, 3) = Get(2, 0) * inTrans[0] + Get(2, 1) * inTrans[1] + Get(2, 2) * inTrans[2] + Get(2, 3);
    Get(3, 3) = Get(3, 0) * inTrans[0] + Get(3, 1) * inTrans[1] + Get(3, 2) * inTrans[2] + Get(3, 3);
    return *this;
}

Matrix4x4f& Matrix4x4f::SetTranslate(const Vector3f& inTrans)
{
    Get(0, 0) = 1.0;   Get(0, 1) = 0.0;   Get(0, 2) = 0.0;   Get(0, 3) = inTrans[0];
    Get(1, 0) = 0.0;   Get(1, 1) = 1.0;   Get(1, 2) = 0.0;   Get(1, 3) = inTrans[1];
    Get(2, 0) = 0.0;   Get(2, 1) = 0.0;   Get(2, 2) = 1.0;   Get(2, 3) = inTrans[2];
    Get(3, 0) = 0.0;   Get(3, 1) = 0.0;   Get(3, 2) = 0.0;   Get(3, 3) = 1.0;
    return *this;
}

Matrix4x4f& Matrix4x4f::SetPerspective(
    float fovy,
    float aspect,
    float zNear,
    float zFar)
{
    float cotangent, deltaZ;
    float radians = Deg2Rad(fovy / 2.0f);
    cotangent = cos(radians) / sin(radians);
    deltaZ = zNear - zFar;

    Get(0, 0) = cotangent / aspect; Get(0, 1) = 0.0F;      Get(0, 2) = 0.0F;                    Get(0, 3) = 0.0F;
    Get(1, 0) = 0.0F;               Get(1, 1) = cotangent; Get(1, 2) = 0.0F;                    Get(1, 3) = 0.0F;
    Get(2, 0) = 0.0F;               Get(2, 1) = 0.0F;      Get(2, 2) = (zFar + zNear) / deltaZ; Get(2, 3) = 2.0F * zNear * zFar / deltaZ;
    Get(3, 0) = 0.0F;               Get(3, 1) = 0.0F;      Get(3, 2) = -1.0F;                   Get(3, 3) = 0.0F;

    return *this;
}

Matrix4x4f& Matrix4x4f::SetPerspectiveCotan(
    float cotangent,
    float zNear,
    float zFar)
{
    float deltaZ = zNear - zFar;

    Get(0, 0) = cotangent;          Get(0, 1) = 0.0F;      Get(0, 2) = 0.0F;                    Get(0, 3) = 0.0F;
    Get(1, 0) = 0.0F;               Get(1, 1) = cotangent; Get(1, 2) = 0.0F;                    Get(1, 3) = 0.0F;
    Get(2, 0) = 0.0F;               Get(2, 1) = 0.0F;      Get(2, 2) = (zFar + zNear) / deltaZ; Get(2, 3) = 2.0F * zNear * zFar / deltaZ;
    Get(3, 0) = 0.0F;               Get(3, 1) = 0.0F;      Get(3, 2) = -1.0F;                   Get(3, 3) = 0.0F;

    return *this;
}

Matrix4x4f& Matrix4x4f::SetOrtho(
    float left,
    float right,
    float bottom,
    float top,
    float zNear,
    float zFar)
{
    SetIdentity();

    float deltax = right - left;
    float deltay = top - bottom;
    float deltaz = zFar - zNear;

    Get(0, 0) = 2.0F / deltax;
    Get(0, 3) = -(right + left) / deltax;
    Get(1, 1) = 2.0F / deltay;
    Get(1, 3) = -(top + bottom) / deltay;
    Get(2, 2) = -2.0F / deltaz;
    Get(2, 3) = -(zFar + zNear) / deltaz;
    return *this;
}

Matrix4x4f& Matrix4x4f::SetFrustum(
    float left,
    float right,
    float bottom,
    float top,
    float nearval,
    float farval)
{
    float x, y, a, b, c, d, e;

    x =  (2.0F * nearval)       / (right - left);
    y =  (2.0F * nearval)       / (top - bottom);
    a =  (right + left)         / (right - left);
    b =  (top + bottom)         / (top - bottom);
    c = -(farval + nearval)        / (farval - nearval);
    d = -(2.0f * farval * nearval) / (farval - nearval);
    e = -1.0f;

    Get(0, 0) = x;    Get(0, 1) = 0.0;  Get(0, 2) = a;   Get(0, 3) = 0.0;
    Get(1, 0) = 0.0;  Get(1, 1) = y;    Get(1, 2) = b;   Get(1, 3) = 0.0;
    Get(2, 0) = 0.0;  Get(2, 1) = 0.0;  Get(2, 2) = c;   Get(2, 3) = d;
    Get(3, 0) = 0.0;  Get(3, 1) = 0.0;  Get(3, 2) = e;  Get(3, 3) = 0.0;
    return *this;
}

Matrix4x4f& Matrix4x4f::AdjustDepthRange(float newNear, float newFar)
{
    float deltaz = newFar - newNear;
    if (IsPerspective())
    {
        Get(2, 2) = -(newFar + newNear) / deltaz;
        Get(2, 3) = -2.0f * newFar * newNear / deltaz;
    }
    else
    {
        Get(2, 2) = -2.0F / deltaz;
        Get(2, 3) = -(newFar + newNear) / deltaz;
    }
    return *this;
}

TransformType ComputeTransformType(const Matrix4x4f& matrix, float epsilon)
{
    Vector3f outLocalScale;
    outLocalScale.x = SqrMagnitude(matrix.GetAxisX());
    outLocalScale.y = SqrMagnitude(matrix.GetAxisY());
    outLocalScale.z = SqrMagnitude(matrix.GetAxisZ());
    float minAxis = std::min(std::min(outLocalScale.x, outLocalScale.y), outLocalScale.z);
    float maxAxis = std::max(std::max(outLocalScale.x, outLocalScale.y), outLocalScale.z);
    TransformType transType = kNoScaleTransform;
    if (minAxis < 1.0f - epsilon || maxAxis > 1.0f + epsilon)
    {
        if (minAxis != 0.0f && Sqrt(maxAxis) / Sqrt(minAxis) < 1.0f + epsilon)
            transType = kUniformScaleTransform;
        else
            transType = kNonUniformScaleTransform;
    }
    return transType;
}

float ComputeUniformScale(const Matrix4x4f& matrix)
{
    return Magnitude(matrix.GetAxisX());
}

#define MAT(m, r, c) (m)[(c)*4+(r)]

#define RETURN_ZERO PP_WRAP_CODE(\
    for (int i=0;i<16;i++) \
        out[i] = 0.0F; \
    return false;\
)

// 4x4 matrix inversion by Gaussian reduction with partial pivoting followed by back/substitution;
// with loops manually unrolled.

#define SWAP_ROWS(a, b) PP_WRAP_CODE(float *_tmp = a; (a)=(b); (b)=_tmp;)
bool InvertMatrix4x4_Full(const float* m, float* out)
{
    float wtmp[4][8];
    float m0, m1, m2, m3, s;
    float *r0, *r1, *r2, *r3;

    r0 = wtmp[0], r1 = wtmp[1], r2 = wtmp[2], r3 = wtmp[3];

    r0[0] = MAT(m, 0, 0); r0[1] = MAT(m, 0, 1);
    r0[2] = MAT(m, 0, 2); r0[3] = MAT(m, 0, 3);
    r0[4] = 1.0; r0[5] = r0[6] = r0[7] = 0.0;

    r1[0] = MAT(m, 1, 0); r1[1] = MAT(m, 1, 1);
    r1[2] = MAT(m, 1, 2); r1[3] = MAT(m, 1, 3);
    r1[5] = 1.0; r1[4] = r1[6] = r1[7] = 0.0;

    r2[0] = MAT(m, 2, 0); r2[1] = MAT(m, 2, 1);
    r2[2] = MAT(m, 2, 2); r2[3] = MAT(m, 2, 3);
    r2[6] = 1.0; r2[4] = r2[5] = r2[7] = 0.0;

    r3[0] = MAT(m, 3, 0); r3[1] = MAT(m, 3, 1);
    r3[2] = MAT(m, 3, 2); r3[3] = MAT(m, 3, 3);
    r3[7] = 1.0; r3[4] = r3[5] = r3[6] = 0.0;

    /* choose pivot - or die */
    if (Abs(r3[0]) > Abs(r2[0]))
        SWAP_ROWS(r3, r2);
    if (Abs(r2[0]) > Abs(r1[0]))
        SWAP_ROWS(r2, r1);
    if (Abs(r1[0]) > Abs(r0[0]))
        SWAP_ROWS(r1, r0);
    if (0.0F == r0[0])
        RETURN_ZERO;

    /* eliminate first variable     */
    m1 = r1[0] / r0[0]; m2 = r2[0] / r0[0]; m3 = r3[0] / r0[0];
    s = r0[1]; r1[1] -= m1 * s; r2[1] -= m2 * s; r3[1] -= m3 * s;
    s = r0[2]; r1[2] -= m1 * s; r2[2] -= m2 * s; r3[2] -= m3 * s;
    s = r0[3]; r1[3] -= m1 * s; r2[3] -= m2 * s; r3[3] -= m3 * s;
    s = r0[4];
    if (s != 0.0F)
    {
        r1[4] -= m1 * s; r2[4] -= m2 * s; r3[4] -= m3 * s;
    }
    s = r0[5];
    if (s != 0.0F)
    {
        r1[5] -= m1 * s; r2[5] -= m2 * s; r3[5] -= m3 * s;
    }
    s = r0[6];
    if (s != 0.0F)
    {
        r1[6] -= m1 * s; r2[6] -= m2 * s; r3[6] -= m3 * s;
    }
    s = r0[7];
    if (s != 0.0F)
    {
        r1[7] -= m1 * s; r2[7] -= m2 * s; r3[7] -= m3 * s;
    }

    /* choose pivot - or die */
    if (Abs(r3[1]) > Abs(r2[1]))
        SWAP_ROWS(r3, r2);
    if (Abs(r2[1]) > Abs(r1[1]))
        SWAP_ROWS(r2, r1);
    if (0.0F == r1[1])
        RETURN_ZERO;

    /* eliminate second variable */
    m2 = r2[1] / r1[1]; m3 = r3[1] / r1[1];
    r2[2] -= m2 * r1[2]; r3[2] -= m3 * r1[2];
    r2[3] -= m2 * r1[3]; r3[3] -= m3 * r1[3];
    s = r1[4]; if (0.0F != s)
    {
        r2[4] -= m2 * s; r3[4] -= m3 * s;
    }
    s = r1[5]; if (0.0F != s)
    {
        r2[5] -= m2 * s; r3[5] -= m3 * s;
    }
    s = r1[6]; if (0.0F != s)
    {
        r2[6] -= m2 * s; r3[6] -= m3 * s;
    }
    s = r1[7]; if (0.0F != s)
    {
        r2[7] -= m2 * s; r3[7] -= m3 * s;
    }

    /* choose pivot - or die */
    if (Abs(r3[2]) > Abs(r2[2]))
        SWAP_ROWS(r3, r2);
    if (0.0F == r2[2])
        RETURN_ZERO;

    /* eliminate third variable */
    m3 = r3[2] / r2[2];
    r3[3] -= m3 * r2[3]; r3[4] -= m3 * r2[4];
    r3[5] -= m3 * r2[5]; r3[6] -= m3 * r2[6];
    r3[7] -= m3 * r2[7];

    /* last check */
    if (0.0F == r3[3])
        RETURN_ZERO;

    s = 1.0F / r3[3];          /* now back substitute row 3 */
    r3[4] *= s; r3[5] *= s; r3[6] *= s; r3[7] *= s;

    m2 = r2[3];                /* now back substitute row 2 */
    s  = 1.0F / r2[2];
    r2[4] = s * (r2[4] - r3[4] * m2), r2[5] = s * (r2[5] - r3[5] * m2),
    r2[6] = s * (r2[6] - r3[6] * m2), r2[7] = s * (r2[7] - r3[7] * m2);
    m1 = r1[3];
    r1[4] -= r3[4] * m1; r1[5] -= r3[5] * m1,
    r1[6] -= r3[6] * m1; r1[7] -= r3[7] * m1;
    m0 = r0[3];
    r0[4] -= r3[4] * m0; r0[5] -= r3[5] * m0,
    r0[6] -= r3[6] * m0; r0[7] -= r3[7] * m0;

    m1 = r1[2];                /* now back substitute row 1 */
    s  = 1.0F / r1[1];
    r1[4] = s * (r1[4] - r2[4] * m1); r1[5] = s * (r1[5] - r2[5] * m1),
    r1[6] = s * (r1[6] - r2[6] * m1); r1[7] = s * (r1[7] - r2[7] * m1);
    m0 = r0[2];
    r0[4] -= r2[4] * m0; r0[5] -= r2[5] * m0,
    r0[6] -= r2[6] * m0; r0[7] -= r2[7] * m0;

    m0 = r0[1];                /* now back substitute row 0 */
    s  = 1.0F / r0[0];
    r0[4] = s * (r0[4] - r1[4] * m0); r0[5] = s * (r0[5] - r1[5] * m0),
    r0[6] = s * (r0[6] - r1[6] * m0); r0[7] = s * (r0[7] - r1[7] * m0);

    MAT(out, 0, 0) = r0[4]; MAT(out, 0, 1) = r0[5], MAT(out, 0, 2) = r0[6]; MAT(out, 0, 3) = r0[7];
    MAT(out, 1, 0) = r1[4]; MAT(out, 1, 1) = r1[5], MAT(out, 1, 2) = r1[6]; MAT(out, 1, 3) = r1[7];
    MAT(out, 2, 0) = r2[4]; MAT(out, 2, 1) = r2[5], MAT(out, 2, 2) = r2[6]; MAT(out, 2, 3) = r2[7];
    MAT(out, 3, 0) = r3[4]; MAT(out, 3, 1) = r3[5], MAT(out, 3, 2) = r3[6]; MAT(out, 3, 3) = r3[7];

    return true;
}

#undef SWAP_ROWS

// Invert 3D transformation matrix (not perspective). Adapted from graphics gems 2.
// Inverts upper left by calculating its determinant and multiplying it to the symmetric
// adjust matrix of each element. Finally deals with the translation by transforming the
// original translation using by the calculated inverse.
bool InvertMatrix4x4_General3D(const float* in, float* out)
{
#if MATH_HAS_SIMD_FLOAT && defined(__clang__)
    // non-clang compilers don't support complex shuffle instructions such as yxxy
    math::float4 in_c0, in_c1, in_c2, in_c3;
    in_c0 = math::vload4f(in);
    in_c1 = math::vload4f(in + 4);
    in_c2 = math::vload4f(in + 8);
    in_c3 = math::vload4f(in + 12);

    // Calculate the determinant of upper left 3x3 sub-matrix and
    // determine if the matrix is singular.
    math::float4 vdet = in_c0 * in_c1.yzxy * in_c2.zxyz -
        in_c0 * in_c1.zxyz * in_c2.yzxy;

    float det = vdet.x + vdet.y + vdet.z;

    if (det * det < 1e-25)
    {
        math::float4 vzero(math::ZERO);
        math::vstore4f(out, vzero);
        math::vstore4f(out + 4, vzero);
        math::vstore4f(out + 8, vzero);
        math::vstore4f(out + 12, vzero);
        return false;
    }

    det = 1.0f / det;

    math::float4 vdet_pmp = math::float4(det) * math::float4(1, -1, 1, 0);
    math::float4 vdet_mpm = -vdet_pmp;

    // note that the last element is zero in all three following vectors
    math::float4 out_c00_c10_c20 = vdet_pmp * (in_c1.yxxy * in_c2.zzyy - in_c1.zzyy * in_c2.yxxy);
    math::float4 out_c01_c11_c21 = vdet_mpm * (in_c0.yxxy * in_c2.zzyy - in_c0.zzyy * in_c2.yxxy);
    math::float4 out_c02_c12_c22 = vdet_pmp * (in_c0.yxxy * in_c1.zzyy - in_c0.zzyy * in_c1.yxxy);

    math::float4 out_c0, out_c1, out_c2, out_c3;

    // transpose preserving zero in the last element
    math::float4 out_c00_c20_c01_c21 = math::float4(out_c00_c10_c20.xz, out_c01_c11_c21.xz);
    math::float4 out_c10_xxx_c11_xxx = math::float4(out_c00_c10_c20.yw, out_c01_c11_c21.yw);
    out_c0 = math::float4(out_c00_c20_c01_c21.xz, out_c02_c12_c22.xw);
    out_c1 = math::float4(out_c10_xxx_c11_xxx.xz, out_c02_c12_c22.yw);
    out_c2 = math::float4(out_c00_c20_c01_c21.yw, out_c02_c12_c22.zw);

    // Do the translation part
    out_c3 = -(in_c3.xxxx * out_c0 + in_c3.yyyy * out_c1 + in_c3.zzzz * out_c2);
    out_c3 = math::float4(out_c3.xyz, 1.0);

    math::vstore4f(out, out_c0);
    math::vstore4f(out + 4, out_c1);
    math::vstore4f(out + 8, out_c2);
    math::vstore4f(out + 12, out_c3);
    return true;

#else
    float det = 0;

    // Calculate the determinant of upper left 3x3 sub-matrix and
    // determine if the matrix is singular.
    det += MAT(in, 0, 0) * MAT(in, 1, 1) * MAT(in, 2, 2);
    det += MAT(in, 1, 0) * MAT(in, 2, 1) * MAT(in, 0, 2);
    det += MAT(in, 2, 0) * MAT(in, 0, 1) * MAT(in, 1, 2);
    det -= MAT(in, 2, 0) * MAT(in, 1, 1) * MAT(in, 0, 2);
    det -= MAT(in, 1, 0) * MAT(in, 0, 1) * MAT(in, 2, 2);
    det -= MAT(in, 0, 0) * MAT(in, 2, 1) * MAT(in, 1, 2);

    if (det * det < 1e-25)
        RETURN_ZERO;

    det = 1.0F / det;
    MAT(out, 0, 0) = ((MAT(in, 1, 1) * MAT(in, 2, 2) - MAT(in, 2, 1) * MAT(in, 1, 2)) * det);
    MAT(out, 0, 1) = (-(MAT(in, 0, 1) * MAT(in, 2, 2) - MAT(in, 2, 1) * MAT(in, 0, 2)) * det);
    MAT(out, 0, 2) = ((MAT(in, 0, 1) * MAT(in, 1, 2) - MAT(in, 1, 1) * MAT(in, 0, 2)) * det);
    MAT(out, 1, 0) = (-(MAT(in, 1, 0) * MAT(in, 2, 2) - MAT(in, 2, 0) * MAT(in, 1, 2)) * det);
    MAT(out, 1, 1) = ((MAT(in, 0, 0) * MAT(in, 2, 2) - MAT(in, 2, 0) * MAT(in, 0, 2)) * det);
    MAT(out, 1, 2) = (-(MAT(in, 0, 0) * MAT(in, 1, 2) - MAT(in, 1, 0) * MAT(in, 0, 2)) * det);
    MAT(out, 2, 0) = ((MAT(in, 1, 0) * MAT(in, 2, 1) - MAT(in, 2, 0) * MAT(in, 1, 1)) * det);
    MAT(out, 2, 1) = (-(MAT(in, 0, 0) * MAT(in, 2, 1) - MAT(in, 2, 0) * MAT(in, 0, 1)) * det);
    MAT(out, 2, 2) = ((MAT(in, 0, 0) * MAT(in, 1, 1) - MAT(in, 1, 0) * MAT(in, 0, 1)) * det);

    // Do the translation part
    MAT(out, 0, 3) = -(MAT(in, 0, 3) * MAT(out, 0, 0) +
        MAT(in, 1, 3) * MAT(out, 0, 1) +
        MAT(in, 2, 3) * MAT(out, 0, 2));
    MAT(out, 1, 3) = -(MAT(in, 0, 3) * MAT(out, 1, 0) +
        MAT(in, 1, 3) * MAT(out, 1, 1) +
        MAT(in, 2, 3) * MAT(out, 1, 2));
    MAT(out, 2, 3) = -(MAT(in, 0, 3) * MAT(out, 2, 0) +
        MAT(in, 1, 3) * MAT(out, 2, 1) +
        MAT(in, 2, 3) * MAT(out, 2, 2));

    MAT(out, 3, 0) = 0.0f;
    MAT(out, 3, 1) = 0.0f;
    MAT(out, 3, 2) = 0.0f;
    MAT(out, 3, 3) = 1.0f;

    return true;
#endif
}

#undef MAT
#undef RETURN_ZERO

Matrix4x4f& Matrix4x4f::Transpose()
{
    std::swap(Get(0, 1), Get(1, 0));
    std::swap(Get(0, 2), Get(2, 0));
    std::swap(Get(0, 3), Get(3, 0));
    std::swap(Get(1, 2), Get(2, 1));
    std::swap(Get(1, 3), Get(3, 1));
    std::swap(Get(2, 3), Get(3, 2));
    return *this;
}

Matrix4x4f& Matrix4x4f::SetFromToRotation(const Vector3f& from, const Vector3f& to)
{
    Matrix3x3f mat;
    mat.SetFromToRotation(from, to);
    *this = mat;
    return *this;
}

bool CompareApproximately(const Matrix4x4f& lhs, const Matrix4x4f& rhs, float dist)
{
    for (int i = 0; i < 16; i++)
    {
        if (!CompareApproximately(lhs[i], rhs[i], dist))
            return false;
    }
    return true;
}

void Matrix4x4f::SetTR(const Vector3f& pos, const Quaternionf& q)
{
    QuaternionToMatrix(q, *this);
    m_Data[12] = pos[0];
    m_Data[13] = pos[1];
    m_Data[14] = pos[2];
}

void Matrix4x4f::SetTRS(const Vector3f& pos, const Quaternionf& q, const Vector3f& s)
{
    QuaternionToMatrix(q, *this);

    m_Data[0] *= s[0];
    m_Data[1] *= s[0];
    m_Data[2] *= s[0];

    m_Data[4] *= s[1];
    m_Data[5] *= s[1];
    m_Data[6] *= s[1];

    m_Data[8] *= s[2];
    m_Data[9] *= s[2];
    m_Data[10] *= s[2];

    m_Data[12] = pos[0];
    m_Data[13] = pos[1];
    m_Data[14] = pos[2];
}

void Matrix4x4f::SetTRInverse(const Vector3f& pos, const Quaternionf& q)
{
    QuaternionToMatrix(::Inverse(q), *this);
    Translate(Vector3f(-pos[0], -pos[1], -pos[2]));
}

void TransformPoints3x3(const Matrix4x4f& matrix, const Vector3f* in, Vector3f* out, int count)
{
    Matrix3x3f m = Matrix3x3f(matrix);
    for (int i = 0; i < count; i++)
        out[i] = m.MultiplyPoint3(in[i]);
}

void TransformPoints3x4(const Matrix4x4f& matrix, const Vector3f* in, Vector3f* out, int count)
{
    for (int i = 0; i < count; i++)
        out[i] = matrix.MultiplyPoint3(in[i]);
}

void TransformPoints3x3(const Matrix4x4f& matrix, const Vector3f* in, size_t inStride, Vector3f* out, size_t outStride, int count)
{
    Matrix3x3f m = Matrix3x3f(matrix);
    for (int i = 0; i < count; ++i, in = Stride(in, inStride), out = Stride(out, outStride))
    {
        *out = m.MultiplyPoint3(*in);
    }
}

void TransformPoints3x4(const Matrix4x4f& matrix, const Vector3f* in, size_t inStride, Vector3f* out, size_t outStride, int count)
{
    for (int i = 0; i < count; ++i, in = Stride(in, inStride), out = Stride(out, outStride))
    {
        *out = matrix.MultiplyPoint3(*in);
    }
}

FrustumPlanes Matrix4x4f::DecomposeProjection() const
{
    FrustumPlanes planes;

    if (IsPerspective())
    {
        planes.zNear = Get(2, 3) / (Get(2, 2) - 1.0f);
        planes.zFar = Get(2, 3) / (Get(2, 2) + 1.0f);
        planes.right = planes.zNear * (1.0f + Get(0, 2)) / Get(0, 0);
        planes.left = planes.zNear * (-1.0f + Get(0, 2)) / Get(0, 0);
        planes.top = planes.zNear  * (1.0f + Get(1, 2)) / Get(1, 1);
        planes.bottom = planes.zNear  * (-1.0f + Get(1, 2)) / Get(1, 1);
    }
    else
    {
        planes.zNear = (Get(2, 3) + 1.0f) / Get(2, 2);
        planes.zFar =  (Get(2, 3) - 1.0f) / Get(2, 2);
        planes.right = (1.0f - Get(0, 3)) / Get(0, 0);
        planes.left = (-1.0f - Get(0, 3)) / Get(0, 0);
        planes.top = (1.0f - Get(1, 3)) / Get(1, 1);
        planes.bottom = (-1.0f - Get(1, 3)) / Get(1, 1);
    }

    return planes;
}

Quaternionf Matrix4x4f::GetRotation() const
{
    Assert(ValidTRS());

    math::float3x3 rotMat = Matrix4x4fTofloat3x3(*this);
    rotMat = math::rotation(rotMat);

    Quaternionf result;
    MatrixToQuaternion(float3x3ToMatrix3x3f(rotMat), result);

    return result;
}

Vector3f Matrix4x4f::GetLossyScale() const
{
    Assert(ValidTRS());

    Vector3f result;
    result.x = Magnitude(GetAxisX());
    result.y = Magnitude(GetAxisY());
    result.z = Magnitude(GetAxisZ());

    float determinant = Matrix3x3f(*this).GetDeterminant();
    if (determinant < 0)
        result.x *= -1;

    return result;
}

bool Matrix4x4f::ValidTRS() const
{
    return Get(3, 0) == 0 && Get(3, 1) == 0 && Get(3, 2) == 0 && fabs(Get(3, 3)) == 1;
}

#if ENABLE_UNIT_TESTS
#include "External/UnitTest++/Runtime/MemoryOutStream.h"
UnitTest::MemoryOutStream& operator<<(UnitTest::MemoryOutStream& stream, const Matrix4x4f& m)
{
    stream << "(" <<
    m[0] << ", " << m[1] << ", " << m[2] << ", " << m[3] << ", " <<
    m[4] << ", " << m[5] << ", " << m[6] << ", " << m[7] << ", " <<
    m[8] << ", " << m[9] << ", " << m[10] << ", " << m[11] << ", " <<
    m[12] << ", " << m[13] << ", " << m[14] << ", " << m[15] << ")";
    return stream;
}

#endif
