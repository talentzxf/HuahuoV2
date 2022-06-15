//
// Created by VincentZhang on 6/15/2022.
//

#include "TimeLineCellManager.h"
#include "Serialize/SerializeUtility.h"

IMPLEMENT_REGISTER_CLASS(TimeLineCellManager, 10006);

IMPLEMENT_OBJECT_SERIALIZE(TimeLineCellManager);
INSTANTIATE_TEMPLATE_TRANSFER(TimeLineCellManager);

template<class TransferFunction>
void TimeLineCellManager::Transfer(TransferFunction &transfer) {
    Super::Transfer(transfer);

    printf("Transfering cellspanmap. Size:%d\n", cellSpanMap.size());
    TRANSFER(cellSpanMap);
    printf("Transfering merged cells. Size:%d\n", mergedCells.size());
    TRANSFER(mergedCells);
}