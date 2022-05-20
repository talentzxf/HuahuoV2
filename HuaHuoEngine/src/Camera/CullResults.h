//
// Created by VincentZhang on 5/20/2022.
//

#ifndef HUAHUOENGINE_CULLRESULTS_H
#define HUAHUOENGINE_CULLRESULTS_H

#include "CullingParameters.h"

void SetCullingPlanes(CullingParameters& parameters, const Plane* planes, int planeCount);
void CalculateCustomCullingParameters(CullingParameters& cullingParameters, const LODParameters& lodParams, UInt32 cullingMask, UInt64 sceneCullingMask, const Plane* planes, int planeCount);

#endif //HUAHUOENGINE_CULLRESULTS_H
