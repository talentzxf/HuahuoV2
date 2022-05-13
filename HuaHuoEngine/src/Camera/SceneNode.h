//
// Created by VincentZhang on 5/13/2022.
//

#ifndef HUAHUOENGINE_SCENENODE_H
#define HUAHUOENGINE_SCENENODE_H
#include "baselib/include/IntegerDefinitions.h"
#include "SharedRendererDataTypes.h"

class BaseRenderer;

struct SceneNode
{
    SceneNode() :
#if UNITY_EDITOR
            sceneMask(kDefaultSceneCullingMask),
#endif
            renderer(NULL),
            layer(0), pvsHandle(-1), lodManagerID(0), lodIndexMask(0), dynamicOccludee(1),
            lodGroupIndex(0), needsCullCallback(0), disable(0), shadowCastingMode(0)
    {}

    enum { kLODGroupIndexBits = 28 };

#if UNITY_EDITOR
    UInt64          sceneMask;
#endif
    BaseRenderer*   renderer;
    UInt32          layer;
    SInt32          pvsHandle;
    UInt16          lodManagerID;
    UInt8           lodIndexMask;
    UInt8           dynamicOccludee;
    UInt32          lodGroupIndex : kLODGroupIndexBits;
    UInt32          needsCullCallback : 1;
    UInt32          disable : 1;
    UInt32          shadowCastingMode : kShadowCastingModeBitSize;
};

typedef int SceneHandle;
const int kInvalidSceneHandle = -1;

#endif //HUAHUOENGINE_SCENENODE_H
