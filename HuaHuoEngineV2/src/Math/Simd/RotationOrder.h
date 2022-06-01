//
// Created by VincentZhang on 4/23/2022.
//

#ifndef HUAHUOENGINE_ROTATIONORDER_H
#define HUAHUOENGINE_ROTATIONORDER_H
namespace math
{
    enum RotationOrder
    {
        kOrderXYZ,
        kOrderXZY,
        kOrderYZX,
        kOrderYXZ,
        kOrderZXY,
        kOrderZYX,
        kRotationOrderLast = kOrderZYX,
        kOrderUnityDefault = kOrderZXY
    };

    const int kRotationOrderCount = kRotationOrderLast + 1;
}
#endif //HUAHUOENGINE_ROTATIONORDER_H
