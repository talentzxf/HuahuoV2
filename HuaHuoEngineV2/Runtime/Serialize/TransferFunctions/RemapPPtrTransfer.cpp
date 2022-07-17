//#include "UnityPrefix.h"
#include "RemapPPtrTransfer.h"

RemapPPtrTransfer::RemapPPtrTransfer(TransferInstructionFlags flags, bool readPPtrs)
{
    m_ReadPPtrs = readPPtrs;
    m_Flags = flags;
    m_UserData = NULL;
    m_GenerateIDFunctor = NULL;
    m_ReferencedObjectFunctor = NULL;
    m_MetaMaskStack.reserve(4);
    m_MetaMaskStack.push_back(kNoTransferFlags);
    m_CachedMetaMaskStackTop = kNoTransferFlags;
}

void RemapPPtrTransfer::PushMetaFlag(TransferMetaFlags flag)
{
    m_MetaMaskStack.push_back(m_MetaMaskStack.back() | flag);
    m_CachedMetaMaskStackTop = m_MetaMaskStack.back();
}

void RemapPPtrTransfer::PopMetaFlag()
{
    m_MetaMaskStack.pop_back();
    m_CachedMetaMaskStackTop = m_MetaMaskStack.back();
}

void RemapPPtrTransfer::AddMetaFlag(TransferMetaFlags flag)
{
    m_MetaMaskStack.back() |= flag;
    m_CachedMetaMaskStackTop = m_MetaMaskStack.back();
}
