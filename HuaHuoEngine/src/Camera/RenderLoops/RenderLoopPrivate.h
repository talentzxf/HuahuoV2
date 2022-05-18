//
// Created by VincentZhang on 5/18/2022.
//

#ifndef HUAHUOENGINE_RENDERLOOPPRIVATE_H
#define HUAHUOENGINE_RENDERLOOPPRIVATE_H

#include "Configuration/IntegerDefinitions.h"

struct RenderObjectData
{
    // Shader* shader;
    UInt32  materialSortKey;
    UInt32  nodeIndex;
    SInt16  queueIndex;
    UInt16  subsetIndex;
    SInt16  subShaderIndex;
    SInt32  rendererPriority;
    UInt16  srpBatcherCompatible : 1;
    UInt16  sourceMaterialIndex : 15;
    float   distance;
    float   distanceAlongView;
    UInt32  geometrySortKey;
    // GlobalLayeringData globalLayeringData;
};

#endif //HUAHUOENGINE_RENDERLOOPPRIVATE_H
