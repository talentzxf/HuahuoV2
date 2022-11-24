//
// Created by VincentZhang on 2022-11-24.
//

#include "NailShape.h"


IMPLEMENT_REGISTER_CLASS(NailShape, 10019);

IMPLEMENT_OBJECT_SERIALIZE(NailShape);

INSTANTIATE_TEMPLATE_TRANSFER(NailShape);

template<class TransferFunction>
void NailShape::Transfer(TransferFunction &transfer) {
    Super::Transfer(transfer);

    TRANSFER(shapeLocalPointMap);
    TRANSFER(boundShapes);
}

bool NailShape::AddShape(BaseShape *targetShape, float localX, float localY, float localZ) {
    for (auto itr: boundShapes) {
        if (!GetNailManagerPtr()->CheckDuplication(itr, targetShape))
            return false;
    }

    this->shapeLocalPointMap[targetShape].Set(localX, localY, localZ);
    GetNailManagerPtr()->AddNailShapeMapping(targetShape, this);
    return true;
}

IMPLEMENT_REGISTER_CLASS(NailManager, 10023);

IMPLEMENT_OBJECT_SERIALIZE(NailManager);

INSTANTIATE_TEMPLATE_TRANSFER(NailManager);

template<class TransferFunction>
void NailManager::Transfer(TransferFunction &transfer) {
    Super::Transfer(transfer);
    TRANSFER(nails);
    TRANSFER(shapeNailMap);
}

NailManager *gNailManager = NULL;

NailManager *GetNailManagerPtr() {
    if (gNailManager == NULL) {
        gNailManager = Object::Produce<NailManager>();
        GetPersistentManagerPtr()->MakeObjectPersistent(gNailManager->GetInstanceID(), StoreFilePath);
    }

    return gNailManager;
}

void NailManager::AwakeFromLoad(AwakeFromLoadMode awakeMode) {
    Super::AwakeFromLoad(awakeMode);

    gNailManager = this;
    printf("gNailManager set.\n");
}

