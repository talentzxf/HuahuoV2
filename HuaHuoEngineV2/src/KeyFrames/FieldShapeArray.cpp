//
// Created by VincentZhang on 2022-11-02.
//

#include "FieldShapeArray.h"
#include "Shapes/BaseShape.h"

bool operator==(BaseShape* p1, PPtr<BaseShape> p2)
{
return p1->GetInstanceID() == p2->GetInstanceID();
}