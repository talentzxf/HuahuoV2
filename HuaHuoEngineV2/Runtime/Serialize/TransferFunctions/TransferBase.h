//
// Created by VincentZhang on 4/21/2022.
//

#ifndef HUAHUOENGINE_TRANSFERBASE_H
#define HUAHUOENGINE_TRANSFERBASE_H

#include "Serialize/SerializationMetaFlags.h"

extern const char *kTransferNameIdentifierBase;

class TransferBase {
public:
    TransferBase()
            : m_ReferenceFromIDCache(NULL) {

    }

    void AddMetaFlag(int /*mask*/) {}


    /// @}

    /// @name Transfers
    /// @{

    /// Alignment in the serialization system is done manually.
    /// The serialization system only ever cares about 4 byte alignment.
    /// When writing data that has an alignment of less than 4 bytes, followed by data that has 4 byte alignment,
    /// then Align must be called before the 4 byte aligned data.
    /// TRANSFER (1byte);
    /// TRANSFER (1byte);
    /// transfer.Align ();
    /// TRANSFER (4byte);
    void Align() {}

    /// Internal function. Should only be called from SerializeTraits
    template<class T>
    void TransferBasicData(T &) {}

    bool IsReading() { return false; }

    bool IsReadingPPtr() { return false; }

    bool IsWriting() { return false; }

    bool IsWritingPPtr() { return false; }

    /// @name Versioning
    /// @{

    /// Sets the "version of the class currently transferred", default version is 1.
    void SetVersion(int) {}

    /// @name Predicates
    /// @{

    /// Get the TransferInstructionFlags.
    /// Commonly used to special case transfer functions for specific operations.
    TransferInstructionFlags GetFlags() const { return m_Flags; }

    bool NeedsInstanceIDRemapping() { return m_Flags & kReadWriteFromSerializedFile; }

    /// Deprecated: use IsVersionSmallerOrEqual instead.
    bool IsOldVersion(int /*version*/) { return false; }

    bool IsCurrentVersion() { return true; }

    /// Are we serializing data for use by the player.
    /// This includes reading/writing/generating typetrees. And can be when creating data from the editor for player or when simply reading/writing data in the player.
    /// Commonly used to not serialize data that does not exist in the player.
    bool IsSerializingForGameRelease() {
#if HUAHUO_EDITOR
        return m_Flags & kSerializeGameRelease;
#else
        return true;
#endif
    }

    /// Is this a RemapPPtrTransfer backend. Commonly used to do very specialized code when generating dependencies using RemapPPtrTransfer.
    bool IsRemapPPtrTransfer() { return false; }

protected:
    TransferInstructionFlags m_Flags;
    void *m_UserData;
    void *m_ReferenceFromIDCache;
};


#endif //HUAHUOENGINE_TRANSFERBASE_H
