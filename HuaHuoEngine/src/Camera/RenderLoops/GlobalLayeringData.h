#pragma once
#include "Configuration/IntegerDefinitions.h"
struct GlobalLayeringData
{
    enum
    {
        kMaxSortingGroupBits = 12,
        kMaxSortingGroupOrder = (1 << kMaxSortingGroupBits) - 1,
        kInvalidSortingGroupID = (1 << (32 - kMaxSortingGroupBits)) - 1
    };

    UInt32 layerAndOrder; // Layer and order combined

    union
    {
        struct
        {
#if PLATFORM_ARCH_BIG_ENDIAN
            UInt32 id : (32 - kMaxSortingGroupBits);
            UInt32 order : kMaxSortingGroupBits;
#else
            UInt32 order : kMaxSortingGroupBits;
            UInt32 id : (32 - kMaxSortingGroupBits);
#endif
        } sortingGroup;
        UInt32 sortingGroupAll;
    };


    static UInt32 CalculateLayerAndOrder(SInt16 layer, SInt16 order)
    {
        return (UInt32(SInt32(layer) + (1 << 15)) << 16) | UInt32(SInt32(order) + (1 << 15));
    }

    GlobalLayeringData(SInt16 layer = 0, SInt16 order = 0)
    {
        layerAndOrder = CalculateLayerAndOrder(layer, order);
        sortingGroup.id = kInvalidSortingGroupID;
        sortingGroup.order = 0;
    }

    void SetLayerAndOrder(SInt16 layer, SInt16 order)
    {
        layerAndOrder = CalculateLayerAndOrder(layer, order);
    }

    void SetSortingGroupID(UInt32 id)
    {
        sortingGroup.id = id;
    }

    void SetSortingGroupOrder(UInt32 order)
    {
        sortingGroup.order = order;
    }
};

inline int CompareGlobalLayeringData(const GlobalLayeringData& lhs, const GlobalLayeringData& rhs)
{
    return (lhs.layerAndOrder > rhs.layerAndOrder) - (lhs.layerAndOrder < rhs.layerAndOrder);
}

inline int CompareSortingGroupData(const GlobalLayeringData& lhs, const GlobalLayeringData& rhs)
{
    if (lhs.sortingGroup.id != GlobalLayeringData::kInvalidSortingGroupID || rhs.sortingGroup.id != GlobalLayeringData::kInvalidSortingGroupID)
    {
        return (lhs.sortingGroupAll > rhs.sortingGroupAll) - (lhs.sortingGroupAll < rhs.sortingGroupAll);
    }
    return 0;
}
