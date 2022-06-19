//
// Created by VincentZhang on 6/15/2022.
//

#ifndef HUAHUOENGINEV2_TIMELINECELLMANAGER_H
#define HUAHUOENGINEV2_TIMELINECELLMANAGER_H
#include "TypeSystem/Object.h"
#include <map>

// All cellIds start from 0. But when show in the UI, will+1 to make it more human readable.
class TimeLineCellManager : public Object{
    REGISTER_CLASS(TimeLineCellManager);
    DECLARE_OBJECT_SERIALIZE();

public:
    TimeLineCellManager(MemLabelId& labelId, const ObjectCreationMode& creationMode)
    :Super(labelId,creationMode){

    }
    unsigned int GetSpanHead(unsigned int cellId){
        if(!mergedCells.contains(cellId)){
            return cellId;
        }

        return mergedCells[cellId];
    }

    bool IsSpanHead(unsigned int cellId){
        return !this->mergedCells.contains(cellId) || this->mergedCells[cellId] == cellId;
    }

    unsigned int GetCellSpan(unsigned int cellId){
        if(!this->cellSpanMap.contains(cellId))
            return 1;
        return this->cellSpanMap[cellId];
    }

    void MergeCells(unsigned int startCellId, unsigned int endCellId){
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
    }

    void AwakeFromLoad(AwakeFromLoadMode awakeMode) override;

private:
    // key -- startCellId; value -- the length of the span.
    std::map<unsigned int, unsigned int> cellSpanMap;

    // key -- a cellId; value -- the beginning of this span.
    std::map<unsigned int, unsigned int> mergedCells;
};


#endif //HUAHUOENGINEV2_TIMELINECELLMANAGER_H
