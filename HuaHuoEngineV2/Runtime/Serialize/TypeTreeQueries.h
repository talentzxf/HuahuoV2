//
// Created by VincentZhang on 4/29/2023.
//

#ifndef HUAHUOENGINEV2_TYPETREEQUERIES_H
#define HUAHUOENGINEV2_TYPETREEQUERIES_H

#include "TypeTree.h"
#include "TypeSystem/Object.h"
#include "Utilities/Hash128.h"

namespace TypeTreeQueries
{
    TypeTree::Signature GenerateTypeTreeSignature(TransferInstructionFlags flags, const Object &object);
    int GetTypeChildrenCount(const TypeTreeIterator& type);

    /// Calculates a hash from the sub tree rotted at typeTree
    Hash128 HashTypeTree(const TypeTreeIterator& type);
}

#endif //HUAHUOENGINEV2_TYPETREEQUERIES_H
