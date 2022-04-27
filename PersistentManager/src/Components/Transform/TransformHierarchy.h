//
// Created by VincentZhang on 4/24/2022.
//

#ifndef PERSISTENTMANAGER_TRANSFORMHIERARCHY_H
#define PERSISTENTMANAGER_TRANSFORMHIERARCHY_H

#include "Math/Simd/vec-types.h"
#include "TransformAccess.h"
#include "TransformChangeSystemMask.h"
#include "Math/Simd/vec-trs.h"
#include "TransformHierarchyTypes.h"

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

inline void InverseTransformPosition(TransformAccessReadOnly transformAccess, math::float3& p)
{
    ASSERT_TRANSFORM_ACCESS(transformAccess);

    using namespace math;

    if (transformAccess.index > 0)
        InverseTransformPosition(GetParent(transformAccess), p);

    const trsX &trs = GetLocalTRS(transformAccess);
    p = invMul(trs, p);
}

inline bool SetGlobalR(TransformAccess transformAccess, const math::float4& gr)
{
    ASSERT_TRANSFORM_ACCESS(transformAccess);

    math::float4 lr = gr;
    if (transformAccess.index > 0)
        InverseTransformRotation(GetParent(transformAccess), lr);
    return SetLocalR(transformAccess, lr);
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

inline bool SetGlobalT(TransformAccess transformAccess, const math::float3& gt)
{
    ASSERT_TRANSFORM_ACCESS(transformAccess);

    math::float3 lt = gt;
    if (transformAccess.index > 0)
        InverseTransformPosition(GetParent(transformAccess), lt);
    return SetLocalT(transformAccess, lt);
}

#endif //PERSISTENTMANAGER_TRANSFORMHIERARCHY_H
