//
// Created by VincentZhang on 6/1/2022.
//

#include "LineShape.h"
#include "Serialize/SerializeUtility.h"

IMPLEMENT_REGISTER_CLASS(LineShape, 10003);

IMPLEMENT_OBJECT_SERIALIZE(LineShape);
INSTANTIATE_TEMPLATE_TRANSFER(LineShape);

template<class TransferFunction>
void LineShape::Transfer(TransferFunction &transfer) {
    Super::Transfer(transfer);

    printf("Transfering p1: %f,%f,%f\n", this->p1.x, this->p1.y, this->p1.z);
    TRANSFER(this->p1);
    printf("After transfering p1: %f,%f,%f\n", this->p1.x, this->p1.y, this->p1.z);
    printf("Transfering p2: %f,%f,%f\n", this->p2.x, this->p2.y, this->p2.z);
    TRANSFER(this->p2);
    printf("After transfering p2: %f,%f,%f\n", this->p2.x, this->p2.y, this->p2.z);
}
