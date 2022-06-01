#pragma once
#include "Serialize/SerializeUtility.h"

struct BitField
{
    DECLARE_SERIALIZE_OPTIMIZE_TRANSFER(BitField)
    UInt32 m_Bits;
};

enum { kPreUnity2UnusedLayerMask = 1 << 5 };

template<class TransferFunc>
void BitField::Transfer(TransferFunc& transfer)
{
    transfer.SetVersion(2);
    transfer.Transfer(m_Bits, "m_Bits", kHideInEditorMask | kGenerateBitwiseDifferences);

    if (transfer.IsOldVersion(1))
    {
        if (m_Bits & kPreUnity2UnusedLayerMask)
            m_Bits |= 0xFFFF << 16;
    }
}
