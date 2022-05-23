//
// Created by VincentZhang on 5/23/2022.
//

#ifndef HUAHUOENGINE_BATCHRENDERERGROUP_H
#define HUAHUOENGINE_BATCHRENDERERGROUP_H
#include "Core/SharedObject.h"
#include "Job/JobTypes.h"
#include "Math/Matrix4x4.h"
#include "Geometry/Plane.h"
#include <vector>

struct BatchVisibility
{
    int offset;
    int instancesCount;
    int visibleCount;
};

struct BatchRendererCullingOutput
{
    BatchRendererCullingOutput() {}

    mutable JobFence    cullingJobsFence;

    // general culling

    // safety
    Matrix4x4f          cullingMatrix;
    Plane*              cullingPlanes;  // for shadow culling, these will hold the receiver shadow frustum
    BatchVisibility*    batchVisibility;
    int*                visibleIndices;
    int*                visibleIndicesY;

    int                 cullingPlanesCount;
    int                 batchVisibilityCount;
    int                 visibleIndicesCount;
    float               nearZ;

    void SetCullingPlanes(int planesCount, const Plane* planes)
    {
        cullingPlanesCount = planesCount;
        cullingPlanes = HUAHUO_NEW(Plane, kMemTempJobAlloc)[planesCount];
        memcpy(cullingPlanes, planes, planesCount * sizeof(Plane));
    }

    void Deallocate();
};

struct BatchRendererCullingOutputs : public ThreadSharedObject<BatchRendererCullingOutputs>
{
    std::vector<BatchRendererCullingOutput> contexts;

    BatchRendererCullingOutputs()
            : ThreadSharedObject<BatchRendererCullingOutputs>(kMemTempJobAlloc)
            /*, contexts(kMemTempJobAlloc)*/ {}

    void    SyncFence();
    ~BatchRendererCullingOutputs();

    void GetDependencies(JobFence* dependencies);
    size_t GetDependenciesCount() const { return contexts.size(); }
};
#endif //HUAHUOENGINE_BATCHRENDERERGROUP_H
