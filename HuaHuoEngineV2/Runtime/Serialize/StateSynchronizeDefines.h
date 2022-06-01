#pragma once

#include "Serialize/TransferFunctions/StreamedBinaryRead.h"
#include "Serialize/TransferFunctions/StreamedBinaryWrite.h"

#if ENABLE_CLUSTER_SYNC
    #define DECLARE_STATE_SYNCHRONIZE(x) \
        template<class TransferFunc> void TransferState (TransferFunc& transfer); \
        void EXPORT_COREMODULE VirtualTransferState(StreamedBinaryRead& transfer); \
        void EXPORT_COREMODULE VirtualTransferState(StreamedBinaryWrite& transfer);

    #define IMPLEMENT_STATE_SYNCHRONIZE(x) \
        void x::VirtualTransferState(StreamedBinaryRead& transfer) { this->TransferState(transfer); } \
        void x::VirtualTransferState(StreamedBinaryWrite& transfer) { this->TransferState(transfer); }
#else
    #define DECLARE_STATE_SYNCHRONIZE(x) \
        template<class TransferFunc> void TransferState (TransferFunc& transfer);
    #define IMPLEMENT_STATE_SYNCHRONIZE(x)
#endif
