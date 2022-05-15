//
// Created by VincentZhang on 4/24/2022.
//

#ifndef HUAHUOENGINE_TRANSFORMHIERARCHY_H
#define HUAHUOENGINE_TRANSFORMHIERARCHY_H

#include "Math/Simd/vec-types.h"
#include "TransformAccess.h"
#include "TransformChangeSystemMask.h"
#include "Math/Simd/vec-trs.h"
#include "TransformHierarchyTypes.h"
#include "Math/TransformType.h"

#if UNITY_RELEASE
#define TRANSFORM_FORCEINLINE_IN_RELEASE UNITY_FORCEINLINE
#else
#define TRANSFORM_FORCEINLINE_IN_RELEASE inline
#endif

#define ASSERT_TRANSFORM_ACCESS(transformAccess) DebugAssert(transformAccess.index < transformAccess.hierarchy->transformCapacity && transformAccess.hierarchy->deepChildCount[transformAccess.index] > 0)

#define RETURN_VECTOR3(x) Vector3f outVec; math::vstore3f(outVec.GetPtr(), x); return outVec;
#define RETURN_QUATERNION(x) Quaternionf outQuat; math::vstore4f(outQuat.GetPtr(), x); return outQuat;
#define RETURN_MATRIX3X3(x) Matrix3x3f outMat; float3x3ToMatrix3x3(x,outMat.m_Data); return outMat;
#define RETURN_AFFINE4X4(x) Matrix4x4f outMat; affineXToMatrix4x4(x,outMat.m_Data); return outMat;

inline UInt32 GetDeepChildCount(const TransformHierarchy& hierarchy, UInt32 index)
{
    DebugAssert(index < hierarchy.transformCapacity && hierarchy.deepChildCount[index] > 0);
    return hierarchy.deepChildCount[index];
}

inline UInt32 GetDeepChildCount(const TransformAccessReadOnly& transformAccess)
{
    ASSERT_TRANSFORM_ACCESS(transformAccess);
    return transformAccess.hierarchy->deepChildCount[transformAccess.index];
}

inline const math::trsX& GetLocalTRS(TransformAccessReadOnly transformAccess)
{
    ASSERT_TRANSFORM_ACCESS(transformAccess);
    return transformAccess.hierarchy->localTransforms[transformAccess.index];
}

inline TransformAccessReadOnly GetParent(TransformAccessReadOnly transformAccess)
{
    ASSERT_TRANSFORM_ACCESS(transformAccess);

    TransformAccess output;
    output.index = transformAccess.hierarchy->parentIndices[transformAccess.index];
    output.hierarchy = transformAccess.hierarchy;
    return output;
}

namespace TransformInternal {
    struct ChangeMaskCache {
        TransformChangeSystemMask localT;
        TransformChangeSystemMask localR;
        TransformChangeSystemMask localS;
        TransformChangeSystemMask globalT;
        TransformChangeSystemMask globalR;
        TransformChangeSystemMask globalS;
    };

    extern ChangeMaskCache g_ChangeMaskCache;
    inline math::trsX& GetLocalTRSWritable(TransformAccess transformAccess)
    {
        ASSERT_TRANSFORM_ACCESS(transformAccess);
        return transformAccess.hierarchy->localTransforms[transformAccess.index];
    }


    inline bool SetLocalR(math::trsX &trs, const math::float4 &r) {
        using namespace math;

        float4 normalizedR = quatNormalize(r);
        bool ret = any(normalizedR != trs.q);
        trs.q = normalizedR;

        return ret;
    }

    inline bool SetLocalT(math::trsX& trs, const math::float3& t)
    {
        using namespace math;

        bool ret = any(t != trs.t);
        trs.t = t;

        return ret;
    }

    void InitLocalTRS(TransformAccess transformAccess, const math::float3& t, const math::float4& r, const math::float3& s);

    // Apply transform change down the children hierarchy.
    inline void OnTransformChangedMask(TransformAccess access, TransformChangeSystemMask localOnlyMask, TransformChangeSystemMask commonMask, TransformChangeSystemMask childrenOnlyMask) {
        TransformHierarchy &hierarchy = *access.hierarchy;
        UInt32 index = access.index;

        TransformChangeSystemMask localSystemInterested =
                hierarchy.systemInterested[index] & (localOnlyMask | commonMask);
        hierarchy.systemChanged[index] |= localSystemInterested;
        hierarchy.combinedSystemChanged |= localSystemInterested;

        UInt32 count = GetDeepChildCount(hierarchy, index);
        SInt32 cur = hierarchy.nextIndices[index];
        for (UInt32 i = 1; i < count; i++) {
            TransformChangeSystemMask systemInterested =
                    hierarchy.systemInterested[cur] & (commonMask | childrenOnlyMask);

            hierarchy.systemChanged[cur] |= systemInterested;
            hierarchy.combinedSystemChanged |= systemInterested;

            cur = hierarchy.nextIndices[cur];
        }
    }

    TransformHierarchy* CreateTransformHierarchy(UInt32 transformCapacity);
    void DestroyTransformHierarchy(TransformHierarchy* hierarchy);
    void AllocateTransformThread(TransformHierarchy& hierarchy, UInt32 threadFirst, UInt32 threadLast);

    void AddTransformSubhierarchy(TransformHierarchy& srcHierarchy, UInt32 srcIndex, TransformHierarchy& dstHierarchy, UInt32& dstFirst, UInt32& dstLast, TransformChangeSystemMask interestMask, TransformChangeSystemMask changeMask, UInt32 hierarchyInterestMask, bool copyForCloning);
    void CopyTransformSubhierarchy(TransformHierarchy& srcHierarchy, UInt32 srcIndex, TransformHierarchy& dstHierarchy, TransformChangeSystemMask interestMask, TransformChangeSystemMask changeMask, UInt32 hierarchyInterestMask, bool copyForCloning);

    void InsertTransformThreadAfter(TransformHierarchy& hierarchy, UInt32 index, UInt32 threadFirst, UInt32 threadLast);
    void DetachTransformThread(TransformHierarchy& hierarchy, UInt32 threadFirst, UInt32 threadLast);
    void FreeTransformThread(TransformHierarchy& hierarchy, UInt32 threadFirst, UInt32 threadLast);
    void UpdateDeepChildCountUpwards(TransformHierarchy& hierarchy, SInt32 index, SInt32 addedNodeCount);

    TransformType CalculateTransformType(TransformAccess transformAccess);

    inline void CalculateGlobalPositionAndRotation(TransformAccessReadOnly transformAccess, math::float3 &position, math::float4 &rotation)
    {
        ASSERT_TRANSFORM_ACCESS(transformAccess);

        using namespace math;

        const trsX* localTransforms = transformAccess.hierarchy->localTransforms;
        const trsX& trs = localTransforms[transformAccess.index];

        position = trs.t;
        rotation = trs.q;

        SInt32 *parentIndices = transformAccess.hierarchy->parentIndices;
        SInt32 parentIndex = parentIndices[transformAccess.index];

        while (parentIndex >= 0)
        {
            const trsX& ptrs = localTransforms[parentIndex];

            position = mul(ptrs, position);
            rotation = scaleMulQuat(ptrs.s, rotation);
            rotation = quatMul(ptrs.q, rotation);

            parentIndex = parentIndices[parentIndex];
        }
    }



    TRANSFORM_FORCEINLINE_IN_RELEASE void AssertTransformAccess(const TransformAccessReadOnly& transformAccess)
    {
        ASSERT_TRANSFORM_ACCESS(transformAccess);
    }



    inline bool SetLocalS(math::trsX& trs, const math::float3& s)
    {
        using namespace math;

        bool ret = any(s != trs.s);
        trs.s = s;

        return ret;
    }

    void OnScaleChangedCalculateTransformType(TransformAccess transformAccess);

    TRANSFORM_FORCEINLINE_IN_RELEASE void InverseTransformPositionAndRotationNonRecursive(TransformAccessReadOnly transformAccess, math::float3& p, math::float4& r)
    {
        ASSERT_TRANSFORM_ACCESS(transformAccess);

        using namespace math;

        const trsX &trs = GetLocalTRS(transformAccess);

        p = invMul(trs, p);

        r = quatMul(quatConj(trs.q), r);
        r = scaleMulQuat(trs.s, r);
    }

}

// returns true if local position change
inline bool SetLocalT(TransformAccess transformAccess, const math::float3& t, TransformChangeSystemMask extraSystems = TransformChangeSystemMask(0))
{
    if (TransformInternal::SetLocalT(TransformInternal::GetLocalTRSWritable(transformAccess), t))
    {
        TransformChangeSystemMask mask = TransformInternal::g_ChangeMaskCache.globalT | extraSystems;
        TransformInternal::OnTransformChangedMask(transformAccess, TransformInternal::g_ChangeMaskCache.localT, mask, TransformChangeSystemMask(0));
        return true;
    }

    return false;
}

// returns true if local rotation change
inline bool SetLocalR(TransformAccess transformAccess, const math::float4 &r,
                      TransformChangeSystemMask extraSystems = TransformChangeSystemMask(0)) {
    if (TransformInternal::SetLocalR(TransformInternal::GetLocalTRSWritable(transformAccess), r)) {
        // note:
        // Rotation change may induce global scale matrix (or global lossy scale) change in presence of non-uniform scale
        // Old code was not trapping it either. It is an accepatable known issue.
        TransformChangeSystemMask mask = TransformInternal::g_ChangeMaskCache.globalR | extraSystems;
        TransformInternal::OnTransformChangedMask(transformAccess, TransformInternal::g_ChangeMaskCache.localR, mask,
                                                  TransformInternal::g_ChangeMaskCache.globalT);
        return true;
    }

    return false;
}

inline void InverseTransformPosition(TransformAccessReadOnly transformAccess, math::float3& p)
{
    ASSERT_TRANSFORM_ACCESS(transformAccess);

    using namespace math;

    if (transformAccess.index > 0)
        InverseTransformPosition(GetParent(transformAccess), p);

    const trsX &trs = GetLocalTRS(transformAccess);
    p = invMul(trs, p);
}

inline void InverseTransformRotation(const TransformAccessReadOnly &transformAccess, math::float4& r)
{
    ASSERT_TRANSFORM_ACCESS(transformAccess);

    using namespace math;

    if (transformAccess.index > 0)
        InverseTransformRotation(GetParent(transformAccess), r);

    const trsX &trs = GetLocalTRS(transformAccess);

    r = quatMul(quatConj(trs.q), r);
    r = scaleMulQuat(trs.s, r);
}

inline bool SetGlobalR(TransformAccess transformAccess, const math::float4& gr)
{
    ASSERT_TRANSFORM_ACCESS(transformAccess);

    math::float4 lr = gr;
    if (transformAccess.index > 0)
        InverseTransformRotation(GetParent(transformAccess), lr);
    return SetLocalR(transformAccess, lr);
}

inline math::float3x3 CalculateGlobalRS(TransformAccessReadOnly transformAccess)
{
    ASSERT_TRANSFORM_ACCESS(transformAccess);

    using namespace math;

    float3x3 parentRS;
    float3x3 globalRS;

    const trsX* localTransforms = transformAccess.hierarchy->localTransforms;
    const trsX& trs = localTransforms[transformAccess.index];

    quatToMatrix(trs.q, globalRS);
    globalRS = mulScale(globalRS, trs.s);

    SInt32 *parentIndices = transformAccess.hierarchy->parentIndices;
    SInt32 parentIndex = parentIndices[transformAccess.index];

    while (parentIndex >= 0)
    {
        const trsX& ptrs = localTransforms[parentIndex];

        quatToMatrix(ptrs.q, parentRS);
        parentRS = mulScale(parentRS, ptrs.s);
        globalRS = mul(parentRS, globalRS);
        parentIndex = parentIndices[parentIndex];
    }

    return globalRS;
}

inline math::float3x3 CalculateGlobalSM(TransformAccessReadOnly transformAccess, const math::float4 &globalR)
{
    using namespace math;

    float3x3 grmInv;
    quatToMatrix(quatConj(globalR), grmInv);
    float3x3 grsm = CalculateGlobalRS(transformAccess);
    return mul(grmInv, grsm);
}

inline void InverseTransformPositionAndRotation(const TransformAccessReadOnly &transformAccess, math::float3& p, math::float4& r)
{
    TransformInternal::AssertTransformAccess(transformAccess);

    if (transformAccess.index > 0)
        InverseTransformPositionAndRotation(GetParent(transformAccess), p, r);

    TransformInternal::InverseTransformPositionAndRotationNonRecursive(transformAccess, p, r);
}

inline bool SetLocalTR(TransformAccess transformAccess, const math::float3& t, const math::float4& r, TransformChangeSystemMask extraSystems = TransformChangeSystemMask(0))
{
    math::trsX& trs = TransformInternal::GetLocalTRSWritable(transformAccess);
    bool tChanged = TransformInternal::SetLocalT(trs, t);
    bool rChanged = TransformInternal::SetLocalR(trs, r);

    if (tChanged | rChanged)
    {
        // Check internal state (We use multiply later so value really must be 0 or 1)
        DebugAssert(tChanged == 0 || tChanged == 1);
        DebugAssert(rChanged == 0 || rChanged == 1);

        TransformChangeSystemMask localOnlyMask = tChanged * TransformInternal::g_ChangeMaskCache.localT | rChanged * TransformInternal::g_ChangeMaskCache.localR;
        TransformChangeSystemMask commonMask = tChanged * TransformInternal::g_ChangeMaskCache.globalT | rChanged * TransformInternal::g_ChangeMaskCache.globalR | extraSystems;
        TransformChangeSystemMask childOnlyMask = rChanged * TransformInternal::g_ChangeMaskCache.globalT;
        TransformInternal::OnTransformChangedMask(transformAccess, localOnlyMask, commonMask, childOnlyMask);
        return true;
    }

    return false;
}

inline bool SetGlobalTR(TransformAccess transformAccess, const math::float3& gt, const math::float4& gr)
{
    ASSERT_TRANSFORM_ACCESS(transformAccess);

    math::float3 lt = gt;
    math::float4 lr = gr;
    if (transformAccess.index > 0)
        InverseTransformPositionAndRotation(GetParent(transformAccess), lt, lr);
    return SetLocalTR(transformAccess, lt, lr);
}


// returns true if local scale change
inline bool SetLocalS(TransformAccess transformAccess, const math::float3& s, TransformChangeSystemMask extraSystems = TransformChangeSystemMask(0))
{
    if (TransformInternal::SetLocalS(TransformInternal::GetLocalTRSWritable(transformAccess), s))
    {
        TransformInternal::OnScaleChangedCalculateTransformType(transformAccess);
        TransformChangeSystemMask mask = TransformInternal::g_ChangeMaskCache.globalS | extraSystems;
        TransformInternal::OnTransformChangedMask(transformAccess, TransformInternal::g_ChangeMaskCache.localS, mask, TransformInternal::g_ChangeMaskCache.globalR | TransformInternal::g_ChangeMaskCache.globalT);
        return true;
    }

    return false;
}

inline bool SetGlobalMatrixLossy(TransformAccess transformAccess, const math::float3& gt, const math::float4& gr, const math::float3x3& gsm)
{
    ASSERT_TRANSFORM_ACCESS(transformAccess);

    using namespace math;

    bool trChanged = SetGlobalTR(transformAccess, gt, gr);

    float3 ls = float3(gsm.m0.x, gsm.m1.y, gsm.m2.z);

    if (transformAccess.index > 0)
    {
        float3x3 grmInv;
        quatToMatrix(quatConj(gr), grmInv);

        float3x3 lrm;
        quatToMatrix(GetLocalTRS(transformAccess).q, lrm);

        float3x3 pgsm = mul(grmInv, mul(CalculateGlobalRS(GetParent(transformAccess)), lrm));
        ls = inverseScale(float3(pgsm.m0.x, pgsm.m1.y, pgsm.m2.z)) * ls;
    }

    bool sChanged = SetLocalS(transformAccess, ls);

    return trChanged | sChanged;
}

inline bool SetGlobalT(TransformAccess transformAccess, const math::float3& gt)
{
    ASSERT_TRANSFORM_ACCESS(transformAccess);

    math::float3 lt = gt;
    if (transformAccess.index > 0)
        InverseTransformPosition(GetParent(transformAccess), lt);
    return SetLocalT(transformAccess, lt);
}

inline math::float3 CalculateGlobalPosition(TransformAccessReadOnly transformAccess)
{
    ASSERT_TRANSFORM_ACCESS(transformAccess);

    using namespace math;

    const trsX* localTransforms = transformAccess.hierarchy->localTransforms;

    float3 globalT = localTransforms[transformAccess.index].t;

    SInt32 *parentIndices = transformAccess.hierarchy->parentIndices;
    SInt32 parentIndex = parentIndices[transformAccess.index];

    while (parentIndex >= 0)
    {
        globalT = mul(localTransforms[parentIndex], globalT);
        parentIndex = parentIndices[parentIndex];
    }

    return globalT;
}

#endif //HUAHUOENGINE_TRANSFORMHIERARCHY_H
