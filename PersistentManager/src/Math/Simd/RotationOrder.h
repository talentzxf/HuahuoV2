//
// Created by VincentZhang on 4/23/2022.
//

#ifndef PERSISTENTMANAGER_ROTATIONORDER_H
#define PERSISTENTMANAGER_ROTATIONORDER_H
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
#endif //PERSISTENTMANAGER_ROTATIONORDER_H
