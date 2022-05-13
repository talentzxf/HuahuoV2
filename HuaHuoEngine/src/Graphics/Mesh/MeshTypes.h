#pragma once

#include "Serialize/SerializeUtility.h"
#include "Utilities/EnumFlags.h"


enum class MeshUpdateFlags
{
    kDefault = 0,
    kDontValidateIndices = 1 << 0,
    kDontResetBoneBounds = 1 << 1,
    kDontNotifyMeshUsers = 1 << 2,
    kDontRecalculateBounds = 1 << 3,
};
ENUM_FLAGS(MeshUpdateFlags);


struct BoneWeight
{
    BoneWeight(float w, int b) : weight(w), boneIndex(b) {}
    BoneWeight() {}

    float weight;
    int   boneIndex;

    DECLARE_SERIALIZE_OPTIMIZE_TRANSFER(BoneWeight)
};
// BIND_MANAGED_TYPE_NAME(BoneWeight, UnityEngine_BoneWeight1);

template<class TransferFunc>
void BoneWeight::Transfer(TransferFunc& transfer)
{
    TRANSFER(weight);
    TRANSFER(boneIndex);
}

struct BoneWeights2
{
    float weight[2];
    int   boneIndex[2];
};

struct BoneWeightsShort4
{
    UInt16 weight[4];
    UInt16 boneIndex[4];
};

struct BoneWeights4
{
    float weight[4];
    int   boneIndex[4];

    DECLARE_SERIALIZE_OPTIMIZE_TRANSFER(BoneWeights4)
};
//BIND_MANAGED_TYPE_NAME(BoneWeights4, UnityEngine_Experimental_Graphics_BoneWeights);

template<class TransferFunc>
void BoneWeights4::Transfer(TransferFunc& transfer)
{
    TRANSFER(weight[0]);
    TRANSFER(weight[1]);
    TRANSFER(weight[2]);
    TRANSFER(weight[3]);

    TRANSFER(boneIndex[0]);
    TRANSFER(boneIndex[1]);
    TRANSFER(boneIndex[2]);
    TRANSFER(boneIndex[3]);
}

//#include "Runtime/Scripting/BindingsDefs.h"
//BIND_MANAGED_TYPE_NAME(BoneWeights4, UnityEngine_BoneWeight);
