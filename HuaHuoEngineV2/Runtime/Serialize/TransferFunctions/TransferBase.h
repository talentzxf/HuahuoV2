//
// Created by VincentZhang on 4/21/2022.
//

#ifndef HUAHUOENGINE_TRANSFERBASE_H
#define HUAHUOENGINE_TRANSFERBASE_H

#include "Serialize/SerializationMetaFlags.h"

extern const char * kTransferNameIdentifierBase;
class TransferBase {
public:
    void AddMetaFlag(int /*mask*/) {}

    /// Internal function. Should only be called from SerializeTraits
    template<class T>
    void TransferBasicData(T&) {}

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
    TransferInstructionFlags GetFlags() const  { return m_Flags; }

    bool NeedsInstanceIDRemapping()          { return m_Flags & kReadWriteFromSerializedFile; }

    /// Deprecated: use IsVersionSmallerOrEqual instead.
    bool IsOldVersion(int /*version*/) { return false; }
    bool IsCurrentVersion() { return true; }

    /// Are we serializing data for use by the player.
    /// This includes reading/writing/generating typetrees. And can be when creating data from the editor for player or when simply reading/writing data in the player.
    /// Commonly used to not serialize data that does not exist in the player.
    bool IsSerializingForGameRelease()
    {
#if HUAHUO_EDITOR
        return m_Flags & kSerializeGameRelease;
#else
        return true;
#endif
    }

    /// Is this a RemapPPtrTransfer backend. Commonly used to do very specialized code when generating dependencies using RemapPPtrTransfer.
    bool IsRemapPPtrTransfer() { return false; }
protected:
    TransferInstructionFlags          m_Flags;
    void*                             m_UserData;
};


#endif //HUAHUOENGINE_TRANSFERBASE_H
