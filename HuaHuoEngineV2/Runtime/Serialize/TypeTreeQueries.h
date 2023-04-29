//
// Created by VincentZhang on 4/29/2023.
//

#ifndef HUAHUOENGINEV2_TYPETREEQUERIES_H
#define HUAHUOENGINEV2_TYPETREEQUERIES_H

#include "TypeTree.h"
#include "TypeSystem/Object.h"

namespace TypeTreeQueries
{
    TypeTree::Signature GenerateTypeTreeSignature(TransferInstructionFlags flags, const Object &object);
}

#endif //HUAHUOENGINEV2_TYPETREEQUERIES_H
