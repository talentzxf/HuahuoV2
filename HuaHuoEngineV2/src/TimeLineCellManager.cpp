//
// Created by VincentZhang on 6/15/2022.
//

#include "TimeLineCellManager.h"
#include "Serialize/SerializeUtility.h"
#include "Layer.h"
#include "ObjectStore.h"

IMPLEMENT_REGISTER_CLASS(TimeLineCellManager, 10006);

IMPLEMENT_OBJECT_SERIALIZE(TimeLineCellManager);
INSTANTIATE_TEMPLATE_TRANSFER(TimeLineCellManager);

template<class TransferFunction>
void TimeLineCellManager::Transfer(TransferFunction &transfer) {
    Super::Transfer(transfer);

    TRANSFER(cellSpanMap);
    TRANSFER(layer);
}

void TimeLineCellManager::SetLayer(Layer *pLayer) {
    this->layer = pLayer;
}

void TimeLineCellManager::AwakeFromLoad(AwakeFromLoadMode awakeMode) {
    Object::AwakeFromLoad(awakeMode);

    for(auto cellSpanMapItr: cellSpanMap){
        unsigned int startFrameId = cellSpanMapItr.first;
        unsigned int spanLength = cellSpanMapItr.second;

        for(unsigned int frameId = startFrameId; frameId < startFrameId + spanLength; frameId++){
            this->mergedCells[frameId] = startFrameId;
        }
    }
}

void TimeLineCellManager::MergeCells(unsigned int startCellId, unsigned int endCellId) {
    unsigned int minCell = std::min(startCellId, endCellId);
    unsigned int maxCell = std::max(startCellId, endCellId);

    unsigned int currentMaxCellSpan = this->GetCellSpan(maxCell);

    unsigned int newMinCellSpan = maxCell - minCell + currentMaxCellSpan;

    // Update all spans in the middle
    for(unsigned int cellId = minCell; cellId <= minCell + newMinCellSpan - 1; cellId++){
        // 1. Delete all cells in the middle
        if(this->cellSpanMap.contains(cellId)){
            this->cellSpanMap.erase(cellId);
        }

        this->mergedCells[cellId] = minCell;
    }

    this->cellSpanMap[minCell] = newMinCellSpan;

    if(this->layer.IsValid() && this->layer->GetObjectStore() != NULL){
        this->layer->GetObjectStore()->UpdateMaxFrameId(maxCell);
    }
}
