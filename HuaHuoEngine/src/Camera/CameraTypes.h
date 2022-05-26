//
// Created by VincentZhang on 5/24/2022.
//

#ifndef HUAHUOENGINE_CAMERATYPES_H
#define HUAHUOENGINE_CAMERATYPES_H

#include "Math/Matrix4x4.h"

struct CameraTemporarySettings
{
    int   renderingPath;
    float fieldOfView;
    float aspect;
    int fovAxisMode;
    bool  implicitAspect;
};


struct CameraRenderingParams
{
    Matrix4x4f matView;
    Matrix4x4f matProj;
    Vector3f worldPosition;
    float stereoSeparation;
};

#endif //HUAHUOENGINE_CAMERATYPES_H
