//
// Created by VincentZhang on 5/18/2022.
//

#ifndef HUAHUOENGINE_SCRIPTABLEDRAWRENDERERSPRIVATE_H
#define HUAHUOENGINE_SCRIPTABLEDRAWRENDERERSPRIVATE_H
#include "Configuration/IntegerDefinitions.h"
#include "Camera/RenderLoops/RenderLoopPrivate.h"
#include "Shaders/SharedMaterialData.h"
#include "Camera/RenderNodeQueue.h"
#include <vector>

namespace ShaderLab { class Pass; }
struct ScriptableLoopObjectData
{
    RenderObjectData            data;
    const SharedMaterialData*   sharedMaterial;
    const ShaderLab::Pass*      pass;
    UInt32                      passIndex;
    UInt32                      passOrder;
};

typedef std::vector<ScriptableLoopObjectData> ScriptableLoopObjectDatas;

struct OverrideMaterialInfo : MaterialInfo
{
    int passIndex;
};

#endif //HUAHUOENGINE_SCRIPTABLEDRAWRENDERERSPRIVATE_H
