//
// Created by VincentZhang on 5/18/2022.
//

#ifndef HUAHUOENGINE_SHAREDRENDERERSCENE_H
#define HUAHUOENGINE_SHAREDRENDERERSCENE_H


#include "RenderNodeQueue.h"

struct SharedRendererScene {
    RenderNodeQueue                         queue;

    explicit SharedRendererScene(/*MemLabelRef label*/)
            // : ThreadSharedObject<SharedRendererScene>(label)
            // , queue(label)
//            , commandBufferQueue(label)
//            , perObjectLightCulling(label)
//            , reflectionProbesContext(label)
//            , lightProbeProxyVolumeContext(label)
//            , sortingGroupDataArray(label)
//            , lightmapSettingsData(NULL)
//            , enableRealtimeLightmaps(false)
    {
    }

    ~SharedRendererScene()
    {
//        queue.SyncDependentJobs();
//        commandBufferQueue.SyncDependentJobs();
//        SyncFence(perObjectLightCulling.lightCullingFence);
//        SyncFence(perObjectLightCulling.probeCullingFence);
//        SAFE_RELEASE(lightmapSettingsData);
    }
};


#endif //HUAHUOENGINE_SHAREDRENDERERSCENE_H
