//
// Created by VincentZhang on 4/23/2022.
//

#include "Vector3f.h"
#include <limits>

const float     Vector3f::epsilon = 0.00001F;
const float     Vector3f::infinity = std::numeric_limits<float>::infinity();
const Vector3f  Vector3f::infinityVec = Vector3f(std::numeric_limits<float>::infinity(), std::numeric_limits<float>::infinity(), std::numeric_limits<float>::infinity());

const Vector3f  Vector3f::zero  = Vector3f(0, 0, 0);
const Vector3f  Vector3f::one  = Vector3f(1.0F, 1.0F, 1.0F);
const Vector3f  Vector3f::xAxis = Vector3f(1, 0, 0);
const Vector3f  Vector3f::yAxis = Vector3f(0, 1, 0);
const Vector3f  Vector3f::zAxis = Vector3f(0, 0, 1);