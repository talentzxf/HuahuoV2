//
// Created by VincentZhang on 2023-01-31.
//

#include "ParticleSystem.h"

IMPLEMENT_REGISTER_CLASS(ParticleSystem, 10024);

IMPLEMENT_OBJECT_SERIALIZE(ParticleSystem);
INSTANTIATE_TEMPLATE_TRANSFER(ParticleSystem);

template<class TransferFunction>
void ParticleSystem::Transfer(TransferFunction &transfer) {
    Super::Transfer(transfer);
    TRANSFER(this->p1);
    TRANSFER(this->p2);
}