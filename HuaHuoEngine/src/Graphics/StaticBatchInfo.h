#pragma once
#include "Serialize/SerializeUtility.h"

struct StaticBatchInfo
{
    StaticBatchInfo() : firstSubMesh(0), subMeshCount(0) {}
    UInt16 firstSubMesh;
    UInt16 subMeshCount;

    DECLARE_SERIALIZE(StaticBatchInfo)
};

template<class TransferFunc>
void StaticBatchInfo::Transfer(TransferFunc& transfer)
{
    TRANSFER(firstSubMesh);
    TRANSFER(subMeshCount);
}

inline bool IsPartOfBatch(const StaticBatchInfo& info)
{
    return info.subMeshCount != 0;
}

inline int CalculateSubMeshIndex(const StaticBatchInfo& info, int materialIndex)
{
    if (info.subMeshCount > 0 && materialIndex >= info.subMeshCount)
        materialIndex = info.subMeshCount - 1;
    return info.firstSubMesh + materialIndex;
}
